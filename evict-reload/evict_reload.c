/*
 * Evict+Reload (same-process teaching PoC).
 * Same pipeline as Flush+Reload, but probe lines are evicted by scanning a large
 * buffer instead of using clflush (useful when clflush is unavailable or filtered).
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

static unsigned char secret_byte = 0x48; /* 'H' */
static int probe[256 * 4096];

static unsigned char *evict_buf;
static size_t evict_size;

static void alloc_eviction(void) {
    evict_size = (size_t)512 * 4096;
    evict_buf = (unsigned char *)malloc(evict_size);
    if (!evict_buf) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    memset(evict_buf, 0xAB, evict_size);
}

/* Evict cache by touching many lines (no clflush on probe) */
static void evict_cache_bulk(void) {
    volatile unsigned char sink = 0;
    for (size_t i = 0; i < evict_size; i += 64) {
        sink ^= evict_buf[i];
    }
    (void)sink;
    _mm_mfence();
}

__attribute__((noinline)) static void victim(void) {
    volatile int *p = &probe[(size_t)secret_byte * 4096];
    *p = 1;
}

static void flush_probe_clflush(void) {
    for (int i = 0; i < 256; i++) {
        _mm_clflush(&probe[i * 4096]);
    }
    _mm_mfence();
}

static uint64_t reload_cycles(int k) {
    unsigned int aux;
    volatile int *p = &probe[k * 4096];
    uint64_t t0 = __rdtscp(&aux);
    int s = *p;
    (void)s;
    uint64_t t1 = __rdtscp(&aux);
    return t1 - t0;
}

static int guess_byte_evict_reload(int repeats) {
    uint64_t sum[256] = {0};

    for (int k = 0; k < 256; k++) {
        for (int r = 0; r < repeats; r++) {
            evict_cache_bulk();
            victim();
            sum[k] += reload_cycles(k);
        }
    }

    int best = 0;
    for (int i = 1; i < 256; i++) {
        if (sum[i] < sum[best]) {
            best = i;
        }
    }
    return best;
}

int main(void) {
    memset(probe, 0, sizeof(probe));
    alloc_eviction();

    const int repeats = 1500;
    printf("Evict+Reload: secret byte = 0x%02x (eviction buffer %zu MB)\n", secret_byte,
           evict_size / (1024 * 1024));

    int g = guess_byte_evict_reload(repeats);
    printf("Guessed: %d (%s)\n", g, g == (int)secret_byte ? "OK" : "retry / tune repeats");

    /* Optional: show clflush path works on this machine for comparison */
    {
        uint64_t sum = 0;
        for (int r = 0; r < 100; r++) {
            flush_probe_clflush();
            victim();
            sum += reload_cycles((int)secret_byte);
        }
        printf("Sanity (Flush+Reload avg cycles for correct slot, 100 trials): %.1f\n",
               (double)sum / 100.0);
    }

    free(evict_buf);
    return 0;
}

/*
 * Prime + Probe (same-process teaching demo).
 *
 * Prime fills probe lines; the victim touches one line; probe measures reload latency.
 * Read-only eviction is noisy on VMs/WSL; default uses clflush (Flush+Reload style).
 * Define USE_READ_PRIME to 1 to compare read-based prime instead.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

#ifndef USE_READ_PRIME
#define USE_READ_PRIME 0
#endif

#define PROBE_SLOTS 256
#define STRIDE      4096

static unsigned char secret_byte = 0x48; /* 'H' */

static int probe[PROBE_SLOTS * STRIDE];

__attribute__((noinline)) static void victim(void) {
    volatile int *p = &probe[(size_t)secret_byte * STRIDE];
    *p = 1;
}

#if USE_READ_PRIME
static void prime_probe_buffer(void) {
    for (int i = 0; i < PROBE_SLOTS; i++) {
        volatile int *p = &probe[i * STRIDE];
        (void)*p;
    }
    _mm_mfence();
}
#endif

static void flush_probe_set(void) {
    for (int i = 0; i < PROBE_SLOTS; i++) {
        _mm_clflush(&probe[i * STRIDE]);
    }
    _mm_mfence();
}

static void cache_prepare(void) {
#if USE_READ_PRIME
    prime_probe_buffer();
#else
    flush_probe_set();
#endif
}

static uint64_t reload_cycles(int slot) {
    unsigned int aux;
    volatile int *p = &probe[slot * STRIDE];
    uint64_t t0 = __rdtscp(&aux);
    int s = *p;
    (void)s;
    uint64_t t1 = __rdtscp(&aux);
    return t1 - t0;
}

static int guess_byte_prime_probe(int rounds) {
    uint64_t sum[PROBE_SLOTS] = {0};

    for (int k = 0; k < PROBE_SLOTS; k++) {
        for (int r = 0; r < rounds; r++) {
            cache_prepare();
            victim();
            sum[k] += reload_cycles(k);
        }
    }

    int best = 0;
    for (int i = 1; i < PROBE_SLOTS; i++) {
        if (sum[i] < sum[best]) {
            best = i;
        }
    }
    return best;
}

int main(void) {
    memset(probe, 0, sizeof(probe));
    const int rounds = 2000;
    printf("Prime+Probe demo (cache prep: %s): secret_byte = 0x%02x ('%c')\n",
           USE_READ_PRIME ? "read prime" : "clflush (Flush+Reload style)", secret_byte,
           (secret_byte >= 32 && secret_byte < 127) ? secret_byte : '?');

    int g = guess_byte_prime_probe(rounds);
    printf("Guessed slot (byte value): %d  (%s)\n", g, (g == (int)secret_byte) ? "OK" : "retry/higher rounds");

    return 0;
}

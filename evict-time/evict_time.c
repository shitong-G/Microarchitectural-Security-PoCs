/*
 * Evict + Time (teaching demo).
 *
 * Walk a large buffer to evict cache lines, then time victim access (cold vs warm).
 * Unlike Flush+Reload, eviction uses natural replacement (discussion for no-clflush cases).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

#define SECRET_LINE 64
#define EVICT_PAGES 512
#define PAGE_STRIDE 4096

static int secret[PAGE_STRIDE]; /* Separate from eviction buffer */

static unsigned char *evict_buf;
static size_t evict_size;

static void alloc_eviction_buffer(void) {
    evict_size = (size_t)EVICT_PAGES * PAGE_STRIDE;
    evict_buf = (unsigned char *)malloc(evict_size);
    if (!evict_buf) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    memset(evict_buf, 1, evict_size);
}

/* Scan eviction buffer to displace cache lines */
static void evict_cache(void) {
    volatile unsigned char sink = 0;
    for (int pass = 0; pass < 2; pass++) {
        for (size_t i = 0; i < evict_size; i += 64) {
            sink ^= evict_buf[i];
        }
    }
    (void)sink;
    _mm_mfence();
}

__attribute__((noinline)) static void victim_touch_secret(void) {
    volatile int *p = &secret[SECRET_LINE];
    /* Stretch work per call so rdtscp sees cold vs warm more clearly */
    int v = *p;
    for (int i = 0; i < 400; i++) {
        v = v * 3 + 1;
    }
    *p = v;
}

static uint64_t time_victim_calls(int n) {
    unsigned int aux;
    uint64_t t0 = __rdtscp(&aux);
    for (int i = 0; i < n; i++) {
        victim_touch_secret();
    }
    uint64_t t1 = __rdtscp(&aux);
    return (t1 - t0) / (uint64_t)n;
}

/* Optional: clflush secret line as a strong cold-cache baseline */
static void flush_secret_line(void) {
    _mm_clflush(&secret[SECRET_LINE]);
    _mm_mfence();
}

int main(void) {
    alloc_eviction_buffer();
    secret[SECRET_LINE] = 0x5a5a;

    const int calls = 80;

    /* Warm cache: many calls then measure */
    for (int i = 0; i < 5000; i++) {
        victim_touch_secret();
    }
    uint64_t warm = time_victim_calls(calls);

    /* Cold: after bulk eviction */
    evict_cache();
    uint64_t cold_evict = time_victim_calls(calls);

    /* Cold: after clflush of the victim line */
    for (int i = 0; i < 2000; i++) {
        victim_touch_secret();
    }
    flush_secret_line();
    uint64_t cold_flush = time_victim_calls(calls);

    printf("Evict+Time: avg cycles per victim() call (rdtscp)\n");
    printf("  warm cache:        %llu\n", (unsigned long long)warm);
    printf("  after big evict:   %llu\n", (unsigned long long)cold_evict);
    printf("  after clflush line:%llu\n", (unsigned long long)cold_flush);
    printf("  delta evict-warm:  %lld\n", (long long)cold_evict - (long long)warm);
    printf("  delta flush-warm:  %lld\n", (long long)cold_flush - (long long)warm);

    free(evict_buf);
    return 0;
}

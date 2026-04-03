/*
 * Time-driven / trace-style cache leakage (Bernstein 2005; Osvik–Shamir–Tromer 2006 style).
 * Minimal: table lookup indexed by key-dependent byte — total time varies with index.
 * Not a full AES; illustrates secret-dependent memory access timing.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

#define T_ENTRIES 256
static volatile uint8_t T[T_ENTRIES];

static uint64_t measure_cycles(void (*fn)(uint8_t), uint8_t idx) {
    unsigned int aux;
    uint64_t t0 = __rdtscp(&aux);
    fn(idx);
    uint64_t t1 = __rdtscp(&aux);
    return t1 - t0;
}

/* Each index touches a different cache line in T — time correlates with idx */
__attribute__((noinline)) static void secret_indexed_load(uint8_t idx) {
    volatile uint8_t sink = T[idx];
    (void)sink;
}

int main(void) {
    for (int i = 0; i < T_ENTRIES; i++) {
        T[i] = (uint8_t)i;
    }

    printf("Time-driven demo: cycles for load T[secret_index]\n");
    printf("(Warm T in cache first — variance is illustrative, not a full attack.)\n\n");

    /* Prime T into cache */
    for (int i = 0; i < T_ENTRIES; i++) {
        (void)T[i];
    }
    _mm_mfence();

    const int trials = 5000;
    const uint8_t indices[] = {0, 16, 64, 128, 255};
    for (size_t k = 0; k < sizeof indices / sizeof indices[0]; k++) {
        uint8_t idx = indices[k];
        uint64_t total = 0;
        for (int t = 0; t < trials; t++) {
            total += measure_cycles(secret_indexed_load, idx);
        }
        printf("  index %3u: avg %.2f cycles (%d trials)\n", idx, (double)total / trials,
               trials);
    }

    printf("\nDifferent indices → different lines → different timing when caches are contended.\n");
    return 0;
}

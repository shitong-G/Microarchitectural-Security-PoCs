#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

unsigned char secret = 42;
int array[256 * 4096];

/* noinline + volatile: keep a single architecturally visible touch of the secret line */
__attribute__((noinline)) void victim(void) {
    volatile int *p = &array[(unsigned)secret * 4096];
    *p = 1;
}

static void flush_probe_set(void) {
    for (int i = 0; i < 256; i++) {
        _mm_clflush(&array[i * 4096]);
    }
    _mm_mfence();
}

static uint64_t reload_cycles(int k) {
    unsigned int aux;
    volatile int *p = &array[k * 4096];
    uint64_t t0 = __rdtscp(&aux);
    int sink = *p;
    (void)sink;
    uint64_t t1 = __rdtscp(&aux);
    return t1 - t0;
}

int main(void) {
    /* Per-candidate k: flush, victim, measure only reload(k) — avoids 256 probes evicting each other */
    const int repeats = 2000;
    uint64_t sum[256] = {0};

    for (int k = 0; k < 256; k++) {
        for (int r = 0; r < repeats; r++) {
            flush_probe_set();
            victim();
            sum[k] += reload_cycles(k);
        }
    }

    int best = 0;
    for (int k = 1; k < 256; k++) {
        if (sum[k] < sum[best]) {
            best = k;
        }
    }

    printf("Guessed index (min avg reload): %d  |  actual secret byte: %u\n\n", best,
           (unsigned)secret);

    for (int i = 0; i < 256; i++) {
        printf("index %3d: avg %.2f cycles\n", i, (double)sum[i] / (double)repeats);
    }

    return 0;
}

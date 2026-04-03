/*
 * Meltdown (conceptual) — the *receiver* half: Flush+Reload on a probe buffer.
 * The full Meltdown attack (Lipp et al., USENIX 2018) adds a *transient* load
 * from kernel-mapped memory that faults architecturally but still affects cache.
 * That step requires an unpatched OS / KPTI-off environment and is not reproduced
 * here (would fault or be blocked on modern Linux).
 *
 * This program is identical in spirit to flush-reload/attack.c: encode a byte
 * in the probe and recover it — illustrating the *side channel* Meltdown uses.
 */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

static unsigned char secret = 0x37;
static int probe[256 * 4096];

__attribute__((noinline)) static void victim_kernel_load_simulated(void) {
    /* In real Meltdown, a speculative load from a kernel address would run here. */
    volatile int *p = &probe[(size_t)secret * 4096];
    *p = 1;
}

static void flush_probe(void) {
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

int main(void) {
    printf("Meltdown *receiver* demo (Flush+Reload decode only).\n");
    printf("Full Meltdown + kernel transient load is not implemented (see README).\n\n");

    const int repeats = 1500;
    uint64_t sum[256] = {0};

    for (int k = 0; k < 256; k++) {
        for (int r = 0; r < repeats; r++) {
            flush_probe();
            victim_kernel_load_simulated();
            sum[k] += reload_cycles(k);
        }
    }

    int best = 0;
    for (int i = 1; i < 256; i++) {
        if (sum[i] < sum[best]) {
            best = i;
        }
    }

    printf("Decoded byte index: %d (actual secret 0x%02x) — %s\n", best, secret,
           best == (int)secret ? "OK" : "retry");
    return 0;
}

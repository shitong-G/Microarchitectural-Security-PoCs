/*
 * Flush+Flush (Gruss et al., DIMVA 2016) — teaching sketch.
 * Idea: measure duration of clflush on a line; a line recently loaded by the victim
 * may change flush timing vs a cold line. No reload step — only flush timing.
 * (Effect is subtle; often weaker than Flush+Reload.)
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

#define SLOTS 256
#define STRIDE 4096

static int probe[SLOTS * STRIDE];
static unsigned char secret_byte = 0x48;

__attribute__((noinline)) static void victim(void) {
    volatile int *p = &probe[(size_t)secret_byte * STRIDE];
    *p = 1;
}

static uint64_t flush_only_cycles(int slot) {
    unsigned int aux;
    volatile char *line = (volatile char *)&probe[slot * STRIDE];
    uint64_t t0 = __rdtscp(&aux);
    _mm_clflush((void *)line);
    uint64_t t1 = __rdtscp(&aux);
    return t1 - t0;
}

int main(void) {
    memset(probe, 0, sizeof(probe));
    const int repeats = 800;

    printf("Flush+Flush: guess which slot the victim touched via clflush *duration*.\n");
    printf("Secret byte index = %u\n", (unsigned)secret_byte);

    uint64_t sum[SLOTS] = {0};

    for (int k = 0; k < SLOTS; k++) {
        for (int r = 0; r < repeats; r++) {
            for (int i = 0; i < SLOTS; i++) {
                _mm_clflush(&probe[i * STRIDE]);
            }
            _mm_mfence();
            victim();
            sum[k] += flush_only_cycles(k);
        }
    }

    int best = 0;
    for (int i = 1; i < SLOTS; i++) {
        if (sum[i] < sum[best]) {
            best = i;
        }
    }

    printf("Lowest avg flush time (cycles): slot %d (%s)\n", best,
           best == (int)secret_byte ? "OK" : "often unclear — channel is weak");

    return 0;
}

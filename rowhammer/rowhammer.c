#define _POSIX_C_SOURCE 200809L
/*
 * Rowhammer (ISCA 2014) — aggressive DRAM row activation teaching sketch.
 * Repeated opens of two "aggressor" rows may flip bits in an adjacent "victim" row.
 * This may stress memory and is intended ONLY on machines you own.
 * Default: modest iteration count; increase ROWHAMMER_ROUNDS with care.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef ROWHAMMER_ROUNDS
#define ROWHAMMER_ROUNDS (1ULL << 22)
#endif

/* Two pages + middle page — classic double-sided hammer layout */
#define PAGES 3
#define PAGE_SIZE 4096

static volatile uint8_t *hammer_region;

static void hammer_pair(size_t rounds) {
    volatile uint8_t *a = hammer_region;
    volatile uint8_t *b = a + 2 * PAGE_SIZE; /* two pages apart (sketch) */
    for (size_t i = 0; i < rounds; i++) {
        *a;
        *b;
        __asm__ __volatile__("" ::: "memory");
    }
}

int main(void) {
    void *p = NULL;
    if (posix_memalign(&p, 1 << 21, PAGES * PAGE_SIZE) != 0) {
        fprintf(stderr, "posix_memalign failed\n");
        return 1;
    }
    hammer_region = (volatile uint8_t *)p;
    memset(p, 0xFF, PAGES * PAGE_SIZE);

    printf("Rowhammer sketch: %zu rounds (double-sided pattern).\n",
           (size_t)ROWHAMMER_ROUNDS);
    printf("Bit flips are rare on DDR4 with ECC and on VMs; this is educational.\n");

    uint8_t *victim = (uint8_t *)hammer_region + PAGE_SIZE;
    uint8_t before = victim[0];

    hammer_pair(ROWHAMMER_ROUNDS);

    uint8_t after = victim[0];
    printf("Victim byte before=0x%02x after=0x%02x (flip=%s)\n", before, after,
           before != after ? "YES" : "no");

    free(p);
    return 0;
}

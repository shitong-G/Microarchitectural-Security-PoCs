#define _GNU_SOURCE
/*
 * Non-constant-time compare — classic byte-at-a-time timing leak (software layer).
 *
 * Early exit on first mismatch leaks prefix length via timing. Mitigate with crypto_memcmp.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

static const char *const real_password = "Key{42}";

/* Intentionally unsafe: return on first mismatch */
__attribute__((noinline)) static int unsafe_compare(const char *guess) {
    const char *p = real_password;
    for (size_t i = 0;; i++) {
        if (guess[i] != p[i]) {
            return 0;
        }
        if (p[i] == '\0') {
            return 1;
        }
    }
}

static uint64_t measure_ns(void (*fn)(void)) {
    struct timespec a, b;
    clock_gettime(CLOCK_MONOTONIC, &a);
    fn();
    clock_gettime(CLOCK_MONOTONIC, &b);
    return (uint64_t)(b.tv_sec - a.tv_sec) * 1000000000ull +
           (uint64_t)(b.tv_nsec - a.tv_nsec);
}

static char input[64];

static void run_compare(void) {
    (void)unsafe_compare(input);
}

int main(void) {
    printf("Timing attack demo (unsafe compare). Real password hidden from strcmp path.\n");
    printf("Measure average ns for prefix matches (1000 trials each):\n\n");

    const char *prefixes[] = {"", "K", "Ke", "Key", "Key{", "Key{4", "Key{42", "Key{42}"};
    for (size_t p = 0; p < sizeof(prefixes) / sizeof(prefixes[0]); p++) {
        strncpy(input, prefixes[p], sizeof(input) - 1);
        input[sizeof(input) - 1] = '\0';

        uint64_t total = 0;
        const int trials = 1000;
        for (int t = 0; t < trials; t++) {
            total += measure_ns(run_compare);
        }
        printf("  prefix %-8s -> avg %llu ns\n", prefixes[p] ? prefixes[p] : "(empty)",
               (unsigned long long)(total / (uint64_t)trials));
    }

    printf("\nLonger matching prefix tends to take more time (more chars compared).\n");
    return 0;
}

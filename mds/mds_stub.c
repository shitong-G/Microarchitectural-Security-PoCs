/*
 * MDS / ZombieLoad-style *concept* (May 2019 CVE family).
 * Real exploits use CPU-internal buffers (fill buffer, load ports) and are
 * strongly microarchitecture-dependent. This program only:
 *   - prints CPU model from /proc/cpuinfo
 *   - runs a harmless load/fence loop (no secret recovery)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void print_cpu_model(void) {
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (!f) {
        printf("Cannot open /proc/cpuinfo (not Linux?)\n");
        return;
    }
    char line[256];
    while (fgets(line, sizeof line, f)) {
        if (strncmp(line, "model name", 10) == 0) {
            printf("%s", line);
            break;
        }
    }
    fclose(f);
}

int main(void) {
    printf("MDS family (RIDL / ZombieLoad / Fallout) — stub only.\n");
    printf("Real attacks need specific Intel CPUs + microcode state.\n\n");
    print_cpu_model();

    volatile uint64_t sink = 0;
    char buf[4096];
    memset((void *)buf, 0x5a, sizeof buf);
    for (int i = 0; i < 100000; i++) {
        sink += (unsigned char)buf[i % 4096];
    }
    printf("\nHarmless load loop done (sink=%llu).\n", (unsigned long long)sink);
    return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

int main(void) {
    int array[10000] = {0};
    unsigned int aux = 0;

    uint64_t start, end;

    /* Measure cache miss */
    _mm_clflush(&array[0]);
    _mm_mfence();

    _mm_mfence();
    start = __rdtscp(&aux);
    _mm_mfence();
    {
        volatile int sink = array[0];
        (void)sink;
    }
    _mm_mfence();
    end = __rdtscp(&aux);
    _mm_mfence();

    printf("Cache miss time: %lu cycles\n", (unsigned long)(end - start));

    /* Measure cache hit (access again) */
    _mm_mfence();
    start = __rdtscp(&aux);
    _mm_mfence();
    {
        volatile int sink = array[0];
        (void)sink;
    }
    _mm_mfence();
    end = __rdtscp(&aux);
    _mm_mfence();

    printf("Cache hit time: %lu cycles\n", (unsigned long)(end - start));

    return 0;
}
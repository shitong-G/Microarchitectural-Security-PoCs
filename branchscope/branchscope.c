/*
 * BranchScope-style directional inference (Evtyushkin et al., 2018) — teaching sketch.
 * Train a conditional branch mostly taken vs mostly not taken, then time a victim
 * branch whose direction differs. Timing differences are often tiny and noisy.
 */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

static volatile int sink;

__attribute__((noinline)) static void victim_branch(int cond) {
    if (cond) {
        for (int i = 0; i < 200; i++) {
            sink += i;
        }
    } else {
        for (int i = 0; i < 3; i++) {
            sink += i;
        }
    }
}

static uint64_t time_branch_after_train(int train_taken, int victim_cond, int reps) {
    for (int i = 0; i < 5000; i++) {
        victim_branch(train_taken);
    }
    _mm_mfence();

    unsigned int aux;
    uint64_t t0 = __rdtscp(&aux);
    for (int r = 0; r < reps; r++) {
        victim_branch(victim_cond);
    }
    uint64_t t1 = __rdtscp(&aux);
    return (t1 - t0) / (uint64_t)reps;
}

int main(void) {
    printf("BranchScope sketch: compare cycles for victim_branch(0) after training.\n");

    const int reps = 50;
    uint64_t after_train_taken = time_branch_after_train(1, 0, reps);
    uint64_t after_train_not = time_branch_after_train(0, 0, reps);

    printf("After training mostly TAKEN:    ~%llu cycles per call\n",
           (unsigned long long)after_train_taken);
    printf("After training mostly NOT taken: ~%llu cycles per call\n",
           (unsigned long long)after_train_not);
    printf("(Difference is often small; real BranchScope uses finer probes.)\n");
    return 0;
}

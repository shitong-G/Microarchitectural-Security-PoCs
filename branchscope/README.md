# BranchScope (sketch)

## Idea

**Evtyushkin et al., 2018**: infer **branch direction** (taken vs not taken) in a victim by **training** the predictor and **observing** **timing** or **resource** effects.

## Build

```bash
gcc -O0 -std=c11 branchscope.c -o branchscope
```

## Run

```bash
./branchscope
```

## Notes

- This is a **coarse** timing sketch, not a full BranchScope attack.

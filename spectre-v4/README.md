# Spectre variant 4 (speculative store bypass)

## Idea

**CVE-2018-3639**: loads may **speculatively** bypass pending stores when the CPU **mis-predicts** memory dependencies.

## Build

```bash
gcc -O0 -std=c11 spectre_v4.c -o spectre_v4
```

## Run

```bash
./spectre_v4
```

## Notes

- This is a **minimal sketch**; real SSB PoCs are **CPU-specific**. Expect **no clear leak** on many systems.

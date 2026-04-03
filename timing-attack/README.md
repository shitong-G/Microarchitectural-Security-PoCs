# Timing attack (software layer)

## What this is

Demonstrates **non-constant-time** string comparison: the function returns on the **first mismatch**, so average wall-clock time grows with the length of the **correct prefix**. This is a classic **software** side channel (not specific to caches).

## Files

- `timing_attack.c`

## Build

```bash
gcc -O0 -std=c11 timing_attack.c -o timing_attack
```

Or from repo root: `make`.

## Run

```bash
./timing_attack
```

Requires `clock_gettime` (glibc on Linux / WSL).

## Mitigation

Use **constant-time** comparisons (e.g. `crypto_memcmp`) for secrets.

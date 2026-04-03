# Spectre variant 1

## What this is

Classic **Spectre v1** structure: a bounds check on `array1` followed by `array2[array1[x] * stride]`, training the branch predictor with in-bounds indices, then using a **malicious index** that points past `array1` into attacker-chosen memory (here, a secret string placed at a **fixed positive offset** inside an extended `array1` buffer).

## Files

- `spectre_v1.c` — victim, training loop, Flush+Reload scoring, optional `argv` override for pointer-based experiments.

## Build

```bash
gcc -O0 -std=c11 spectre_v1.c -o spectre_v1
```

Or from repo root: `make`.

## Run

```bash
./spectre_v1
```

Optional (two arguments): `argv[1]` is scanned as a pointer (`%p`), `argv[2]` as length; see source for exact behavior.

## Notes

- The secret is **copied into** `array1` at `SECRET_OFF` so the malicious offset is a **small positive** index (avoids negative `ptrdiff` wrapped to `size_t`).
- **Spectre** may fail or be noisy under **WSL2**, hypervisors, or strong mitigations; **native Linux** on older Intel CPUs often works better.

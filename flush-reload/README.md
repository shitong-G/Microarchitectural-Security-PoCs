# Flush+Reload

## What this is

A minimal **Flush+Reload** cache side-channel demo. A secret byte is encoded by touching one of 256 widely spaced probe lines (`array[byte * 4096]`). The attacker flushes all probe lines, runs the victim once, then measures **reload latency** for each candidate byte value. The correct byte typically yields the **lowest average** reload time.

## Files

- `attack.c` — victim encodes a single secret byte; measurement loop follows the “one candidate per inner repeat” pattern to avoid probe–probe eviction in one round.

## Build

```bash
gcc -O0 -std=c11 attack.c -o attack
```

Or from repo root: `make` (builds `flush-reload/attack`).

## Run

```bash
./attack
```

## Notes

- Tuning: increase per-candidate repeats if results are noisy.
- This is a **same-process** teaching PoC; real targets differ.

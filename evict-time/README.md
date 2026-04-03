# Evict+Time

## What this is

**Evict+Time** measures how long a **victim** function takes after **cache cold** vs **warm** states. A large buffer is walked to **evict** cache lines; optionally the victim’s line is **clflush**’d for a strong cold-cache baseline. Compare average cycles per call (via `rdtscp`).

## Files

- `evict_time.c`

## Build

```bash
gcc -O0 -std=c11 evict_time.c -o evict_time
```

Or from repo root: `make`.

## Run

```bash
./evict_time
```

## Notes

- Deltas depend on CPU, cache sizes, and scheduling; **pinning** to one core can reduce noise (see code).

# Cache timing demo (`rdtscp`)

## What this is

Minimal program: **flush** one word, time a load (**miss**), then load again (**hit**). Uses `__rdtscp` and `_mm_clflush` to show the **cycle** gap between cold and warm access.

## Files

- `timing.c`

## Build

```bash
gcc -O0 -std=c11 timing.c -o timing
```

Or from repo root: `make`.

## Run

```bash
./timing
```

## Notes

- Useful baseline before studying Flush+Reload or Prime+Probe.

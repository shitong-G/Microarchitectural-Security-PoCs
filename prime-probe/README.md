# Prime+Probe (teaching)

## What this is

A **Prime → victim → probe** pipeline on a 256-slot probe buffer. By default the “cache prepare” step uses **`clflush`** on all probe lines (same as Flush+Reload style) for **reproducibility** on VMs. You can compare with a **read-based prime** instead.

## Files

- `prime_probe.c`

## Build

```bash
gcc -O0 -std=c11 prime_probe.c -o prime_probe
```

Read-based prime (optional):

```bash
gcc -O0 -std=c11 -DUSE_READ_PRIME=1 prime_probe.c -o prime_probe
```

Or from repo root: `make`.

## Run

```bash
./prime_probe
```

## Notes

- Full cross-process **Prime+Probe** on the LLC requires careful set indexing; this PoC is simplified for learning.

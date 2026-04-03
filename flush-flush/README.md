# Flush+Flush

## Idea

From **Gruss et al. (DIMVA 2016)**: infer access by **timing `clflush`** alone (no explicit reload). Recently cached lines can exhibit different flush timings than cold lines.

## Build

```bash
gcc -O0 -std=c11 flush_flush.c -o flush_flush
```

## Run

```bash
./flush_flush
```

## Notes

- This channel is **much weaker** than Flush+Reload on many CPUs; results may be wrong or environment-dependent.

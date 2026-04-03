# Evict+Reload

## Idea

Same **encode secret in probe lines → measure reload** pipeline as Flush+Reload, but the **prepare** step uses **cache eviction** (walking a large buffer) instead of **`clflush`** on the probe array.

## Build

```bash
gcc -O0 -std=c11 evict_reload.c -o evict_reload
```

## Run

```bash
./evict_reload
```

## Notes

- Noisy on VMs; may need more `repeats` than Flush+Reload.
- The program also prints a **Flush+Reload sanity** line for comparison.

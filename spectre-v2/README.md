# Spectre variant 2 (indirect branch)

## Idea

**CVE-2017-5715**: poison the **branch target buffer** / indirect predictor so speculation jumps to a **gadget** that encodes data in the cache.

## Build

```bash
gcc -O0 -std=c11 spectre_v2.c -o spectre_v2
```

## Run

```bash
./spectre_v2
```

## Notes

- Often **fails** on systems with **retpoline**, **IBRS**, or aggressive microcode; this is expected for a minimal PoC.

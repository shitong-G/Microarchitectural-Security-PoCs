# NetSpectre (local simulation)

## Idea

**Schwarz et al., USENIX Security 2018**: observe **Spectre**-style leakage **remotely** via **network timing**. This demo uses **loopback TCP** and a server that does **extra work** when a compile-time “secret bit” is set.

## Build

```bash
gcc -O0 -std=c11 -pthread netspectre_local.c -o netspectre_local
```

## Run

```bash
./netspectre_local
```

## Notes

- RTT noise usually **swamps** the signal; this is a **concept** demo only.

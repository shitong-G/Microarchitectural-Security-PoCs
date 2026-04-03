# MDS (microarchitectural data sampling)

## Idea

**2019** disclosures (RIDL, Fallout, ZombieLoad, etc.): **CVE-2018-12126** and related — **sample** stale data from **CPU buffers** (fill buffer, load ports) under **speculative** or **faulting** conditions.

## What is implemented

- **`mds_stub.c`** — prints **CPU model** and runs a **harmless** loop. It does **not** recover secrets.

## Build

```bash
gcc -O0 -std=c11 mds_stub.c -o mds_stub
```

## Run

```bash
./mds_stub
```

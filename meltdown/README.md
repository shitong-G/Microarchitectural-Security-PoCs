# Meltdown (conceptual)

## Full attack (not reproduced here)

**CVE-2017-5754** (Lipp et al., USENIX Security 2018): a **transient** load from **kernel** memory (typically mapped in user space on vulnerable systems) executes **speculatively** before the fault commits; the loaded byte is **encoded** into the cache (e.g. via a probe array), then recovered with **Flush+Reload**.

Modern Linux with **KPTI**, microcode, and **kernel** hardening blocks this in practice.

## What is implemented

- **`meltdown_receiver.c`** — only the **Flush+Reload decoding** path (the “receiver”), with a **user-space** simulated “transient” touch. It **does not** read kernel memory.

## Build

```bash
gcc -O0 -std=c11 meltdown_receiver.c -o meltdown_receiver
```

## Run

```bash
./meltdown_receiver
```

# AES-style time-driven cache demo

## Idea

Classical **time-driven** attacks on table-based crypto (AES T-tables): the **index** into a large table depends on key material, so **total execution time** or **per-access timing** leaks information. This PoC is **not** full AES—only a **256-entry table** and a **load indexed by `idx`**.

## Build

```bash
gcc -O0 -std=c11 aes_timing.c -o aes_timing
```

## Run

```bash
./aes_timing
```

## Notes

- Real attacks (Bernstein 2005; Osvik–Shamir–Tromer 2006) use **AES** and **many** samples; this is a **pedagogical** microbenchmark.

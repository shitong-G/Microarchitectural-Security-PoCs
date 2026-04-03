# Rowhammer

## Idea

**Kim et al., ISCA 2014**: repeated **activation** of two DRAM rows (**disturbs** the row between them), possibly flipping bits (**bit flip**).

## Build

```bash
gcc -O0 -std=c11 rowhammer.c -o rowhammer
```

## Run

```bash
./rowhammer
```

## Notes

- **Flips are rare** on modern DDR4/DDR5, **ECC**, and inside **VMs**.
- Increasing `ROWHAMMER_ROUNDS` (see source) increases stress; use only on **authorized** hardware.

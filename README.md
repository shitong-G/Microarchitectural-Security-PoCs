# Microarchitectural Security PoCs

Educational **x86-64 / Linux** proof-of-concept code for **cache side channels**, **transient-execution (Spectre-style) demos**, and **software timing leaks**. Use only on systems you own or are explicitly authorized to test.

This document doubles as a **short survey**: it places the implemented demos in the wider landscape of **microarchitectural attacks** and notes **when and how** major ideas entered the literature or public disclosure.

---

## 1. Repository layout

Each attack type has its **own directory** with a local **`README.md`** (build, run, notes). **`make`** builds all listed binaries next to their sources.

| Directory | Topic |
|-----------|--------|
| [`flush-reload/`](flush-reload/) | Flush+Reload covert channel / byte recovery |
| [`evict-reload/`](evict-reload/) | Evict+Reload (eviction instead of `clflush` on probes) |
| [`flush-flush/`](flush-flush/) | Flush+Flush (`clflush` timing only) |
| [`prime-probe/`](prime-probe/) | Prime+Probe (optional read-based prime) |
| [`evict-time/`](evict-time/) | Evict+Time (victim latency after eviction) |
| [`aes-timing/`](aes-timing/) | Time-driven table lookup (AES-style teaching microbenchmark) |
| [`spectre-v1/`](spectre-v1/) | Spectre v1 (bounds check + cache encoding) |
| [`spectre-v2/`](spectre-v2/) | Spectre v2 (indirect branch / BTB sketch) |
| [`spectre-v4/`](spectre-v4/) | Spectre v4 (speculative store bypass sketch) |
| [`meltdown/`](meltdown/) | Meltdown **receiver** only (Flush+Reload decode; no kernel transient load) |
| [`mds/`](mds/) | MDS stub (CPU info + harmless loop; no secret recovery) |
| [`rowhammer/`](rowhammer/) | Rowhammer double-sided sketch |
| [`netspectre/`](netspectre/) | NetSpectre-style loopback RTT (local) |
| [`branchscope/`](branchscope/) | BranchScope-style branch timing sketch |
| [`plundervolt/`](plundervolt/) | Documentation only (no PoC — privileged MSRs) |
| [`pacman/`](pacman/) | Documentation only (ARM PAC; not x86) |
| [`timing-attack/`](timing-attack/) | Non-constant-time string compare (software layer) |
| [`cache-timing-demo/`](cache-timing-demo/) | Minimal `rdtscp` + `clflush`: miss vs hit |

---

## 2. A concise review: attack types, origins, and representativeness

The list below is **representative**, not exhaustive. Dates refer to **widely cited public disclosure** (paper, report, or coordinated advisory); parallel discovery or industry-private knowledge may exist.

### 2.1 Cache-based timing and contention

| Attack / pattern | Idea (one line) | Typical “discovered / named” context | In this repo |
|------------------|-----------------|----------------------------------------|--------------|
| **Time-driven / trace-driven cache attacks** | Infer secrets from total execution time or access traces when memory references depend on secrets. | Crypto timing on AES and symmetric ciphers (e.g., Bernstein 2005; Osvik–Shamir–Tromer 2006 on AES). | [`aes-timing/`](aes-timing/) (minimal table-index timing) |
| **Prime+Probe** | Attacker fills a cache set (*prime*), victim runs, attacker measures which of its own lines were evicted (*probe*). | Popularized for **last-level cache** (LLC) attacks on **Intel** (e.g., Liu et al., USENIX Security **2015**); roots trace to **L1** contention on **SMT** (Percival, BSDCan **2005**). | [`prime-probe/`](prime-probe/) (simplified, same-process) |
| **Flush+Reload** | Attacker maps shared memory, **clflush**es a line, victim runs, attacker **reloads** and times—miss vs hit reveals victim access. | **Yarom & Falkner**, IACR **2014** (GnuPG/spy process); became a standard **LLC** primitive. | [`flush-reload/`](flush-reload/) |
| **Evict+Reload** | Same as Flush+Reload but eviction uses **access patterns** instead of `clflush` when flush is unavailable or undesirable. | Discussed alongside Flush+Reload in the **2014** line of work; refined in later systematizations. | [`evict-reload/`](evict-reload/) |
| **Flush+Flush** | Uses **timing of** `clflush` on unmodified lines to **avoid** reload; weakens need for shared **writable** mapping. | **Gruss et al.**, DIMVA **2016**. | [`flush-flush/`](flush-flush/) |
| **Evict+Time** | Evict cache lines (or whole hierarchy), then **time** how long victim code takes—cold vs warm. | Classic **cache-timing** methodology; often cited with **Prime+Probe** in surveys (e.g., **2015–2018** LLC work). | [`evict-time/`](evict-time/) |

**Why these represent the field:** they cover the three dominant **cache-side-channel primitives**—**contention** (Prime+Probe), **shared-line reuse** (Flush+Reload family), and **timing** without per-line reload scoring (Evict+Time)—that underpin most practical cache attacks on crypto and OS structures.

---

### 2.2 Transient execution

| Attack | Idea | Public disclosure context | In this repo |
|--------|------|---------------------------|--------------|
| **Spectre (variant 1)** | Mistrained branch or bounds check; **speculative** execution reads out-of-bounds and **encodes** data into microarchitectural state (cache). | **Kocher et al.**, Jan **2018** (coordinated disclosure; later USENIX Security **2019**); **CVE-2017-5753** (variant 1). | [`spectre-v1/`](spectre-v1/) |
| **Spectre (variant 2)** | **Indirect branch** poisoning (BTB) to steer speculation. | Same **2018** wave; **CVE-2017-5715**. | [`spectre-v2/`](spectre-v2/) (sketch; often blocked by mitigations) |
| **Spectre (variant 4)** | **Speculative** store bypass. | **2018**; **CVE-2018-3639**. | [`spectre-v4/`](spectre-v4/) (sketch) |
| **Meltdown** | Speculative load after **faulting** access to kernel/user data, combined with **cache** channel to read kernel memory from user space. | **Lipp et al.**, USENIX Security **2018**; **CVE-2017-5754**. | [`meltdown/`](meltdown/) — **receiver** path only; **no** kernel transient load |

**Representativeness:** **Spectre v1** is the canonical **“bounds check + covert channel”** pattern; **Meltdown** is the canonical **“faulting load + transient”** pattern. Together they defined the **2018** crisis and most follow-up mitigations (retpoline, KPTI, microcode, etc.).

---

### 2.3 DRAM and physical-layer effects

| Attack | Idea | Public context | In this repo |
|--------|------|----------------|--------------|
| **Rowhammer** | Repeated **row activations** in DRAM flip bits in **adjacent** rows (disturbance errors). | **Kim et al.**, ISCA **2014**; practical exploitation (e.g., **2015** “Project Zero” style). **CVE-2015-0569** (early example). | [`rowhammer/`](rowhammer/) (sketch; flips rare on DDR4/ECC/VMs) |

---

### 2.4 Microarchitectural data sampling (MDS) — 2019

| Attack | Idea | Public context | In this repo |
|--------|------|----------------|--------------|
| **MDS family** (RIDL, Fallout, ZombieLoad, …) | **Buffer** or **fill-buffer** leftovers sampled after fault or assist; **speculative** or **asynchronous** exposure. | **May 2019** coordinated disclosure; multiple **CVEs** (e.g., **CVE-2018-12126** family). Strongly **CPU/microcode** dependent. | [`mds/`](mds/) (stub only — no leakage) |

---

### 2.5 Other representative variants

| Name | Idea | Typical disclosure context | In this repo |
|------|------|----------------------------|--------------|
| **NetSpectre** | **Spectre**-style leakage over **network** (remote timing). | **Schwarz et al.**, USENIX Security **2018**. | [`netspectre/`](netspectre/) (loopback RTT sketch) |
| **BranchScope** | **Directional** branch predictor inference. | **Evtyushkin et al.**, **2018**. | [`branchscope/`](branchscope/) (timing sketch) |
| **Plundervolt** | **Software-controlled** voltage scaling to **fault** instructions and leak secrets. | **2019** CVEs (e.g., **CVE-2019-11157**). | [`plundervolt/`](plundervolt/) — **README only** (privileged MSRs) |
| **PACMAN** (ARM) | Defeats **pointer authentication** (not x86-centric). | **2022** IEEE S&P. | [`pacman/`](pacman/) — **README only** |

---

### 2.6 Software-only (not tied to caches or speculation)

| Pattern | Idea | Context | In this repo |
|---------|------|---------|--------------|
| **Microarchitectural timing** | **rdtscp** / cycle counters to distinguish **cache hit vs miss** (building block for above). | Used across **2010s** cache attack papers; **Intel** `rdtsc`/`rdtscp` documentation. | [`cache-timing-demo/`](cache-timing-demo/) |
| **Algorithmic timing** | **Non-constant-time** comparisons (e.g., early return on password check) leak via **wall-clock** time. | **1990s–2000s** crypto engineering (e.g., **Kocher 1996** timing on RSA); still relevant in API design. | [`timing-attack/`](timing-attack/) |

---

## 3. How this collection maps to the literature

- **Cache primitives:** [`flush-reload`](flush-reload/), [`evict-reload`](evict-reload/), [`flush-flush`](flush-flush/), [`prime-probe`](prime-probe/), and [`evict-time`](evict-time/) cover the main **cache timing** families in **small**, **same-process** form factors. [`aes-timing`](aes-timing/) adds a **time-driven** table-access toy.
- **Transient execution:** [`spectre-v1`](spectre-v1/) is the full **v1** PoC pattern; [`spectre-v2`](spectre-v2/) and [`spectre-v4`](spectre-v4/) are **sketches** that often fail under **retpoline / IBRS / SSBD** but show **code structure**.
- **Meltdown:** [`meltdown`](meltdown/) implements only the **Flush+Reload receiver**; the **kernel transient load** is **not** included (modern OSes block it).
- **MDS / Rowhammer:** [`mds`](mds/) and [`rowhammer`](rowhammer/) are **stubs** or **low-rate** experiments—**not** full exploits.
- **Remote / other:** [`netspectre`](netspectre/) and [`branchscope`](branchscope/) are **noise-heavy** teaching demos; [`plundervolt`](plundervolt/) and [`pacman`](pacman/) are **documentation-only** for safety or ISA mismatch.

---

## 4. Requirements

- **GCC** or **Clang**, **x86-64**, `x86intrin.h` (`__rdtscp`, `_mm_clflush`).
- **Linux** recommended.

---

## 5. Build and run

```bash
make          # build all demos
make clean    # remove built binaries next to sources
```

Examples:

```bash
./flush-reload/attack
./evict-reload/evict_reload
./flush-flush/flush_flush
./aes-timing/aes_timing
./spectre-v1/spectre_v1
./spectre-v2/spectre_v2
./meltdown/meltdown_receiver
./rowhammer/rowhammer
./netspectre/netspectre_local
```

See each subdirectory **`README.md`** for details.

---

## 6. Further reading

- **Kocher et al., “Spectre Attacks: Exploiting Speculative Execution,”** IEEE S&P **2019** (expanded from the **Jan 2018** disclosure).  
- **Lipp et al., “Meltdown: Reading Kernel Memory from User Space,”** USENIX Security **2018**.  
- **Yarom & Falkner, “Flush+Reload: a High Resolution, Low Noise, L3 Cache Side-Channel Attack,”** **2014**.  
- **Liu et al., “Last-Level Cache Side-Channel Attacks are Practical,”** USENIX Security **2015**.  
- **Kim et al., “Flipping Bits in Memory Without Accessing Them,”** ISCA **2014** (Rowhammer).  
- **Kocher, “Timing Attacks on Implementations of Diffie-Hellman, RSA, DSS, and Other Systems,”** CRYPTO **1996** (classic software timing).  
- **Survey-style** overviews of **Meltdown/Spectre** and **microarchitectural** threats appeared widely in **2018–2020** (IEEE, ACM, USENIX); use those for breadth beyond this repo.

---

## 7. License

### MIT License

This project is released under the **[MIT License](https://opensource.org/licenses/MIT)**. The full legal text is in [`LICENSE`](LICENSE) in the repository root (2026 Shitong Guo).
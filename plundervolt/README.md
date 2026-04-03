# Plundervolt

## Idea

**CVE-2019-11157** and related: **software-controlled** voltage / frequency scaling (**undervolting**) to induce **faults** in cryptographic or privileged code and leak secrets.

## Why there is no PoC here

- Requires **privileged** access to **MSRs** or firmware interfaces (`msr` driver, vendor tools).
- Incorrect settings can **damage** hardware or cause **instability**.

## Further reading

Search for **Plundervolt** (2019) and **Clarence** follow-up work for academic references.

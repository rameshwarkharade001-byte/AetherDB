<div align="center">

# ⚡ AetherDB ⚡
### *Next-Gen High-Concurrency In-Memory Engine in C++20*

![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Build](https://img.shields.io/badge/Build-Passing-brightgreen?style=for-the-badge&logo=cmake)
![Throughput](https://img.shields.io/badge/Throughput-150k%2B%20ops%2Fsec-blueviolet?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-orange?style=for-the-badge)

<p align="center">
  <b>A sub-millisecond, multi-threaded storage engine equipped with LRU cache eviction and crash-resilient Write-Ahead Logging (WAL).</b>
</p>

---

</div>

## 🧬 Architectural Topology

```text
       ┌───────────────────────────────┐
       │   Concurrent Client Threads   │
       └──────────────┬────────────────┘
                      │
                      ▼
       ┌───────────────────────────────┐
       │     std::shared_mutex         │  <--- Read-Write Lock Isolation
       └──────┬─────────────────┬──────┘
              │                 │
              ▼                 ▼
   ┌────────────────────┐   ┌───────────────────────────┐
   │ In-Memory LRU Pool │   │ Sequential Append-Only Log│
   │ (Hash-Indexed DLL) │   │ (Durable Crash Recovery)  │
   └────────────────────┘   └───────────────────────────┘

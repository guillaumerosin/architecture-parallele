# Matrix Multiplication Report
**UE: Parallel Architectures — AA: Parallel Computing Lab Work**  
**Master in Industrial Engineering Sciences — Specialization: Life Data Technologies**  
**Academic Year: 2025–2026**  
**Author:** Rosin Guillaume | **Supervisor:** CREMER Samuel

---

## Table of Contents

- [Equipment](#equipment)
- [Project Overview](#project-overview)
- [Tasks Summary](#tasks-summary)
  - [Task 1 — Sequential](#task-1--sequential)
  - [Task 1.1 — Optimization 1: Transposed Matrix BT](#task-11--optimization-1-transposed-matrix-bt)
  - [Task 1.2 — Optimization 2: Float + AVX/FMAD](#task-12--optimization-2-float--avxfmad)
  - [Task 1.3 — All Three Optimizations Combined](#task-13--all-three-optimizations-combined)
  - [Task 2 — One Thread per Element](#task-2--one-thread-per-element)
  - [Task 3 — One Thread per Row](#task-3--one-thread-per-row)
  - [Task 4 — Dynamic Thread Allocation](#task-4--dynamic-thread-allocation)
  - [Task 4.1 — Optimization 1: BT Transposition](#task-41--optimization-1-bt-transposition)
  - [Task 4.2 — Optimization 2: Float + AVX/FMAD](#task-42--optimization-2-float--avxfmad)
  - [Task 4.3 — Optimization 3: 64-byte Memory Alignment](#task-43--optimization-3-64-byte-memory-alignment)
  - [Task 5 — CUDA on GPU](#task-5--cuda-on-gpu)
  - [Task 5.1 — CUDA with Transpose Optimization (BT)](#task-51--cuda-with-transpose-optimization-bt)
- [Key Results](#key-results)
- [General Conclusion](#general-conclusion)

---

## Equipment

| Component | Details |
|---|---|
| **OS 1** | Debian GNU/Linux 12 (Bookworm) x86_64 |
| **OS 2** | Windows 11 |
| **Kernel** | 6.1.0-43-amd64 |
| **Processor** | 11th Gen Intel Core i7-11700KF (16 threads) @ 4.900GHz |
| **RAM** | 32 GB |
| **GPU** | NVIDIA GeForce RTX 3070 Lite Hash Rate |
| **Compiler** | GCC 12.2.0 |

---

## Project Overview

This project explores and benchmarks different approaches to matrix multiplication in C++, on both CPU and GPU. The goal is to progressively move from a simple sequential implementation to increasingly parallel and optimized solutions.

All tests were performed on square matrices of sizes: **100×100, 500×500, 1000×1000, 1500×1500, 2000×2000** — on the same physical machine running both Debian Linux and Windows 11.

Three successive optimizations were applied across multiple tasks:
1. **BT Transposition** — cache-friendly memory access (eliminates column-jumping cache misses)
2. **Float + AVX/FMAD** — hardware-level floating-point acceleration
3. **64-byte Memory Alignment** — eliminates cache line splits via `_mm_malloc`

---

## Tasks Summary

### Task 1 — Sequential

Baseline single-threaded matrix multiplication. Complexity: **O(N³)**.

- CPU usage: ~100% (fully CPU-bound)
- Memory scales as expected with N²
- **2000×2000:** 16.02s (Linux) / 16.35s (Windows)
- Confirms the need for parallelization — 15 out of 16 CPU cores sit idle

---

### Task 1.1 — Optimization 1: Transposed Matrix BT

Instead of reading columns of B (cache-unfriendly), a transposed matrix BT is created and rows are read instead — eliminating cache misses.

> **Expected gain: ~2–3×**

- **2000×2000:** 3.14s (Linux) / 3.43s (Windows)
- **Speedup vs naive: ~5×**
- Memory usage increases slightly due to BT matrix

---

### Task 1.2 — Optimization 2: Float + AVX/FMAD

Data converted from `int` to `float` to leverage AVX units and the **FMAD** (Fused Multiply-Add) instruction — performing multiply + add in a single instruction.

> **Expected gain: ~1.5×** — actual results were slower due to compiler not fully vectorizing without explicit flags (`-mavx2`, `-mfma`)

- **2000×2000:** 6.85s (Linux) / 7.03s (Windows)
- Slower than optimization 1 — AVX benefits depend heavily on compiler vectorization

---

### Task 1.3 — All Three Optimizations Combined

BT transposition + float/AVX/FMAD + 64-byte memory alignment via `_mm_malloc`.

> **Best sequential result of the entire lab**

- **2000×2000:** **0.68s (Linux)** / 7.33s (Windows)
- **Speedup vs naive on Linux: ~24×**
- CPU usage on Linux exceeds 1100% → compiler successfully triggered AVX vectorization
- Windows sees minimal benefit — GCC + Linux kernel interact far more effectively with alignment

---

### Task 2 — One Thread per Element

Creates N² threads simultaneously — one per matrix element.

- Massive overhead and instability beyond 100×100 on Linux
- Windows crashed beyond 1500×1500
- **Not practical** — too much thread creation/management cost cancels all parallelism gains

---

### Task 3 — One Thread per Row

One thread per row (N threads instead of N²).

- **2000×2000:** 2.1s (Linux) / 4.32s (Windows)
- **Speedup vs sequential: ~8×**
- CPU usage reaches 1200%+ — cores effectively utilized
- Stable and complete results for all matrix sizes

---

### Task 4 — Dynamic Thread Allocation

User chooses the number of threads. Both **alternating** and **contiguous** row distribution modes tested.

Values tested: `[0.25T, 0.5T, 0.75T, T, 1.5T, 2T]` where T = 16 logical threads

- **Optimal: T=16** for all matrix sizes and both distributions
- Beyond T=16, context switching overhead degrades performance
- **2000×2000 with 16 threads:** 0.518s (Linux) / 4.34s (Windows) → factor of **8×** between OSes
- Alternating vs contiguous: performance nearly identical in all configurations

---

### Task 4.1 — Optimization 1: BT Transposition

BT transposition applied to the multithreaded version.

- **2000×2000, 16 threads:** 0.349s (Linux) / 0.382s (Windows)
- **Speedup vs unoptimized TP4: ~1.5×**
- Confirms cache-friendly access improves performance independently of parallelism level

---

### Task 4.2 — Optimization 2: Float + AVX/FMAD

Float conversion added on top of BT transposition.

- **2000×2000, 16 threads:** 0.461s (Linux) / 0.529s (Windows)
- Slightly slower than optimization 1 — compiler vectorization not fully triggered without explicit flags
- CPU usage exceeds 1500% on Linux for large matrices

---

### Task 4.3 — Optimization 3: 64-byte Memory Alignment

Data aligned on 64-byte boundaries using `_mm_malloc(..., 64)`.

> **Expected gain: ~1.2×**

- **2000×2000, 16 threads:** 0.559s (Linux) / 0.563s (Windows)
- Smallest and most marginal gain — can be masked by measurement variability
- Nearly identical results between Linux and Windows → alignment benefits both equally

**Summary of all multithreaded optimizations (2000×2000, Linux, 16 threads):**

| Version | Time (s) |
|---|---|
| TP4 no optimization | 0.518 |
| TP4 optim 1 (BT) | 0.349 |
| TP4 optim 2 (float + AVX) | 0.461 |
| TP4 optim 3 (64B alignment) | 0.559 |

> Optimization 1 (BT transposition) is by far the most impactful — it directly addresses the main bottleneck: **cache misses**.

---

### Task 5 — CUDA on GPU

Matrix multiplication on GPU using CUDA — one thread per matrix element.

- Tested configurations: 128, 256, 512, 1024 blocks × threads
- **Computation times drop to fractions of a millisecond**
- **Key finding:** memory transfers (CPU↔GPU) often represent **2–5× the computation time itself**
- Windows faster than Linux for pure computation — likely due to GPU driver differences
- `cudaDeviceSynchronize()` essential before timing — without it, measurements are completely misleading

---

### Task 5.1 — CUDA with Transpose Optimization (BT)

BT transposition applied on GPU — rows of BT read instead of columns of B, improving **memory coalescing**.

> Creation and deletion of BT included in the compute time measurement, as required.

- **2000×2000 without transfers:** ~0.025ms → ~0.010ms (Linux) → **speedup ~2.5×**
- Small matrices (100×100): gain is negligible or slightly negative — BT creation cost not amortized
- Windows: less impact since base times were already very low
- Confirms: **memory organization on GPU has as much impact as thread count**

---

## Key Results

| Task | Method | Time — 2000×2000 (Linux) |
|---|---|---|
| Task 1 | Sequential (naive) | 16.02s |
| Task 1.1 | Sequential + BT | 3.14s |
| Task 1.2 | Sequential + BT + float | 6.85s |
| Task 1.3 | Sequential + all 3 optims | **0.68s** |
| Task 3 | 1 thread/row | 2.1s |
| Task 4 | 16 threads (dynamic) | 0.518s |
| Task 4.1 | 16 threads + BT | 0.349s |
| Task 5 | CUDA (no transfer) | ~0.009ms |
| Task 5.1 | CUDA + BT (no transfer) | ~0.010ms |

---

## General Conclusion

This project illustrates that **performance optimization is never a one-size-fits-all solution**. Each layer of optimization — algorithmic, memory-related, or hardware-specific — contributes differently depending on problem size and execution context.

The single most important lesson from this lab:

> **Before adding more parallelism, always make sure the memory access pattern is efficient.**

This principle explained the most impactful results observed on both CPU and GPU — from the 5× speedup of BT transposition in sequential mode, to the 2.5× gain from memory coalescing on the GPU.
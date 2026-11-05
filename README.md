# Beat

`Beat` is a small, deterministic 2D engine written in C++11 (used strictly as "C with perks") using SDL3 and Dear ImGui.

The goal of this project is to explore and demonstrate **Data-Oriented Design (DoD)** and hardware cache efficiency from first principles. Instead of relying on traditional object-oriented hierarchies, virtual methods, or dynamic heap allocations, `Beat` organizes memory linearly so modern CPUs can do what they do best: stream contiguous data at maximum speed.

Under heavy stress testing on desktop hardware, the engine simulates and renders up to **100,000 active colliding entities** at a deterministic 60 Hz physics tick rate while keeping the render pipeline running at full speed.

---

## Philosophy & Design

Most modern game engines wrap simple operations in layers of indirection: pointers to objects, abstract base classes, and frequent dynamic allocations. `Beat` takes the opposite path:

- **Flat Data over OOP:** Entities do not exist as classes or objects. An entity is simply an integer ID. All component data (positions, velocities, rotations, colliders) lives in flat, contiguous memory pools (**Structure of Arrays / SoA**).
- **Robin Hood Hash Map (`BeatMap.h`):** To avoid the cache-unfriendly pointer chasing of `std::unordered_map`, the engine uses an open-addressing hash table with Robin Hood hashing, backward-shift deletion, and 64-bit FNV-1a string hashing. It uses a **hash-as-identity** model: only 64-bit hashes and values are stored in a contiguous, power-of-two array with zero string retention and zero dynamic node allocations.
- **Double-Buffered Deferred Event Queue (`Event.h`):** Mutating contiguous entity arrays while actively iterating over collision pairs causes index invalidation. A statically preallocated, double-buffered event queue (2,048 slots) defers structural changes (projectile spawns, asteroid destructions) to an isolated end-of-step pass.
- **Zero Heap Allocations in the Loop:** Once initialized, the engine never calls `malloc`, `new`, or resizes dynamic containers during frames. All entity pools, event queues, and spatial grids have fixed, preallocated capacities.
- **Uniform Spatial Grid ($O(N)$ Broadphase):** Collision detection for 100,000 bodies avoids complex tree structures. It uses a static 2D grid where cells and entities are linked via flat integer index arrays (`CellHeads` and `EntityNext`), offering $O(1)$ cell insertion and completely eliminating pointer chasing.
- **Dynamic Geometry Batching & Culling:** Visible sprites are filtered via frustum culling, transformed, and batched into a contiguous dynamic vertex buffer, minimizing GPU draw calls.
- **Decoupled Simulation & Rendering:** The physics engine steps time forward in fixed, deterministic chunks (`TargetDelta = 1/60s`), while rendering runs unlocked with sub-frame interpolation (`Alpha`) to eliminate visual stutter.

---

## Performance & Benchmarking

The engine was benchmarked under an extreme stress test of **100,000 active colliding entities** with unlocked framerate across a continuous 10,000-frame automated capture on an **Intel Core i7-11850H (locked at 2.50 GHz base clock)** running Linux.

### Frametime Distribution (10,000 frames @ 100k entities)

| Metric | Total Frametime | Physics Loop (60 Hz) | Render Pipeline |
| :--- | :---: | :---: | :---: |
| **Median ($p50$)** | **1.96 ms** | **11.99 ms** | **1.90 ms** |
| **99th Percentile ($p99$)** | **14.36 ms** | **12.57 ms** | **2.00 ms** |
| **Average** | **6.85 ms** | **12.02 ms** | **1.90 ms** |

Due to the decoupled loop, render-only frames complete in ~1.9 ms, while simulation frames execute the full 100,000-body physics step in ~12 ms, well within the 16.67 ms (60 Hz) budget.

### Hardware Profiling (`perf stat` on Linux)

Test platform: **Intel Core i7-11850H @ 2.50 GHz** (pinned frequency, Linux `x86_64`).

User-space hardware counters (`:u`):
- **L3 / Last Level Cache Hit Rate:** **99.70%**
- **Global Cache Hit Rate:** **99.98%** 
- **Instructions Per Cycle (IPC):** **1.05**

---

## Codebase Tour

The entire engine is contained in a few flat files without directory mazes:

- `include/Entity.h` & `src/Entity.cpp`: Contiguous SoA world storage and component arrays.
- `src/Level.cpp`: Uniform spatial grid broadphase without pointer chasing.
- `src/BatchRenderer.cpp`: Dynamic quad and UV vertex buffer batching.
- `include/BeatMap.h`: Robin Hood open-addressing hash map with backward-shift deletion.
- `src/Benchmark.cpp`: Preallocated frame telemetry and percentile calculation.
- `src/Main.cpp`: Fixed timestep simulation loop, input latching, and sub-frame interpolation.

---

## Building and Running

### Requirements
- A C++11 compiler (`g++` or `clang++`)
- `SDL3` and `SDL3_image`
- GNU `make`

### Compiling
```bash
make
```

### Running
```bash
./build/bin/main
```

### Controls
- **A / D:** Rotate spaceship
- **W:** Thrust forward
- **Space:** Fire laser
- **TAB:** Switch between Play Mode and Editor Mode

---

## License

`Beat` is released under the BSD 2-Clause License. See `LICENSE` for details.

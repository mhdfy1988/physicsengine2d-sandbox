# Kernel Optimization TODO

1. [x] O1: Remove repeated narrowphase detect in position solver loop
2. [x] O2: Replace O(constraints) blocked-pair checks with precomputed blocked-pair cache
3. [x] O3: Replace byte pair-flags clear with lower-bandwidth bitset + stamp strategy
4. [x] O4: Move grid broadphase large stack buffers to scratch arena
5. [x] O5: Replace `clock()` with monotonic high-resolution timing source
6. [x] O6: Add compile-time trace switch for near-zero overhead when disabled

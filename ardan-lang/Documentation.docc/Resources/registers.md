# arm64 registers

x29: frame pointer
x30 is the return address (LR)

---

# The three instructions (behavior & intent)

### 1) `stp x29, x30, [sp, #-16]!` — *store-pair, pre-indexed*

**Effect (semantic):**

1. Compute `sp ← sp - 16` (the pre-index `!` writes the updated SP back).
2. Store register `x29` at address `sp + 0`.
3. Store register `x30` at address `sp + 8`.

**So after the instruction:**

* `sp` has decreased by 16 bytes.
* memory at new `sp` holds `x29` (low element) and `x30` (high element).

**Why use it in a prologue?**

* Saves the *frame pointer* (`x29`) and *link register* (`x30`) onto the stack so the callee can use those registers freely and restore them before returning.
* `x29` is the conventional frame pointer; `x30` is the return address (LR). According to the AArch64 (ARM64) calling convention, `x29` and `x30` are callee-saved (i.e., a callee must preserve them if it uses them).

**Addressing mode detail:** pre-indexed `[sp, #-16]!` decrements SP first and then stores (write-back). This is compact and saves an extra instruction to update SP.

**Does it affect flags?** No (stores do not set condition flags).

---

### 2) `mov x29, sp` — *set frame pointer*

**Effect (semantic):**

* Copy the current stack pointer into `x29` so `x29` becomes the frame pointer for this function.

**Why?**

* Establishes a stable frame pointer that the function and debuggers can use to reference local variables and saved registers with fixed offsets. This is useful for:

  * Easier debugging and backtraces.
  * Simpler addressing of locals and spills (constant offsets from `x29`).
  * Exception unwinding/CFI (when present) often refers to frame pointer based offsets.

**Optimization note:** Compilers may omit a frame pointer in leaf/simple functions (frame-pointer omission) and access locals relative to `sp` instead. But when `x29` is used, it must be saved/restored (hence `stp`/`ldp`).

---

### 3) `ldp x29, x30, [sp], #16` — *load-pair, post-indexed*

**Effect (semantic):**

1. Load `x29` from memory at `sp + 0`.
2. Load `x30` from memory at `sp + 8`.
3. Then compute `sp ← sp + 16` (post-index `#16` writes back after the loads).

**Why in epilogue?**

* Restores the saved frame pointer and return address and then adjusts `sp` back to the caller’s stack position (undoing the `stp`/stack allocation). After this, a `ret` will typically use `x30` to return.

**Post-indexing** is compact: load pair and update `sp` in one instruction.

---

# The canonical prologue/epilogue sequence (what compilers emit)

Typical:

```asm
stp x29, x30, [sp, #-16]!   // save FP and LR, allocate 16 bytes
mov x29, sp                 // set FP to new SP
...                         // function body: can push more stuff or use x29 as base
ldp x29, x30, [sp], #16     // restore FP and LR, deallocate 16 bytes
ret                         // return via x30
```

**Stack layout after prologue:**

```
higher addresses
   ...
caller saved area / args
---------
(sp_old)  <--- caller's stack pointer before callee
---------
(sp_new)  after stp: store x29 @ sp_new, x30 @ sp_new+8
   [sp_new + 0]   = old x29 (saved frame pointer)
   [sp_new + 8]   = old x30 (saved return address)
local space (if any)
---------
lower addresses
```

After `mov x29, sp`, `x29` points at the saved `x29` value on stack (so offsets like `x29+#x` can access locals and saved registers).

---

# Why use pair instructions (`stp`/`ldp`) instead of two stores/loads?

* **Compactness:** One 32-bit instruction encodes storing two registers with addressing/write-back, smaller code size.
* **Performance:** Fewer instructions, fewer decode cycles, potential microarchitectural efficiencies.
* **Convenience:** Combined with pre/post-index addressing lets you allocate/deallocate and save/restore in a single instruction.

**BUT** `stp`/`ldp` are not atomic across both registers — they are a single instruction at the ISA level, but they do two memory accesses. If another core could observe memory mid-update (rare in typical stack usage), you must still consider memory ordering; those instructions do not provide cross-core atomicity or memory barriers.

---

# ABI and register conventions relevant here

* **Callee-saved registers:** `x19`–`x28`, `x29` (FP), `x30` (LR). If the callee uses any of these, it must restore them before returning.
* **Caller-saved registers:** `x0`–`x18` (excluding callee-saved) — caller must preserve if needed across calls.
* **Stack alignment:** SP **must be 16-byte aligned** at public ABI call boundaries. That’s why we allocate multiples of 16 bytes (`#-16`) — to maintain alignment.

---

# Interaction with exceptions, unwinding, and debugging

* Unwind/debugging information (DWARF CFI on typical toolchains) will describe the prologue so debuggers and exception handlers can unwind the stack. The presence of `mov x29, sp` makes it easy: the frame pointer points to a fixed frame layout.
* If you manually emit code (JIT), and you don’t emit matching unwind metadata or preserve `x29`/`x30` correctly, debuggers and crash reports will show corrupted stacks / nonsense backtraces.

---

# Pointer Authentication (Apple Arm64e) — brief note

On Apple Silicon, you may see `pacibsp` and `autibsp` instructions around prologues/epilogues. These implement *pointer authentication* (PAC) for branch targets (return addresses):

* A compiler may **sign** the return address (in `x30`) when storing it and **authenticate** it before returning.
* If pointer authentication is enabled, the prologue may do `pacibsp` before storing LR and the epilogue does `autibsp` (or similar) before using LR.
* For JITs you must be careful: if your generated code touches return addresses, PAC must be handled correctly on Arm64e — otherwise authentication will fail and the program traps.

*(If you’re not on an Arm64e target or not using PAC, you can ignore this; but your crash trace earlier had `pacibsp` which indicates the platform may be using PAC.)*

---

# Practical pitfalls (especially for JIT / hand-coded emitters)

1. **Not preserving FP/LR when you should.**
   If your generated function clobbers `x29`/`x30` and doesn’t restore them, return will break.

2. **Wrong SP update timing or misalignment.**
   If `sp` is not 16-byte aligned at call boundaries, external calls or VFP operations may fault or be slower. Some instructions require 16-byte alignment for ABI calls.

3. **Omitting frame-pointer when debugging or unwinding needed.**
   If you don’t set `x29`, backtraces are harder; if you do set it you must save/restore it.

4. **Returning to an invalid address** (via corrupted `x30`).
   Generated code must ensure `x30` contains a valid return address when `ret` executes.

5. **Using `stp`/`ldp` with wrong immediates.**
   Remember immediates are scaled by register width/element size for pair instructions (encoded imm is in units of 8 bytes for 64-bit pair). Encoding mistakes lead to wrong memory addresses.

6. **Pointer authentication mismatch (on Arm64e).**
   If platform uses PAC and you don’t sign/verify LR per ABI, `ret` or `blr` may trap.

7. **Unwind metadata mismatch.**
   If your prologue/epilogue sequence doesn’t match the unwind info, debuggers and profilers will be confused.

---

# Memory ordering & atomics note

* `stp`/`ldp` are normal memory accesses and follow the usual memory model. They do **not** act as memory barriers. If you need ordering across cores (synchronization), you must use explicit barrier instructions (e.g., `dmb ish`) or atomic primitives.

---

# Example: how a simple function call looks (complete)

Caller:

```asm
// caller ensures stack is aligned
bl callee   // branch and link, LR <- return address
...
```

Callee (what compiler often emits):

```asm
stp x29, x30, [sp, #-16]!   // save FP & LR, sp-=16
mov x29, sp                 // set FP
// ... function body (may allocate more stack, save other callee-saved regs) ...
ldp x29, x30, [sp], #16     // restore FP & LR (sp+=16)
ret                         // return using x30
```

---

# Quick checklist for JIT/emitter correctness

* Always emit a matching prologue and epilogue if you modify `x29`/`x30`.
* Maintain `sp` 16-byte aligned at call boundaries.
* If you use `stp`/`ldp`, ensure the immediate encodings are correct (scaled by 8).
* If target supports PAC (Arm64e), follow the platform’s conventions (or disable PAC for JITed code if possible/allowed).
* Provide unwind metadata if you want meaningful crash stacks.

---

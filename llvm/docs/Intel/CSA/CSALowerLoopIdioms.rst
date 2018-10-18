==================
CSALowerLoopIdioms
==================

**Intel Top Secret**

.. contents::
  :local:

.. toctree::
  :hidden:

Introduction
============

CSALowerLoopIdioms is a small pass that lowers memcpy/memmove/memset intrinsics
into plain loops or flat expansions. These can also be lowered through a
combination of custom lowering in instruction selection and/or calls to the C
runtime, but doing the expansion earlier than that is advantageous on CSA
because the expanded memory operations will be directly visible to the memory
ordering pass and because the lowering that we can do with this pass is more
context-aware than runtime calls.

CSALowerLoopIdioms was originally based on the NVPTXLowerAggrCopies pass, which
has since been updated to use common LLVM routines declared in
``include/llvm/Transforms/Utils/LowerMemIntrinsics.h`` for its loop expansions.
It may be worthwhile investigating using the same functions in this pass, but
for now the loop expansions in this pass are implemented separately in order to
give us more flexibility to better tune them for CSA.

Pass Options
============

\-csa-max-stores-per-memintr=<n>
  This option defines the maximum number of store instructions used per memory
  intrinsic expansion. For any memory intrinsic call with a constant length, the
  flat expansion will be chosen if the resulting number of stores will be
  ``<= <n>``; otherwise, the loop expansion will be used. The number of loads
  will be the same as the number of stores for memcpy/memmove intrinsics; memset
  intrinsics will not use any loads. Increase this parameter to enable flat
  expansions of larger sizes to improve performance at the cost of more vc* unit
  usage; decrease it to save vc* units at the cost of performance.

Expansions
==========

This section covers the different intrinsics handled by this pass and how they
are expanded.

@llvm.memcpy
------------

Flat Expansion
~~~~~~~~~~~~~~

Small constant-length memcpy calls are expanded into a series of consecutive
aligned loads and stores of the largest size permitted by the alignment and
copy length. For instance, this is what expansion for a two byte aligned copy of
five bytes would look like:

.. code-block:: llvm

   call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 2 %dst, i8* align 2 %src, i64 5, i1 0)

   ; v flat expansion v

   %a0 = bitcast i8* %src to i16*
   %v0 = load i16, i16* %a0, align 2
   %a1 = getelementptr inbounds i16, i16* %a0, i64 1
   %v1 = load i16, i16* %a1, align 2
   %a2 = getelementptr inbounds i8, i8* %src, 4
   %v2 = load i8, i8* %a2

   %a3 = bitcast i8* %dst to i16*
   store i16 %v0, i16* %a3, align 2
   %a4 = getelementptr inbounds i16, i16* %a3, i64 1
   store i16 %v1, i16* %a4, align 2
   %a5 = getelementptr inbounds i8, i8* %dst, 4
   store i8 %v2, i8* %a5

The first four bytes are copied in two chunks of two and the last byte is copied
by itself.

This is a straightforward expansion but there are some possible improvements
that will need to be investigated for later versions of the pass:

- Non-scratchpad accesses are allowed to be unaligned but may come with a
  performance penalty. The benefits from using fewer memops might outweigh this
  penalty, in which case it would be advantageous to emit larger
  possibly-unaligned accesses instead of always conservatively emitting smaller
  aligned ones.
- If alignment is important and there is a case where one of the addresses is
  known to be more aligned than the other, it might make sense to use
  loads/stores of different sizes and re-pack the values in between.
- Scratchpad accesses need to be aligned, but we also only get one of them per
  scratchpad. Figuring out how to work around this limitation will be part of a
  larger effort to support scratchpads properly in the compiler.
- Because this is is a memcpy, none of the stores is allowed to alias with any
  of the loads. This expansion should mark the accesses so that memory ordering
  can take advantage of this, but this hasn't been implemented yet.

Loop Expansion
~~~~~~~~~~~~~~

Large or non-constant-length memcpy calls are expanded into simple copy loops
according to the known alignment of the addresses and the known divisibility of
the length:

.. code-block:: llvm

   entry:
     %mulen = mul nuw i64 %len, 6
     call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %dst, i8* align 4 %src, i64 %mulen, i1 0)
     ret void

   ; v loop expansion v

   entry:
     %newlen = mul nuw i64 %len, 3
     %ssrc = bitcast i8* %src to i16*
     %sdst = bitcast i8* %dst to i16*
     %docpy = icmp ugt i64 %newlen, 0
     br i1 %docpy, label %memcpyloop, label %split

   memcpyloop:
     %i = phi i64 [ 0, %entry ], [ %ip1, %memcpyloop ]
     %a0 = getelementptr inbounds i16, i16* %ssrc, i64 %i
     %v = load i16, i16* %a0, align 2
     %a1 = getelementptr inbounds i16, i16* %sdst, i64 %i
     store i16 %v, i16* %a1, align 2
     %ip1 = add nuw i64 %i, 1
     %cont = icmp ult i64 %ip1, %newlen
     br i1 %cont, label %memcpyloop, label %split

   split:
     ret void

In this case, the addresses are aligned to four bytes but the length is only
known to be a multiple of two, so the loop copies two bytes at a time.

Some more considerations for future versions of the pass:

- As with the flat expansion, using larger unaligned accesses or mixing accesses
  of different sizes might be beneficial.
- It might make sense to use cleanup loops if the length is not known to be a
  multiple of the desired access size, but cleanup loops on CSA are
  disproportionately expensive.
- This loop is parallel and should be marked as such to make sure that memory
  ordering can take advantage of it, but this has also not been implemented yet.

@llvm.memmove
-------------

Flat Expansion
~~~~~~~~~~~~~~

The flat expansion for memmove is identical to the one for memcpy - the only
difference is that the ordering needs to be enforced between the loads and the
stores and so they cannot be marked as independent.

Loop Expansion
~~~~~~~~~~~~~~

Since the arrays for memmove are allowed to overlap, the implementation needs to
ensure that it's copying things in the right direction so that it doesn't
overwrite parts of the src array that it will read later. The standard
implementation uses two loops inside of an if for this, but since code size is
a limiting factor on CSA it is better to use one loop with a varying step size:

.. code-block:: llvm

   entry:
     %shlen = shl nuw i64 %len, 1
     call void @llvm.memmove.p0i8.p0i8.i64(i8* align 2 %dst, i8* align 2 %src, i64 %shlen, i1 0)
     ret void

   ; v loop expansion v

   entry:
     %ssrc = bitcast i8* %src to i16*
     %sdst = bitcast i8* %dst to i16*
     %backward = icmp ult i64* %ssrc, %sdst
     %stride = select i1 %backward, i64 -1, i64 1
     %lenm1 = sub i64 %len, 1
     %start = select i1 %backward, i64 %lenm1, i64 0
     %domove = icmp ugt i64 %len, 0
     br i1 %domove, label %memmoveloop, label %split

   memmoveloop:
     %i = phi i64 [ 0, %entry ], [ %ip1, %memmoveloop ]
     %off = phi i64 [ %start, %entry ], [ %offpstride, %memmoveloop ]
     %a0 = getelementptr inbounds i16, i16* %ssrc, i64 %off
     %v = load i16, i16* %a0, align 2
     %a1 = getelementptr inbounds i16, i16* %sdst, i64 %off
     store i16 %v, i16* %a1, align 2
     %offpstride = add i64 %off, %stride
     %ip1 = add nuw i64 %i, 1
     %cont = icmp ult i64 %ip1, %len
     br i1 %cont, label %memmoveloop, label %split

   split:
     ret void

As in the standard expansion, this will copy backwards from the end of both
arrays if ``src < dst`` and forwards from the beginning otherwise.

The loop expansion for memmove might also be able to benefit from larger/mixed
access sizes and maybe a cleanup loops, but the loop cannot be marked parallel
because there are dependencies across loop iterations. However, something like
the simd pragma could be used on this loop because there are no backwards
dependencies.

@llvm.memset
------------

Flat Expansion
~~~~~~~~~~~~~~

The flat expansion for memset is nearly identical to the one for memcpy except
that the loads are replaced with shifts and ors of the value:

.. code-block:: llvm

   call void @llvm.memset.p0i8.i64(i8* align 2 %dst, i8 %src, i64 5, i1 0)

   ; v flat expansion v

   %src16lo = zext i8 %src to i16
   %src16hi = shl i16 %src16lo, 8
   %src16 = or %src16lo, %src16hi
   %a0 = bitcast i8* %dst to i16*
   store i16 %src16, i16* %a3, align 2
   %a1 = getelementptr inbounds i16, i16* %a3, i64 1
   store i16 %src16, i16* %a4, align 2
   %a2 = getelementptr inbounds i8, i8* %dst, 4
   store i8 %src, i8* %a5

Like the flat memcpy expansion, more investigation of the interaction between
alignment and access sizes could be warranted. Since there are no loads and the
stores are at constant offsets, they will already be kept parallel by the memory
ordering pass.

Loop Expansion
~~~~~~~~~~~~~~

The loop expansion for memset is also very close to the one for memcpy:

.. code-block:: llvm

   entry:
     %shlen = shl nuw i64 %len, 1
     call void @llvm.memset.p0i8.i64(i8* align 2 %dst, i8 %src, i64 %shlen, i1 0)
     ret void

   ; v loop expansion v

   entry:
     %src16lo = zext i8 %src to i16
     %src16hi = shl i16 %src16lo, 8
     %src16 = or %src16lo, %src16hi
     %sdst = bitcast i8* %dst to i16*
     %docpy = icmp ugt i64 %len, 0
     br i1 %docpy, label %memsetloop, label %split

   memsetloop:
     %i = phi i64 [ 0, %entry ], [ %ip1, %memcpyloop ]
     %a0 = getelementptr inbounds i16, i16* %sdst, i64 %i
     store i16 %src16, i16* %a1, align 2
     %ip1 = add nuw i64 %i, 1
     %cont = icmp ult i64 %ip1, %len
     br i1 %cont, label %memsetloop, label %split

   split:
     ret void

As with memcpy, investigation into alignment and access sizes could be
warranted and the loop can be marked parallel, though in this case it is already
parallel as a result of the self-ordering of the single store.

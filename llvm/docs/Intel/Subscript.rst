===================================================================
Subscript intrinsic for addressing dynamic multi-dimensional arrays
===================================================================

.. contents::
   :local:

Syntax:
=======

::

    declare <ty>* @llvm.intel.subscript...(i8 <rank>, <ty> <lb>,
        <ty> <stride>, <ty>* <base>, <ty> <index>) norecurse readnone speculatable


Introduction
============

This is an overloaded intrinsic corresponding to ``getelementptr`` instruction
computing address inside array objects, which may have variable extents in
several dimensions, like C99 variable length arrays or Fortran arrays.

Overview
========

Consider simplified view of LLVM types::

    <ty> ::= [ N x <ty> ]       ; array type, N is constant
        | { <ty1>, <ty2>, ... } ; structure type depending on list of types,
                                ;   potentially empty
        |  <ty>*                ; pointer type
        | ... other types

Type system is static and does not represent C99 length arrays, which may have
run-time dependent array size.

``getelementptr`` instruction computes addresses in a structured way for static
LLVM types using the following recursive definition:

.. _subscript_node_gep_definition:

.. code-block:: llvm

    ; Array type case. First index is zero.
    ; If <ind2> has inrange attribute,
    ; then the following relations should hold
    ;   0 <= <ind2> < N
    getelementptr [ N x <ty> ],
                  [ N x <ty> ]* <ptrval>,
                  <ty1> 0, <ty2> <ind2>, ...remaining args =>
        getelementptr
            <ty>,
            <ty>* (<ptrval> +
                  ; <ind2> * sizeof(<ty>)) == index_offset([ N x <ty> ], <ind2>)
                  ; Reason for new terminology will become clear below.
                  index_offset([ N x <ty> ], <ind2>)),
            i32 0, remaining args...

    ; Structure type case. First index is zero.
    getelementptr { <fty1>, <fty2> ... },
                  { <fty1>, <fty2> ... }* <ptrval>,
                  <ty1> 0,
                  ; <const_ind> should be compile-time constant
                  <ty2> <const_ind>,
                  ...remaining args =>
        getelementptr
            <fty<const_ind> >, ; const_ind'th element of list { <fty1>, ... }
            <fty<const_ind> >* (<ptrval> +
                   ; offsetof depends on layout specification
                   offsetof({ <fty1>, ... }, fty<const_ind>)),
            i32 0, remaining args...

    ; Case of first index is nonzero.
    ;
    ; If <ind1> has inrange attribute, then getelementptr's result is within
    ; bounds of some object, whose element is pointed to by <ptrval>.
    ;
    ; That object is implicit to the instruction.
    getelementptr <ty>,
                  <ty>* <ptrval>,
                  <ty1> <ind1>, ...remaining args =>
        getelementptr
            <ty>,
            <ty>* (<ptrval> +
                ; stride(<ty>) == sizeof(<ty>)
                ; Reason for new terminology will become clear below.
                <ind1> * stride(<ty>)),
            i32 0, remaining args...

    ; Case of single zero offset.
    getelementptr <ty>, <ty>* <ptrval>, <ty1> 0 => <ptrval>

    ; otherwise getelementptr is undefined

Structured address computations have at least 3 advantages.
    1. Exact data layout is abstracted away in ``getelementptr`` instruction
       (but data layout is used in some optimizations of ``getelementptr``);
    2. inrange attribute could be provided for indexes (support is incomplete
       at the time of writing).
    3. ``based on`` relation from :ref:`Pointer Aliasing Rules <pointeraliasing>`
       treats ``getelementptr`` instruction in a special way and provides
       more guarantees compared to, for example, ``inttoptr`` instruction.

If these attributes are important for transformations, then type system
extension is desirable (at least hypothetically) for run-time dependent types::

    ; lower_bound, upper_bound and step can be run-time values.
    ; [lower_bound, upper_bound) is normal range for indexes
    ;
    ; step is negative or positive value
    ;
    ; static arrays [ N x <ty> ] are presented as [ 0 : N : 1 x <ty> ]
    <ty> ::= ...
        [ lower_bound : upper_bound : step x <ty> ]

To specify hypothetical extension of ``getelementptr`` one needs auxiliary
functions already referred to in the ``getelementptr``'s definition above:

.. code-block:: llvm

    ; layout specific definition
    stride [ lb : ub : s x <ty> ] => (ub - lb) * s * stride(<ty>)
    ; Remaining cases are treated in an original way
    ; taking into account
    ;  1. updated definition for array;
    ;  2. target's layout for structures.
    stride { <fty1>, <fty2> ... } => sizeof { <fty1>, <fty2> ... }

    ; layout specific definition
    index_offset [ lb : ub : s x <ty> ], <ty1> <ind> =>
        (<ind> - lb) * s * stride(<ty>)

    ; rank of array type
    ; [ N x <ty> ] is considered a subcase of [ lb : ub : s x <ty> ]
    rank [ lb : ub : s x <ty> ] => 1 + (rank <ty>)
    ; remaining cases
    rank <ty> => 0

Updated definition of ``getelementptr``:

.. code-block:: llvm

    ; Array type case. First index is zero.
    ; If <ind2> has inrange attribute,
    ; then the following relations should hold
    ;   lb <= <ind2> < ub
    getelementptr [ lb : ub : s x <ty> ],
                  [ lb : ub : s x <ty> ]* <ptrval>,
                  <ty1> 0, <ty2> <ind2>, ...remaining args =>
        getelementptr
            <ty>,
            <ty>* (<ptrval> +
                   index_offset ([ lb : ub : s x <ty> ], <ind2>)),
            i32 0, remaining args...

    ; Definition for remaining cases does not change.

Several notes follow:
    1.  It is not expected that type's parameters are provided in type objects,
        because it contradicts static nature of LLVM type system. To keep
        address computations structured, one needs to provide sufficient
        information about values of ``lower_bound``, ``upper_bound`` and
        ``step`` type parameters to address computations.
    2. If run-time parameters are not provided inside structures, then
       ``offsetof`` will be compile-time constant.
    3. To compute address one needs only ``stride(<ty>)`` and
       ``lower_bound`` of ``<ty>``. ``upper_bound`` could be kept
       implicit.
    4. ``<rank>`` argument specifies rank of the returned value.

``llvm.intel.subscript`` provides means to compute structured
addresses for updated ``getelementptr`` definition without LLVM type system
extension while preserving layout independence and keeping inrange attribute.

.. code-block:: llvm

    ; Array type case. First index is zero.
    ; If <ind2> has inrange attribute,
    ; then the following relations should hold
    ;   lb <= <ind2> < ub
    getelementptr [ lb : ub : s x <ty> ],
                  [ lb : ub : s x <ty> ]* <ptrval>,
                  <ty1> 0, <ty2> <ind2>, ...remaining args =>
        getelementptr
            <ty>,
            <ty>*
            ; <ty'> is <ty> with all arrays specifications removed,
            ; because <ty> cannot be represented directly
            call <ty'>* @llvm.intel.subscript...(
                ; There is no implicit dimension from first index
                i8 rank <ty>,
                ; <tya> and <tylb> should be wide enough to represent
                ; all values if interpreted as signed integers
                <tylb> lb,
                <tya> stride(<ty>),
                <ty'> <ptrval>, <ty2> <ind2>),
            i32 0, remaining args...

    ; Case of first index is nonzero.
    ;
    ; If <ind1> has inrange attribute, then getelementptr's result is within
    ; bounds of some object, whose element is pointed to by <ptrval>.
    ;
    ; That object is implicit to the instruction.
    getelementptr <ty>,
                  <ty>* <ptrval>,
                  <ty1> <ind1>, ...remaining args =>
        getelementptr
            <ty>,
            <ty>*
            ; <ty'> is <ty> with all arrays specifications removed,
            call <ty'>* @llvm.intel.subscript...(
                ; There is an implicit dimension from first index,
                ; but this case is consdered like adjustments in
                ; implicit dimension, which do not change rank.
                i8 rank <ty>,
                0, stride(<ty>), <ty'> <ptrval>, <ty1> <ind1>),
            i32 0, remaining args...

    ; Definition for remaining cases does not change.
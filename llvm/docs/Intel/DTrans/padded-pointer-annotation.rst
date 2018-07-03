=============================================================
Passing pointer padding information using pointer annotations
=============================================================

.. contents::
   :local:

.. toctree::
   :hidden:

Overview
========
For the purpose of the set of optimizations it is required to know whether
a memory access is safe to be performed or not. Especially this makes sense
for the accesses beyond of originally *(in terms as it was written in
a program source code)* allocated memory.

This document provides an example of such optimizations, as well as a description
of how pointer annotations help to solve particular optimization problem,
and a description of infrastructure implemented in xmain compiler.


Vectorization of a search loop with unknown trip count
======================================================

Vectorization of a search loop with unknown trip count is an optimization which
benefits from reading out of bounds of originally allocated data.

The code example below gives an idea of programming pattern considering in this
example. The function foo() scans an input string and returns a pointer to the
first position which doesn't contain 'c' character

.. code-block:: c++

    char* foo(char* str)
        char* p = str;
        while (*p != 'c') {
            p++;
        }
        return p;
    }

Vectorization of such loop is problematic by the following reasons:
    - loop trip count is not known
    - vectorized code loads data by chunks equal to vector length
    - both of above combined may lead to the load of data out of allocated memory


In order to enable the vectorization of such loops compiler could
allocate data pointed by str with additional padding bytes following
right after original data and equals to the length of the vector.
Thus every vector read becomes safe, since it reads from allocated data.


Using pointer annotations
=========================

In order to keep tracking of pointers allocated with a padding and are safe for
out of bound accesses llvm.ptr.annotation intrinsic is used.

.. code-block:: llvm

    declare i8*   @llvm.ptr.annotation.p<address space>i8(i8* <val>, i8* <annotation>, i8* <file>, i32  <line>)

- Syntax and placement
    llvm.ptr.annotation call is inserted into def-use chain to mark that all uses
    dominated by this call access to padded memory through the pointer returned
    by llvm.ptr.annotation.p  and have padding as specified in second parameter of the call.
    Second parameter of llvm.ptr.annotation.p is a string in a form "padded <n> bytes"

- Semantics
    The annotation indicates that a value, is a pointer, and that memory pointed to
    by this pointer p is allocated so that if p + k points to some address within
    the memory allocated, then it is safe to load at least <n> bytes starting with
    this address. Here <n> is amount of bytes specified by the annotation.
    The value of padding bytes is undefined and should be not use for any purpose.

- Example
    In the following code example it is safe to access up to 16 bytes after any memory access
    referenced by '%8'. The same time the accesses referenced by %x are not safe to access any
    additional data.

    .. code-block:: llvm

        @1 = private unnamed_addr constant [16 x i8] c"padded 16 bytes\00"
        %8 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %x, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @1, i64 0, i64 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 13)

Passes
=====================
- **PaddedPtrPropPass** - Pass for new pass manager
- **PaddedPtrPropWrapper** - Pass for legacy pass manager

*The passes get a set of initially placed annotations as an input and propagate
padding information across whole program and as a result place additional
annotations on function parameters, returns and call results.*


Interface routines
==================
.. code-block:: c++

        // A routine to query if the Value points to a padded memory.
        // The routine performs an intra function analysis by traversing
        // def-use chains and calculating a padding for the Value based
        // on annotation available inside the function.
        int llvm::getPaddingForValue(Value *V);

        // A routine to insert padding markup for a value
        void llvm::insertPaddedMarkUp(Value *V, int Padding);

        // A routine for padding markup removal
        void llvm::removePaddedMarkUp(IntrinsicInst *I);

*The Value is supposed to have pointer type pointing to Integer or Float.*

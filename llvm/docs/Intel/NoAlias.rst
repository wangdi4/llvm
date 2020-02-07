==================================================
Explicit Memory Aliasing Representation in LLVM IR
==================================================

.. contents::
   :local:

Introduction
============

LLVM provides an ability to express information about aliasing between memory
instructions using several approches:

- Function and Parameter attribute - 'noalias'.

.. code-block:: llvm

    declare noalias i8* @bar()
    declare void @foo(float* noalias %p)

See :ref:`paramattrs`.

- Instruction level metadata.

There are two kinds of metadata for this purpose:

- !alias.scope - identifies a list of scopes to which the instruction belongs,
- !noalias - identifies a list of scopes which are independent to the current instruction.

Each scope specifies a domain where it is defined. Both scopes and domains are
represented by unnamed self-referenced metadata nodes:

.. code-block:: llvm

    !0 = !{!0}
    !1 = !{!1, !0}

where !0 is a domain and !1 is a scope in that domain.

To imply noalias for a pair of instructions, the set of scopes in !alias.scope
of one instruction should be a subset of scopes in !noalias of another
instruction (within one domain).

For example:

.. code-block:: llvm

    %0 = load float, float* %p, !alias.scope !4, !noalias !5
    store float %0, float* %q !alias.scope !5, !noalias !4

    ; Domain
    !1 = !{!1}

    ; Scopes
    !2 = !{!2, !1}
    !3 = !{!3, !1}

    ; Lists of scopes
    !4 = !{2}
    !5 = !{3}

The load and store instructions are independent because !alias.scope !4 is a
subset or !noalias !4.

See :ref:`noalias_metadata` for more details.

C language 'restrict' keyword
=============================

The "C" type system has its own way to tell the compiler that accessing objects
are distinct. The goal of the compiler front-end and backend is to preserve this
information for the optimizations.

See: https://en.cppreference.com/w/c/language/restrict

There are multiple cases where 'restrict' keyword may be used:
    - file scope,
    - function parameter,
    - block scope,
    - struct members

Currently, only the following cases are supported in LLVM:

Function parameter
------------------

The function restrict-qualified pointer parameters are translated into 'noalias'
parameter attributes.

For example,

.. code-block:: c

    void foo(float* restrict p)

Will be translated into

.. code-block:: llvm

    declare void @foo(float* noalias %p)

The semantic of noalias parameter attribute is intentionally similar to the
semantic of restrict keyword. Objects accessed via a pointer, based on the
noalias argument, are not also accessed via pointers NOT based on that argument.

Note that unlike C99's 'restrict' keyword the 'noalias' attribute may be
specified on the return values. Such attribute indicates that the function
returns a pointer to a completely distinct memory as if the memory had just
been allocated by the function call.

Translating noalias attribute into !alisa.scope and !noalias metadata
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

Information preserved in function arguments can not survive a function inlining,
therefore it's converted to a metadata representation in the call site. The
implementation of the process is done in AddAliasScopeMetadata() function of
`InlineFunction.cpp <https://llvm.org/doxygen/InlineFunction_8cpp.html>`_.

The new alias scope "!X" is created for each noalias argument then all derived
values are marked with "!alias.scope !X". The rest of non-derived loads, stores
and memory intrinsics are marked with "!noalias !X".

The essential part is to track if the noalias pointer escaped via a global value
or was captured by a function call, making it impossible to prove that other
values do not alias with the escaped one.

Block scope
-----------

*This section is xmain specific.*

A block scope restrict qualifier makes an assumption limited to the block where
it is declared.

.. code-block:: c

    void foo(float* p) {
        float *restrict rp = p;
        ...
    }

Only limited support is implemented for block scopes - the compiler assumes
independence between two objects only when both pointers are restrict-qualified.
This information is represented as !alias.scope and !noalias metadata. The
metadata is emitted early by the clang front-end.

Clang creates a new alias scope for each restrict-qualified pointer declaration
and tags every load, store and memory intrinsic which is derived from that
pointer with !alias.scope metadata and sets !noalias to every other alias scope
created in the currect block.

Block restrict pointers and function inlining
"""""""""""""""""""""""""""""""""""""""""""""

Consider the following function 'foo()' being inlined into 'bar()'.

.. code-block:: llvm

    define @foo(i32* %p, i32* %q) {
      %0 = load i32, i32* %q
      store i32 %0, i32* %p
      ret void
    }

.. code-block:: c

    void foo(int *p, int *q) {
      *rp = *rq;
    }

    void bar(int *p, int *q) {
      ...

      {
        int *restrict rp = p;
        int *restrict rq = q;

        foo(rp, rq); // assume inlining
      }
    }

The !alias.scope metadata needs to be propagated to the inlined load and store
instructions, but inlining is happening later in the backend. The call to
'foo()' is the only instruction where restrict pointers are being used. To
preserve !alias.scopes there is intermediate metadata that is only used by the
function inliner:

.. code-block:: llvm

    !intel.args.alias.scope

The metadata contains a list of scopes associated with the function arguments.
During inlining the scopes will be picked-up and used the same as if the
arguments were declared as 'noalias', but existing !alias.scope metadata will be
used instead of creating a new one.

Aggregate copy assignment
"""""""""""""""""""""""""

The 'C' standard allows clang to emit a memcpy call when translating copy
assignment for aggregate types. If both pointers in memcpy call are restrict
the call instruction is marked with both !alias.scope and !noalias.

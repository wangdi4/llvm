============================================
TBAA Extension: New Metadata Nodes for TBAA
============================================

.. contents::
   :local:

.. toctree::
   :hidden:

   ../../LangRef


Introduction
============

LLVM Type Based Alias Analysis (TBAA) is a metadata-based TBAA where metadata
is added to the IR to describe a type system of a higher level language. LLVM
TBAA supports three TBAA metadata nodes: ``Scalar`` type descriptors,
``Struct`` type descriptors and ``Access Tag`` nodes. See the `TBAA metadata
section <../../LangRef.html#tbaa-metadata>`_ of the LLVM language reference
manual for their detailed description.


Limitations of LLVM Type Based Alias Analysis
==============================================

LLVM Type Based Alias Analysis does not support array and pointer types.
Therefore, it can not represent the relationship between array types and array
element types. It can not even differentiate between loads and stores of
different pointer types. As an example, the following 'tbaa_pointer' function
involves two pointer types:

.. code-block:: c

    int* tbaa_pointer(int **p, float **q) {
        *p = *p +1;
        *q = *q +1;
        return *p;
    }

with the LLVM IR as:

.. code-block:: llvm

    %0 = load i32*, i32** %p, align 8, !tbaa !1
    %add.ptr = getelementptr inbounds i32, i32* %0, i64 1
    store i32* %add.ptr, i32** %p, align 8, !tbaa !1
    %1 = load float*, float** %q, align 8, !tbaa !1
    %add.ptr1 = getelementptr inbounds float, float* %1, i64 1
    store float* %add.ptr1, float** %q, align 8, !tbaa !1
    %2 = load i32*, i32** %p, align 8, !tbaa !1
    ret i32* %2

where TBAA metadata for LLVM would be:

.. code-block:: llvm

    !0 = !{!"clang version 3.8.0 (trunk 1684)"}
    !1 = !{!2, !2, i64 0}
    !2 = !{!"any pointer", !3, i64 0}
    !3 = !{!"omnipotent char", !4, i64 0}
    !4 = !{!"Simple C/C++ TBAA"}

Though ``*p`` and ``*q`` are two different pointer types and any aliasing
between them is prohibited in C/C++, both of their memory operations are
annotated with the same TBAA metadata !1. As LLVM TBAA does not keep the exact
type of the memory accesses, they are assumed to be aliased with each other.

For array element accesses, the type of the element is tracked, but not the
type of the containing array. If the following 'tbaa_array' function is
considered:

.. code-block:: c

    typedef int AA[3][3];
    typedef int BB[2][4];

    int tbaa_array(AA *a, BB *b) {
        (*a)[0][0] = 1;
        (*b)[0][0] = 2;

        return (*a)[0][0];
    }

then the LLVM IR and TBAA metadata will be:

.. code-block:: llvm

    %arrayidx1 = getelementptr inbounds [3 x [3 x i32]], [3 x [3 x i32]]* %a,
    i64 0, i64 0, i64 0
    store i32 1, i32* %arrayidx1, align 4, !tbaa !1
    %arrayidx3 = getelementptr inbounds [2 x [4 x i32]], [2 x [4 x i32]]* %b,
    i64 0, i64 0, i64 0
    store i32 2, i32* %arrayidx3, align 4, !tbaa !1
    %0 = load i32, i32* %arrayidx1, align 4, !tbaa !1
    ret i32 %0

    !0 = !{!"clang version 3.8.0 (trunk 1684)"}
    !1 = !{!2, !2, i64 0}
    !2 = !{!"int", !3, i64 0}
    !3 = !{!"omnipotent char", !4, i64 0}
    !4 = !{!"Simple C/C++ TBAA"}

According to C/C++ language rules, the memory operations ``(*a)[0][0]`` and
``(*b)[0][0]`` cannot alias with each other and yet annotated with the same
TBAA metadata !1 here.

In order to enable aggressive optimization effectively, the type based alias
analysis needs to have a unified TBAA framework that supports not only scalar
and structure types but also different array types and pointer types.


TBAA New Metadata Node Extension
=================================

The existing LLVM TBAA framework is extended by adding new metadata nodes.
This TBAA extension is a low overhead improvement to the LLVM TBAA framework.
It reuses the ``Scalar`` type nodes from the LLVM framework, hence it does not
require any modification to the TBAA metadata for the load and store
instructions. These new types allow memory references to have distinct
nodes in the type tree.

New metadata nodes supported by the extension are: ``Pointer`` and ``Array``
type nodes. See the LLVM reference manual for the
:ref:`Semantics<tbaa_node_semantics>` and
:ref:`Representation<tbaa_node_representation>` of these nodes. They help
distinguishing different array and pointer types which otherwise would alias
with each other. With the ``Pointer`` type node, the LLVM IR and TBAA tags for
the 'tbaa_pointer' function would be:

.. code-block:: llvm

   store float* %add.ptr1, float** %q, align 8, !tbaa !8
   %2 = load i32*, i32** %p, align 8, !tbaa !1

   !0 = !{!"clang version 3.8.0 (trunk 1684)"}
   !1 = !{!7, !7, 0}
   !2 = !{!"omnipotent char", !3, i64 0}
   !3 = !{!"Simple C/C++ TBAA"}
   !4 = !{!"int", !2, i64 0}
   !5 = !{!"pointer@float*", !2, i64 0}
   !6 = !{!"float", !2, i64 0}
   !7 = !{!"pointer@int*", !2, i64 0}
   !8 = !{!5, !5, 0}

Unlike the LLVM TBAA, two different TBAA metadata nodes !5 and !7 are
created for two different pointer types which ensures that they are
not aliased with each other.

For the 'tbaa_array' function, the LLVM IR with ``Array`` type nodes will be as
follows:

.. code-block:: llvm

   store i32 2, i32* %arrayidx3, align 4, !tbaa !7
   %0 = load i32, i32* %arrayidx1, align 4, !tbaa !1

   !0 = !{!"clang version 3.8.0 (trunk 1684)"}
   !1 = !{!6, !2, 0}
   !2 = !{!"int", !3, i64 0}
   !3 = !{!"omnipotent char", !4, i64 0}
   !4 = !{!"Simple C/C++ TBAA"}
   !5 = !{!"array@int[4]", !2, i64 0}
   !6 = !{!"array@int[2][4]", !5, i64 0}
   !7 = !{!9, !2, 0}
   !8 = !{!"array@int[3]", !2, i64 0}
   !9 = !{!"array@int[3][3]", !8, i64 0}

Here, TBAA metadata nodes !6 and !9 are generated for the two arrays
and they are not aliasing each other.


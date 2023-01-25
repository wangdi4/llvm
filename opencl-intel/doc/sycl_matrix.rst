.. ..

  <!---
  Copyright (C) 2022 Intel Corporation

  This software and the related documents are Intel copyrighted materials, and
  your use of them is governed by the express license under which they were
  provided to you ("License"). Unless the License provides otherwise, you may
  not use, modify, copy, publish, distribute, disclose or transmit this
  software or the related documents without Intel's prior written permission.

  This software and the related documents are provided as is, with no express
  or implied warranties, other than those that are expressly stated in the
  License.
  --->

SYCL Matrix Implementation Design
=================================

.. contents::
  :local:

SYCL Matrix Interface
*********************

The ``joint_matrix`` class is introduced by the `SYCL matrix extension <https://github.com/intel/llvm/blob/74369c84989b61d95798c528070212b8692024c0/sycl/doc/extensions/experimental/sycl_ext_oneapi_matrix.asciidoc>`_. *joint* emphasizes that the matrix is shared among a group of work items and is not private to each work item.

This design doc focuses on the implementation details of the following APIs on CPU:

* Matrix initialization: ``joint_matrix_fill``
* Element indexing and element-wise operations

Matrix initialization
*********************

SYCL interface: ``joint_matrix_fill``
-------------------------------------

``joint_matrix_fill`` (`spec <https://github.com/intel/llvm/blob/fill-piece-wise-ops/sycl/doc/extensions/experimental/sycl_ext_oneapi_matrix.asciidoc#matrix-initialization-joint_matrix_fill>`_) makes it possible to multiply a matrix which is not directly loaded from memory but rather initialized directly in the register.

.. code-block:: c++

    template <typename Group, typename T, size_t NumRows, size_t NumCols,
            matrix_layout Layout>
    void joint_matrix_fill(Group sg,
                    joint_matrix<T, NumRows, NumCols, Layout, Group> &res,
                    const T &v)

FrontEnd-MiddleEnd interface: ``llvm.experimental.matrix.fill``
---------------------------------------------------------------

The ``joint_matrix_fill`` call is translated into the ``llvm.experimental.matrix.fill`` intrinsic (`doc <http://iclqa.intel.com/BuildBoard/XmainDocs/LangRef.html#llvm-experimental-matrix-fill-intrinsic>`_) by llvm-spirv/SPIRVReader.

.. code-block::

  declare <vectorty> @llvm.experimental.matrix.fill.*(
      <type> %Input, i32 <Rows>, i32 <Cols>,
      metadata <Layout>, metadata <Scope>, metadata <Use>)

The first argument (the value to be filled with) of the intrinsic could be any type. And the matrix is represented by a vector type in LLVM IR.

Implementation: ``ResolveMatrixFill`` pass
------------------------------------------

``ResolveMatrixFill`` pass is designed to resolve all ``llvm.experimental.matrix.fill`` calls in the module, with the exception when the value to be filled with is natively supported by the hardware. Currently AMX hardware supports zero-filling natively via the tile zero instruction.

* The pass will first preprocess pointer-type ``%Input`` argument to loaded element type if needed. Note, if it's an opaque pointer, the load element type is determined by the matrix element type.

.. code-block:: llvm

  %mat = call @llvm.experimental.matrix.fill.v16i32.i32(ptr %Input, ...)

  ; -->

  %loaded = load i32, ptr %Input
  %mat = call @llvm.experimental.matrix.fill.v16i32.i32(i32 %loaded, ...)

* If ``%Input`` is a constant zero (integer or floating point), then ``ResolveMatrixFill`` won't process the intrinsic as X86 CodeGen will lower it to a native ``tile.zero`` instruction.

* Otherwise, the intrinsic will be transformed into a ``llvm.experimental.matrix.fill(0, ...)`` initialization and a loop assigning target value to each element of the matrix.

.. code-block:: llvm

  %mat = call @llvm.experimental.matrix.fill.v16i32.i32(i32 %Input, ...)
  call void @foo(<16 x i32> %mat)

  ; -->

  %mat.init = call @llvm.experimental.matrix.fill.v16i32.i32(i32 0, ...) ; will be lowered to @tile.zero by CodeGen

  ; A loop to perform element-wise assignment for %mat.init
  %slice.length = call i64 @llvm.experimental.matrix.wi.slice.length.v16i32(...)
  br label %loop.header

  loop.header:
    %index = phi i64 [ 0, %entry ], [ %index.inc, %loop.body ]
    %mat = phi <16 x i32> [ %mat.init, %entry ], [ %mat.update, %loop.body ]
    %cmp = icmp slt i64 %index, %slice.length
    br i1 %cmp, label %loop.body, label %loop.end

  loop.body:
    %mat.update = call <16 x i32> @llvm.experimental.matrix.wi.slice.insertelement.v16i32.i64(<16 x i32 %mat, ..., i32 %Input, i64 %index, ...)
    %index.inc = add nuw i64 %index, 1
    br label %loop.header

  loop.end:
    call void @foo(<16 x i32> %mat)

We're using another two matrix intrinsics ``llvm.experimental.matrix.wi.slice.length`` and ``llvm.experimental.matrix.wi.slice.insertelement`` for element-wise assignment implementation to fill values (which are not natively supported by the hardware) into the matrix. The semantic details will be explained in the `Element indexing and element-wise operations`_ section.

Element indexing and element-wise operations
********************************************

SYCL interface: ``get_wi_data``
-------------------------------

The extension introduces a new function ``get_wi_data`` (`spec <https://github.com/intel/llvm/blob/fill-piece-wise-ops/sycl/doc/extensions/experimental/sycl_ext_oneapi_matrix.asciidoc#explicit-conversion-with-mapping-from-simd-to-spmd>`_) that provides the portion of the matrix that is assigned to the WI. This returns a slice type that has two members: (1) element indexing into that slice, and (2) the length of this slice per WI.

.. code-block:: c++

  template <typename Group, typename T, size_t NumRows, size_t NumCols,
            matrix_layout L>
    wi_slice<T, NumRows, NumCols, Layout, Group> get_wi_data(joint_matrix<T, NumRows, NumCols, Layout, Group> &M);

Example that implements element wise multiplication:

.. code-block:: c++

  joint_matrix<T, rows, cols> C(sg);
  int length = C.get_wi_data().length()
  for(int i = 0; i < length; ++i)
    C.get_wi_data()[i] *= alpha;

FrontEnd-MiddleEnd interface
----------------------------

* ``wi_slice::length()`` is translated to ``llvm.experimental.matrix.wi.slice.length`` intrinsic (`doc <http://iclqa.intel.com/BuildBoard/XmainDocs/LangRef.html#llvm-experimental-matrix-wi-slice-length-intrinsic>`_), which returns the number of elements owned by the current work-item in a joint matrix.

* ``wi_slice::operator[]()`` is translated to ``llvm.experimental.matrix.wi.slice.extractelement`` (`doc <http://iclqa.intel.com/BuildBoard/XmainDocs/LangRef.html#llvm-experimental-matrix-wi-slice-extractelement-intrinsic>`_) and/or ``llvm.experimental.matrix.wi.slice.insertelement`` (`doc <http://iclqa.intel.com/BuildBoard/XmainDocs/LangRef.html#llvm-experimental-matrix-wi-slice-insertelement-intrinsic>`_) depending on the side of assignment. The intrinsic semantics is to extract/insert one scalar element from/to the slice owned by the current work-item.

Implementation: ``ResolveMatrixWISlice`` pass
---------------------------------------------

The ``ResolveMatrixWISlice`` pass is designed to resolve matrix WI slice intrinsics into internal builtins that can be further vectorized by VPlan vectorizer.

Term definitions
""""""""""""""""

A WI slice represents the collection of matrix elements owned by a work-item, and how each matrix element maps to WI slices is implementation-defined. What is returned by ``get_wi_data()`` is exactly a WI slice.

A row slice represents the collection of consecutive matrix elements (in row-major order) and is not associated to a specific work-item. Instead a row slice is owned by the whole group in our implementation (you can call it a joint row slice, just like a joint matrix).

Pass details
""""""""""""

* ``llvm.experimental.matrix.wi.slice.length`` will be transformed into the ``get_sub_group_slice_length`` internal builtin.

* ``llvm.experimental.matrix.wi.slice.extractelement`` will be transformed into two consecutive internal builtin calls: ``get_sub_group_rowslice_id`` and ``sub_group_rowslice_extractelement``.

  * The return value of ``get_sub_group_rowslice_id`` serves as a token representing the uniform joint row slice owned by the current group.
  * And ``sub_group_rowslice_extractelement`` performs extraction from a joint row slice.

.. code-block:: llvm

  %elem = call i32 @llvm.experimental.matrix.wi.slice.extractelement.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  ; -->
  %id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
  %elem = call i32 @sub_group_rowslice_extractelement.i32(i64 %id)

* ``llvm.experimental.matrix.wi.slice.insertelement`` will be transformed into 3 consecutive internal builtin calls:

  * ``get_sub_group_rowslice_id`` which returns the token of a row slice.
  * ``sub_group_rowslice_insertelement`` which semantically inserts an element into the row slice corresponding to the token. Note this builtin doesn't return anything.
  * ``sub_group_insert_rowslice_to_matrix`` which inserts a row slice as a whole, into the matrix and returns the updated matrix.

.. code-block:: llvm

  call <144 x i32> @llvm.experimental.matrix.wi.slice.insertelement.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i32 %val, i64 %element.index, metadata !"matrix.rowmajor", metadata !"scope.subgroup", metadata !"matrix.use.unnecessary")
  ; -->
  %id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
  call void @sub_group_rowslice_insertelement.i32(i64 %id, i32 %val)
  %mat.update = call <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64 %id)

Implementation: mapping matrix elements to work-items
-----------------------------------------------------

The elements are mapped to each work-item by spreading each subgroup (workgroup is not supported for now) across the matrix in row-major order.

e.g. Given a 4-by-5 subgroup-scoped matrix, and assuming the subgroup size is 4.

The matrix elements are mapped to each work-item:

.. code-block::

  --------------------------
  | W0 | W1 | W2 | W3 | W0 |
  | W1 | W2 | W3 | W0 | W1 |
  | W2 | W3 | W0 | W1 | W2 |
  | W3 | W0 | W1 | W2 | W3 |
  --------------------------

Work-item #0 owns the SLICE(WI0) = [Mat[0,0], Mat[0,4], Mat[1,3], Mat[2,2], Mat[3,1]]

The stride is equal to subgroup size if we view the matrix in row-major order. And the length of a workitem's slice is specified by the internal builtin: ``get_sub_group_slice_length()``.

In the example above, there're 5 row slices:

.. code-block::

  ROWSLICE(0) = [Mat[0,0], Mat[0,1], Mat[0,2], Mat[0,3]]
  ROWSLICE(1) = [Mat[0,4], Mat[1,0], Mat[1,1], Mat[1,2]]
  ROWSLICE(2) = [Mat[1,3], Mat[1,4], Mat[2,0], Mat[2,1]]
  ROWSLICE(3) = [Mat[2,2], Mat[2,3], Mat[2,4], Mat[3,0]]
  ROWSLICE(4) = [Mat[3,1], Mat[3,2], Mat[3,3], Mat[3,4]]

The length of a row slice, is equal to the subgroup size.

The element extraction/insertion operation will be performed at the row slice level after vectorization.

Implementation: attaching VectInfo for internal subgroup rowslice builtins
--------------------------------------------------------------------------

``SYCLKernelVecClone`` pass will generate VectInfo dynamically for ``sub_group_rowslice_extractelement`` and ``sub_group_rowslice_insertelement`` builtins, so that their argument/return values get vectorized.

.. code-block:: llvm

  %rowslice.id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
  %extract.elem = call i32 @sub_group_rowslice_extractelement.i32(i64 %rowslice.id)
  %val = mul i32 %extract.elem, 42
  %rowslice.id1 = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32 12, i32 12, i64 %element.index)
  call void @sub_group_rowslice_insertelement.i32(i64 %rowslice.id1, i32 %val)
  %mat.update = call <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64 %rowslice.id1)
  ret void

  ; --> (after vectorization)

  %0 = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %load.mat, i32 12, i32 12, i64 %load.element.index)
  %1 = call <16 x i32> @_ZGVbN16u_sub_group_rowslice_extractelement.i32(i64 %0)
  %2 = mul <16 x i32> %1, <i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42>
  %3 = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %load.mat, i32 12, i32 12, i64 %load.element.index)
  call void @_ZGVbN16uv_sub_group_rowslice_insertelement.i32(i64 %3, <16 x i32> %2)
  %4 = call <144 x i32> @sub_group_insert_rowslice_to_matrix.v144i32(i64 %3)

After vectorization, the semantics of widened calls:

* ``call <16 x i32> @_ZGVbN16u_sub_group_rowslice_extractelement.i32(i64 %0)`` extracts the whole row slice corresponding to the token `%0`.
* ``call void @_ZGVbN16uv_sub_group_rowslice_insertelement.i32(i64 %3, <16 x i32> %2)`` inserts widened data `%2` to the row slice corresponding to the token `%3`.

Implementation: ``ResolveSubGroupWICall`` pass
----------------------------------------------

The last step is to resolve these internal slice builtins after vectorization, which is done by ``ResolveSubGroupWICall`` pass.

MiddleEnd-BackEnd interface
"""""""""""""""""""""""""""

We need to operate matrix elements at row slice level after vectorization, and CodeGen is capable of resolving the following intrinsics, which are the outputs of ``ResolveSubGroupWICall`` resolution:

* ``llvm.experimental.matrix.extract.row.slice`` (`doc <http://iclqa.intel.com/BuildBoard/XmainDocs/LangRef.html#llvm-experimental-matrix-extract-row-slice-intrinsic>`_) extracts consecutive values (in row-major order) from the matrix. A collection of the row-major consecutive values is called a row slice of the matrix.

* ``llvm.experimental.matrix.insert.row.slice`` (`doc <http://iclqa.intel.com/BuildBoard/XmainDocs/LangRef.html#llvm-experimental-matrix-insert-row-slice-intrinsic>`_) inserts data into consecutive positions (in row-major order) of the matrix.

Resolving internal subgroup slice builtins
""""""""""""""""""""""""""""""""""""""""""

* ``i64 @get_sub_group_slice_length.(i32 immarg %total.element.count)`` will be resolved as a constant == ``ceil(%total.element.count / get_max_sub_group_size())``

* ``<N x i32> sub_group_rowslice_extractelement(i64 %rowslice.id)`` together with the corresponding ``i64 @get_sub_group_rowslice_id(<R*C x i32> %mat, i32 R, i32 C, i64 %index)`` call will be resolved as a ``llvm.experimental.matrix.extract.row.slice`` intrinsic call with proper row slice offset.

* ``<R*C x i32> @sub_group_insert_rowslice_to_matrix(i64 %rowslice.id)`` together with the corresponding ``get_sub_group_rowslice_id`` and ``void @sub_group_rowslice_insertelement(i64 %rowslice.id, <N x i32> %data)`` will be resolved as a ``llvm.experimental.matrix.insert.row.slice`` intrinsic call with proper row slice offset.

TODOs
*****

Nonspirv support
----------------

Since llvm matrix instrinsics (e.g. ``llvm.experimental.matrix.fill``) are generated by llvm-spirv/SPIRVReader at the moment, the CPU matrix passes won't work for the nonspirv CPU path directly.

We need to move the translation from llvm-spirv/SPIRVReader to llvm-spirv/SPIRVToLLVM so that matrix intrinsics can be generated as part of SPIRV-friendly-IR.

Workgroup support
-----------------

The workgroup scoped joint matrix is not supported yet.

Irregular matrix
----------------

There might be potential bugs in the current implementation when the matrix size is not divisible by the subgroup size.

Mapping info query
------------------

Since how matrix elements are mapped to each work-item is implementation-defined, we may need to add extra runtime query APIs to provide the mapping info.

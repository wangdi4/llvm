..
  Copyright (C) 2022 Intel Corporation

  This software and the related documents are Intel copyrighted materials, and
  your use of them is governed by the express license under which they were
  provided to you ("License"). Unless the License provides otherwise, you may
  not use, modify, copy, publish, distribute, disclose or transmit this
  software or the related documents without Intel's prior written permission.

  This software and the related documents are provided as is, with no express
  or implied warranties, other than those that are expressly stated in the
  License.

===================
OpenCL Metadata API
===================

.. contents::

   :local:

Introduction
============

The OpenCL Metadata API is a framework for working with OpenCL-specific
Metadata, designed to be used in an OpenCL backend by a developer. It provides
necessary building blocks for working with Metadata as well as some pre-defined
APIs for querying/recording CPU specific Metadata, including Stats feature for
Compiler/Vectorizer decision logging (aka CL_CONFIG_DUMP_IR_AFTER_OPTIMIZER).

Metadata example
----------------

The example shows the Metadata layout for kernel attributes.
The following source

.. code-block:: c++

  __attribute__((vec_type_hint(float)))
  __attribute__((work_group_size_hint(8,16,32)))
  __attribute__((reqd_work_group_size(1,2,4)))
  __attribute__((intel_reqd_sub_group_size(1)))

  __kernel void metatest_kernel(float argFloat,
                                __global int * argIntBuffer,
                                __read_only image2d_t argImg) {
    return;
  }

 will turn into using clang 4.0:

.. code-block:: llvm

  define spir_kernel void @metatest_kernel(
    float %argFloat,
    i32 addrspace(1)* %argIntBuffer,
    %opencl.image2d_ro_t addrspace(1)* %argImg)
      !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6
      !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_name !8
      !vec_type_hint !9 {
  entry:
    ret void
  }

  !4 = !{i32 0, i32 1, i32 1}
  !5 = !{!"none", !"none", !"read_only"}
  !6 = !{!"float", !"int*", !"image2d_t"}
  !7 = !{!"", !"", !""}
  !8 = !{!"argFloat", !"argIntBuffer", !"argImg"}
  !9 = !{float undef, i32 0}

The Backend internal Metadata we use is represented in the same way,
it's either list or a single value attached to a llvm::GlobalObject
(llvm::Function or llvm::GlobalVariable) or llvm::Module.

API description
===============

The API is a header-only library consisting of 3 headers.

2 of them are intended to be used by a backend developer:

* *MetadataAPI.h*
  contains all definitions necessary to access OpenCL-specific Metadata.
  All new Metadata attributes should go here.
  This header is used in the production environment and is heavily optimized
  for working with attributes that are known in compile-time
  (that includes everything covered in specifications/extensions as well as
  internal attributes, like whether kernel has a barrier,
  what vectorization width was chosen, etc.).

* *MetadataStatsAPI.h*

  contains functionality for writing dynamic Metadata issued by *Statistic*
  interface (aka CL_CONFIG_DUMP_IR_AFTER_OPTIMIZER) to be consumed by SumStats
  utility.
  From Metadata API point of view the difference is these attributes
  are dynamic, i.e. not resolved in compile time and basically is
  a stream of structs like {Name, value}. This header is not used in production
  environment an is *nop* in *Release* builds (nothing is written).
  This header is not expected to be modified for purpose
  of adding new Stat type as all the Stat objects are defined in-place
  where they are needed.

1 header contains implementation detail:

* *MetadataAPIImpl.h*

  building blocks necessary to abstract away the annoying boilerplate
  of working with LLVM Module/GlobalObject Metadata.
  This header is expected to be touched when the abstractions
  provided in the external headers are not sufficient to reflect some
  new ways of working with Metadata, not even imaginable at the design time.

We will begin with the API provided by *MetadataAPI.h*.

For convenience all the attributes are grouped by their semantics.

KernelList
----------

The corner-stone entity in the Metadata API. Many decisions are made
based on the info it provides.

It is usually used with for-range loops:

.. code-block:: c++

  for (const auto *pFunc : KernelList(pModule)) {
    ...
  }

GlobalVariableMetadataAPI
-------------------------

Reserved for accessing channel Metadata, after the clang will emit channel
Metadata in 4.0 style.

Look for channels.cl in CodeGenOpenCL tests.

FunctionMetadataAPI
-------------------

Provides access to Metadata that makes sense for any function, regardless
of whether it is a kernel or not. Examples are flags for using function
pointers or having recursive calls.

KernelMetadataAPI
-----------------

Covers attributes that can be emitted by clang in the context of kernels.
Includes all attributes described in the OpenCL specification and supported
extensions.

Whenever we want to introduce a new OpenCL kernel attribute it should go here.

ModuleMetadataAPI
-----------------

Accesses Module-level Metadata emitted by clang (OpenCL version, compiler
options, used extensions, optional core features).

KernelInternalMetadataAPI
-------------------------

This is the place to go for any internal to Backend attributes that can be
applied to kernels. Usual flow for an Analysis to iterate over KernelList
and store the info like kernels having barriers, vectorization width,
reference to a vectorized/scalarized and wrapper functions.

ModuleInternalMetadataAPI
-------------------------

Accesses Module-level information internal to Backend, like the size of global
variables, etc.

ModuleStatMetadataAPI
---------------------

Provides methods for accessing Module named metadata intended to store
module-level Statistics.

FunctionStatMetadataAPI
-----------------------

Provides methods for accessing metadata intended to store function Statistics.

Limitations of current design
=============================

The API is fit the best for the homogenous Metadata lists, most
of the attributes in existence can be described as homogenous with regard
to the underlying type.

For example, it can be either be all llvm::MDString:

.. code-block:: llvm

  !6 = !{!"float", !"int*", !"image2d_t"}

Or all some of the llvm::Constant descendants, like llvm::ConstantInt
or llvm::Function

(llvm::Type is handled via llvm::Constant type):

.. code-block:: llvm
  !10 = !{i32 8, i32 16, i32 32}
  !13 = !{void (float, i32 addrspace(1)*, %opencl.image2d_ro_t addrspace(1)*)* @metatest_kernel}

The homogenous Metadata is handled like vector data structure is usually handled.

The only known exception is vec_type_hint attribute.

.. code-block:: llvm

  define spir_kernel void @metatest_kernel() !vec_type_hint !9 {
    ret void
  }

  !9 = !{float undef, i32 0}

The underlying data structure for heterogenous Metadata is tuple.

The limitation is that heterogenous Metadata is not the first class citizen,
so there's no generic abstraction implemented that would cover all possible
combinations of types in Metadata list of any length. The existing solution
is generic enough to give basic blocks to construct a tuple for any given known
length, but this length is not parameterized. Currently a std::tuple-like
structure for only 2 elements is defined. Refer to vec_type_hint implementation
for detail.

Introducing new attributes
==========================

Whenever new attribute is introduced:

1. It needs to be decided which group the attribute is best to belong to.

2. Decide whether is fits the homogenous abstractions in the API.

If yes, follow the other attributes, make decisions about base type, whether
the attribute is a Module-level or Function-level, and declare the
new attribute.

For example, I want to add internal "opencl.all_mighty_attribute" attribute,
that would store a list of i32 values attached to a function.
So I would modify KernelInternalMetadataAPI struct by adding 3 lines of code
(a typedef, class member and new element to initializer list):

.. code-block:: c++

  // internal attributes
  struct KernelInternalMetadataAPI {
    typedef NamedMDList<int32_t, MDValueGlobalObjectStrategy> AllMightyAttributeTy;

    KernelInternalMetadataAPI(llvm::Function *Func)
        : AllMightyAttribute(Func, "opencl.all_mighty_attribute") { }

    AllMightyAttributeTy AllMightyAttribute;
  };

Done! Enjoy your new attribute.

In case when your attribute is heterogenous in nature you need to implement
a std::tuple like structure for any new number of types.

Currently only hetero-list of 2 elements is implemented. This can be templated
to any 2 uderlying types. If your heterogenous list is more than 2 you have two
options.

The first one is to try to implement a generic solution for any N (variadic
templates, maybe?). The second is to take an easy path and implement a similar
structure for your N.

Implementation detail
=====================

For non-Stat attributes the implementation is highly optimized to resolve most
of the behaviour statically by employing static polymorphism for Metadata types
on templates.

Each time a developer introduces an attribute that contains specific type,
C++ template magic make it to generate efficient code that makes bare minimum
of dyn_casts.

MetadataAPIImpl.h
-----------------

MDValueTraits
^^^^^^^^^^^^^

Provides low-level functionality to generate/md::extract Values from
llvm::Metadata nodes. Generic implementation is expected to work with
descendants of llvm::Metadata. Specializations are provided for bool,
int32_t, std::string, llvm::StringRef, llvm::Type, llvm::Function.

MetaDataIterator
^^^^^^^^^^^^^^^^

Allows iteration over the Metadata list with loading of the values.

MDValue
^^^^^^^

Represents one piece of Metadata with plain value inside.

MDValueGlobalObjectStrategy & MDValueModuleStrategy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Allows to abstract away form the inconsistency between the llvm::GlobalObject
and llvm::Module Metadata API coming from LLVM.

NamedMDValue
^^^^^^^^^^^^

One step up from the MDValue, now you can assign a name to MDValue and
actually attach it to  llvm::GlobalObject and llvm::Module.

NamedMDList
^^^^^^^^^^^

A list of values stored with a name. Operates via MetaDataIterator.

NamedHeteroTupleMDList
^^^^^^^^^^^^^^^^^^^^^^

An attempt to write semantically the same thing as NamedMDList, but with
heterogenous elements inside. Apparently my will is not strong enough to finish
this. So this works for 2 elements. These structures are pretty rare in
day-to-day life of CPU Backend.

VecTypeHintTupleMDListAccessor / WorkgroupSizeMDAccessor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Assigns meaningful names with regard to vec_type_hint and work_group_size_hint.
No meaning except bringing niceness.

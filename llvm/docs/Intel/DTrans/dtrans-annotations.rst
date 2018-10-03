=================================================
DTrans IR Metadata and Annotation intrinsic usage
=================================================

.. contents::
   :local:

.. toctree::
   :hidden:


Overview
========
This document describes the metadata and pointer annotations
produced/consumed by the DTrans Analysis and Transformation
passes.

DTransAnnotator class
=====================

A utility class, DTransAnnotator, is available for creating annotations to
mark specific items of interest needed for DTrans Analysis or to communicate
information between separate DTrans transformation classes. This utility class
is not instantiated but contains a set of methods for creating and querying the
information about the annotations. This utility class is intended to avoid a
tight coupling between specific DTrans passes needing to directly interfaces
with each other. This will also allow a DTrans transformation pass to be run
immediately following the DTrans passes that will recognize and remove the
metadata and annotation intrinsic calls that are no longer necessary.

One of the considerations between whether an annotation is done using metadata
or an intrinsic call is the need to keep the annotation up-to-date when the DTrans
base class or other transformation changes the name of a type. The base class
currently has no knowledge of specific annotations, and is unlikely to have this
knowledge in the future, Similarly, DTrans transformations should be able to
be run in a different order without breaking annotations inserted by another
DTrans transformation.

Metadata
========

Description of tags
-------------------
!dtrans-type
~~~~~~~~~~~~
This metadata tag can be placed on an instruction to provide a hint to the
DTrans local pointer analyzer regarding a pointer type that should be
associated with the result of the instruction. This is only a hint to
DTransAnalysis, in that it will add the specified type contained within the
metadata to the list of types produced by an instruction, but it will not bypass
the analysis of the instruction arguments normally done to identify types.
This can be useful for a case where the pointer type is an i8*, but is a known
type created by one of the DTrans Transformations that is not directly
analyzable due to pointer arithmetic.

This is currently limited to allowing a single type per instruction.

The following helper function can be used to create this metadata annotation.

.. code-block:: c++

  // Metadata will be added to the instruction.
  // The Type parameter must be a pointer type.
  void DTransAnnotator::createDTransTypeAnnotation(Instruction *I, llvm::Type *Ty)


The format of the generated metadata is simply a null value for the pointer type.
This enables the type information in the metadata to stay up-to-date when
a DTrans transformation pass remaps the type to a new type.

.. code-block:: llvm

  !0 = !{%__DTT_struct.test01a** null}


The following helper function can be used to check for type associated with an
instruction.

.. code-block:: c++

  // Get the type that exists in an annotation, if one exists, for the
  // instruction.
  llvm::Type *DTransAnnotator::lookupDTransTypeAnnotation(Instruction *I);


Pointer Annotations
===================
Pointer annotations may be used to communicate information from one
DTrans pass to another DTrans pass.

Creating annotations
--------------------

There are two steps required by the DTransAnnotator helper class for creating
a pointer annotation.

1. Get the global variable that will hold the string constant to be used for
the annotation.

.. code-block:: c++

  GlobalVariable &DTransAnnotator::getAnnotationVariable(Module &M,
                                                         DTransAnnotator::DPA_AnnotKind Type,
                                                         StringRef AnnotString,
                                                         const Twine &Extension = "")

This function takes an enumeration value defined within the DTransAnnotator class that
will control the naming of the global variable, a string to hold the content of the annotation
and an optional extension to append to the default global variable name used for this type
of annotation.

2. Create a llvm.ptr.annotation intrinsic instruction.

.. code-block:: c++

  Value *DTransAnnotator::createPtrAnnotation(Module &M,
                                              Value &Ptr,
                                              GlobalVariable &AnnotVar,
                                              StringRef FileName,
                                              unsigned Line,
                                              const Twine &NameStr = "",
                                              Instruction *InsertBefore = nullptr)

This function will create a llvm.ptr.annotation intrinsic call, using 'Ptr' as the pointer
to be annotated, and 'AnnotVar' , and optionally insert it
into the IR.

Querying annotations
--------------------

Check whether a Value is a DTrans annotation intrinsic.

.. code-block:: llvm

  bool DTransAnnotator::isDTransPtrAnnotation(Value *V)


Check whether a Value is a DTrans annotation intrinsic of a specific kind.

.. code-block:: llvm

  bool DTransAnnotator::isDTransAnnotationOfType(Value *V,
                                                 DPA_AnnotKind DPAType)

Get the DTrans annotation kind for an annotation intrinsic.

.. code-block:: llvm

  DTransAnnotator::DPA_AnnotKind DTransAnnotator::getDTransPtrAnnotationKind(Value *V);

Description of annotations
--------------------------

[DTransAnnotator::DPA_AOSToSOAIndexField] AOS-to-SOA Peeling Index
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This annotation marks a pointer as being the address of a variable
that has been converted into a peeling index by the AOS-to-SAO transformation.

For example, if %struct.test01 is transformed, then pointers to this type will
be converted to an integer index as follows:

.. code-block:: llvm

  %struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test01dep*, i8* }
  %struct.test01dep = type { i16, %struct.test01*, %struct.test01* }

to

.. code-block:: llvm

  %__SOA_struct.test01 = type { i32*, i32*, i32*, i32*, %__SOADT_struct.test01dep**, i8** }
  %__SOADT_struct.test01dep = type { i16, i32, i32 }


In this case, a reference to load the first field of %struct.test01dep such as the following:

.. code-block:: llvm

  %field1_addr = getelementptr inbounds %struct.test01dep, %struct.test01dep* %struct_addr, i64 0, i32 1
  %field1_val = load %struct.test01*, %struct.test01** %field1_addr

would get transformed and annotated as:

.. code-block:: llvm

  %field1_addr = getelementptr inbounds %__SOADT_struct.test01dep,
                               %__SOADT_struct.test01dep* %struct_addr, i64 0, i32 1
  %1 = call i32* @llvm.ptr.annotation.p0i32(i32* %field1_addr,
                  i8* getelementptr ([26 x i8], [26 x i8]* @__DTransAOSToSOAIndexFieldName, i32 0, i32 0),
                  i8* getelementptr ([13 x i8], [13 x i8]* @0, i32 0, i32 0), i32 0)
  %field1_val = load i32, i32* %field1_addr

In this case, the annotation needs to be placed on a pointer within the IR
rather than in a metadata description to avoid the need for all the DTrans
passes to keep the information up to date. For example, the fields of a
structure could be reordered after the AOS-to-SOA transformation, without
the field reordering pass needing to do anything special regarding the
annotation.

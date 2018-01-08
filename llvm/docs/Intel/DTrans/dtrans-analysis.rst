==================================
DTrans Infrastructure and Analysis
==================================

.. contents::
   :local:

.. toctree::
   :hidden:


Overview
========
This document describes the design of data transformation (DTrans)
infrastructure and analysis. DTrans optimizations are a group of
module-level transformations that attempt to improve the performance of
generated code by optimizing the memory layout of data structures used by the
program. Individual optimizations will be described briefly below in the
`DTrans Optimizations`_ section, along with the steps that must be taken to
prove the legality of each optimization.

This document will evolve as DTrans analysis and optimizations are implemented.


High Level Design
=================
The DTrans implementation consists of three major units: the DTransAnalysis
pass, a DTransAnalysisInfo object, and a number of DTrans optimization passes.
Each optimization pass will have a dependency on the analysis pass and will
obtain access to the DTransAnalysisInfo by calling the AnalysisManager's
getResult() template method for the DTransAnalysis.

The DTransAnalysis pass will examine global variables and each instruction
in the module to identify all uses of aggregate types in the program. The
analysis will include identification of potential safety issues (defined below)
for each aggregate type, as well as information such as field access frequency
that may be used to estimate whether or not a particular optimization will be
profitable.

Optimization passes will be able to query the DTransAnalysisInfo object to
retrieve this information, but as optimizations are performed the optimization
passes may also update the information to avoid invalidating the analysis.

The DTransAnalysis pass will be conservative in its determination of
optimization legality constraints. The analysis will attempt to identify
expected patterns in the IR that are understood to be safe. Any type usage
which cannot be specifically identified as safe will be marked with an
appropriate safety condition.

The DTrans optimization passes, if enabled, will be run early in the LTO phase.
These passes also require certain basic conditions to be met, such as the
whole program having been seen (that is, no unknown external calls) and the
program being free of exception handling code. The DTrans analysis pass will
verify these conditions and cause the optimization passes to exit without
attempting any optimizations if the conditions were not met.

Because the DTrans optimization passes make global changes to the program, they
must be implemented as module passes. The DTransAnalysis is expected to be an
expensive and easily invalidated analysis, so all optimization passes that
depend on it should be kept together as closely as possible.


Type Identification
===================
The DTrans identification of aggregate types will be based on LLVM's type
system. This may not always correspond exactly to the types used in the source
program, but it will provide a semantically correct representation of all types
as they are used in the LLVM IR and will therefore provide the necessary
foundation for DTrans optimizations.

The type of all values in LLVM IR can be queried directly using the getType()
method. When an instruction is being processed by the DTransAnalysis pass,
its result value and each operand will be examined to determine if they have
a type of interest (generally, an aggregate type or a pointer to an aggregate
type).

When a program allocates memory, the DTransAnalysis pass will follow the
uses of the returned pointer to determine the type of memory being allocated.
Typically allocated memory is returned as an i8* value (because LLVM uses i8*
to represent void pointers) and then bitcast as a pointer to the actual type.

See `Call`_ handling information for details.


Field Access Tracking
=====================
In LLVM IR, fields of an aggregate type are always accessed using the
getelementptr instruction, which returns a pointer to the field that is then
passed to a load or store instruction to read or write the value of the field.

In many cases, field access is obscured by using an i8* representation of the
aggregate pointer and a pre-computed offset to access a field. Therefore,
complex handling of getelementptr instructions will be required. This is
described in more detail below.

See `GetElementPtr`_ handling information for details.


Optimization Legality Checks
============================
The DTransAnalysisInfo object will contain a SafetyData field for each type
that has been analyzed. This SafetyData field will contain a set of all
conditions that have been observed that might restrict the legality of a
DTrans optimization.

Each DTrans optimization will have its own set of conditions which, if seen,
would make the optimization illegal. When the optimization pass is run, it
will check the safety data for each candidate type against its particular
set of constraints to determine whether or not the optimization can be
performed on that type.

Individual safety conditions are documented in the Low Level Details section
below. The set of conditions that must be checked for each optimization
will be documented in the description of the optimization below (`DTrans
Optimizations`_).

Refer to the `Safety Conditions`_ section for more information about the
individual legality checks.


DTrans Optimizations
====================
DTrans optimizations will be described here as they are implemented, and links
will be provided to separate documents providing more detailed documentation
of each optimization.


Low Level Details
=================

Safety Conditions
-----------------

BadCasting
~~~~~~~~~~
This indicates that a cast was seen that may make this type a bad candidate for
optimization. This flag covers multiple casting problems, including casting of
pointers from one type to another and casting of pointers to fields within a
structure to other types.

BadPtrManipulation
~~~~~~~~~~~~~~~~~~
This indicates that a pointer to an aggregate type was manipulated to compute
an address that is not the address of a field within the type or otherwise
couldn't be understood. It means that the program has an arbitrary pointer to
memory within an allocated buffer that DTrans will not be able to reliably
recreate if the allocation is transformed.

AmbiguousGEP
~~~~~~~~~~~~
This indicates that an i8* value that is known to alias to multiple types is
passed to a GetElementPtr instruction. The GetElementPointer instruction can
be used to compute an offset from an i8* base address, but if the i8* may
refer to multiple different aggregate types (for instance, because of a select
or PHI node) DTrans cannot reliably determine the element that is being
referenced.

VolatileData
~~~~~~~~~~~~
This indicates that one or more instructions acting on data of this type were
marked as being volatile.

MismatchedElementAccess
~~~~~~~~~~~~~~~~~~~~~~~
This indicates that a pointer to an element within the type was passed to a
load or store instruction but the type of the value loaded or stored did not
match the type of the element being pointed to. For instance, if a value that
is known to point to an i64 element is bitcast to an i32* and that i32* is
passed to a load instruction, the loaded type would not match the expected
element type.

AmbiguousPointerLoad
~~~~~~~~~~~~~~~~~~~~
This indicates that a load instruction was seen with a pointer operand (the
address of the value being loaded) which was known to alias incompatible
pointer types. For instance, if a %struct.A** value and a %struct.B** value
are both bitcast to i64* and then joined by either a PHI node or a select
and the joined value is passed to a load instruction, there would be no way
to determine the actual type of the loaded pointer.

WholeStructureReference
~~~~~~~~~~~~~~~~~~~~~~~
This indicates that an instruction was seen which references a non-pointer
instance of a structure type. This is unlikely but legal in LLVM IR. Such a
value can be returned by a load instruction when a pointer to a structure is
used as the pointer operand. A value loaded this way can also be passed to a
store instruction, used as an argument to a function call, and merged through
a select or PHI node.

UnsafePointerStore
~~~~~~~~~~~~~~~~~~
This indicates that a pointer that is known to alias to the type for which this
condition was set was stored to a memory location using a pointer operand that
was not otherwise known to alias to a pointer to pointer to the type.

FieldAddressTaken
~~~~~~~~~~~~~~~~~
This indicates that the addresses of one or more fields within the type were
either written to memory or passed to a function call.

UnhandledUse
~~~~~~~~~~~~
This is a catch-all flag that will be used to mark any usage pattern that we
don't specifically recognize. The use might actually be safe or unsafe, but we
will conservatively assume it is unsafe.


Local Pointer Type Analysis
---------------------------
In order to sufficiently analyze the use of a pointer value, it is often
necessary to have additional information about the pointer beyond what is
available by directly examining the value's LLVM type. Specifically, we need to
track whether a given pointer is known to point to other types and we need to
track whether a given pointer points to a field of an aggregate type.

This local analysis is not intended as a complete and robust alias analysis.
Rather, it offers a way to track local type transitions without having to trace
back the use-def chain every time the information is required.

One example where this is needed is when analyzing a bitcast where an i8* value
is cast as a pointer to an aggregate type. This is a common occurance in LLVM.
However, if we can not specifically prove that the source pointer is a known
pointer to the destination type, we must treat this as an unsafe cast (for the
purposes of DTrans optimizations). Consider the following IR:

.. code-block:: llvm

  define %struct.S* @f(%struct.S* %p) {
  entry:
    %origS = bitcast %struct.S* %p to i8*
    %isNull = icmp eq %struct.S* %p, null
    br i1 %flag, label %new, label %end

  new:
    %newS = call i8* @malloc(i64 16)
    br label %end

  end:
    %tmp = phi i8* [%origS, %entry], [%newS, %new]
    %ret = bitcast i8* tmp to %struct.S*
    ret %ret
  }

In this example, the bitcast at the %ret value is safe because the input value
is either the result of casting a %struct.S point to an i8* or the result of
a safe allocation (assuming 16 is a multiple of the size of %struct.S). Rather
than include all of the logic necessary to prove this relationship in the
bitcast analysis handling, we use a helper class (LocalPointerAnalyzer) that
maintains a map of previously analyzed values to an object with the necessary
information about the values (LocalPointerInfo).

When the LocalPointerAnalyzer object is asked for the LocalPointerInfo for
a value, it will first check its map and if the information is available and
indicates that previous analysis was complete, it will just return the existing
information. If there was no previous entry for the value in the map, a default
LocalPointerInfo object will be constructed with a flag indicating analysis
has not been performed. In this case, the LocalPointerAnalyzer object will
follow the use-chain from the value to its local source (either a function
argument, a global value or a local instruction that defined the pointer) and
populate the LocalPointerInfo to be returned.


Instruction Handling
--------------------
(Note: The following sub-sections describe the way each kind of instruction must
be handled to analyze its impact on any aggregate types. This is a work in
progress. As the analysis is implemented this section will be updated to
describe what the analysis is doing.)

Call
~~~~
When a call instruction is encountered, the DTrans analysis will attempt to
determine whether or not the call is allocating memory. Currently this is
done by using LLVM's LibFunc mechanism to check for calls to malloc, calloc,
and realloc. At some point this mechanism will be extended to handle additional
functions, including user-defined allocation functions.

If the call is an allocation function, we look for uses that bitcast the
returned value to a pointer to an aggregate type to determine the type of the
object that was allocated. If the returned value is bitcast to multiple
types, the BadCasting safety condition is set for each of these types.

A typical allocation will look like this:

.. code-block:: llvm

  %p = call i8* @malloc(i64 8)
  %s1 = bitcast i8* %p to %struct.S1*

Other uses of the value returned by an allocation call must also be analyzed
for safety conditions as this value is effectively an i8* alias of a pointer to
the aggregate object. **This tracking is not fully implemented.**

The arguments passed to allocation calls are examined and compared against the
size of the aggregate type being allocated (as reported by the DataLayout
class) in an attempt to verify that the size of the allocated memory is an
exact multiple of the aggregate type size. **Currently, only constant arguments
are handled.**

**The current implementation sets the UnhandledUse safety data for any call
that returns an aggregate type value (or a pointer to an aggregate type) or has
an argument that is an aggregate type (or a pointer to an aggregate type).
This is probably unnecessary, but it is done as part of the conservative
approach to progressive implementation.**


GetElementPtr
~~~~~~~~~~~~~
The :doc:`GetElementPtr <../../GetElementPtr>` instruction is used to obtain
the address of a field within an aggregate structure. The GetElementPtr
instruction itself does not access memory or dereference any pointers. However,
all access to the fields of a structure or elements of an array (except the
first element/field) is done through pointers returned from GetElementPtr. A
typical usage looks like this:

.. code-block:: llvm

  %ps.fb = getelementptr %struct.S, %struct.S* %ps, i64 0, i32 1
  %b = load i32, i32* %ps.fb

In this example %ps is a pointer to some structure S. The first index
argument (i64 0) indicates that we want to access a field in the structure
pointed to directly by %ps (as opposed to some offset from that pointer). The
second index argument (i32 1) indicates that we want the address of the second
field. Finally, a load instruction is necessary to actually read from the
field at that address.

The DTrans analysis must follow all uses of any value returned by a
GetElementPtr instruction in order to track field access and to determine
whether or not the pointer is manipulated in any way that might make a DTrans
optimization unsafe.

In addition to the simple form of GetElementPtr shown above, LLVM will sometimes
use GetElementPtr on an i8* alias of an aggregate type pointer to index directly
into a memory block using the precomputed offset of a field. For instance, if
some structure S contains two i32 fields, the IR to allocate an instance of the
structure and write a value to the second field might look like this:

.. code-block:: llvm

  %p = call i8* @malloc(i64 8)
  %tmp = getelementptr i8, i8* %p, i64 4
  %pb = bitcast i8* %tmp to i32*
  store i32 %val, i32* %pb
  %ps = bitcast i8* %p to %struct.S*
  ...

The DTransAnalysis pass must determine that %p is an alias of a pointer to
an aggregate type (based on the subsequent bitcast) and use the type
information and DataLayout to determine which field is being accessed in this
way and track the field usage.

Finally, a GetElementPtr instruction can be used to compute an offset from
a pointer to treat the pointer as a dynamic array. This occurs when a GEP
instruction is used with a pointer type other than i8* and a single index
operand. A typical use will look like this:

.. code-block:: llvm

  %size = mul i64 %numElems, <sizeof struct.S>
  %p = call i8* @malloc(i64 %size)
  %p_S bitcast i8* %p to %struct.S*
  %p_S_3 = getelementptr %struct.S, %struct.S* %p, i64 3
  ...

The index argument in this case can be a non-constant value. The DTransAnalysis
will recognize this as a safe use.


Bitcast
~~~~~~~
Bitcast instructions are analyzed to determine if the value being cast points
to a type of interest or is a pointer to some field/element in a type of
interest. If it does, the uses of the bitcast must be examined to determine
whether or not the use might present a safety issue for some DTrans
optimization.

If the destination type of the bitcast is a type of interest and the source
value is an i8* value, the cast is only legal if one of the following
conditions are met:

1. The source value is the result of an allocation call.
2. The source value can be proved to locally alias to the destination type and
   is not known to locally alias to any other type. (See `Local Pointer Type
   Analysis`_.)
3. The destination type points to the type of the first element in an aggregate
   type to which the value is known to alias locally.

If the destination type is a type of interest and the source value is not an
i8* value, the cast can only be safe if the source value is a pointer to an
aggregate type and the destination type points to the type of the first element
in the aggregate type pointed to by the source value.

If the destination type is not a type of interest but the source value is a
value of interest, the cast is only legal if one of the following conditions is
met:

1. The destination type is i8*.
2. The source value is a pointer to a pointer and the destination type is
   a pointer to a pointer-sized integer (pointers are often stored to memory
   this way in LLVM IR).
3. The destination type points to the type of the first element in an aggregate
   type to which the source value is known to alias locally.


PtrToInt
~~~~~~~~
The PtrToInt instruction is a type of cast, but it specifically transforms a
pointer instruction to a pointer-sized integer. This is commonly used in LLVM
IR when a pointer value is going to be stored to memory (for instance, in a
structure field that points to the type being stored). In this case, the
pointer being stored will frequently be cast as an integer using PtrToInt
and the target location will be cast as an i64* pointer. A typical case looks
like this:

.. code-block:: llvm

  %psrc.as.i = ptrtoint %struct.S* %psrc to i64
  %pptarget.as.pi = bitcast %struct.S** %pptarget to i64*
  store i64 %psrc.as.i, i64* %pptarget.as.pi

Other uses of PtrToInt should be considered as potentially unsafe.


Load
~~~~
Load instructions must be analyzed according to the source of the memory being
loaded. Field reads are performed using load instructions.

If the pointer operand of the load instruction is known to alias a pointer to a
pointer to a type of interest, the loaded value (typically a pointer-sized
integer) will be considered to alias to a pointer to that type.

If the pointer operand is known to point to an element within an aggregate type,
the size of the value being loaded must be the same as the size of the element
pointed to. In most cases, the value will be loaded with the same type as the
element, but pointer elements are often loaded as pointer-sized integers and
then cast to the appropriate type. In the case of pointer field copies a
pointer loaded as an integer may even be stored to another location as an
integer without ever being cast to a pointer type. In this case the `Local
Pointer Type Analysis`_ information will be important for verifying the
safety of the store.

If the pointer operand is known to point to an element within an aggregate type
and the load instruction is marked as volatile, then the aggregate type which
contains the element will have the `VolatileData`_ safety condition set.


Store
~~~~~
Store instructions must be analyzed according to the source of the memory being
stored. Field writes are performed using store instructions.

If the pointer operand of the store instruction is known to alias a pointer to
a pointer to a type of interest, the stored value (typically a pointer-sized
integer) must also be known to alias to a pointer to that type. Otherwise, the
`UnsafePointerStore`_ safety condition will be set for the type to which the
pointer operand aliases.

If the pointer operand is known to point to an element within an aggregate
type, the value being stored must be the same as the size of the element. If it
does not, the `MismatchedElementAccess`_ safety condition will be set for
the aggregate type containing the element to which the pointer operand points.

If the value operand is known to alias to a type of interest the pointer
operand must be known to alias to a pointer to that type. If it does not, the
`UnsafePointerStore`_ safety condition will be set for the stored type. If the
pointer operand is known to alias some other type of interest, that type will
also receive the UnsafePointerStore safety condition.

If the value operand is known to point to an element within an aggregate type,
the `FieldAddressTaken`_ safety condition will be set for the parent type.


PHI Nodes
~~~~~~~~~
PHI nodes do not create new safety issues as they are only used to describe
the flow of existing values. We will need to process PHI nodes as we
follow the uses of values for other purposes, but the PHI node itself
requires no special processing.


Select
~~~~~~
Select instructions do not create new safety issues as they are only used to
describe the flow of existing values. We will need to process select
instructions as we follow the uses of values for other purposes, but the select
instruction itself requires no special processing.


Return
~~~~~~
Return instructions do not require direct analysis. If the value returned is the
address of a field within a structure, we will need to mark the containing
field with safety data indicating that the address of the field was taken.
However, that will be done as part of the GetElementPtr analysis.


ICmp
~~~~
ICmp instructions are commonly used to check for null pointers or to compare
two pointers for equality. This is a safe usage. If a comparison is made
between pointers to elements of an aggregate type, the use may present a safety
issue for some transformations. **This requires additional code which is not
yet implemented.**


Alloca
~~~~~~
LLVM uses an explicit alloca instruction to allocate stack space for local
variables. **This instruction type is currently not handled.**


Global variables
----------------
Global variables are mostly tracked through their uses in the instructions of
a program. At the module level, global variables will be examined to determine
whether or not they have an initializer list. **This is not yet implemented.**

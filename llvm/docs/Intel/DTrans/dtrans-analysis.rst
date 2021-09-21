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

BadAllocSizeArg
~~~~~~~~~~~~~~~
This indicates that an allocation was detected to construct an aggregate type
(or array of aggregates) where the size parameter of the allocation call could
not be traced to a constant matching the aggregate type size, or a constant
multiple of the aggregate type size.

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
passed to a GetElementPtr instruction. The GetElementPtr instruction can
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
This indicates either that a store instruction was seen where a pointer being
stored was not known to be compatible with the pointer to the address at which
it was being stored. This could mean that the value operand and the pointer
operand (destination) were known to alias to different aggregate types or
that the one was known to alias to an aggregate type and the other was not.

For example, in the following block, %p1 is an arbitrary pointer with no known
aliases. After it is stored in the location referenced by %tmp (%p2) any future
loads from that address will handle it as a %struct.S pointer. Since dtrans
does not know where the original buffer came from or how it is used, we must
assume this is unsafe.

.. code-block:: llvm

  define void @f(i8* %p1, %struct.S** %p2) {
    %tmp = bitcast %struct.S** %p2 to i8**
    ret void store i8* %p1, i8** %tmp
  }

In the next example, the value operand %tmp1 (%p1) is known to be incompatible
with the pointer operand %tmp2 (%p2). In this case, both %struct.A and
%struct.B will be marked with the UnsafePointerStore condition.

.. code-block:: llvm

  define void @f(%struct.A* %p1, %struct.B** %p2) {
    %tmp1 = bitcast %struct.A* %p1 to i8*
    %tmp2 = bitcast %struct.B** %p2 to i8**
    ret void store i8* %tmp1, i8** %tmp2
  }

FieldAddressTakenMemory
~~~~~~~~~~~~~~~~~
This indicates that the addresses of one or more fields within the type were
written to memory.

GlobalPtr
~~~~~~~~~
This indicates that a global variable was found that is a pointer (or a pointer
to a pointer) to the type.

GlobalInstance
~~~~~~~~~~~~~~
This indicates that a global variable was found that results in the
construction of an instance of the type. This can be due to a direct
instantiation of the type, or some combination of the instantiation
for an array of the type, or the type being contained within another
structure type that has a global instantiation. However, if the contained
elements are pointers to types, the instantiation will not cause those types
to be marked with this safety flag because they are not being directly
instantiated.

GlobalArray
~~~~~~~~~~~
This indicates that a global variable was found that is a fixed size array
containing elements of the type. The elements of the array may be pointers
or instances.

HasInitializerList
~~~~~~~~~~~~~~~~~~
This indicates that a global variable was found that is an instance of the type
and a non-zero initializer was specified for the variable.

BadMemFuncSize
~~~~~~~~~~~~~~
This indicates that a pointer to a structure is passed to a memory function
intrinsic (memcpy, memmove or memset) with a size that could not be analyzed
or is invalid. Possible reasons are:

* The size cannot be resolved to be a multiple of the aggregate size
  to read/write the entire aggregate object.
* The size cannot be resolved to be a proper subset of fields being accessed.
* The read/write exceeds the bounds of the native structure size of the source
  or destination object.
* The read/write memory is smaller than the underlying object referenced,
  such as only 2 bytes of a 4 byte element.

MemFuncPartialWrite
~~~~~~~~~~~~~~~~~~~
This indicates that a pointer to a structure is passed to a memory function
intrinsic (memcpy, memmove, or memset) with a size that completely covers a
proper subset of fields within the structure. If the field is itself an
aggregate, the entire field must be accessed, otherwise the BadMemFuncSize
safety will be set (i.e. memset with the address of a member that is a nested
structure must set the entire nested structure, not a portion of it)
Transformations may need to carefully split the intrinsic operation into
separate Load/Stores for the fields of a modified data structure when operating
on structures with this property.


BadMemFuncManipulation
~~~~~~~~~~~~~~~~~~~~~~
This indicates a structure is modified via a memory function intrinsic (memcpy
or memmove) with conflicting or unknown types for the source and destination
parameters.

AmbiguousPointerTarget
~~~~~~~~~~~~~~~~~~~~~~
This indicates that a pointer is passed to an intrinsic or function call,
but the pointer is known to alias incompatible pointer types.

UnsafePtrMerge
~~~~~~~~~~~~~~
This indicates that a PHI node or select instruction was seen which had
incompatible incoming values. This could mean either that the incoming values
were known to alias to incompatible aggregate types or that at least one
incoming value was known to be a pointer to a type of interest and at least one
other incoming value was not known to point to that type.

AddressTaken
~~~~~~~~~~~~
This indicates that the address of an object of the type was returned by
a function using an anonymous type (i8* or i64).

NoFieldsInStruct
~~~~~~~~~~~~~~~~
The type represents a structure which was defined with no fields.

NestedStruct
~~~~~~~~~~~~
The type identifies a structure that was contained as a non-pointer member
of another structure.

ContainsNestedStruct
~~~~~~~~~~~~~~~~~~~~
The type identifies a structure that contains another structure as a
non-pointer member.

SystemObject
~~~~~~~~~~~~
The type was identified as a known system structure type.

LocalPtr
~~~~~~~~~
This indicates that a local variable was found that is a pointer (or pointer
to pointer) to the type.

LocalInstance
~~~~~~~~~~~~~~
This indicates that a local variable was found that is an instance of the type.
This can be due to a direct stack allocation of the type, or some combination
of a stack allocation for an array of the type, or the type being contained
within another structure type that has a stack allocation. However, if the
contained elements are pointers to types, a stack allocation will not cause
those types to be marked with this safety flag because they are not being
directly allocated.

MismatchedArgUse
~~~~~~~~~~~~~~~~
This indicates that a function with at least one i8* argument was called with
a value passed as an i8* argument whose alias set did not match the expected
alias set for the argument based on the uses of the argument within the
function.

HasVTable
~~~~~~~~~
This indicates that the type is a structure with at least one field that seems
as though it may be a vtable. Any field that is a pointer to a pointer to a
function will trigger this safety condition.

HasFnPtr
~~~~~~~~
This indicates that the type is a structure with at least one field that is
a pointer to a function.

HasCppHandling
~~~~~~~~~~~~~~
This indicates that the type has C++ processing:
    - there is a memory allocation and/or deallocation with C++ operators
      new/new[] and or delete/delete[];
    - there is an invoke instructions related to the type.

HasZeroSizedArray
~~~~~~~~~~~~~~~~~
This indicates that the type is an array with zero size or the type is
a structure with a zero-sized array member.

BadCastingPending
~~~~~~~~~~~~~~~~~

A potential bad casting issue that will be either eliminated,
converted to bad casting conditional, or converted to bad casting
at the end of analysis by the bad casting analyzer. (See the description
of the Bad Casting Analyzer below.) `Bad Casting Analyzer`_

BadCastingConditional
~~~~~~~~~~~~~~~~~~~~~

Indicates that bad casting will occur only if specific conditions
are not fulfilled.  These conditions are noted by the bad casting
analyzer, and involve certain functions' arguments being nullptr on
entry to those functions. (See the description of the Bad Casting
Analyzer below.) `Bad Casting Analyzer`_

UnsafePointerStorePending
~~~~~~~~~~~~~~~~~~~~~~~~~

A potential unsafe pointer store issue that will be either eliminated,
converted to unsafe pointer store conditional, or converted to unsafe
pointer store at the end of analysis by the bad casting analyzer.
(See the description of the Bad Casting Analyzer below.)
`Bad Casting Analyzer`_

UnsafePointerStoreConditional
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Indicates that an unsafe pointer store  will occur only if specific
conditions are not fulfilled.  These conditions are noted by the bad
casting analyzer, and involve certain functions' arguments being nullptr
on entry to those functions. (See the description of the Bad Casting
Analyzer below.) `Bad Casting Analyzer`_

DopeVector
~~~~~~~~~~
The type was identified as a dope vector.

BadCastingForRelatedTypes
~~~~~~~~~~~~~~~~~~~~~~~~~

This safety data is used for special bad casting cases that won't affect
related types. These types are structures that have two types in the IR,
where one type represents the base form and the other type has the same
fields with an extra field at the end used for padding.

BadPtrManipulationForRelatedTypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This safety data is used to check if bad pointer manipulation won't affect
the related types.

MismatchedElementAccessRelatedTypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This safety data is used for a special mismatched element access to
the zero field of a structure but won't affect the related types.

UnsafePointerStoreRelatedTypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This safety data is used for special unsafe pointer store to the zero
field of a structure but won't affect related types.

MemFuncNestedStructsPartialWrite
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This safety data is used when a memory handling function (e.g. memcpy)
modifies part of the nested structures, but it won't fully cover the
field zero in the outer most structure.

ComplexAllocSize
~~~~~~~~~~~~~~~~

This safety data is used for a memory allocation that has been analyzed
as safe, but is not an exact multiple of the size of the structure.
Information that is collected about the structure fields is available.
An example of an allocation that would set this is: malloc(c1 + c2 * n),
where c1 and c2 are constants, and n is the size of the structure.

Transformations that need to modify the size of an allocation when changing
a structure definition may not be able to simply adjust the allocation size
with the following formula when this flag is set.
  byte_count = old_byte_count * new_struct_size / old_struct_size

FieldAddressTakenCall
~~~~~~~~~~~~~~~~~~~~~

This safety data is used when the address of a field is passed as an argument
to a callsite.

FieldAddressTakenReturn
~~~~~~~~~~~~~~~~~~~~~~~

This safety data is used when the address of a field is returned by a function.

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
is either the result of casting a %struct.S pointer to an i8* or the result of
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

For DTrans optimizations to be possible, each pointer that may point to a type
that will be optimized will in most cases need to point only to that type
and generic equivalents such as i8* and a pointer-sized integer. If the local
alias analysis finds just one such type, that type will be described as the
unique dominant type for the pointer. Identifying a unique dominant type for
a pointer will simplify the analysis of its uses. For example, when a pointer
is used by a bitcast instruction, we are interested in whether or not the
destination type is safe for the pointer's dominant type. If all other aliases
of the pointer are generic equivalents, they do not need to be analyzed.


Instruction Handling
--------------------
(Note: The following sub-sections describe the way each kind of instruction must
be handled to analyze its impact on any aggregate types. This is a work in
progress. As the analysis is implemented this section will be updated to
describe what the analysis is doing.)

Call
~~~~
When a call instruction is encountered, the DTrans analysis will attempt to
determine whether or not the call is allocating memory. For LibFuncs,
LLVM's LibFunc mechanism is used to check for calls to malloc, calloc, realloc
and for calls to standard new and new[] operators allocating memory.
Note, that standard placement new operators do not allocate memory.

Some user functions are also handled.  Right now, we distinguish two types:

  AK_UserMalloc0:
    The user function may have any number of arguments, but the
    first (0th argument) must specify the "size" of memory to be allocated
    (the "size" argument). Each return of the user function must be post
    dominated by a call to malloc, and the return must return a pointer to
    the malloc'ed memory. (An exception is made for some returns that may
    return nullptr if the user function is passed 0 in its "size" argument,
    or if some call to malloc returns a nullptr.)

  AK_UserMalloc:
    Same as AK_UserMalloc0, but there must be either only 1 argument
    (the "size" argument)

  AK_UserMallocThis:
    Similar to AK_UserMalloc, except there are two arguments (the "this" ptr
    argument and the "size" argument).

  A corresponding set of user memory deallocation functions is also recognized
  with the pointer argument that will be deallocated being the first argument
  (FK_UserFree0 and FK_UserFree), or the second argument (FK_UserFreeThis)

At some point this mechanism will be extended to handle additional user
functions, including those that call calloc and realloc.


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

If the called function is the "free" function, the call is viewed as safe.
Calls to standard delete/delete[] operators are viewed as safe too.

If the called function is an unknown externally defined function and any of the
arguments is a pointer to an aggregate type, that type is marked with the
`AddressTaken`_ safety condition, and if any of the arguments is a pointer to
an element within a structure then that structure is marked with the
`FieldAddressTakenCall`_ safety condition.

If the called function is a locally defined function, its arguments are handled
as described above for external functions except that arguments which point
to aggregate types are accepted without the `AddressTaken`_ condition being set
if the argument type matches the type of the structure. For instance, a
%struct.A* value can be safely passed as an argument to a call if the called
function uses %struct.A* as the type for that argument because the uses of
the argument can be tracked when the function is analyzed. However, if a
%struct.A* value is cast to an i8* and then passed to a function call, we
cannot know what the argument's original type was when we are analyzing the
called function.

Invoke
~~~~~~
Processing is the same as for call instructions. HasCppHandling is set among
safety conditions for a type.

Intrinsic
~~~~~~~~~
When a call to an intrinsic function is encountered, the DTrans analysis will
attempt to check parameters to the function for safe uses.

Currently, only calls to memset, memcpy and memove are supported.

A call to memset is considered safe if the entire structure (based on the
sizeof(%struct) size) is being set.

A call to memcpy or memmove is considered safe if:

- The entire structure is being set.
- The data type of the source structure is the same as the data type of the
  the destination structure.

Otherwise, both the source and destination parameter types are marked as
unsafe uses.

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

In many cases the address obtained using a GetElementPtr instruction will be
passed directly to a load or store instruction to access the field value. In
the case where a GEP is used with an i8* base pointer and an offset, there may
be a bitcast between the GEP and the load or store. For any other uses, the
field is marked with the ComplexUse flag in the FieldInfo. This does not mean
that we are not able to analyze the uses of the pointer. The local pointer
analysis will be able to track this value through any combination of
instructions. The intent of the ComplexUse flag is to serve as a hint to
optimizations (specifically the DeleteField optimization) that it will not be
trivial to erase the uses of this field.

Sub
~~~
The sub instruction cannot operate on pointer values directly, but it is
frequently used to operate on pointer-size integers that have been created
from pointers using the ptrtoint instruction.

If a pointer is subtracted from another pointer the result is a scalar value
that can have no association with any type, implicit or explicit, so this
operation is safe for the input pointer type. An offset computed this way may
be used in ways that present safety concerns but that will be handled when the
instruction using the value is analyzed.

However, because the offset depends on the size of the structure being pointed
to, if the pointers are pointers to structures (as opposed to pointer to
pointers) the result of the sub instruction must only be used by sdiv or udiv
instructions that divide by the size of the structure or PHI nodes or select
instructions that lead to such div instructions. If any other use is found
the type being pointed to will be marked with the `BadPtrManipulation`_
safety condition.

If either operand of a sub instruction is a pointer to an element within a
structure then the offset so computed is dependent on the layout of the
parent structure and so this case is flagged with the `BadPtrManipulation`_
safety condition.

If the operands of a sub instruction are not known to both point to the same
structure type, the types pointed to will be marked with the `UnhandledUse`_
safety condition. This case is not expected and will require further
consideration if it is found to occur.


Bitcast
~~~~~~~
Bitcast instructions are analyzed to determine if the value being cast points
to a type of interest or is a pointer to some field/element in a type of
interest. If it does, the uses of the bitcast must be examined to determine
whether or not the use might present a safety issue for some DTrans
optimization.

If the source pointer operand is not a value of interest but the destination
type is a type of interest, the destination type will be marked with the
`BadCasting`_ safety condition. This would happen, for instance, if an
unknown i8* value were cast as a pointer to an aggregate type.

If the source pointer operand is known to alias to incompatible types then
the cast is ambiguous and each aliased type and the destination type (if
it is a type of interest) will be marked with the `BadCasting`_ safety
condition.

If the source pointer operand is known to point to a single aggregate type
the cast is safe if the destination type is one of the following:

 - The same aggregate type (for instance, an i8* value with a known type alias
   may be cast back to its original type)
 - A generic pointer equivalent (for instance, a pointer to an aggregate may be
   cast to an i8* or a pointer sized integer)
 - A type that points to the first element in the aggregate type (for instance,
   if a structure has an i16 value as its first field, a pointer to the
   structure may be cast to i16* to access that field)

The immediate type (that is, the type as reported by LLVM) of the source
pointer operand is not important since the local pointer analysis will have
already proven that it is equivalent to the aliased type.

If the source pointer operand is known to point to an element within an
aggregate type, then the cast is safe if the destination type is one of the
following:

 - A type that points to the type of the element
 - A generic pointer equivalent (for instance, i8* or a pointer-sized integer)
 - A type that points to the type of the first field within the element (at
   any level of nesting) if the element is an aggregate type


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
type, the size of the value being stored must be the same as the size of the
element. If it does not, the `MismatchedElementAccess`_ safety condition will
be set for the aggregate type containing the element to which the pointer
operand points.

If the value operand is known to alias to a type of interest the pointer
operand must be known to alias to a pointer to that type. If it does not, the
`UnsafePointerStore`_ safety condition will be set for the stored type. If the
pointer operand is known to alias some other type of interest, that type will
also receive the UnsafePointerStore safety condition.

If the value operand is known to point to an element within an aggregate type,
the `FieldAddressTakenMemory`_ safety condition will be set for the parent type.


PHI Nodes
~~~~~~~~~
PHI nodes may be unsafe if the incoming values do not all point to the same
aggregate type. For instance, if one incoming value is known to be a pointer
to some structure and another incoming value is not known to point to that
structure, the resulting merged pointer is unsafe. Similarly, if one incoming
value is known to point to some structure A and another incoming value is known
to point to some structure B, the merged value is ambiguous.

On the other hand, it will be common for PHI nodes to merge pointers which
point to different elements within a structure. As long as the elements being
pointed to all have the same type, this is safe. If pointers to elements of
different sizes are merged, the safety issue will be detected when the merged
value is used.


Select
~~~~~~
Select instructions may be unsafe if the incoming values do not both point to
the same aggregate type. For instance, if one incoming value is known to be a
pointer to some structure and the other incoming value is not known to point to
that structure, the resulting merged pointer is unsafe. Similarly, if one
incoming value is known to point to some structure A and the other incoming
value is known to point to some structure B, the merged value is ambiguous.

On the other hand, it will be common for selects to merge pointers which
point to different elements within a structure. As long as the elements being
pointed to all have the same type, this is safe. If pointers to elements of
different sizes are merged, the safety issue will be detected when the merged
value is used.


Return
~~~~~~
Return instructions must be checked to see if the address of an aggregate
object or the address of an element within an aggregate may escape through
the return.

If the returned value is a pointer to an aggregate type but the type is
preserved in the return value, the return is safe since our analysis will
recognize the type at the call site. However, if the address is returned
via an anonymous type such as i8* or i64 the type of the object whose
address is returned will be marked with the `AddressTaken`_ safety condition.

If the returned value is known to be the address of an element within an
aggregate object, the type of the object containing the element will be
marked with the `FieldAddressTakenReturn`_ safety condition.

In the uncommon case where an instance of an aggregate is returned the type
of the aggregate will be marked with the `WholeStructureReference`_
safety condition.


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
variables. If an alloca instruction is seen that allocates space for a type
of interest, the type will be marked with the `LocalInstance`_ safety condition
if the alloca is creating an instance of the type (including fixed sized arrays)
or the `LocalPtr`_ safety condition if the alloca is creating a pointer to the
type. (Note that the LLVM type of the alloca instruction is always a pointer
type, so local pointer variables will appear as pointers to pointers.)


Global variables
----------------
Global variables are mostly tracked through their uses in the instructions of
a program. At the module level, the definition for each global variable will be
visited and if its type is a type of interest, safety data for that type will
be updated.

All global variables in LLVM IR are defined as pointers. If the variable is
an instance of an aggregate type at the source code level, the LLVM IR global
variable will be a pointer to the global memory for that object. If the
variable is a pointer at the source code level, the LLVM IR global variable
will be a pointer to a pointer.

As global variables with a type of interest are visited, if the variable is
a pointer to a pointer, the `GlobalPtr`_ safety condition will be added to
the type. Otherwise, the `GlobalInstance`_ safety condition will be added.

LLVM IR requires all global variables to have an initializer. This may be
a simple zero-initializer, or it may be specific aggregate data. If the global
variable is an instance of a type and not a pointer to that type and the
initializer is non-zero aggregate data, the type will be marked with the
`HasInitializerList`_ safety condition.

Field Single Value Analysis
===========================

DTrans supports a Field Single Value Analysis that determines whether a field
of a structure can have only one value during the execution of the program.
Whole program is required. Values may be assigned via static initialization
or during dynamic execution. Field Single Value Analysis is an important
base analysis for Indirect Call Specialization.

Query methods for Field Single Value Analysis are defined in the public
member functions of the FieldInfo class in DTrans.h:

.. code-block:: c++

  bool isNoValue() const
    // The field has not been assigned a value
  bool isSingleValue() const
    // The field has been assigned a single value
  bool isMultipleValue() const
    // The field has been assigned multiple values, or we have no idea what
    // value(s) the field has been assigned.

Only one of these three will be true for any field at any point in time.
The represent three classic states of a lattice: Top, Middle, and Bottom.
In the case that isSingleValue() is true, the value can be obtained with

.. code-block:: c++

  llvm::Constant *getSingleValue()

Single Alloc Function Analysis
==============================

DTrans supports a Single Alloc Function Analysis which determines whether
a field points to memory allocated by a specific function.

Query methods for Single Alloc Function Analysis are defined in the public
member functions of the FieldInfo class in DTrans.h:

.. code-block:: c++

  bool isTopAllocFunction() const
    // The field has not been assigned a value
  bool isSingleAllocFunction() const
    // The field is assigned either a nullptr or the return value of a specific
    // function which has returned a pointer to uniquely allocated memory.
  bool isBottomAllocFunction() const
    // Everything else

Only one of these three will be true for any field at any point in time.
The represent three classic states of a lattice: Top, Middle, and Bottom.
In the case of isSingleAllocFunction(), the specific function allocating
the memory can be obtained with

.. code-block:: c++

  llvm::Function *getSingleAllocFunction()

There is no requirement that the return value of this function be assigned
only once to the field value, or that the amount of memory assigned on
successive calls be the same.  There also is no requirement that the specific
function only be called to assign a pointer to the field.

An escape analysis is performed to ensure that the pointer to memory in
a field for which isSingleAllocFunction() is true is not manipulated in
such a way to invalidate the isSingleAllocFunction() property.

Bad Casting Analyzer
====================

The bad casting analyzer was written to allow up to deal with certain
important bad casting and unsafe pointer store safety violations that
arise in the following context:

Consider the sequence of code fragments:

.. code-block:: c++

  (1) typedef struct {
  (2)    void *coder;
  (3)    void (*startup)(void *myarg);
  (4)    void (*shutdown)(void *myarg);
  (5) } mynextcoder;

  (6) typedef struct {
  (7)   int myint1;
  (8)   int *myptr1;
  (9) } mycoder1;

  (10) typedef struct {
  (11)  int *myptr2;
  (12)  int myint2;
  (13) } mycoder2;

  (14) int myglobalint1 = 50;
  (15) int myglobalint2 = 100;

  (16) void init_with_coder1(mynextcoder *next) {
  (17)   if (!next->coder)
  (18)     next->coder = malloc(sizeof(mycoder1));
  (19)  next->startup = coder1_startup;
  (20)  next->shutdown = coder1_shutdown;
  (21)  ((mycoder1 *)(next->coder))->myint1 = 15;
  (22)  ((mycoder1 *)(next->coder))->myptr1 = &myglobalint1;
  (23) }

  (24) void init_with_coder2(mynextcoder *next) {
  (25)   if (!next->coder)
  (26)    next->coder = malloc(sizeof(mycoder2));
  (27)  next->startup = coder2_startup;
  (28)  next->shutdown = coder2_shutdown;
  (29)  ((mycoder2 *)(next->coder))->myint2 = 20;
  (30)  ((mycoder2 *)(next->coder))->myptr2 = &myglobalint2;
  (31) }

  (32) void coder1_startup(void *myarg) {
  (33)   mycoder1 *ptr = (mycoder1 *) myarg;
  (34)   ...
  (35) }

  (36) void coder1_shutdown(void *myarg) {
  (37)   mycoder1 *ptr = (mycoder1 *) myarg;
  (38)   ...
  (39) }

  (40) void coder2_startup(void *myarg) {
  (41)   mycoder2 *ptr = (mycoder2 *) myarg;
  (42)  ...
  (43) );

  (44) void coder2_shutdown(void *myarg) {
  (45)   mycoder2 *ptr = (mycoder2 *) myarg;
  (46)   ...
  (47) );

  (48) void myoperation(mynextcoder *next) {
  (49)  next->startup(next->coder);
  (50)  ...
  (51)  next->shutdown(next->coder);
  (52) }

Note that on lines 21-22 the 'coder' field is cast to the type coder1, while
in lines 29-30 it is cast to the type coder2. This would, in general, yield
"bad casting" and "unsafe pointer store" safety violations. However, note
that as long as the type of structure assigned to a specific INSTANCE of
next->coder is never accessed as a different type, the safety violations
are not a real problem.  This is the key point behind the bad casting analysis.

Note that in the above example, that calls to the functions which
are assigned to function pointer fields in the structure 'mynextcoder' (as in
lines 49 and 51), always have as their zeroth argument the 'coder' field
of the same INSTANCE of the 'mynextcoder' structure.  Furthermore, note that
the zeroth argument of those functions (as illustrated in lines 36-37,
40-41, 44-45, and 48-49 is always used only as the type to which the 'coder'
field was initially assigned.

A special wrinkle to all of this comes in the code on lines 21-22 and 29-30.
Here the pointer to the 'coder' field is cast to the type to which it could
be assigned in the corresponding function, but that assignment is conditional.
If it could happen that the allocaion did NOT happen, we might end up accessing
the 'coder' field as a pointer to a different type than the type it was on
entry.  This truly would be a case of bad casting, and any optimizations,
like "single alloc analysis" that depend on the bad casting being absent
would be invalid.

We deal with this issue in the following way. We insert a test at the
beginning of functions init_with_coder1() and init_with_coder2() (as
shown below in lines 16a-16b and 24a-24b):

.. code-block:: c++

  (16) void init_with_coder1(mynextcoder *next) {
  (16a)  if (next->coder)
  (16b)    GIVE UP ON ANY OPTIMIZATION THAT DEPENDS ON "NO BAD CASTING"
  (17)   if (!next->coder)
  (18)     next->coder = malloc(sizeof(mycoder1));
  (19)  next->startup = coder1_startup;
  (20)  next->shutdown = coder1_shutdown;
  (21)  ((mycoder1 *)(next->coder))->myint1 = 15;
  (22)  ((mycoder1 *)(next->coder))->myptr1 = &myglobalint1;
  (23) }

  (24) void init_with_coder2(mynextcoder *next) {
  (24a)  if (next->coder)
  (24b)    GIVE UP ON ANY OPTIMIZATION THAT DEPENDS ON "NO BAD CASTING"
  (25)   if (!next->coder)
  (26)    next->coder = malloc(sizeof(mycoder2));
  (27)  next->startup = coder2_startup;
  (28)  next->shutdown->coder2_shutdown;
  (29)  ((mycoder2 *)(next->coder))->myint2 = 20;
  (30)  ((mycoder2 *)(next->coder))->myptr2 = &myglobalint2;
  (31) }


The actual code in the bad casting analyzing executes in three steps:

  (1) Find a candidate type and field on which to do the analysis.
      (In our example above this would be the 'mynextcoder' structure and
      the 'coder' field.)

  (2) Analyze loads and stores to the 'mynextcoder' 'coder' field.

  (3) Conclude whether the bad casting and unsafe pointer store safety
      violations have actually occurred and mark the 'mynextcoder'
      structure appropriately.

In terms of the SafetyData violation types, we mark any potential "bad
casting" and "unsafe pointer store" safety violations on the candidate
type and field discovered during step (2) as BadCastingPending and/or
UnsafePointerStorePending. In step (3), we have three choices:

  (1) We can determine that no such safety violations exist, and
      remove the BadCastingPending and UnsafePointerStorePending
      safety violations.
  (2) We can determine that the safety violations cannot be removed,
      and convert the BadCastingPending violations to BadCasting and the
      UnsafePointerStore violations to UnsafePointerStore.
  (3) We can determine that the safety conditions can be removed under
      certain conditions, and change the BadCastingPending violations
      to BadCastingConditional and the UnsafePointerStore violations to
      UnsafePointerStoreConditional. (This is what was illustrated in
      the example above.)

Further details on the operation of the bad casting analyzer, including
examples with IR, can be found in the code in DTransAnalysis.cpp.


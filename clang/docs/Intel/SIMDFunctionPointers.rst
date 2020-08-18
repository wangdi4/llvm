======================
SIMD Function Pointers
======================

Introduction
============

This is a description of this feature from the clang point of view. It includes
the basic design and will serve as internal documentation.

For initial implementation it can be broken into three pieces.

1. `Default DPC++ Behavior`_

  DPC++ device functions will have a set of default variants. For example, a
  CPU device will have masked with all vector arguments and unmasked with all
  vector arguments.  The user does not specify these variants and the compiler
  handles it all.

  Since all function pointers of the same type will have the same variants,
  all are compatible, and no special type checking is required.

2. `DPC++ Wrapper Class`_

  A C++ class will be added to the library which lets the user specify which
  variants are needed for a function.  So instead of using function pointers
  directly a class object is used.  Initialization, assignment, conversions,
  and calls are all handled by the class through template 'magic' and clang
  builtins.

3. `Full Functionality`_

  This adds the ability for users to fully specify variant information using
  OpenMP declare simd syntax. It will be similar to the existing ICC
  implementation and comes with all the same type system problems. The work is
  similar to the "Default DPC++" implementation but since two function
  pointers with the same type can have a different set of variants, we
  can't assume they are compatible and must do special checking and codegen
  to implement it.

.. _Default DPC++ Behavior:

Default DPC++ Behavior
======================

(Preliminary. Specific names used for intrinsics and tables especialy subject
to change.)

This part requires two changes in clang Codegen.

(1) Function Address
--------------------

When the address of a function is needed, we instead generate the address
of a table.  The table will initially contain only a pointer to the scalar
function.  The backend will modify the table with other variants as needed.

.. code-block:: c

 int foo(int i) { return i; }
 int (*fp1)(int) = foo;  // Initialized via the table address.
 int (*fp2)(int) = fp1;  // Normal initialization.

This includes all function pointer cases including initialization of virtual
function tables.

(2) Indirect Calls
------------------

All indirect calls need to use a specially-named function.  Due to toolchain
considerations, the llvm intrinsic mechanism is not used and special
functionality has been implemented.

.. code-block:: c

 fp1(1);  // uses special function
 fp2(2);  // uses special function

An example of the special function is:

 Ret __intel_indirect_call_[0-9]+(<addrVariantTable>, Arg...)

The first argument is the address of the variant table previously generated
when the address of a function was generated.  `Arg...` is the call arguments
and `Ret` is the return value of the call.

Each different function type will generate a unique function with the
"__intel_indirect_call_" prefix followed by a number.

Note that the type of the variant table is not the same as a normal function
pointer.  To deal with this we will keep function pointers the normal type and
use bitcasts where needed at the indirect callsite.

Example:

.. code-block:: c++

 typedef int (*fptr_type)(int);

 int foo(int i) { return i+1; }

 int main()
 {
   fptr_type fp = &foo;
   return fp(6);
 }

.. code-block:: llvm

 @"_Z3fooi$SIMDTable" = weak global [1 x i32 (i32)*] [i32 (i32)* @_Z3fooi], align 8

 define i32 @main() #0 {
 entry:
   %fp = alloca i32 (i32)**, align 8
   store i32 (i32)* bitcast ([1 x i32 (i32)*]* @"_Z3fooi$SIMDTable" to i32 (i32)*), i32 (i32)** %fp, align 8
   %1 = load i32 (i32)*, i32 (i32)** %fp, align 8, !tbaa !5
   %2 = bitcast i32 (i32)* %1 to i32 (i32)**
   %3 = call i32 @__intel_indirect_call_0(i32 (i32)** %2, i32 6)
   ret i32 %call1
 }

.. _DPC++ Wrapper Class:

DPC++ Wrapper Class
===================

(Preliminary)

This will require two clang builtins that are called from the wrapper class.

(1) FType\* __builtin_generate_SIMD_variant(FType \*Func, int VLen, (SpecType\*)())
-----------------------------------------------------------------------------------

* Func is the constant address of the base function.
* VLen is the vector length of the variant.
* SpecType is a function type that represents the variant we want to generate.
* The return is a pointer to the actual specific variant.

This will be used in the constructor of the wrapper. It will be called once
for each variant and the return value will be stored in the class object.

Example:

.. code-block:: c++

 struct uniform; struct linear; struct varying; struct masked; struct unmasked;
 int foo(int i, float f) {return (int)f+i+1;}

 int (*fp)(int, float) =
   __builtin_generate_SIMD_variant(foo, 4, std::add_pointer_t<unmasked(linear,uniform)>());

This will generate a call to this special intrinsic-like function:

 Func* __intel_create_simd_variant_[0-9]+(Func)

A vector ABI (https://software.intel.com/sites/default/files/managed/b4/c8/Intel-Vector-Function-ABI.pdf)
mangled name is created from the passed function type SpecType and provided as
a attribute to the __intel_create_simd_variant call.

.. code-block:: llvm

 %fp = alloca i32 (i32, float)*, align 8
 %call = call i32 (i32, float)* @__intel_create_simd_variant_0(i32 (i32, float)* @foo) #0
 store i32 (i32, float)* %call, i32 (i32, float)** %fp, align 8

 attributes #0 = { "vector-variants"="_ZGVxN4lu_foo" }

(2) Ret __builtin_call_SIMD_variant(detail::variant_list<SpecType...>(), int_list<int...>(), (F**)ptrs, args...)
----------------------------------------------------------------------------------------------------------------

This builtin takes the following arguments:

* A list of SpecTypes in the same form as passed to
  __builtin_generate_SIMD_variant
* A list of vector lengths passed as template parameters
* A pointer to the table of function variants previously returned from
  __builtin_generate_SIMD_variant stored in the same order as the SpecTypes
* Arguments to the function call

As an example, the templates used may expand to something like this:

.. code-block:: c++

 typedef int (*fp)(int,float);
 fp ptrs[2];
 ptrs[0] = __builtin_generate_SIMD_variant(foo, 4, std::add_pointer_t<unmasked(linear,uniform)>());
 ptrs[1] = __builtin_generate_SIMD_variant(foo, 4, std::add_pointer_t<masked(varying,varying)>());
 // The call
 int i = __builtin_call_SIMD_variant(detail::variant_list<unmasked(linear,uniform), masked(varying,varying)>(), int_list<4>, ptrs, 1, 2.0);

This call also uses the special intrinsic-like function
__intel_indirect_call_[0-9]+ but in this case the call attribute must be
specified.

The IR for the call would be something like:

.. code-block:: llvm

 %call = call i32 @__intel_indirect_call_0(i32 (i32, float)** @ptrs, i32 1, float 2.000000e+00) #0
 store i32 %call1, i32* %i, align 4

 attributes #0 = { "vector-variants"="_ZGVxN4lu_foo,_ZGVxM4vv_foo" }

.. _Full Functionality:

Full Functionality
==================

TBD.

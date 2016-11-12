The default x64 calling convention
==================================

The x64 Application Binary Interface (ABI) uses a four register
fast-call calling convention by default .

Space is allocated on the call stack as a shadow store for callees to
save those registers.

There is a strict one-to-one correspondence between the arguments to a
function call and the registers used for those arguments.

The first four integer arguments are passed in registers. Integer values
are passed (in order left to right) in RCX, RDX, R8, and R9. Arguments
five and higher are passed on the stack.

Floating-point and double-precision arguments are passed in XMM0 – XMM3
(up to 4) with the integer slot (RCX, RDX, R8, and R9) that would
normally be used for that cardinal slot being ignored (see example) and
vice versa.

For example:

*func3(int a, struct big b, int c, float d); *

*// a in RCX, ptr to b in RDX, c in R8, d in XMM3*

Implementation in LLVM
----------------------

The implementation assumes that all first four arguments will be passed
in GPR or XMM,

In order to attach each argument to its correct location in the argument
list, LLVM implementation defines a shadow register.

A shadow register is a register that will be marked as allocated in
CCState but will not be attached to a specific value in the argument
list.

For each allocation of a GPR register a corresponding XMM register is
allocated and vice versa.

*CCIfType<[i64], CCAssignToRegWithShadow<[RCX , RDX , R8 , R9 ],*

*[XMM0, XMM1, XMM2, XMM3]>>,*

This way we can assure that:

-  the first argument will be assigned to RCX or XMM0

-  the second argument will be assigned to RDX or XMM1

-  the third argument will be assigned to R8 or XMM2

-  the forth argument will be assigned to R9 or XMM3

VectorCall Calling Convention for x64
=====================================

The \_\_vectorcall calling convention specifies that arguments to
functions are to be passed in registers, when possible. \_\_vectorcall
uses more registers for arguments than \_\_fastcall or the default x64
calling convention use. The \_\_vectorcall calling convention is only
supported in native code on x86 and x64 processors that include
Streaming SIMD Extensions 2 (SSE2) and above.

The definition of HVA types
---------------------------

An HVA type is a composite type of up to four data members that have
identical vector types. An HVA type has the same alignment requirement
as the vector type of its members.

For example:

    *typedef struct {*

    *\_\_m256 x;*

    *\_\_m256 y;*

    *\_\_m256 z;*

    *} hva3; // 3 element HVA type on \_\_m256*

Vectorcall extension
--------------------

Vectorcall extends the standard x64 calling convention while adding
support for HVA and vector types.

There are four main differences:

1. Floating-point types are considered vector types just like \_\_m128,
   \_\_m256 and \_\_m512. The first 6 vector typed arguments are saved
   in physical registers XMM0/YMM0/ZMM0 until XMM5/YMM5/ZMM5.

2. After vector types and integer types are allocated, HVA types are
   allocated, in ascending order, to unused vector registers
   XMM0/YMM0/ZMM0 to XMM5/YMM5/ZMM5.

3. Just like in the default x65 CC, Shadow space is allocated for
   vector/HVA types. The size is fixed to 8 bytes per argument.

4. HVA types are returned in XMM0/YMM0/ZMM0 to XMM3/YMM3/ZMM3 while
   vector types are returned in XMM0/YMM0/ZMM0 and integers in RAX

Examples:

typedef struct {

\_\_m128 array[2];

} hva2; // 2 element HVA type on \_\_m128

typedef struct {

\_\_m256 array[4];

} hva4; // 4 element HVA type on \_\_m256

// Example 1: Mixed int and HVA parameters

// Passes a in RCX, c in R8, d in R9, and e pushed on stack.

// Passes b by element in [XMM0:XMM1];

// b's stack shadow area is 8-bytes of undefined value.

// Return value in XMM0.

\_\_m128 \_\_vectorcall example1(int a, hva2 b, int c, int d, int e) {

return b.array[0];

}

// Example 2: Discontiguous HVA

// Passes a in RCX, b in XMM1, d in XMM3, and e is pushed on stack.

// Passes c by element in [YMM0,YMM2,YMM4,YMM5], discontiguous because

// vector arguments b and d were allocated first.

// Shadow area for c is an 8-byte undefined value.

// Return value in XMM0.

float \_\_vectorcall example2(int a, float b, hva4 c, \_\_m128 d, int e)
{

return b;

}

Observations
------------

-  LLVM IR must preserve the original position of the arguments.

-  Since HVA structures are allocated in lower priority than vector
   types, the vector types should be allocated first. Hence, one pass on
   the argument list is not sufficient anymore, because HVA structures
   are allocated on a second pass.

Issue in Clang
--------------

The current clang implementation expends HVA structures into multiple
vector types.

For example:

**C code:** *int \_\_regcall foo(hva3 a);*

**LLVM IR Output:** *define x86\_regcallcc i32 @foo(\_\_m256 %a.0,
\_\_m256 %a.1, \_\_m256 %a.2); *

\*The example omits the decoration that is added to the function name

Thus the backend can't differentiate between expended HVA structures and
simple vector types, and doesn't know the original position of each
parameter in the argument list.

We cannot rely on debug information or updated argument names to
identify HVA structures.

My proposal
-----------

Clang Updates
~~~~~~~~~~~~~

The following section suggests the required changes to Clang.

Passing arguments
^^^^^^^^^^^^^^^^^

    The ABI in LLVM IR must provide argument position. The information
    is important in order to allocate the correct physical register.

    The information can be achieved by passing HVA structures by value.
    It will replace the existing expansion of the HVA structure
    arguments.

    For Example:

    Instead of: *define x86\_regcallcc i32 @foo(\_\_m256 %a.0, \_\_m256
    %a.1, \_\_m256 %a.2);*

    Pass the following: *define x86\_regcallcc i32 @foo(%struct.hva3
    %a);*

    CodeGen needs to know if the structure is an HVA.

    There are two options:

-  CodeGen will analyze the structures just like currently done in clang
   in order to identify HVA structures

-  Clang will pass an attribute that will indicate if this is an HVA
   structure that should be expended and passed in register. Such an
   attribute could be an existing (Like InReg) or a new attribute.

    I believe that second option is preferred. Since InReg attribute is
    target specific and it is not used for other purposes, I think we
    can use it. Also it implies that the structure should be passed in
    register.

Returning Arguments
^^^^^^^^^^^^^^^^^^^

    Returned HVA structures should be expended by clang.

CodeGen Updates
~~~~~~~~~~~~~~~

The following section suggests the required changes to CodeGen.

Passing Arguments
^^^^^^^^^^^^^^^^^

    The following changes should be applied to the argument analysis
    rounds.

    Add a new method to CCState:

    - A new method will check if a register is a shadow register

    -- It will check if it exists in CCState.Locs list

    *The algorithm*

    It uses a new internal flag (second\_vectorcall\_round) in CodeGen
    to mark the seocnd round of vectorcall.

    It uses a new internal flag (hva\_start) to mark the first vector in
    an HVA structure.

    Two parameter analysis rounds (instead of one) will be performed:

-  On the first round we assign non HVA fields

-  On the second round we assign HVA fields

    *Detailed algorithm*

    Add the following vectorcall sequence:

    - If the parameter is a vector type (HVA expended or native vector
    type) – call custom function

    -- If second\_vectorcall\_round is set

    --- Assign available XMM register (include shadow registers)

    Clang ensures that a register is available

    --- finish argument round

    -- If this is an HVA (InReg attribute set)

    --- If the parameter is hva\_start

    ---- If original parameter position <= 6

    ----- Assign shadow XMM register

    ---- If original parameter position <= 4

    ----- Assign shadow GPR register

    --- finish argument round

    -- If this is not an HVA (no InReg attribute set)

    --- If original parameter position <= 6

    ---- Assign XMM register

    --- If original parameter position <= 4

    ---- Assign shadow GPR register

    - If the value is assigned to stack

    -- Assign shadow XMM4 register

-  Notice that parameters with position >=5 can be assigned to stack

-  This assignment is for the special case in which the fifth argument
   is passed in stack while the sixth should be pass through XMM5 (the
   sixth XMM)

-  Even if we mark XMM4 as shadow for arguments in position > 5, it will
   not harm the shadow logic since no further (non-shadow) XMM/GPR
   allocations are expected

    The change should be integrated into AnalyzeFormalArguments and
    AnalyzeCallOperands.

    When going over the arguments (in LowerCall and LowerFormalArgument)
    we add a shadow stack space as required.

Returning Arguments
^^^^^^^^^^^^^^^^^^^

    Return calling convention can simply pass return values in XMM
    registers and no major changes are required.

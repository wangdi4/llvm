//===--- IntelCustDocs.td - Documentation for intel-specific attrs --------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

// To test that the documentation builds cleanly, you must run clang-tblgen to
// convert the .td file into a .rst file, and then run sphinx to convert the
// .rst file into an HTML file. After completing testing, you should revert the
// generated .rst file so that the modified version does not get checked in to
// version control.
//
// To run clang-tblgen to generate the .rst file:
// clang-tblgen --gen-intel-cust-docs -I <root>/llvm/clang/include
//   <root>/llvm/clang/include/clang/Basic/intel/IntelCustDocs.td -o
//   <root>/llvm/clang/docs/Intel/IntelCustomizations.rst
//
// To run sphinx to generate the .html files (note that sphinx-build must be
// available on the PATH):
// Windows (from within the clang\docs directory):
//   make.bat html
// Non-Windows (from within the clang\docs directory):
//   make -f Makefile.sphinx html

// The documentation is organized by category.
class DocumentationCategory<string name> {
  string Name = name;
  code Content = [{}];
  code UserContent = [{}];
}

// New categories can be added by adding a new record here.
def DocCatIntelLoops : DocumentationCategory<"Intel Loop Pragmas">;
def DocCatIntelOptimization : DocumentationCategory<"Optimization">;
def DocCatIntelHLSAttributes : DocumentationCategory<"HLS/FPGA Attributes">;
def DocCatIntelHLSPragmas : DocumentationCategory<"HLS/FPGA Pragmas">;
def DocCatIntelOpenMP : DocumentationCategory<"OpenMP">;
def DocCatIntelCompat : DocumentationCategory<"Intel Compatibility">;
def DocCatMSCompat : DocumentationCategory<"Microsoft Compatibility">;

// Specifies the documentation to be associated with the given category.
class Documentation {
  DocumentationCategory Category;
  code Content;
  code UserContent;
  // The Option is optional.  If used it allows splitting based on a tag.
  // This works well for options like IntelCompat which control many small
  // "features".
  string Option = "";
  // Default determines if the feature is on by default when the option is
  // specified. It can still be turned on using the -enable option.
  bit Default = 1;
  // The remaining fields are TBD. Additional string or bit fields can be
  // added (with initializers) or removed here without the need to change
  // the tblgen code.
  string BugTracker = "";
  string Customer = "";
  string RegressionTest = "";
  string Owner = "";
  string CompatVersions = "";
}

// Microsoft Compatibility

def StringCharStarCatchableDocs : Documentation {
  let Category = DocCatMSCompat;
  let Option = "IntelCompat";
  let BugTracker = "tr38183,CQ407554,CMPLRS-25559";
  let RegressionTest = "cfeCpp/tr38183";
  let Owner = "mprice1";
  let CompatVersions = "vs2015";
  let Default = 0;
  let Content = [{
Microsoft allows you to catch a string literal with catch(char \*).
This behavior is off-by-default.

.. code-block:: c++

  int main()
  {
    int R;
    try {
      throw "foo";
    }
    catch (char *) {
      R = 1;
    }
    catch (...) {
      R = 2;
    }
    return R;
  }

.. code-block:: console

  $ cl /nologo /EHsc t.cpp && ./t.exe
  t.cpp
  $ echo $?
  1

.. code-block:: console

  $ clang-cl /nologo /EHsc t.cpp && ./t.exe
  $ echo $?
  2

  }];
  let UserContent = [{
Microsoft allows you to catch a string literal with catch (char *). Disabled
by default.
  }];
}

def AllowFewerMacroArgsDocs : Documentation {
  let Category = DocCatMSCompat;
  let Option = "IntelCompat";
  let BugTracker = "cq365448";
  let RegressionTest = "Preprocessor/intel/missing_macro_args.cpp";
  let Owner = "mprice1";
  let Default = 0;
  let Content = [{
Microsoft allows passing fewer arguments to function-like macros, replacing
each missing argument with the empty string. A warning is given. This
behavior is off-by-default.

.. code-block:: c++

  #define FUNC(x,y) (0)
  void foo() {
    FUNC(0);
  }

.. code-block:: console

  $ cl /c /nologo t.cpp
  t.cpp
  t.cpp(3): warning C4003: not enough actual parameters for macro 'FUNC'

.. code-block:: console

  $ clang-cl /c /nologo t.cpp
  t.cpp(3,11): error: too few arguments provided to function-like macro invocation
      FUNC(0);
          ^
  t.cpp(1,9): note: macro 'FUNC' defined here
  #define FUNC(x,y) (0)
          ^
  t.cpp(3,5): error: use of undeclared identifier 'FUNC'
      FUNC(0);
      ^
  2 errors generated.

.. code-block:: console

  $ clang -cc1 -fintel-compatibility-enable=AllowFewerMacroArgs t.cpp
  t.cpp:3:9: warning: too few arguments provided to function-like macro invocation
    FUNC(0);
          ^
  t.cpp:3:9: warning: 1 missing macro argument treated as empty string
  t.cpp:1:9: note: macro 'FUNC' defined here
  #define FUNC(x,y) (0)
          ^
  2 warnings generated.
  }];
  let UserContent = [{
Microsoft allows passing fewer arguments to function-like macros, replacing
each missing argument with the empty string. A warning is given. This behavior
is off-by-default.
  }];
}

def IntelTBAABFDocs : Documentation {
  let Category = DocCatIntelOptimization;
  let Option = "IntelCompat";
  let BugTracker= "CMPLRLLVM-2066,CMPLRLLVM-4118";
  let RegressionTest = "test/CodeGen/intel/bitfield-tbaa1.c,test/CodeGen/intel/tbaa.cpp";
  let Content = [{
Adds tbaa metadata to improve alias analysis for bitfields.
Optimizer team identified a problem with leela benchmark due to missing
TBAA metadata for bitfield load.  When you pick up (or store) a storage
location which holds a bitfield, there are other bitfields which may share
that location.  Optimizer team wants there to be metadata on the bitfield
access, and it should be identical for all bitfields that share the same
storage location. CJ suggested that essentially there is a hidden field
in the struct, matching the size of the storage location loaded, that is
accessed.

With the option disabled, there is no tbaa on the bitfield load or store.

In this example, xage and zage2 are in the same storage location so they
use the same tbaa node.

.. code-block:: c

  struct in_t {
      int x,y,w;
      unsigned char  xage : 2;
      unsigned char  yage1 : 2;
      unsigned int  zage2 : 12;
  };
  struct in_t *sp
  sp->xage = 2;  sp->zage2 = 2;

.. code-block:: llvm

  %xage = getelementptr ...
  %bf.load = load i16, i16* %xage, align 4, !tbaa !6
  %zage2 = getelementptr ...
  %bf.load1 = load i16, i16* %zage2, align 4, !tbaa !6

Note: 12 is the byte offset of the (short) storage location holding
xage and zage2.

.. code-block:: llvm

  !4 = !{!"omnipotent char", !5, i64 0}
  !5 = !{!"Simple C/C++ TBAA"}
  !6 = !{!7, !9, i64 12}
  !7 = !{!"struct@in_t", ... }
  !9 = !{!"short", !4, i64 0}

  }];
  let UserContent = [{
When loading (or storing) a bitfield, other bitfields may share the same
location, which inhibits optimization. This option enables type-based alias
analysis, which allows the compiler to better optimize these references.
  }];
}
def FArgumentNoaliasDocs : Documentation {
  let Category = DocCatIntelOptimization;
  let Option = "IntelCompat";
  let BugTracker= "CMPLRLLVM-4138";
  let RegressionTest = "test/CodeGen/intel/fargument-noalias.c";
  let Content = [{
This patch adds support for the gcc- and icc- -fargument-noalias option.
It sets the internal CodeGen option: NoAliasForPtrArgs and adds the
NoAlias attribute on each parameter.
Example:

.. code-block:: c

  void test1(int *a, int *b) {
      a = b;
  }

IR:

.. code-block:: console

  $ clang -fargument-noalias -emit-llvm test.c -o -

.. code-block:: llvm

  ; Function Attrs: noinline nounwind optnone
  define void @test1(i32* noalias %a, i32* noalias %b) #0 {

  }];
  let UserContent = [{
Add support for -fargument-noalias, which speicfies that arguments do not
alias each other.
  }];
}

def MSVC2015TriviallyConstructibleBehaviorDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let RegressionTest = "SemaCXX/intel/cmplrs-7448.cpp";
  let Owner = "zahiraam";
  let Default = 0;
  let Content = [{
In Microsoft support for trivially copy constructable class with
destructor was enabled for MSVC2017 (and up).

.. code-block:: c++

   template<typename T, typename ...Args>
   struct itc {
     static const bool v = __is_trivially_constructible(T, Args...);
   };

   class C1 {
   public:
	C1(const C1&) = default;
	~C1(){}
   };

   static_assert(!itc<C1, const C1&>::v, "not a trivially copy constructable");
   static_assert(!itc<C1, C1&&>::v, "not a trivially copy constructable");

.. code-block:: console

   $ cl t1.cpp
   Microsoft (R) C/C++ Optimizing Compiler Version 19.00.24215.1 for x64
   Copyright (C) Microsoft Corporation.  All rights reserved.

   t1.cpp
   t1.cpp(12): error C2338: not a trivially copy constructable
   t1.cpp(13): error C2338: not a trivially copy constructable

   $ cl -c t1.cpp
   Microsoft (R) C/C++ Optimizing Compiler Version 19.16.27045 for x64
   Copyright (C) Microsoft Corporation.  All rights reserved.

   t1.cpp
  }];
  let UserContent = [{
In Microsoft support for trivially copy constructable class with
destructor was enabled for MSVC2017 (and up).
  }];
}

// Intel Compatibility

def InstantiateDefaultArgsDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker = "CMPLRLLVM-18148";
  let RegressionTest = "test/CodeGenCXX/intel/default-arg-closures.cpp";
  let Owner = "zahiraam";
  let Default = 1;
  let Content = [{
The MS ABI requires that the default argument of a default constructor be
instantiated.

Syntax:

.. code-block:: c++

   template <typename>
   class __declspec(dllexport) foo {
     foo(int x = 0);
   };
   template <> foo<int>::foo(int);

IR:

.. code-block:: console

   $ clang -cc1 -fintel-compatibility
     -fintel-compatibility-enable=InstantiateDefaultArgs -emit-obj t1.cpp

.. code-block:: text

   $ dumpbin /symbols t1.o | grep foo
   008 00000000 SECT4  notype ()    External     | ??4?$foo@H@@QEAAAEAV0@AEBV0@@Z (public: class foo<int> & __cdecl foo<int>::operator=(class foo<int> const &))
   00B 00000000 SECT5  notype ()    External     | ??_F?$foo@H@@QEAAXXZ (public: void __cdecl foo<int>::`default constructor closure'(void))
   013 00000000 UNDEF  notype       External     | ??0?$foo@H@@QEAA@H@Z (public: __cdecl foo<int>::foo<int>(int))

  }];
  let UserContent = [{
Instantiate default arguments for exported default constructors.
  }];
}

def AllowMissingTypenameDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker = "CQ#368310,CMPLRS-25573";
  let RegressionTest = "cfeCpp/cq117771";
  let Owner = "";
  let CompatVersions = "";
  let Default = 0;
  let Content = [{
Microsoft allows you to leave off typename in some contexts.  Clang also
allows this in MSVCCompat mode.  See isMicrosoftMissingTypename in
SemaDecl.cpp. This behavior is off-by-default.

The Intel compiler allows this in more cases than Clang and in some cases
more than Microsoft.

Recommendation: Run full testing without this customization.  If tests fail
the same with Microsoft and GCC remove it.

.. code-block:: c++

  template <class T>
  struct X {
    typedef T type1;
    type1 foo2();
  };

  template<class T>
  X<T>::type1 X<T>::foo2() {
    type1 t;
    return t;
  }

  int main() {
    X<float> x;
    return x.foo2();
  }

.. code-block:: console

  $ clang-cl -c -GX -GR  -Tpt.cpp
  t.cpp(8,1):  error: missing 'typename' prior to dependent type name 'X<T>::type1'
  X<T>::type1 X<T>::foo2() {
  ^~~~~~~~~~~
  typename
  1 error generated.

.. code-block:: console

  [0:windev-lab:test]$ icx -c -nologo t.cpp
  t.cpp
  t.cpp:8:1: warning: missing 'typename' prior to dependent type name 'X<T>::type1' [-Wtypename-missing]
  X<T>::type1 X<T>::foo2() {
  ^~~~~~~~~~~
  typename
  1 warning generated.

  }];
  let UserContent = [{
Enables Microsoft compatibility by allowing a missing typename in some contexts.
  }];
}

def AllowExtraArgumentDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker = "CQ#364630,CMPLRS-15158";
  let RegressionTest = "c9xcC/c9x_7_24_4_5_8_10_a,...";
  let Owner = "";
  let CompatVersions = "";
  let Default = 0;
  let Content = [{
Microsoft allows the call with extra arguments with warning in C. Clang and gcc emit error.

The Intel compiler allows this with warning in C. This behavior is off-by-default.

.. code-block:: c

  void foo(int *arg1, const int *arg2);

  void bar()
  {
    int a1[20];
    int a2[20];
    int offset1, offset2;
    int arg3;

    foo(a1+offset1, a2+offset2, &arg3);
  }

.. code-block:: console

  $ clang x.c
  x.c:10:31: error: too many arguments to function call, expected 2, have 3
    foo(a1+offset1, a2+offset2, &arg3);
    ~~~                         ^~~~~
  x.c:1:1: note: 'foo' declared here

  void foo(int *arg1, const int *arg2);
  ^
  1 error generated.

.. code-block:: console

  $ icx x.c -c
  x.c:10:31: warning: too many arguments to function call, expected 2, have 3
        [-Wintel-compat]
    foo(a1+offset1, a2+offset2, &arg3);
    ~~~                         ^~~~~
  x.c:1:1: note: 'foo' declared here
  void foo(int *arg1, const int *arg2);
  ^
  1 warning generated.

.. code-block:: console

  $ cl l:/x.c -c
  Microsoft (R) C/C++ Optimizing Compiler Version 19.10.25019 for x64
  Copyright (C) Microsoft Corporation.  All rights reserved.

  x.c
  l:/x.c(10): warning C4020: 'foo': too many actual parameters

  }];
  let UserContent = [{
Microsoft allows, with warnings, extra arguments to function calls. Disabled
by default (generates errors).
  }];
}

def VaArgPackDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker= "CMPLRS-8519";
  let RegressionTest = "regC/cq523159,regC/cq234106";
  let Owner = "ekeane1";
  let CompatVersions = "";
  let Content = [{
GCC 4.3 and newer supports __builtin_va_arg_pack and __builtin_va_arg_pack_len,
two builtins that enable forwarding of variadic arguments to a function.  These
builtins are rarely used, however glibc's buffer overflow protection scheme
(enabled by defining _FORTIFY_SOURCE) implementation depends on these two
builtins when the detected GCC version is 4.3 or newer.

These builtin functions are severely limited as to when they can be used. They
can only be used inside of an inline, variadic function that also forces
inlining (such as with attribute always_inline or gnu_inline). Additionally,
__builtin_va_arg_pack can only be used as a parameter to a variadic argument.

__builtin_va_arg_pack() forwards the variadic argument pack and
__builtin_va_arg_pack_len() returns the variadic argument count.

A correct usage of these builtin functions are as follows:

.. code-block:: c

  static inline __attribute__((always_inline))
  int safer_printf(const char * format, ...) {
    // If number of variadic arguments doesn't match the number requested in the
    // format string, teriminate.
    if(get_format_arg_count(format) != __builtin_va_arg_pack_len())
      abort();
    // Check passed, call normal printf.
    return printf(format, __builtin_va_arg_pack());
  }

  }];
  let UserContent = [{
Support GCC __builtin_va_arg_pack and __builtin_va_arg_pack_len builtins.
  }];
}

def PragmaInlineDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Option = "IntelCompat";
  let BugTracker= "CMPLRLLVM-22959";
  let RegressionTest = "";
  let Owner = "zahiraam";
  let Content = [{
The ``inline`` pragmas and its options are pragmas used at call point. It gives
a hint to the compiler that called function is to be inlined, no inlined and
sometimes forced inline. Each pragma can take the option ``recursive`` to
indicate that the pragma holds for each callee of the called function.

Syntax:

.. code-block:: text

  #pragma inline [recursive]
  #pragma forceinline [recursive]
  #pragma noinline

Example:

.. code-block:: c++

  void foo() { }

  void bar1() {
    foo();
  }

  void bar2() {
  #pragma inline
    foo();
  }

  void bar3() {
  #pragma noinline
    foo();
  }

  void bar4() {
  #pragma forceinline
    foo();
  }

  void bar5() {
  #pragma inline recursive
    foo();
  }

  void bar6() {
  #pragma forceinline recursive
    foo();
  }

AST:

.. code-block:: console

  FunctionDecl 0x1209c1f8 <line:7:1, line:10:1> line:7:6 bar2 'void ()'
   -CompoundStmt 0x1209c388 <col:13, line:10:1>
     -AttributedStmt 0x1209c370 <<invalid sloc>, line:9:7>
       -IntelInlineAttr 0x1209c348 <line:8:9> Implicit inline NotRecursive
       -CallExpr 0x1209c328 <line:9:3, col:7> 'void'

  FunctionDecl 0x1209c3c0 <line:12:1, line:15:1> line:12:6 bar3 'void ()'
   -CompoundStmt 0x1209c550 <col:13, line:15:1>
     -AttributedStmt 0x1209c538 <<invalid sloc>, line:14:7>
       -IntelInlineAttr 0x1209c510 <line:13:9> Implicit noinline NotRecursive
       -CallExpr 0x1209c4f0 <line:14:3, col:7> 'void'

  FunctionDecl 0x1209c588 <line:17:1, line:20:1> line:17:6 bar4 'void ()'
   -CompoundStmt 0x1209c718 <col:13, line:20:1>
     -AttributedStmt 0x1209c700 <<invalid sloc>, line:19:7>
       -IntelInlineAttr 0x1209c6d8 <line:18:9> Implicit forceinline NotRecursive
       -CallExpr 0x1209c6b8 <line:19:3, col:7> 'void'

  FunctionDecl 0x1209c750 <line:22:1, line:25:1> line:22:6 bar5 'void ()'
   -CompoundStmt 0x1209c8f0 <col:13, line:25:1>
     -AttributedStmt 0x1209c8d8 <<invalid sloc>, line:24:7>
       -IntelInlineAttr 0x1209c8b0 <line:23:9, col:16> Implicit inline Recursive
       -CallExpr 0x1209c890 <line:24:3, col:7> 'void'

  FunctionDecl 0x1209c928 <line:27:1, line:30:1> line:27:6 bar6 'void ()'
   -CompoundStmt 0x1209cac8 <col:13, line:30:1>
     -AttributedStmt 0x1209cab0 <<invalid sloc>, line:29:7>
       -IntelInlineAttr 0x1209ca88 <line:28:9, col:21> Implicit forceinline Recursive
       -CallExpr 0x1209ca68 <line:29:3, col:7> 'void'

IR:

.. code-block:: console

  $ clang -cc1 -disable-llvm-passes -fintel-compatibility -emit-llvm inline.cpp -O2 -o inline.ll

.. code-block:: llvm

  define void @_Z4bar2v() {
  entry:
    call void @_Z3foov() #1
    ret void
  }

  define void @_Z4bar3v() {
  entry:
    call void @_Z3foov() #2
    ret void
  }

  define void @_Z4bar4v() {
  entry:
    call void @_Z3foov() #3
    ret void
  }

  define void @_Z4bar5v() {
  entry:
    call void @_Z3foov() #4
    ret void
  }

  define void @_Z4bar6v() {
  entry:
    call void @_Z3foov() #5
    ret void
  }

  attributes #1 = { inlinehint }
  attributes #2 = { noinline }
  attributes #3 = { alwaysinline }
  attributes #4 = { inlinehint_recursive }
  attributes #5 = { alwaysinline_recursive }
  }];
  let UserContent = [{
Pragma inline gives a hint to the compiler that the called function and its
callees are to be inlined, not inlined, forced inlined according to its keyword
argument 'recursive'.
  }];
}

def PragmaNoVectorDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Option = "IntelCompat";
  let BugTracker= "";
  let RegressionTest = "test/Sema/intel/vector.cpp,test/CodeGenCXX/intel/vector.cpp";
  let Owner = "jyu2";
  let CompatVersions = "";
  let Content = [{
The ``novector`` pragma specifies that a particular loop should never be
vectorized, even if it is legal to do so. When avoiding vectorization of a
loop is desirable (when vectorization results in a performance regression
rather than improvement), the ``novector`` pragma can be used in the source
text to disable vectorization of a loop. This behavior is in contrast to the
``vector always`` pragma.

This pragma is implemented as an additional spelling for the existing
community-supported ``pragma vector(disable)``.

Syntax:

.. code-block:: text

  #pragma novector

Example:

.. code-block:: c

  void foo(int lb, int ub, int *a, int *b) {
    #pragma novector
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -ast-dump t.c

.. code-block:: text

  AttributedStmt 0xedc1b28 <<invalid sloc>, line:3:31>
  |-LoopHintAttr 0xedc1b00 <line:2:11> Implicit novector Vectorize Disable

IR:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2

  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.vector.vectorize.width", i32 1}
  }];
  let UserContent = [{
Accept #pragma novector to indicate loop should never be vectorized.
  }];
}

def PragmaVectorDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Option = "IntelCompat";
  let BugTracker= "";
  let RegressionTest = "test/Sema/intel/vector.cpp,test/CodeGenCXX/intel/vector.cpp";
  let Owner = "jyu2";
  let CompatVersions = "";
  let Content = [{
The ``pragma vector`` indicates to the compiler that the loop should be
vectorized according to the argument keywords,
'assert', 'aligned', 'unaligned', 'vecremainder', and 'novecremainder'.


Syntax:

.. code-block:: text

  #pragma vector {always[assert]|aligned|unaligned|temporal|nontemporal|[no]vecremainder|[no]mask_readwrite|[no]dynamic_align|[no]vecremainder|vectorlength(n1[, n2]...)}

``pragma vector`` with no other keyword will do nothing.

Currently we only support ``pragma vector``, ``pragma vector always``,
``pragma vector aligned``, ``pragma vector dynamic_align``,
``pragma vector nodynamic_align``, ``pragma vector vecremainder``,
``pragma vector novecremainder``, ``pragma vector always assert``,
``pragma vector temporal``, ``pragma vecotor nontemporal`` and
``pramga vector vectorlength(n1[,n2]...)``.
When the 'aligned' keyword is used, it indicates to the compiler that the loop
should be vectorized using aligned data movement instructions for all array
references inside the loop.

Example:

.. code-block:: c

  void foo(int lb, int ub, int *a, int *b) {
    #pragma vector
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
    #pragma vector always
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
    #pragma vector aligned
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
    #pragma vector dynamic_align
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
    #pragma vector nodynamic_align
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
    #pragma vector vecremainder
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
    #pragma vector novecremainder
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
    #pragma vector always assert
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
    #pragma vector temporal
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
    #pragma vector nontemporal
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
    #pragma vector vectorlength(2,4)
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -ast-dump t.c

.. code-block:: text

  AttributedStmt 0x2d888a36e48 <line:2:11, line:3:46>
  | |-LoopHintAttr 0x2d888a36e08 <line:2:11> Implicit vector Vectorize Enable
  | | |-<<<NULL>>>
  | | `-<<<NULL>>>
  | `-ForStmt 0xed8d090 <line:3:3, col:46>

  AttributedStmt 0x2d888a372c8 <line:4:11, line:5:46>
  |-LoopHintAttr 0x2d888a37288 <line:4:11> Implicit vector VectorizeAlways Enable
  | |-<<<NULL>>>
  | `-<<<NULL>>>
  `-ForStmt 0xed8d5a0 <line:5:3, col:46>

  AttributedStmt 0x2d888a37748 <line:6:11, line:7:46>
  |-LoopHintAttr 0x2d888a37708 <line:6:11> Implicit vector VectorizeAligned Enable
  | |-<<<NULL>>>
  | `-<<<NULL>>>


  AttributedStmt 0x2d888a37748 <line:6:11, line:7:46>
  |-LoopHintAttr 0x2d888a37708 <line:6:11> Implicit vector VectorizeDynamicAlign Enable
  | |-<<<NULL>>>
  | `-<<<NULL>>>

  AttributedStmt 0x2d888a37748 <line:6:11, line:7:46>
  |-LoopHintAttr 0x2d888a37708 <line:6:11> Implicit vector VectorizeNoDynamicAlign Enable
  | |-<<<NULL>>>
  | `-<<<NULL>>>
  AttributedStmt 0x2d888a37748 <line:8:11, line:9:46>
  |-LoopHintAttr 0x2d888a37708 <line:8:11> Implicit vector VectorizeVecremainder Enable
  | |-<<<NULL>>>
  | `-<<<NULL>>>

  AttributedStmt 0x2d888a37748 <line:10:11, line:11:46>
  |-LoopHintAttr 0x2d888a37708 <line:10:11> Implicit vector VectorizeNoVecremainder Enable
  | |-<<<NULL>>>
  | `-<<<NULL>>>

  -AttributedStmt 0x120c38f8 <<invalid sloc>, line:17:48>
      |-LoopHintAttr 0x120c38b8 <line:16:13, col:27> Implicit vector VectorizeAlwaysAssert Enable
      | |-<<<NULL>>>
      | `-<<<NULL>>>

  -AttributedStmt 0x110e2c38 <<invalid sloc>, line:19:48>
      |-LoopHintAttr 0x110e2bf8 <line:18:13, col:27> Implicit vector VectorizeTemporal Enable
      | |-<<<NULL>>>
      | `-<<<NULL>>>

  -AttributedStmt 0x110e30e0 <<invalid sloc>, line:21:48>
      |-LoopHintAttr 0x110e30a0 <line:20:13, col:27> Implicit vector VectorizeNonTemporal Enable
      | |-<<<NULL>>>
      | `-<<<NULL>>>

  -AttributedStmt 0x110e3620 <<invalid sloc>, line:23:48>
      |-LoopHintAttr 0x110e35a0 <line:22:20, col:34> Implicit vector VectorizeLength Numeric
      | |-IntegerLiteral 0x110e3118 <col:33> 'int' 2
      | `-<<<NULL>>>
      |-LoopHintAttr 0x110e35e0 <col:20, col:36> Implicit vector VectorizeLength Numeric
      | |-IntegerLiteral 0x110e3158 <col:35> 'int' 4
      | `-<<<NULL>>>

Note that ``#pragma vector always`` and ``#pragma vector aligned`` are
represented as two LoopHint attributes.

IR:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond6, !llvm.loop !2
  br label %for.cond6, !llvm.loop !4
  br label %for.cond20, !llvm.loop !7
  br label %for.cond34, !llvm.loop !9
  br label %for.cond48, !llvm.loop !11
  br label %for.cond62, !llvm.loop !13
  br label %for.cond76, !llvm.loop !15
  br label %for.cond90, !llvm.loop !17
  br label %for.cond104, !llvm.loop !19
  br label %for.cond118, !llvm.loop !21

  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.mustprogress"}
  !4 = distinct !{!4, !3, !5, !6}
  !5 = !{!"llvm.loop.vectorize.ignore_profitability"}
  !6 = !{!"llvm.loop.vectorize.enable", i1 true}
  !7 = distinct !{!7, !3, !8}
  !8 = !{!"llvm.loop.intel.vector.aligned"}
  !9 = distinct !{!9, !3, !10}
  !10 = !{!"llvm.loop.intel.vector.dynamic_align", !"true"}
  !11 = distinct !{!11, !3, !12}
  !12 = !{!"llvm.loop.intel.vector.nodynamic_align", !"true"}
  !13 = distinct !{!13, !3, !14}
  !14 = !{!"llvm.loop.intel.vector.vecremainder", !"true"}
  !15 = distinct !{!15, !3, !16}
  !16 = !{!"llvm.loop.intel.vector.novecremainder", !"true"}
  17 = distinct !{!17, !3, !5, !18, !6}
  !18 = !{!"llvm.loop.intel.vector.assert"}
  !19 = distinct !{!19, !3, !20}
  !20 = !{!"llvm.loop.intel.vector.temporal"}
  !21 = distinct !{!21, !3, !22}
  !22 = !{!"llvm.loop.intel.vector.nontemporal"}
  !23 = distinct !{!23, !3, !24}
  !24 = !{!"llvm.loop.intel.vector.vectorlength", i32 2, i32 4}
  }];
  let UserContent = [{
Accept #pragma vector - indicates a loop should be vectorized according to the
argument keywords 'assert', 'aligned', 'always assert', 'dynamic_align',
'nodynamic_align', 'vecremainder', 'novecremainder', 'temporal',
'nontemporal' and 'vectorlength'.
  }];
}

def PragmaFusionDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Option = "IntelCompat";
  let Owner = "pmrao";
  let Content = [{
The ``fusion`` pragma instructs the compiler to fuse the loop with adjacent
loops.

Syntax:

.. code-block:: text

  #pragma fusion

Example:

.. code-block:: c

  void foo() {
    #pragma fusion
    for (int i=0; i<1024;++i) { }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -ast-dump t.c

.. code-block:: text

  AttributedStmt 0xee32b28 <<invalid sloc>, line:3:31>
  |-LoopHintAttr 0xee32b00 <line:2:11> Implicit fusion Fusion Enable
  | |-<<<NULL>>>
  | `-<<<NULL>>>
  `-ForStmt 0xee32ac8 <line:3:3, col:31>

IR:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2
  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.fusion.enable"}

  }];
  let UserContent = [{
Pragma fusion instructs the compiler to fuse the following loop with adjacent
loops.
  }];
}

def PragmaNoFusionDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Option = "IntelCompat";
  let Owner = "pmrao";
  let Content = [{
The ``nofusion`` pragma instructs the compiler not to fuse the loop with
adjacent loops.

Syntax:

.. code-block:: text

  #pragma nofusion

Example:

.. code-block:: c

  void foo() {
    #pragma nofusion
    for (int i=0; i<1024;++i) { }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -ast-dump t.c

.. code-block:: text

  AttributedStmt 0xda3cb28 <<invalid sloc>, line:3:31>
  |-LoopHintAttr 0xda3cb00 <line:2:11> Implicit nofusion Fusion Disable

IR:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2
  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.fusion.disable"}
  }];
  let UserContent = [{
The ``nofusion`` pragma instructs the compiler not to fuse the loop with
adjacent loops.
  }];
}

def UnrollZeroDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker = "cq366562";
  let Owner = "zahiraam";
  let Default = 1;
  let Content = [{
Clang doesn't allow an unroll value of zero. We allow that as Fortran uses this
feature. It will have the same semantics as nounroll, an 'llvm.loop.fusion.disable'
attribute is attached to the loop.

Syntax:

.. code-block:: c

   int main(void) {
     int i = 0, s = 0;

     #pragma unroll(0)
     for (i = 0; i < 10; ++i)
       s = s + i;
     return s;
   }

IR:

.. code-block:: console

   $ clang -cc1 -fintel-compatibility -fintel-compatibility-enable=UnrollZero  -emit-llvm t1.c

.. code-block:: llvm

   br label %for.cond, !llvm.loop !2
   !2 = distinct !{!2, !3}
   !3 = !{!"llvm.loop.unroll.disable"}

.. code-block:: console

   $ clang -cc1 -fintel-compatibility -fintel-compatibility-disable=UnrollZero  -emit-llvm t1.c

.. code-block:: text

   t1.c:4:16: error: invalid value '0'; must be positive
   #pragma unroll(0)
               ^
   1 error generated.
  }];
  let UserContent = [{
Allow unroll value of zero, withe same semantics as nounroll.
  }];
}

def PragmaUnrollAndJamDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Option = "IntelCompat";
  let Owner = "pmrao";
  let Content = [{
The ``unroll_and_jam`` pragma instructs the compiler to enable unrolling and
jamming loops.

Syntax:

.. code-block:: text

  #pragma unroll_and_jam [(<integral-constant>)]

Example:

.. code-block:: c

  void foo() {
    #pragma unroll_and_jam
    for(int i=0;i<8;++i) {}
    #pragma unroll_and_jam(4)
    for(int i=0;i<8;++i) {}
  }

AST:

.. code-block:: console

  $ clang -cc1 -ast-dump -o - -fintel-compatibility t.c

.. code-block:: text

  AttributedStmt
  |-LoopHintAttr Implicit unroll_and_jam UnrollAndJam Enable

  AttributedStmt 0xe3d1e28 <<invalid sloc>, line:6:25>
  |-LoopHintAttr 0xe3d1e00 <line:5:11, col:28> Implicit unroll_and_jam UnrollAndJamCount Numeric
  | |-IntegerLiteral 0xe3d1c18 <col:26> 'int' 4

IR:

.. code-block:: console

  $ clang -cc1 -emit-llvm -o - -fintel-compatibility t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2

  br label %for.cond2, !llvm.loop !4

  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.unroll_and_jam.enable"}
  !4 = distinct !{!4, !5}
  !5 = !{!"llvm.loop.unroll_and_jam.count", i32 4}
  }];
  let UserContent = [{
The ``unroll_and_jam`` pragma instructs the compiler to enable unrolling and
jamming loops.
  }];
}

def PragmaNoUnrollAndJamDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Option = "IntelCompat";
  let Owner = "pmrao";
  let Content = [{
The ``nounroll_and_jam`` pragma instructs the compiler to disable unrolling
and jamming loops.

Syntax:

.. code-block:: text

  #pragma nounroll_and_jam

Example:

.. code-block:: c

  void foo() {
    #pragma nounroll_and_jam
    for (int i=0; i<1024;++i) { }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -ast-dump t.c

.. code-block:: text

  AttributedStmt 0xd992b28 <<invalid sloc>, line:3:31>
  |-LoopHintAttr 0xd992b00 <line:2:11> Implicit nounroll_and_jam UnrollAndJam Disable

IR:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2
  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.unroll_and_jam.disable"}
  }];
  let UserContent = [{
The ``nounroll_and_jam`` pragma instructs the compiler to disable unrolling
and jamming loops.
  }];
}

def PragmaBlockLoopDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Option = "IntelCompat";
  let RegressionTest = "Parser/intel/block_loop.cpp, Sema/intel/block_loop.cpp, CodeGenCXX/intel/block_loop.cpp, Parser/intel/noblock_loop.cpp, Sema/intel/noblock_loop.cpp, CodeGenCXX/intel/noblock_loop.cpp";
  let Owner = "jyu2,smanna";
  let Content = [{
The ``block_loop`` pragma lets you exert greater control over optimizations
on a specific loop inside a nested loop. The ``noblock_loop`` pragma tunes the
performance by disabling loop blocking for nested loops.

Syntax:

.. code-block:: text

  #pragma block_loop [level(levels)] [private(vars)] [factor(expr)]
  #pragma noblock_loop

Implements a new attribute (IntelBlockLoopAttr) instead of using the LoopHint
attribute framework used.  LoopHintAttrs create metadata but this pragma
generates only directives.

Example:

.. code-block:: c

  void foo(int var, void (*bar)(int)) {
    #pragma block_loop level(1:2) factor(256) private(var)
    for (int i=0; i<1024;++i)
    for (int j=0; j<512; ++j) { bar(var); }
  }

  void bar(int i, int *x, int *y) {
    #pragma noblock_loop
     for (i = 0; i < 10; ++i) {
       x[i] = y[i];
     }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -ast-dump -o - t.c
  $ clang -cc1 -fintel-compatibility -ast-dump -o - t1.c

.. code-block:: text

  AttributedStmt 0xdb151f8 <<invalid sloc>, line:5:41>
  |-IntelBlockLoopAttr 0xdb15190 <line:3:11, col:56> Implicit block_loop 1 2
  | |-IntegerLiteral 0xdb14cb0 <col:40> 'int' 256
  | |-IntegerLiteral 0xdb14cb0 <col:40> 'int' 256
  | `-DeclRefExpr 0xdb14d30 <col:53> 'int' lvalue ParmVar 0xdab48c0 'var' 'int'
  `-ForStmt 0xdb15158 <line:4:3, line:5:41>

   AttributedStmt 0x1091f290 <<invalid sloc>, line:5:3>
  |-IntelBlockLoopAttr 0x1091f220 <line:2:9> Implicit noblock_loop -1
  | `-<<<NULL>>>
  `-ForStmt 0x1091f1e0 <line:3:3, line:5:3>

IR:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t.c
  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t1.c

.. code-block:: llvm

  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.PRIVATE"(i32* %var.addr), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 256), "QUAL.PRAGMA.LEVEL"(i32 2), "QUAL.PRAGMA.FACTOR"(i32 256) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]

  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 -1), "QUAL.PRAGMA.FACTOR"(i32 0) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  }];
  let UserContent = [{
The ``block_loop`` pragma lets you exert greater control over optimizations on
a specific loop inside a nested loop. The ``noblock_loop`` pragma lets you tune
the performance by disabling loop blocking for nested loops.
  }];
}

def PragmaPrefetchDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let RegressionTest = "CodeGenCXX/intel/prefetch.cpp, CodeGenCXX/intel/prefetch_messages.cpp";
  let Owner = "dpagan";
  let Content = [{
The ``prefetch`` pragma enables data prefetching for some memory references.
Prefetching data can minimize the effects of memory latency. The
``noprefetch`` disables data prefetching for some memory references.

Syntax:

.. code-block:: text

  #pragma prefetch [var1 [: hint1 [: distance1]] [, var2 [: hint2 [: distance2]]]...]
  #pragma noprefetch [var1, [var2, [varN, ... ]]]

Implements a new attribute (IntelPrefetchAttr) instead of using the LoopHint
attribute framework. LoopHintAttrs create metadata but this pragma generates
only directives.

Example:

.. code-block:: c

  void *  sub(float *A, float *B, float *C,  int N) {

    int i;
    #pragma noprefetch C
    #pragma prefetch A:1:40
    #pragma prefetch B
    for (i=0; i< N; i++) {
      A[i] = B[i] + C[i] *  2.0;
    }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -ast-dump -o - t.c

.. code-block:: text

  AttributedStmt 0x11356a98 <<invalid sloc>, line:10:3>
  |-IntelPrefetchAttr 0x113569d0 <line:5:9, col:21> Implicit noprefetch
  | `-DeclRefExpr 0x113564f0 <col:20> 'float *' lvalue ParmVar 0x113561c0 'C' 'float *'
  |-IntelPrefetchAttr 0x11356a08 <line:6:9, col:24> Implicit prefetch
  | |-DeclRefExpr 0x11356520 <col:18> 'float *' lvalue ParmVar 0x113560c0 'A' 'float *'
  | |-IntegerLiteral 0x11356540 <col:20> 'int' 1
  | `-IntegerLiteral 0x11356560 <col:22> 'int' 40
  |-IntelPrefetchAttr 0x11356a58 <line:7:9, col:19> Implicit prefetch
  | `-DeclRefExpr 0x11356590 <col:18> 'float *' lvalue ParmVar 0x11356140 'B' 'float *'
  `-ForStmt 0x11356998 <line:8:3, line:10:3>

IR:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t.c

.. code-block:: llvm

  %1 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ATTRIB"(i32 0), "QUAL.PRAGMA.VAR"(float** %C.addr) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ATTRIB"(i32 1), "QUAL.PRAGMA.VAR"(float** %A.addr), "QUAL.PRAGMA.HINT"(i32 1), "QUAL.PRAGMA.DISTANCE"(i32 40) ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ATTRIB"(i32 1), "QUAL.PRAGMA.VAR"(float** %B.addr) ]
  store i32 0, i32* %i, align 4, !tbaa !6
  br label %for.cond

  call void @llvm.directive.region.exit(token %1) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  }];
  let UserContent = [{
The ``prefetch`` pragma enables data prefetching for memory references
specified with the pragma, or all suitable memory references if no arguments
were specified. Prefetching data can minimize the effects of memory latency.
The ``noprefetch`` pragma disables data prefetching for memory references
specified with the pragma, or all memory references if no arguments
were specified with the pragma.
  }];
}

def PragmaLoopCountDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Option = "IntelCompat";
  let RegressionTest = "Parser/intel/loop_count.cpp, Sema/intel/loop_count.cpp, CodeGen/intel/loop_count.cpp";
  let BugTracker = "CMPLRLLVM-5991, CMPLRLLVM-1022";
  let Owner = "jyu2";
  let Content = [{
The ``loop_count`` pragma specifies the minimum, maximum, or average number of
iterations for a for loop. In addition, a list of commonly occurring values can
be specified to help the compiler generate multiple versions and perform
complete unrolling.

Syntax:

.. code-block:: text

  constant-int-list := (n1[,n2]...) | = n1[,n2]...
  minmod := min(n) | min=n
  maxmod := max(n) | max=n
  avgmid := avg(n) | avg=n

  loopcount_component := constant-int-list | minmod | maxmod | avgmod

  #pragma loop_count loopcount_component [loopcount_component]...

.. csv-table::
  :header: Syntax, Option, State, Metadata
  :widths: 24,13,13,25

  #pragma loop_count(4), LoopCount, Numeric, llvm.loop.intel.loopcount(4)
  #pragma loop_count min(4), LoopCountMin, Numeric, llvm.loop.intel.loopcount_minimum(4)
  #pragma loop_count max(4), LoopCountMax, Numeric, llvm.loop.intel.loopcount_maximum(4)
  #pragma loop_count avg(4), LoopCountAvg, Numeric, llvm.loop.intel.loopcount_average(4)

Example:

.. code-block:: c

  void foo() {
    #pragma loop_count(1,2) min=1 max=5 avg=2
    for (int i=0; i<1024;++i)
    for (int j=0; i<1024;++i) { }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -ast-dump t.c

.. code-block:: text

  AttributedStmt 0xf03bea0 <<invalid sloc>, line:4:31>
  |-LoopHintAttr 0xf03bdd8 <line:2:11, col:23> Implicit loop_count LoopCount Numeric
  | |-IntegerLiteral 0xf03b938 <col:22> 'int' 1
  | `-<<<NULL>>>
  |-LoopHintAttr 0xf03be00 <col:11, col:25> Implicit loop_count LoopCount Numeric
  | |-IntegerLiteral 0xf03b978 <col:24> 'int' 2
  | `-<<<NULL>>>
  |-LoopHintAttr 0xf03be28 <col:11, col:33> Implicit loop_count LoopCountMin Numeric
  | |-IntegerLiteral 0xf03b9b8 <col:31> 'int' 1
  | `-<<<NULL>>>
  |-LoopHintAttr 0xf03be50 <col:11, col:39> Implicit loop_count LoopCountMax Numeric
  | |-IntegerLiteral 0xf03b9f8 <col:37> 'int' 5
  | `-<<<NULL>>>
  |-LoopHintAttr 0xf03be78 <col:11, col:44> Implicit loop_count LoopCountAvg Numeric
  | |-IntegerLiteral 0xf03ba38 <col:43> 'int' 2
  | `-<<<NULL>>>
  `-ForStmt 0xf03bda0 <line:3:3, line:4:31>

IR:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2

  !2 = distinct !{!2, !3, !4, !5, !6}
  !3 = !{!"llvm.loop.intel.loopcount", i32 1, i32 2}
  !4 = !{!"llvm.loop.intel.loopcount_minimum", i32 1}
  !5 = !{!"llvm.loop.intel.loopcount_maximum", i32 5}
  !6 = !{!"llvm.loop.intel.loopcount_average", i32 2}
  }];
  let UserContent = [{
The ``loop_count`` pragma specifies the minimum, maximum, or average number of
iterations for a for loop. In addition, a list of commonly occurring values
can be specified to help the compiler generate multiple versions and perform
complete unrolling.
  }];
}

def PragmaLoopFuseDocs : Documentation {
  let Category = DocCatIntelHLSPragmas;
  let Option = "IntelCompat";
  let RegressionTest = "SemaIntelHLS/loop_fuse.cpp, CodeGenIntelHLS/loop_fuse.cpp";
  let Owner = "achikin";
  let Content = [{
A ``loop_fuse`` construct specifies a region of code where the compiler will
attempt to fuse adjacent loops.
Syntax:

.. code-block:: text

  #pragma loop_fuse [depth(depth-val)] [independent]
  { // structured-block: compound statement
    ...
  }

Implements a new attribute (LoopFuseAttr). Generates IR directives that
surround the applicable scope.

Example:

.. code-block:: c

  void foo(float* A, float* B, float* C) {
    #pragma loop_fuse depth(2) independent
    {
      for (int i = 0; i < 1024; ++i)
        C[i] += A[i];
      for (int j = 0; j < 1024; ++j)
        C[j] += B[j];
    }
  }


AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.c

.. code-block:: text

  `-FunctionDecl 0x896bef8 <doc.c:1:1, line:9:1> line:1:6 foo 'void (float *, float *, float *)'
  |-ParmVarDecl 0x896bd10 <col:10, col:17> col:17 used A 'float *'
  |-ParmVarDecl 0x896bd90 <col:20, col:27> col:27 used B 'float *'
  |-ParmVarDecl 0x896be10 <col:30, col:37> col:37 used C 'float *'
  `-CompoundStmt 0x896c6d0 <col:40, line:9:1>
    `-AttributedStmt 0x896c6b8 <line:3:3, line:8:3>
      |-LoopFuseAttr 0x896c670 <line:3:3, line:8:3> 2 "independent"
      `-CompoundStmt 0x896c650 <line:3:3, line:8:3>
        |-ForStmt 0x896c310 <line:4:5, line:5:18>

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - doc.c

.. code-block:: llvm

  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.FUSE"(), "QUAL.PRAGMA.FUSE.DISTANCE"(i32 2), "QUAL.PRAGMA.FUSE.INDEPENDENT"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.FUSE"() ]
  }];
 let UserContent = [{
The ``loop_fuse`` pragma specifies a region of code where the compiler will
attempt to fuse adjacent loops, letting you exert greater control over
optimizations on adjacent loops.
  }];
}

def PragmaIVDepDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Owner = "mprice1,pmrao,smanna";
  let Content = [{
The ``ivdep`` pragma instructs the compiler to ignore assumed vector
dependencies in a loop.

The ``ivdep`` pragma applies a safelen value of 0 or 1 that has
no effect on the loop and emits a suppressible warning when safelen
value of 0 or 1 is used.

Intel form (enabled with -fintel-compatibility) syntax:

.. code-block:: text

  #pragma ivdep [back|loop]

HLS form (enabled with -fhls) syntax:

.. code-block:: text

  #pragma ivdep [safelen(n)] [array(<arrayname>)]

The implementation uses the existing clang LoopHint attribute framework.  We
are using five new Options and four States and produce both LoopHint metadata
and intrinsic directives.  The table below shows how they relate.

.. csv-table::
  :header: Syntax, Option, State, Metadata, Directive
  :widths: 24,13,13,25,25

  #pragma #ivdep, IVDep, Enable, llvm.loop.vectorize.ivdep_back,
  "", IVDepHLS, Enable, llvm.loop.ivdep.enable, ""
  "", IVDepHLSIntel, Enable, llvm.loop.vectorize.ivdep_back llvm.loop.ivdep.enable, ""

  #pragma ivdep back, IVDepBack, Enable, llvm.loop.vectorize.ivdep_back, ""
  #pragma ivdep loop, IVDepLoop, Enable, llvm.loop.vectorize.ivdep_loop, ""

  #pragma ivdep safelen(4), IVDepHLS, Numeric, llvm.loop.ivdep.safelen(4), ""

  #pragma ivdep array(Arr), IVDepHLS, LoopExpr, "", "QUAL.PRAGMA.ARRAY(%Arr, i32 -1)"

  #pragma ivdep safelen(4) array(Arr), IVDepHLS, Full, "", "QUAL.PRAGMA.ARRAY(%Arr, i32 4)"

IntelCompat form example:

.. code-block:: c

  void foo() {
    #pragma ivdep
    for (int i=0; i<1024;++i) { }
    #pragma ivdep back
    for (int i=0; i<1024;++i) { }
    #pragma ivdep loop
    for (int i=0; i<1024;++i) { }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -ast-dump t.c

.. code-block:: text

  AttributedStmt 0xe26ac08 <<invalid sloc>, line:3:31>
  |-LoopHintAttr 0xe26abe0 <line:2:11> Implicit ivdep IVDep Enable

  AttributedStmt 0xe26ae38 <<invalid sloc>, line:5:31>
  |-LoopHintAttr 0xe26ae10 <line:4:11> Implicit ivdep IVDepBack Enable

  AttributedStmt 0xe26b068 <<invalid sloc>, line:7:31>
  |-LoopHintAttr 0xe26b040 <line:6:11> Implicit ivdep IVDepLoop Enable

IR:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2

  br label %for.cond2, !llvm.loop !4

  br label %for.cond9, !llvm.loop !5

  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.vectorize.ivdep_back"}
  !4 = distinct !{!4, !3}
  !5 = distinct !{!5, !6}
  !6 = !{!"llvm.loop.vectorize.ivdep_loop"}

HLS form example:

.. code-block:: c

  void foo() {
    int Arr[16];
    #pragma ivdep
    for (int i=0; i<1024;++i) { }
    #pragma ivdep safelen(4)
    for (int i=0; i<1024;++i) { }
    #pragma ivdep array(Arr)
    for (int i=0; i<1024;++i) { }
    #pragma ivdep safelen(4) array(Arr)
    for (int i=0; i<1024;++i) { }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.c

.. code-block:: text

  AttributedStmt 0x10977398 <line:3:13, line:4:33>
    | |-SYCLIntelFPGAIVDepAttr 0x10977358 <line:3:13> 0

  AttributedStmt 0x11dee5f8 <line:5:13, line:6:33>
    | |-SYCLIntelFPGAIVDepAttr 0x11dee5b8 <line:5:13, col:29> 4

  AttributedStmt 0x11dee840 <line:7:13, line:8:33>
    | |-SYCLIntelFPGAIVDepAttr 0x11dee800 <line:7:13, col:29> 0

  AttributedStmt 0x11deeac0 <line:9:13, line:10:33>
      |-SYCLIntelFPGAIVDepAttr 0x11deea80 <line:9:13, col:30> 4

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.c

.. code-block:: llvm

  %Arr = alloca [16 x i32], align 16

  br label %for.cond, !llvm.loop !2

  br label %for.cond2, !llvm.loop !4

  br label %for.cond9, !llvm.loop !6

  br label %for.cond16, !llvm.loop !9

  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.ivdep.enable"}
  !4 = distinct !{!4, !5}
  !5 = !{!"llvm.loop.ivdep.safelen", i32 4}
  !6 = distinct !{!6, !7}
  !7 = !{!"llvm.loop.parallel_access_indices", !8}
  !8 = distinct !{}
  !9 = distinct !{!9, !10}
  !10 = !{!"llvm.loop.parallel_access_indices", !11, i32 4}
  !11 = distinct !{}

  }];
  let UserContent = [{
The ``ivdep`` pragma instructs the compiler to ignore assumed vector
dependencies in a loop.
  }];
}

def MaxInterleavingDocs : Documentation {
  let Category = DocCatIntelHLSPragmas;
  let BugTracker="CMPLRLLVM-9099";
  let Owner = "achikin,smanna";
  let Content = [{
The ``max_interleaving`` pragma instructs the compiler to restrict the maximum number
of interleaved invocations executing the loop to a limit specified on the integral parameter.

Syntax:

.. code-block:: text

  #pragma max_interleaving [<constant-int>]

Note: pragma max_interleaving accepts 0.

Example:

.. code-block:: c

  void foo() {
    for(int i=0;i<8;++i) {
        #pragma max_interleaving(1)
        for(int j=0;j<8;++j) {}
    }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump -o - -fintel-compatibility t.c

.. code-block:: text

  AttributedStmt 0x1137ca10 <line:3:17, line:4:31>
          |-SYCLIntelFPGAMaxInterleavingAttr 0x1137c9e0 <line:3:17, col:36>
          | `-IntegerLiteral 0x1137c7f0 <col:34> 'int' 1
          `-ForStmt 0x1137c9a8 <line:4:9, col:31>

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - -fintel-compatibility t.c

.. code-block:: llvm

  br label %for.cond1, !llvm.loop !2

  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.max_interleaving.count", i32 1}
  }];
  let UserContent = [{
The ``max_interleaving`` pragma instructs the compiler to restrict the maximum
number of interleaved invocations executing the loop to a limit specified on
the integral parameter.
  }];
}

def SimplePragmasDocs : Documentation {
  let Category = DocCatIntelHLSPragmas;
  let BugTracker = "CMPLRLLVM-5584";
  let RegressionTest = "SemaIntelHLS/pragma.cpp, CodeGenIntelHLS/pragma.cpp";
  let Owner = "jyu2,smanna";
  let Content = [{
These are simple HLS/FPGA loop pragmas that either have one constant integral
argument, or no arguments.  ** Note: Let's add new topics for any additional
pragmas instead of adding onto to this topic. **

Syntax:

.. code-block:: text

  #pragma [ii|ii_at_most|ii_at_least] [<constant-int>]
  #pragma [speculated_iterations|max_concurrency] [<constant-int>]
  #pragma min_ii_at_target_fmax
  #pragma disable_loop_pipelining

Note: the speculated_iterations accept 0.
Note: pragma max_concurrency accepts 0.

The implementation uses the existing clang LoopHint attribute framework to
produce LoopHint metadata.  The table below shows the Options, States, and the
Metadata produced.

.. csv-table::
  :header: Syntax, Option, State, Metadata
  :widths: 24,13,13,25

  #pragma ii 4, II, Numeric, llvm.loop.ii.count(4)
  #pragma max_concurrency 4, MaxConcurrency, Numeric, llvm.loop.max_concurrency.count(4)
  #pragma ii_at_most 4, IIAtMost, Numeric, llvm.loop.intel.ii.at.most.count(4)
  #pragma ii_at_least 4, IIAtLeast, Numeric, llvm.loop.intel.ii.at.least.count(4)
  #pragma speculated_iterations 4, SpeculatedIterations, Numeric, llvm.loop.intel.speculated.iterations.count(4)
  #pragma min_ii_at_target_fmax, MinIIAtFmax, Enable, llvm.loop.intel.min.ii.at.target.fmax
  #pragma disable_loop_pipelining, DisableLoopPipelining, Enable, llvm.loop.intel.pipelining.enable

Example:

.. code-block:: c

  void foo() {
    int var = 0;
    #pragma ii_at_least 4
    for (int i = 0; i < 10; ++i) var++;
    #pragma disable_loop_pipelining
    for (int i = 0; i < 10; ++i) var++;
    #pragma max_concurrency 4
    for (int i = 0; i < 10; ++i) var++;
    #pragma speculated_iterations 4
    for (int i = 0; i < 10; ++i) var++;
    #pragma ii 4
    for (int i = 0; i < 10; ++i) var++;
  }

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump  t.c

.. code-block:: text

  AttributedStmt 0x12040aa8 <line:3:13, line:4:37>
    | |-LoopHintAttr 0x12040a68 <line:3:13, col:26> Implicit ii_at_least IIAtLeast Numeric
    | | |-IntegerLiteral 0x12040850 <col:25> 'int' 4
    | | `-<<<NULL>>>
    | `-ForStmt 0x12040a30 <line:4:5, col:37>

  AttributedStmt 0x10954c50 <line:5:13, line:6:37>
      |-SYCLIntelFPGADisableLoopPipeliningAttr 0x10954c28 <line:5:13>
      `-ForStmt 0x10954bf0 <line:6:5, col:37>
        |-DeclStmt 0x10954af0 <col:10, col:19>
        | `-VarDecl 0x10954a68 <col:10, col:18> col:14 used i 'int' cinit
        |   `-IntegerLiteral 0x10954ad0 <col:18> 'int' 0
        |-<<<NULL>>>
  AttributedStmt 0x12040f60 <line:7:13, line:8:37>
    | |-SYCLIntelFPGAMaxConcurrencyAttr 0x12040f30 <line:7:13, col:30>
    | | `-IntegerLiteral 0x12040d18 <col:29> 'int' 4
    | `-ForStmt 0x12040ef8 <line:8:5, col:37>

  AttributedStmt 0x120411e0 <line:9:13, line:10:37>
    | |-SYCLIntelFPGASpeculatedIterationsAttr 0x120411b0 <line:9:13, col:36>
    | | `-IntegerLiteral 0x12040f98 <col:35> 'int' 4
    | `-ForStmt 0x12041178 <line:10:5, col:37>

  AttributedStmt 0x12041460 <line:11:13, line:12:37>
      |-SYCLIntelFPGAIIAttr 0x12041430 <line:11:13, col:17>
      | `-IntegerLiteral 0x12041218 <col:16> 'int' 4
      `-ForStmt 0x120413f8 <line:12:5, col:37>

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2

  br label %for.cond2, !llvm.loop !4

  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.intel.ii.at.least.count", i32 4}
  !4 = distinct !{!4, !5}
  !5 = !{!"llvm.loop.intel.pipelining.enable", i32 0}
  }];
  let UserContent = [{
These are simple HLS/FPGA loop pragmas that either have one constant integral
argument, or no arguments.
  }];
}

def ForceHyperoptDocs : Documentation {
  let Category = DocCatIntelHLSPragmas;
  let BugTracker = "CMPLRLLVM-7128";
  let RegressionTest = "SemaIntelHLS/pragma.cpp, CodeGenIntelHLS/pragma.cpp";
  let Owner = "mprice1";
  let Content = [{
These pragmas will force enable or disable "hyperoptimizations" on the
attached loop.  For example, on some architectures, hyperoptimization will
optimize for initiation of loop iterations of a single loop invocation, at the
possible expense of a delay between consecutive loop invocations.

.. csv-table::
  :header: Syntax, Option, State, Metadata
  :widths: 24,13,13,25

  #pragma force_hyperopt, ForceHyperopt, Enable, llvm.loop.intel.hyperopt
  #pragma force_no_hyperopt, ForceHyperopt, Disable, llvm.loop.intel.nohyperopt


The implementation uses the existing clang LoopHint attribute framework to
produce LoopHint metadata.  The table above shows the Options, States, and the
Metadata produced.

Example:

.. code-block:: c

  void foo() {
    int var = 0;
    #pragma force_hyperopt
    for (int i = 0; i < 10; ++i) var = var+1;
    #pragma force_no_hyperopt
    for (int i = 0; i < 10; ++i) var = var+1;
  }

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.c

.. code-block:: text

  AttributedStmt 0xd1d9a90 <<invalid sloc>, line:4:44>
  |-LoopHintAttr 0xd1d9a68 <line:3:13> Implicit force_hyperopt ForceHyperopt Enable

  AttributedStmt 0xd1d9d88 <<invalid sloc>, line:6:44>
  |-LoopHintAttr 0xd1d9d60 <line:5:13> Implicit force_no_hyperopt ForceHyperopt Disable

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2

  br label %for.cond2, !llvm.loop !4

  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.intel.hyperopt"}
  !4 = distinct !{!4, !5}
  !5 = !{!"llvm.loop.intel.nohyperopt"}
  }];
  let UserContent = [{
These pragmas will force enable or disable 'hyperoptimizations' on the
attached loop.  For example, on some architectures, hyperoptimization will
optimize for initiation of loop iterations of a single loop invocation, at the
possible expense of a delay between consecutive loop invocations.
  }];
}

def RotateIntrinsicsDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "";
  let BugTracker= "CMPLRS-4684";
  let RegressionTest = "CodeGen/intel/intrinsics-rotations.c";
  let Owner = "bwyma";
  let Content = [{

Microsoft supports a set of intrinsics for performing rotations.  These include:
  1.  _rotl8
  2.  _rotl16
  3.  _rotl
  4.  _lrotl
  5.  _rrotl64
  6.  _rotr8
  7.  _rotr16
  8.  _rotr
  9.  _lrotr
  10. _rotr64

The Intel compiler also recognized these intrinsics on Linux. The LLVM
compiler recognizes these intrinsics on Windows (to match Microsoft) but not on
Linux. These intrinsics were enabled on Linux in the Intel Xmain compiler to
be compatible with the Intel classic compiler. It doesn't seem unreasonable
for customers writing code for multiple platforms to want these intrinsics to
work on all of them.

Here is an example:

.. code-block:: c

  int main () {
      unsigned int result = 1;
      result = _rotl(result, 4);
      return result;
  }

The rotate intrinsic above is translated into shift's like this:

.. code-block:: llvm

  %0 = load i32, i32* %result, align 4
  %1 = shl i32 %0, 4
  %2 = lshr i32 %0, 28
  %3 = or i32 %1, %2
  store i32 %3, i32* %result, align 4

  }];
  let UserContent = [{
Support Microsoft's set of intrinsics for performing rotations on Linux:
_rotl8 _rotl16 _rotl _lrotl _rrotl64 _rotr8 _rotr16 _rotr _lrotr _rotr64.
  }];
}

// Optimization

def FakeLoadDocs : Documentation {
  let Category = DocCatIntelOptimization;
  let Option = "IntelCompat";
  let BugTracker= "CQ414386/CMPLRS-915";
  let RegressionTest = "CodeGen/intel/tbaa-prop1.cpp,tbaa-prop2.cpp";
  let Content = [{
Adds an intrinsic to improve alias analysis for pointers returned from a
function.

For example, in the case below, it helps the compiler prove that s.getA(x)
and s.getB(y) do not overlap.

.. code-block:: c

  struct S {
    int A[4];
    int B[4];
    int& getA(int i) { return A[i]; }
    int& getB(int i) { return B[i]; }
  };

  int foo(S& s, int i, int j) {
    s.getA(i) = 0;
    s.getB(j) = 1;
    return s.getA(i);
  }

The output code generated for the return in getA() would normally be:

.. code-block:: llvm

  %arrayidx = getelementptr inbounds [4 x i32], [4 x i32]* %A, i64 0, i64 %idxprom
  ret i32* %arrayidx

The intrinsic is added between with tbaa metadata:

.. code-block:: llvm

  %arrayidx = getelementptr inbounds [4 x i32], [4 x i32]* %A, i64 0, i64 %idxprom
  %1 = call i32* @llvm.intel.fakeload.p0i32(i32* %arrayidx, metadata !10) #2
  ret i32* %1

  !7 = !{!"int", !4, i64 0}
  !10 = !{!11, !7, i64 0}
  !11 = !{!"struct@_ZTS1S", !12, i64 0, !12, i64 16}
  !12 = !{!"array@_ZTSA4_i", !7, i64 0}

This is implemented during clang CodeGen of the ReturnStmt. Return statements
are considered if the function returns a reference type or the return value is
an addressof expression.

.. code-block:: c++

  int& getA(int i) { return A[i]; }
  int* getB(int i) { return &B[i]; }

The function IsFakeLoadCand further narrows the cases handled to array
subscripts and member select expressions.

.. code-block:: c++

  int& getA(int i) { return A.b; }
  int* getB(int i) { return &B.b; }
  int& getC(int i) { return C->b; }
  int* getD(int i) { return &D->b; }

The function EmitFakeLoadForRetPtr is then called to generate a new load with
metadata.  The load and metadata is saved in a map and the fakeload intrinsic
is created and emitted during EmitFunctionEpilog.
  }];
  let UserContent = [{
Adds an intrinsic to improve alias analysis for pointers returned from a
function.
  }];
}

#if INTEL_FEATURE_SW_DTRANS
def WholeProgramVTableWrapDocs : Documentation {
  let Category = DocCatIntelOptimization;
  let Option = "IntelCompat";
  let BugTracker= "CMPLRLLVM-5603";
  let RegressionTest = "CodeGenCXX/intel/whole-prog-vtable-wrap.cpp";
  let Owner = "mprice1";
  let Content = [{
Community clang/llvm supports whole program devirtualization when the
``-fwhole-program-vtables`` option is used and the class has hidden
`LTO Visibility <https://clang.llvm.org/docs/LTOVisibility.html>`_

This optimization enables devirtualization with whole program analysis instead
of relying on hidden LTO visibility.

Example:

.. code-block:: c++

  struct A { A(); virtual void f(); };
  struct B : virtual A { B(); virtual void g(); virtual void h(); };

  A::A() {}
  B::B() {}
  void A::f() { }
  void B::g() { }

  void af(A *a) { a->f(); }

Compiled with:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility w.cpp -flto -flto-unit -fwhole-program-vtables -emit-llvm -o -

The compiler only calls type.test and assume when the wholeprogramsafe
intrinsic returns true:

.. code-block:: llvm

    %vtable = load void (%struct.A*)**, void (%struct.A*)*** %1, align 8
    %2 = call i1 @llvm.intel.wholeprogramsafe()
    br i1 %2, label %whpr.wrap, label %whpr.continue

  whpr.wrap:                                        ; preds = %entry
    %3 = bitcast void (%struct.A*)** %vtable to i8*
    %4 = call i1 @llvm.type.test(i8* %3, metadata !"_ZTS1A")
    call void @llvm.assume(i1 %4)
    br label %whpr.continue

  whpr.continue:                                    ; preds = %whpr.wrap, %entry
    %vfn = getelementptr inbounds void (%struct.A*)*, void (%struct.A*)** %vtable, i64 0
  }];
  let UserContent = [{
When used with ``-fwhole-program-vtables`` option, this optimization enables
devirtualization with whole program analysis without hidden LTO visibility.
  }];
}
#endif // INTEL_FEATURE_SW_DTRANS

def IntelTBAADocs : Documentation {
  let Category = DocCatIntelOptimization;
  let Option = "IntelCompat";
  let BugTracker = "LCPT-1003";
  let Content = [{
This implements alias analysis improvements for LCPT-1003. It helps in the case
where an array is nested in a struct. Previously we tried to add an "assume
index < upper bound" node on top of the index expression but that was not
successful.  Instead we introduce metadata to do the annotation on the GEP. And
we give that metadata such meaning that combining the GEPs would be valid.
There was a long email discussion between C++ experts on the cfe and the
codegen teams. We determined that if the address of a struct member is taken,
it is valid to do arithmetic which yields addresses to other fields within the
struct. Therefore this patch doesn't allow propagation of array decay
expressions.

For example in this test case showing treatment of array decay:

.. code-block:: c++

  typedef struct S {
    int a[10], b[10]
  } s_type;

  void foo(void) {
    S s;
    int *p1 = s.a;
    int *p2 = s.b;
    int *p3 = (int *) ((char *)p1 - __builtin_offsetof(s_type,a) +
                                __builtin_offsetof(s_type,b));
    // at this point, p2 and p3 are equivalent
  }

The output tbaa generated for the field reference s.a is intel-tbaa and is
unique from the intel-tbaa generated for s.b

.. code-block:: llvm

 %a = getelementptr inbounds %struct.S, %struct.S* %s, i32 0, i32 0, !intel-tbaa !2
 %b = getelementptr inbounds %struct.S, %struct.S* %s, i32 0, i32 1, !intel-tbaa !10

However, the output tbaa generated for the array decay into p1 and p2 is unchanged and represents an undistinguished pointer into the struct object:

.. code-block:: llvm

  %arraydecay = getelementptr inbounds [10 x i32], [10 x i32]* %a, i32 0, i32 0
  store i32* %arraydecay, i32** %p1, align 8, !tbaa !8
  %arraydecay1 = getelementptr inbounds [10 x i32], [10 x i32]* %b, i32 0, i32 0
  store i32* %arraydecay1, i32** %p2, align 8, !tbaa !8

Where:

.. code-block:: llvm

 !8 = !{!9, !9, i64 0}
 !9 = !{!"pointer@_ZTSPi", !6, i64 0}

  }];
  let UserContent = [{
Enables type-based alias analysis for when an array is nested in a struct.
Allows assumption that if the address of a struct member is taken, it is valid
to do arithmetic which yields addresses to other fields within the struct.
  }];
}

def IMFAttributesDocs : Documentation {
  let Category = DocCatIntelOptimization;
  let Option = "IntelCompat";
  let BugTracker = "CMPLRLLVM-5052";
  let Owner = "mprice1";
  let Content = [{
This is support to attach IMF attributes specified by the user to the
applicable LLVM functions.  They have this form:

  -fimf-{kind}[=value[:funclist]]

For example, for this user option:

  -fimf-precision:low:sinf,expr2 -fimf-absolute-error=0.0001:sinf,cosf

The driver sends the following single option to clang:

  "-mGLOB_imf_attr=precision:low:sinf,expr2 absolute-error:0.0001:sinf,cosf"

When a funclist is used, the attributes are applied only to functions with
the exact name specified.  When there is no funclist the attribute is applied
to all functions.

Example:

.. code-block:: c

  extern double cos(double), sin(double);
  extern void otherfunc(void);

  void foo(double d0) {
    (void)cos(d0); // prec=low
    (void)sin(d0); // prec=high abs-error=0.00001
    otherfunc();   // prec=high
  }

IR:

.. code-block:: console

  $ clang -cc1 -ffreestanding -fintel-compatibility "-mGLOB_imf_attr=precision:high precision:low:cos absolute-error:0.00001:sin" -emit-llvm -o - imf.c

.. code-block:: llvm

  %0 = load double, double* %d0.addr, align 8
  %call = call double @cos(double %0) #4
  %1 = load double, double* %d0.addr, align 8
  %call1 = call double @sin(double %1) #5
  call void @otherfunc() #6

  attributes #4 = { nobuiltin "imf-precision"="low" }
  attributes #5 = { nobuiltin "imf-absolute-error"="0.00001" "imf-precision"="high" }
  attributes #6 = { nobuiltin "imf-precision"="high" }
  }];
  let UserContent = [{
Attaches IMF attributes specified by the user to the applicable LLVM math
functions.
  }];
}

def LdexpCallDocs : Documentation {
  let Category = DocCatIntelOptimization;
  let Option = "IntelCompat";
  let BugTracker = "CMPLRLLVM-25133";
  let Owner = "pmrao";
  let Content = [{
This change adds calls to the ldexp intrinsics when appropriate, instead
of the calling the equivalent math library functions.

For example:

.. code-block:: c

  #include <math.h>

  void foo() {
    (void)ldexp(2.0, -32);
    (void)ldexpf(3.0f, -32);
    (void)ldexpl(4.0l, -32);
  }

Compiled with:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility ldexp.c -emit-llvm -o -

The compiler calls the appropriate lvm.ldexp.* instead:

.. code-block:: llvm

  entry:
    %0 = call double @llvm.ldexp.f64(double 2.000000e+00, i32 -32)
    %1 = call float @llvm.ldexp.f32(float 3.000000e+00, i32 -32)
    %2 = call x86_fp80 @llvm.ldexp.f80(x86_fp80 0xK40018000000000000000, i32 -32)

  ; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
  declare double @llvm.ldexp.f64(double, i32) #1

  ; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
  declare float @llvm.ldexp.f32(float, i32) #1

  ; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
  declare x86_fp80 @llvm.ldexp.f80(x86_fp80, i32) #1

  attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

  }];
  let UserContent = [{
When ``-fmath-errno`` option is not used, this change enables calling the
llvm.ldexp.* intrinsics instead of the equivalent math library routines.
  }];
}

#if INTEL_FEATURE_SW_DTRANS
def IntelMempoolCtorDtorDocs : Documentation {
  let Category = DocCatIntelOptimization;
  let Option = "IntelCompat";
  let BugTracker= "CMPLRLLVM-28196";
  let RegressionTest = "CodeGenCXX/intel/ctor-dtor-attributes.cpp";
  let Content = [{
Marks constructors and destructors with intel-mempool-constructor and
intel-mempool-destructor attributes when the ``-fwhole-program-vtables``
option is used for LTO compilation.

Example:

.. code-block:: c++

  struct A {
    double d;
    A() { d = 20; }
    ~A() { d = 0;}
  };

  A a;

Compiled with:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility w.cpp -flto -flto-unit -fwhole-program-vtables -emit-llvm -o -

.. code-block:: llvm

    define linkonce_odr void @_ZN1AC2Ev(%struct.A* nonnull dereferenceable(8) %this) unnamed_addr #1

    define linkonce_odr void @_ZN1AD2Ev(%struct.A* nonnull dereferenceable(8) %this) unnamed_addr #2

    attributes #1 = { "intel-mempool-constructor" }
    attributes #2 = { "intel-mempool-destructor" }

  }];
  let UserContent = [{
LTO transformations use these attributes to identify constructor and
destructor member functions.
  }];
}
#endif // INTEL_FEATURE_SW_DTRANS

def PredeclareAlignValTDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker= "CMPLRLLVM-3383,cq367961";
  let RegressionTest = "cfeCpp/align_new(1-24)";
  let Owner = "mprice1";
  let Content = [{
This is part of the support for automatically-aligned dynamic allocation.
See https://software.intel.com/en-us/node/522531 for a description.  This
is now part of C++17 but the Intel compiler supported it earlier.

In C++17 std::align_val_t will be defined in <new> but this extension
predeclares it to enable earlier code. This extension will not be
enabled in C++17 and later modes.

Note that in clang the LangOpt AlignedAllocation (-faligned-allocation)
controls whether this feature is enabled.  In C++17 mode it is enabled
by default but can be turned off with -fno-aligned-allocation.

Example:

.. code-block:: c

  typedef long unsigned int size_t;
  int posix_memalign(void **__memptr, size_t __alignment, size_t __size);
  void *foo(size_t _S, std::align_val_t _A) {
    void *_P;
    posix_memalign(&_P, (size_t)_A, _S);
    return _P;
  }

With unmodified clang:

.. code-block:: console

  $ clang -cc1 -faligned-allocation t.cpp
  t.cpp:3:22: error: use of undeclared identifier 'std'
  void *foo(size_t _S, std::align_val_t _A)
                     ^
  1 error generated.

With -fintel-compatibility:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -faligned-allocation t.cpp
  $
  }];
  let UserContent = [{
Enables support for automatically-aligned dynamic allocation per C++17
std::align_val_t.
  }];
}

def VolatileInOMPRegionsDocs : Documentation {
  let Category = DocCatIntelOpenMP;
  let Option = "IntelCompat";
  let BugTracker= "CMPLRLLVM-1443";
  let RegressionTest = "";
  let Owner = "mprice1";
  let Content = [{
Community clang is removing volatile from variables captured in OpenMP
regions. Whether it is a good idea or not, there doesn't seem to be
justification for removing volatile in the OpenMP spec. This extension,
controlled by -fintel-compatibility, retains volatile.

.. code-block:: console

  $ clang -cc1 -fopenmp -fintel-compatibility -emit-llvm -o - t.c

.. code-block:: c

  int get_thread_num();
  void foo() {
    int volatile lck = 0;
    #pragma omp parallel
    {
      if (get_thread_num() == 0)
        lck = 1;
    }
  }

.. code-block:: llvm

  %call = call i32 (...) @get_thread_num()
  %cmp = icmp eq i32 %call, 0
  br i1 %cmp, label %if.then, label %if.end

  if.then:                                          ; preds = %entry
    store volatile i32 1, i32* %0, align 4
    br label %if.end

  if.end:                                           ; preds = %if.then, %entry
    ret void
  }];
  let UserContent = [{
Community clang removes volatile from variables captured in OpenMP regions.
This extension, controlled by -fintel-compatibility, retains volatile.
  }];
}

def MayIUseFeatureDocs : Documentation {
  let Category = DocCatIntelCompat;
  let RegressionTest ="test/CodeGen/intel/may-i-use-cpu-feature.cpp";
  let Owner = "ekeane1";
  let Content = [{
The Intel Compiler implements a cpu-id like function to determine the
current processor's features called _may_i_use_cpu_feature. This feature
consists of 3 different intrinsics:

int _may_i_use_cpu_feature(uint64_t mask) accepts a bitmask that is validated
against the current processor's feature list. It returns an integer that is
contextually convertible to bool (in C++) that would evaluate to 'true' if every
feature in the bitmask parameter is supported by the current host processor.
This function is limited to bitmask values in the first 'page' of the libirc
cpu-id information.

int _may_i_use_cpu_feature_ext(uint64_t mask, unsigned page) accepts a bitmask
that is validated against the current processor's feature list. This works
identically to the previous variant, except it also accepts a 'page' index that
permits checking features on the 2nd page of the libirc information. When
provided with a '0' in the 'page' parameter, this works identically to
_may_i_use_cpu_feature.

int _may_i_use_cpu_feature_str(<string literal> feature, ...) accepts a series
of compile-time string literals that contain a list of feature names. These
feature names are converted to a bitmask and uses the same infrastructure as
_may_i_use_cpu_feature_ext to validate it. The behavior is the same as the
previous variants.

Most of this functionality is implemented in libirc. This library contains
an init function (__intel_cpu_features_init/__intel_cpu_features_init_x)
that initializes a global variable (__intel_cpu_feature_indicator/
__intel_cpu_feature_indicator_x) with a bitmask that represents the features
supported by the target machine architecture. Because of this, this feature
requires linking with libirc. The '_x' versions do not check the Genuine Intel
string, so this currently always checks those versions.

From the compiler's perspective, this builtin ensures it is given a compile
time constant integer parameter, then emits an and with the global variable,
followed by an equality check against it, similar to this pseudo-code:

.. code-block:: c

  int _may_i_use_cpu_feature(unsigned __int64 param) {
    return (param & __intel_cpu_feature_indicator_x) == param;
  }

See additional documentation here:
https://software.intel.com/en-us/node/523363
  }];
  let UserContent = [{
Implement _may_i_use_cpu_feature function.
  }];
}

def ClusterDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-5585";
  let RegressionTest = "SemaIntelHLS/cluster.cpp, CodeGenIntelHLS/cluster.cpp, SemaOpenCL/intel-fpga-cluster.cl, CodeGenOpenCL/intel-fpga-cluster.cl";
  let Owner = "eandrews";
  let Content = [{
The ``cluster`` attribute is an FPGA function and lambda function attribute
used in HLS and OpenCL to generate FPGA hardware for the function, grouping the
functionality of clusters with the same name. The attribute takes 1 optional
string argument and adds metadata to the function/lambda definition.

Syntax:

.. code-block:: text

  __attribute__((cluster[("<user_string>")]))

C++11 attribute syntax is also supported for HLS:

.. code-block:: text

  [[clang::cluster[("<user_string>")]]]

The optional argument ``<user_string>`` determines the metadata produced.

Example:

.. code-block:: c

  void __attribute__((cluster)) foo1() {}
  void __attribute__((cluster("clustername"))) foo2() {}
  void __attribute__((cluster(""))) foo3() {}

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.c

.. code-block:: text

  FunctionDecl 0xd776498 <x.cpp:1:1, col:39> col:31 foo1 'void ()'
  |-CompoundStmt 0xd7765c0 <col:38, col:39>
  `-ClusterAttr 0xd776530 <col:21> ""

  FunctionDecl 0xd7766a0 <line:2:1, col:54> col:46 foo2 'void ()'
  |-CompoundStmt 0xd776798 <col:53, col:54>
  `-ClusterAttr 0xd776738 <col:21, col:42> "clustername" HasName

  FunctionDecl 0xd776870 <line:3:1, col:43> col:35 foo3 'void ()'
  |-CompoundStmt 0xd776958 <col:42, col:43>
  `-ClusterAttr 0xd776908 <col:21, col:31> "" HasName

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.c

.. code-block:: llvm

  define void @_Z4foo1v() #0 !cluster !2 {}

  define void @_Z4foo2v() #0 !cluster !3 {}

  define void @_Z4foo3v() #0 !cluster !4 {}

  !2 = !{!"", i32 0}
  !3 = !{!"clustername", i32 1}
  !4 = !{!"", i32 1}

The second operand in the metadata is i32 1 if ``<user_string>`` is specified.
It is i32 0 if not.
  }];
  let UserContent = [{
The ``cluster`` attribute is an FPGA function and lambda function attribute
used in HLS and OpenCL to generate FPGA hardware for the function, grouping
the functionality of clusters with the same name. The attribute takes 1
optional string argument and adds metadata to the function/lambda definition.
  }];
}

def StallEnableDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-5585";
  let RegressionTest = "SemaIntelHLS/stall_enable.cpp, CodeGenIntelHLS/stall_enable.cpp, SemaOpenCL/intel-fpga-stall_enable.cl, CodeGenOpenCL/intel-fpga-stall_enable.cl";
  let Owner = "eandrews";
  let Content = [{
The ``use_stall_enable_clusters`` attribute is an FPGA function and lambda
function attribute used in HLS and OpenCL to generate FPGA hardware for the
function using stall/enable protocol. The attribute takes no arguments and adds
metadata to the function/lambda definition.

Syntax:

.. code-block:: text

  __attribute__((use_stall_enable_clusters))

C++11 attribute syntax is also supported on HLS:

.. code-block:: text

  [[clang::use_stall_enable_clusters]]

Example:

.. code-block:: c

  void __attribute__((use_stall_enable_clusters)) foo() {}

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.c

.. code-block:: text

  FunctionDecl 0x235b69b8d98 <t.c:1:1, col:43> col:36 foo 'void ()'
  |-CompoundStmt 0x235b69b8eb0 <col:42, col:43>
  `-SYCLIntelUseStallEnableClustersAttr 0x235b69b8e30 <col:21>

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.c

.. code-block:: llvm

  define void @_Z4foo1v() #0 !stall_enable !2 {}
  !2 = !{i32 1}
  }];
  let UserContent = [{
The ``use_stall_enable_clusters`` attribute is an FPGA function and lambda function
attribute used in HLS and OpenCL to generate FPGA hardware for the function
using stall/enable protocol. The attribute takes no arguments and adds
metadata to the function/lambda definition.
  }];
}

def StallLatencyDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-7128";
  let RegressionTest = "SemaIntelHLS/stall_latency.cpp, CodeGenIntelHLS/stall_latency.cpp, SemaOpenCL/intel-fpga-stall_latency.cl, CodeGenOpenCL/intel-fpga-stall_latency.cl";
  let Owner = "mprice1";
  let Content = [{
This attribute turns on/off the stall latency protocol for the component or
kernel. When off, a node immediately asserts a stall signal when it cannot
accept data. When on, we pipeline signals and data between nodes to increase
fmax at the expense of area.

These attributes can be applied only to HLS components or OpenCL kernels.

Syntax:

.. code-block:: text

  __attribute__((stall_latency_enabled))
  __attribute__((stall_latency_disabled))

HLS Example:

.. code-block:: c++

  __attribute__((ihc_component,stall_latency_enabled)) void foo1() {}
  __attribute__((ihc_component,stall_latency_disabled)) void foo2() {}

HLS AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.cpp

.. code-block:: text

  FunctionDecl 0xed2e6f8 <t.cpp:1:1, col:67> col:59 foo1 'void ()'
  |-CompoundStmt 0xed2e830 <col:66, col:67>
  |-ComponentAttr 0xed2e790 <col:16>
  |-ComponentInterfaceAttr 0xed2e7d0 <<invalid sloc>> Implicit Streaming
  `-StallLatencyAttr 0xed2e7e0 <col:30> stall_latency_enabled

  FunctionDecl 0xed2e860 <line:2:1, col:68> col:60 foo2 'void ()'
  |-CompoundStmt 0xed2e958 <col:67, col:68>
  |-ComponentAttr 0xed2e8f8 <col:16>
  |-ComponentInterfaceAttr 0xed2e938 <<invalid sloc>> Implicit Streaming
  `-StallLatencyAttr 0xed2e948 <col:30> stall_latency_disabled

HLS IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.cpp

.. code-block:: llvm

  define void @_Z4foo1v() #0 !ihc_component !stall_latency !6
  define void @_Z4foo2v() #0 !ihc_component !stall_latency !5
  !5 = !{i32 0}
  !6 = !{i32 1}

OpenCL Example:

.. code-block:: c

  __kernel __attribute__((stall_latency_enabled)) void foo1() {}
  __kernel __attribute__((stall_latency_disabled)) void foo2() {}

OpenCL AST:

.. code-block:: console

  $ clang -cc1 -triple spir-unknown-unknown-intelfpga -ast-dump t.cl

.. code-block:: text

  FunctionDecl 0xd867f98 <t.cl:1:1, col:62> col:54 foo1 'void (void)'
  |-CompoundStmt 0xd8680c0 <col:61, col:62>
  |-OpenCLKernelAttr 0xd868030 <col:1>
  `-StallLatencyAttr 0xd868070 <col:25> stall_latency_enabled

  FunctionDecl 0xd8680f0 <line:2:1, col:63> col:55 foo2 'void (void)'
  |-CompoundStmt 0xd8681d8 <col:62, col:63>
  |-OpenCLKernelAttr 0xd868188 <col:1>
  `-StallLatencyAttr 0xd8681c8 <col:25> stall_latency_disabled


OpenCL IR:

.. code-block:: console

  $ clang -cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm -o - t.cl

.. code-block:: llvm

  define spir_kernel void @foo1() local_unnamed_addr #0 !stall_latency !5
  define spir_kernel void @foo2() local_unnamed_addr #0 !stall_latency !6
  !5 = !{i32 1}
  !6 = !{i32 0}
  }];
  let UserContent = [{
This attribute turns on/off the stall latency protocol for the component or
kernel. When off, a node immediately asserts a stall signal when it cannot
accept data. When on, we pipeline signals and data between nodes to increase
fmax at the expense of area.
  }];
}

def MaxConcurrencyDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-7539";
  let RegressionTest = "SemaIntelHLS/local.cpp, CodeGenIntelHLS/local.cpp, SemaOpenCL/intel-fpga-local-hls.cl, CodeGenOpenCL/intel-fpga-local-hls.cl";
  let Owner = "mprice1";
  let Content = [{
This attribute provides the user of the PSG HLD compiler explicit control over
an optimization applied to arrays declared in a loop body. On the FPGA, arrays
and other large local variables are physically implemented in FPGA RAM. To
support pipeline-parallel execution of loop iterations, the HLD compiler must
create multiple physical copies of the array variable on the FPGA, so that
each simultaneously executing loop iteration operates on an independent copy
of the array. The user can apply `__attribute__((max_concurrency(n)))` to
such an array to limit parallelism in the region of the pipeline containing
array accesses to at most n, thus limiting the number of copies created by the
compiler.

This attribute can be applied to functions and non-const local variables.

Syntax:

.. code-block:: text

  __attribute__((max_concurrency(n)))

  where n is a non-negative integer constant.

Example:

.. code-block:: c++

  __attribute__((max_concurrency(2048))) void foo() { }

  int bar() {
    __attribute__((max_concurrency(128))) int arr[16];
    return arr[0];
  }

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.cpp # HLS
  $ clang -cc1 -triple spir-unknown-unknown-intelfpga -ast-dump t.cl # OpenCL

.. code-block:: text

  FunctionDecl 0xdab9d08 <t.cpp:1:1, col:67> col:59 foo 'void ()'
  |-CompoundStmt 0xdab9e60 <col:65, col:67>
  `-MaxConcurrencyAttr 0xdab9e08 <col:30, col:50>
    `-ConstantExpr 0xdab9df0 <col:46> 'int'
      `-IntegerLiteral 0xdab9c98 <col:46> 'int' 2048

  DeclStmt 0xdaba130 <line:4:3, col:52>
  `-VarDecl 0xdaba060 <col:3, col:51> col:45 used arr 'int [16]'
    |-MemoryAttr 0xdaba0d8 <<invalid sloc>> Implicit Default
    `-MaxConcurrencyAttr 0xdaba118 <col:18, col:37>
      `-ConstantExpr 0xdaba0c0 <col:34> 'int'
        `-IntegerLiteral 0xdab9fa8 <col:34> 'int' 128

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.cpp # HLS
  $ clang -cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm -o - t.cl  # OpenCL

.. code-block:: llvm

  define void @_Z3foov() #0 !max_concurrency !6 {
  entry:
    ret void
  }
  !6 = !{i32 2048}

  @.str = private unnamed_addr constant [38 x i8] c"{memory:DEFAULT}{max_concurrency:128}\00", section "llvm.metadata"
  @.str.1 = private unnamed_addr constant [6 x i8] c"t.cpp\00", section "llvm.metadata"

  define i32 @_Z3barv() #0 !ihc_component {
  entry:
    %arr = alloca [16 x i32], align 16
    %arr1 = bitcast [16 x i32]* %arr to i8*
    call void @llvm.var.annotation(i8* %arr1, i8* getelementptr inbounds ([38 x i8], [38 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0), i32 4)
  }
  }];
  let UserContent = [{
This attribute provides the user of the PSG HLD compiler explicit control over
an optimization applied to arrays declared in a loop body. On the FPGA, arrays
and other large local variables are physically implemented in FPGA RAM. To
support pipeline-parallel execution of loop iterations, the HLD compiler must
create multiple physical copies of the array variable on the FPGA, so that
each simultaneously executing loop iteration operates on an independent copy
of the array. The user can apply `__attribute__((max_concurrency(n)))` to such
an array to limit parallelism in the region of the pipeline containing array
accesses to at most n, thus limiting the number of copies created by the
compiler.
  }];
}

def MaxReplicatesDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-8810";
  let RegressionTest = "SemaIntelHLS/local.cpp, CodeGenIntelHLS/local.cpp, SemaOpenCL/intel-fpga-local-hls.cl, CodeGenOpenCL/intel-fpga-local-hls.cl";
  let Owner = "zahiraam";
  let Content = [{
This attribute is applied to variables (local, statics, constants ...) in both
OpenCL and HLS. It specifies the maximum number of replicates the compiler is
allowed to make for the associated variable. This attributes infers
an attribute memory:default and can't be applied with register attribute.

Syntax:

.. code-block:: text

  __attribute__((max_replicates(n)))

  where n is a non-negative integer constant greater than 1.

Example:

.. code-block:: c++

   const int __attribute__((max_replicates(2))) gc = 0;

   void foo () { int __attribute__((max_replicates(2))) lc; }

   struct st { int __attribute__((max_replicates(2))) f1; };

   static st s1;
   void bar() {
     s1.f1 = 0;
   }

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.cpp # HLS
  $ clang -cc1 -triple spir-unknown-unknown-intelfpga -ast-dump t.cl # OpenCL

.. code-block:: text

   -VarDecl 0x1cfcadd4ef8 <t.cpp:1:1, col:51> col:46 gc 'const int' cinit
    |-IntegerLiteral 0x1cfcadd5018 <col:51> 'int' 0
    |-MemoryAttr 0x1cfcadd4f78 <<invalid sloc>> Implicit Default
    `-MaxReplicatesAttr 0x1cfcadd4fb8 <col:26, col:42>
      `-ConstantExpr 0x1cfcadd4f60 <col:41> 'int'
        `-IntegerLiteral 0x1cfcadd4eb8 <col:41> 'int' 2

   `-VarDecl 0x1cfcb1d94c8 <col:3, col:42> col:42 lc 'int'
     |-MemoryAttr 0x1cfcb1d9548 <<invalid sloc>> Implicit Default
     `-MaxReplicatesAttr 0x1cfcb1d9588 <col:22, col:38>
       `-ConstantExpr 0x1cfcb1d9530 <col:37> 'int'
         `-IntegerLiteral 0x1cfcb1d9488 <col:37> 'int' 2

    -FieldDecl 0x1cfcb1d97b8 <line:9:3, col:42> col:42 referenced f1 'int'
     |-MemoryAttr 0x1cfcb1d9828 <<invalid sloc>> Implicit Default
     `-MaxReplicatesAttr 0x1cfcb1d9868 <col:22, col:38>
       `-ConstantExpr 0x1cfcb1d9810 <col:37> 'int'
         `-IntegerLiteral 0x1cfcb1d9778 <col:37> 'int' 2

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.cpp # HLS
  $ clang -cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm -o - t.cl  # OpenCL

.. code-block:: llvm

  @.str = private unnamed_addr constant [47 x i8] c"{memory:DEFAULT}{sizeinfo:4}{max_replicates:2}\00", section "llvm.metadata"
  @.str.1 = private unnamed_addr constant [6 x i8] c"t.cpp\00", section "llvm.metadata"

  define void @"?foo@@YAXXZ"() #0 {
  entry:
    %lc = alloca i32, align 4
    %lc1 = bitcast i32* %lc to i8*
    call void @llvm.var.annotation(i8* %lc1, i8* getelementptr inbounds ([47 x i8], [47 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0), i32 5)
    ret void
  }

  define void @"?bar@@YAXXZ"() #0 {
  entry:
    %0 = call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.st, %struct.st* @s1, i32 0, i32 0), i8* getelementptr inbounds ([47 x i8], [47 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0), i32 9)
    store i32 0, i32* %0, align 4
    ret void
  }
  }];
  let UserContent = [{
This attribute is applied to variables (local, statics, constants ...) in both
OpenCL and HLS. It specifies the maximum number of replicates the compiler is
allowed to make for the associated variable. This attribute infers an
attributes memory:default and can't be applied with register, numreadports or
numwriteports attributes.  This attribute replaces the deprecated
numreadports/numwriteports attributes.
  }];
}

def SimpleDualPortDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-8810";
  let RegressionTest = "SemaIntelHLS/local.cpp, CodeGenIntelHLS/local.cpp, SemaOpenCL/intel-fpga-local-hls.cl, CodeGenOpenCL/intel-fpga-local-hls.cl";
  let Owner = "zahiraam";
  let Content = [{
This attribute applies to the same variables (local, statics, constants ...) that
the memory attribute applies to in both OpenCl and HLS. It allows the user to
control the memory associated with the variable, requesting that the memory is
simple with dual port configuration mode. This attribute infers an attribute
memory:default and can't be applied with register attribute.

Syntax:

.. code-block:: text

  __attribute__((simple_dual_port))

Example:

.. code-block:: c++

   const int __attribute__((simple_dual_port)) gc = 0;

   void foo ()
   {
     int __attribute__((simple_dual_port)) lc;
   }

   struct st {
     int __attribute__((simple_dual_port)) f1;
   };

   static st s1;
   void bar() {
     s1.f1 = 0;
   }

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.cpp # HLS
  $ clang -cc1 -triple spir-unknown-unknown-intelfpga -ast-dump t.cl # OpenCL

.. code-block:: text

   -VarDecl 0x244738c7030 <t.cpp:1:1, col:50> col:45 gc 'const int' cinit
    |-IntegerLiteral 0x244738c7130 <col:50> 'int' 0
    |-MemoryAttr 0x244738c7098 <<invalid sloc>> Implicit Default
    `-SimpleDualPortAttr 0x244738c70d8 <col:26>

   `-VarDecl 0x24473d212a0 <col:3, col:41> col:41 lc 'int'
     |-MemoryAttr 0x24473d21308 <<invalid sloc>> Implicit Default
     `-SimpleDualPortAttr 0x24473d21348 <col:22>

   -FieldDecl 0x24473d21550 <line:9:3, col:41> col:41 referenced f1 'int'
    |-MemoryAttr 0x24473d215a8 <<invalid sloc>> Implicit Default
    `-SimpleDualPortAttr 0x24473d215e8 <col:22>

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.cpp # HLS
  $ clang -cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm -o - t.cl  # OpenCL

.. code-block:: llvm

   @.str = private unnamed_addr constant [49 x i8] c"{memory:DEFAULT}{sizeinfo:4}{simple_dual_port:1}\00", section "llvm.metadata"
   @.str.1 = private unnamed_addr constant [6 x i8] c"t.cpp\00", section "llvm.metadata"

   define void @"?foo@@YAXXZ"() #0 {
   entry:
     %lc = alloca i32, align 4
     %lc1 = bitcast i32* %lc to i8*
     call void @llvm.var.annotation(i8* %lc1, i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0), i32 5)
     ret void
   }

   define void @"?bar@@YAXXZ"() #0 {
   entry:
     %0 = call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.st, %struct.st* @s1, i32 0, i32 0), i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0), i32 9)
     store i32 0, i32* %0, align 4
     ret void
   }
  }];
  let UserContent = [{
This attribute applies to the same variables (local, statics, constants ...)
that the memory attribute applies to in both OpenCl and HLS. It allows the
user to control the memory associated with the variable, requesting that the
memory is simple with dual port configuration mode. This attribute infers an
attribute memory:default and can't be applied with register attribute.
  }];
}

def ReadWriteModeDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-10373";
  let RegressionTest = "";
  let Owner = "zahiraam";
  let Content = [{
The 'readwrite_mode' attribute is applied to HLS agent memory pointer
arguments. These are pointer arguments that already have the attribute
"agent_memory_argument". The argument "type" is mandatory, and its values are
"readonly", "writeonly", or "readwrite".
It is illegal to apply the attribute readwrite_mode on pointer arguments that
do not already have an attribute agent_memory_argument applied.

Syntax:

.. code-block:: text

   Type ::= 'readonly' | 'writeonly' | 'readwrite';
   __attribute__((readwrite_mode(<Type>)));

Example:

.. code-block:: c++

   void foo0_1(agent_arg __attribute__((readwrite_mode("readonly"))) int *i)
   {}


AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.cpp # HLS

.. code-block:: text

   FunctionDecl 0x20cec836848 <t1.cpp:6:19, line:21:1> foo0_1 'void (int *)'
   |-ParmVarDecl 0x20cec836710 <line:2:19, line:18:82>
     |-OpenCLLocalMemSizeAttr 0x20cec836778 <line:2:34, col:51> 32
     |-AgentMemoryArgumentAttr 0x20cec8367b8 <col:54>
     `-ReadWriteModeAttr 0x20cec8367c8 <line:18:48, col:73> "readonly"

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.cpp # HLS

.. code-block:: llvm

   define void @_Z6foo0_1Pi(i32* %i) {
   entry:
        %i.addr = alloca i32*, align 8
        store i32* %i, i32** %i.addr, align 8
        ret void
   }
   !2 = !{!"_Z6foo0_1Pi", !3}
   !3 = !{}
   !4 = !{!"mm_agent"}
   !5 = !{!"wire"}
   !6 = !{i32 0}
   !7 = !{!"i"}
   !8 = !{!"readwritememory: READONLY}"}

   }];
  let UserContent = [{
The 'readwrite_mode' attribute is applied to HLS agent memory pointer
arguments. These are pointer arguments that already have the attribute
"agent_memory_argument". The argument "type" is mandatory, and its values are
"readonly", "writeonly", or "readwrite".
It is illegal to apply the attribute readwrite_mode on pointer arguments that
do not already have an attribute agent_memory_argument applied.
  }];
}

def IntelFPGAForcePow2DepthDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-11256";
  let RegressionTest = "";
  let Owner = "zahiraam";
  let Content = [{
The 'force_pow2_depth' attribute is applied to a variable in OpenCl, HLS and
SYCL. It takes an integer argument between 0 and 1 that determines whether
the memory implemented for the variable should have a power-of-2 depth or not.
This attribute infers the 'memory' attribute, and it is illegal to apply it
to the attribute 'register'.
Syntax:

.. code-block:: text

   __attribute__((force_pow2_depth(n)));

   where n is a non-negative integer constant between 0 and 1.

Example:

.. code-block:: c++

   const int __attribute__((__force_pow2_depth__(0))) gc = 0;

   void foo ()
   {
     int __attribute__((__force_pow2_depth__(1))) lc;
   }

   struct st {
     int __attribute__((__force_pow2_depth__(0))) f1;
   };

   static st s1;
   void bar() {
     s1.f1 = 0;
   }

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.cpp

.. code-block:: text

   -VarDecl 0xf5ec2d8 <t.cpp:1:1, col:57> col:52 gc 'const int' cinit
    |-IntegerLiteral 0xf5ec430 <col:57> 'int' 0
    |-IntelFPGAMemoryAttr 0xf5ec360 <<invalid sloc>> Implicit Default
    `-ForcePow2DepthAttr 0xf5ec3b8 <col:26, col:48>
      `-ConstantExpr 0xf5ec340 <col:47> 'int' Int: 0
        `-IntegerLiteral 0xf5ec2a0 <col:47> 'int' 0

   -VarDecl 0xf5ec5d0 <col:3, col:48> col:48 lc 'int'
    |-IntelFPGAMemoryAttr 0xf5ec658 <<invalid sloc>> Implicit Default
    `-ForcePow2DepthAttr 0xf5ec6b0 <col:22, col:44>
      `-ConstantExpr 0xf5ec638 <col:43> 'int' Int: 1
        `-IntegerLiteral 0xf5ec598 <col:43> 'int' 1

   -FieldDecl 0xf5ec8f0 <line:9:3, col:48> col:48 referenced f1 'int'
    |-IntelFPGAMemoryAttr 0xf5ec960 <<invalid sloc>> Implicit Default
     `-ForcePow2DepthAttr 0xf5ec9b8 <col:22, col:44>
       `-ConstantExpr 0xf5ec940 <col:43> 'int' Int: 0
         `-IntegerLiteral 0xf5ec8b8 <col:43> 'int' 0

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.cpp

.. code-block:: llvm

   @.str = private unnamed_addr constant [49 x i8] c"{memory:DEFAULT}{sizeinfo:4}{force_pow2_depth:0}\00"
   @.str.2 = private unnamed_addr constant [49 x i8] c"{memory:DEFAULT}{sizeinfo:4}{force_pow2_depth:1}\00"

   define void @_Z3foov() #0 {
   entry:
     %lc = alloca i32, align 4
     %lc1 = bitcast i32* %lc to i8*
     call void @llvm.var.annotation(i8* %lc1, i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str.2, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0), i32 5)
     ret void
   }

   define void @_Z3barv() #0 {
   entry:
     %0 = call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.st, %struct.st* @_ZL2s1, i32 0, i32 0), i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0), i32 9)
  store i32 0, i32* %0, align 4
     ret void
   }
   }];
  let UserContent = [{
The 'force_pow2_depth' attribute is applied to a variable in OpenCl, HLS and
SYCL. It takes an integer argument between 0 and 1 that determines whether
the memory implemented for the variable should have a power-of-2 depth or not.
This attribute infers the 'memory' attribute, and it is illegal to apply it
to the attribute 'register'.
  }];
}

def HLSIIDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-7080";
  let RegressionTest = "SemaIntelHLS/hls_ii.cpp, CodeGenIntelHLS/hls_ii.cpp";
  let Owner = "eandrews";
  let Content = [{
The ``hls_ii`` attribute is a function attribute used to indicate that
consecutive invocations of an HLS component will be exactly 'k' cycles apart,
where 'k' is an unsigned integer argument greater than 0. This attribute is
incompatible with ``hls_force_loop_pipelining(off)`` , ``hls_min_ii`` and
``hls_max_ii``.

Syntax:

.. code-block:: text

  __attribute__((hls_ii(k)))

Example:

.. code-block:: c++

  __attribute__((hls_ii(8))) void foo() {}

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.cpp

.. code-block:: text

  FunctionDecl 0xed2eb38 <line:76:1, line:78:10> col:1 foo 'void ()'
  |-CompoundStmt 0xed2ec50 <col:9, col:10>
  `-HLSIIAttr 0xed2ec38 <line:77:16, col:24>
    `-ConstantExpr 0xed2ec20 <col:23> 'int'
      `-IntegerLiteral 0xed2eaf8 <col:23> 'int' 8

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.cpp

.. code-block:: llvm

  define void @_Z3foov() #0 !loop_ii_count !2
  !2 = !{i32 8}

  }];
  let UserContent = [{
The ``hls_ii`` attribute is a function attribute used to indicate that
consecutive invocations of an HLS component will be exactly 'k' cycles apart,
where 'k' is an unsigned integer argument greater than 0. This attribute is
incompatible with ``hls_force_loop_pipelining(off)``, ``hls_min_ii`` and
``hls_max_ii``.
  }];
}

def HLSMinIIDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-7080";
  let RegressionTest = "SemaIntelHLS/hls_min_ii.cpp, CodeGenIntelHLS/hls_min_ii.cpp";
  let Owner = "eandrews";
  let Content = [{
The ``hls_min_ii`` attribute is a function attribute used to indicate that
consecutive invocations of an HLS component will be atleast 'k' cycles apart,
where 'k' is an unsigned integer argument greater than 0. This attribute is
incompatible with ``hls_force_loop_pipelining(off)`` , ``hls_ii`` and
``hls_max_ii``.

Syntax:

.. code-block:: text

  __attribute__((hls_min_ii(k)))

Example:

.. code-block:: c++

  __attribute__((hls_min_ii(8))) void foo() {}

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.cpp

.. code-block:: text

  FunctionDecl 0xed2eb38 <line:76:1, line:78:10> col:1 foo 'void ()'
  |-CompoundStmt 0xed2ec50 <col:9, col:10>
  `-HLSMinIIAttr 0xed2ec38 <line:77:16, col:24>
    `-ConstantExpr 0xed2ec20 <col:23> 'int'
      `-IntegerLiteral 0xed2eaf8 <col:23> 'int' 8

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.cpp

.. code-block:: llvm

  define void @_Z3foov() #0 !min_ii !2
  !2 = !{i32 8}

  }];
  let UserContent = [{
The ``hls_min_ii`` attribute is a function attribute used to indicate that
consecutive invocations of an HLS component will be at least 'k' cycles apart,
where 'k' is an unsigned integer argument greater than 0. This attribute is
incompatible with ``hls_force_loop_pipelining(off)``, ``hls_ii`` and
``hls_max_ii``.
  }];
}

def HLSMaxIIDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-7080";
  let RegressionTest = "SemaIntelHLS/hls_max_ii.cpp, CodeGenIntelHLS/hls_max_ii.cpp";
  let Owner = "eandrews";
  let Content = [{
The ``hls_max_ii`` attribute is a function attribute used to indicate that
consecutive invocations of an HLS component will be at most 'k' cycles apart,
where 'k' is an unsigned integer argument greater than 0. This attribute is
incompatible with ``hls_force_loop_pipelining(off)`` , ``hls_ii`` and
``hls_min_ii``.

Syntax:

.. code-block:: text

  __attribute__((hls_max_ii(k)))

Example:

.. code-block:: c++

  __attribute__((hls_max_ii(8))) void foo() {}

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.cpp

.. code-block:: text

  FunctionDecl 0xed2eb38 <line:76:1, line:78:10> col:1 foo 'void ()'
  |-CompoundStmt 0xed2ec50 <col:9, col:10>
  `-HLSMaxIIAttr 0xed2ec38 <line:77:16, col:24>
    `-ConstantExpr 0xed2ec20 <col:23> 'int'
      `-IntegerLiteral 0xed2eaf8 <col:23> 'int' 8

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.cpp

.. code-block:: llvm

  define void @_Z3foov() #0 !max_ii !2
  !2 = !{i32 8}

  }];
  let UserContent = [{
The ``hls_max_ii`` attribute is a function attribute used to indicate that
consecutive invocations of an HLS component will be at most 'k' cycles apart,
where 'k' is an unsigned integer argument greater than 0. This attribute is
incompatible with ``hls_force_loop_pipelining(off)``, ``hls_ii`` and
``hls_min_ii``.
  }];
}

def HLSMaxInvocationDelayDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-7080";
  let RegressionTest = "SemaIntelHLS/hls_max_invocation_delay.cpp, CodeGenIntelHLS/hls_max_invocation_delay.cpp";
  let Owner = "eandrews";
  let Content = [{
The ``hls_max_invocation_delay`` attribute is a function attribute used to
indicate that the delay between the end of an invocation and start of the next
invocation, of an HLS component, will be at most 'k', where 'k' is an unsigned
integer argument greater than or equal to 0. This attribute is
incompatible with ``hls_force_loop_pipelining(off)``.

Syntax:

.. code-block:: text

  __attribute__((hls_max_invocation_delay(k)))

Example:

.. code-block:: c++

  __attribute__((hls_max_invocation_delay(8))) void foo() {}

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.cpp

.. code-block:: text

  FunctionDecl 0xed2eb38 <line:76:1, line:78:10> col:1 foo 'void ()'
  |-CompoundStmt 0xed2ec50 <col:9, col:10>
  `-HLSMaxInvocationDelayAttr 0xed2ec38 <line:77:16, col:24>
    `-ConstantExpr 0xed2ec20 <col:23> 'int'
      `-IntegerLiteral 0xed2eaf8 <col:23> 'int' 8

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.cpp

.. code-block:: llvm

  define void @_Z3foov() #0 !max_invocation_delay !2
  !2 = !{i32 8}

  }];
  let UserContent = [{
The ``hls_max_invocation_delay`` attribute is a function attribute used to
indicate that the delay between the end of an invocation and start of the next
invocation, of an HLS component, will be at most 'k', where 'k' is an unsigned
integer argument greater than or equal to 0. This attribute is incompatible
with ``hls_force_loop_pipelining(off)``.
  }];

}

def HLSForceLoopPipeliningDocs : Documentation {
  let Category = DocCatIntelHLSAttributes;
  let BugTracker = "CMPLRLLVM-7080";
  let RegressionTest = "SemaIntelHLS/hls_force_loop_pipelining.cpp, CodeGenIntelHLS/hls_force_loop_pipelining.cpp";
  let Owner = "eandrews";
  let Content = [{
The ``hls_force_loop_pipelining`` attribute is a function attribute used to
indicate whether the component will be pipelined to support multiple
invocations running concurrently. Valid attribute arguments are 'on' and 'off'.
When 'off', this attribute is incompatible with ``hls_ii`` , ``hls_min_ii`` ,
``hls_max_ii`` and ``hls_max_invocation_delay``.

Syntax:

.. code-block:: text

  __attribute__((hls_force_loop_pipelining(F)))

Example:

.. code-block:: c++

  __attribute__((hls_force_loop_pipelining("on"))) void foo1() {}
  __attribute__((hls_force_loop_pipelining("off"))) void foo2() {}

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.cpp

.. code-block:: text

  FunctionDecl 0xdff19d8 <t.cpp:2:1, col:64> col:6 foo1 'void ()'
  -CompoundStmt 0xdff1b00 <col:63, col:64>
  `-HLSForceLoopPipeliningAttr 0xdff1a70 <col:29, col:59> "on"
  FunctionDecl 0xdff1bd8 <line:3:1, col:66> col:6 foo2 'void ()'
  -CompoundStmt 0xdff1cc0 <col:65, col:66>
  `-HLSForceLoopPipeliningAttr 0xdff1c70 <col:30, col:61> "off"

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.cpp

.. code-block:: llvm

  define void @_Z4foo1v() #0 !force_loop_pipelining !2
  define void @_Z4foo2v() #0 !force_loop_pipelining !3
  !2 =  !{!"on"}
  !3 =  !{!"off"}

  }];
  let UserContent = [{
The ``hls_force_loop_pipelining`` attribute is a function attribute used to
indicate whether the component will be pipelined to support multiple
invocations running concurrently. Valid attribute arguments are 'on' and
'off'.  When 'off', this attribute is incompatible with ``hls_ii``,
``hls_min_ii``, ``hls_max_ii``, and ``hls_max_invocation_delay``.
  }];
}

def LongDoubleSizeConfigDocs : Documentation {
  let Category = DocCatIntelCompat;
  let BugTracker = "CMPLRLLVM-1870";
  let RegressionTest = "CodeGen/intel/long-double-config-size.c";
  let Content = [{
The ``fintel-long-double-size`` CC1 option controls the size of a long-double.
This is used as a part of the ICC compatibility option /Qlong-double, which on
Windows sets the compiler to use 80 bit long doubles instead of 64 bit.

This option has 3 valid values: 64, 80 and 128.

When set to 64, long double will be 64 bits and have a 64 bit alignment. 80
results in the x87 implementation, consistent with the behavior on Linux,
which is 96-bit width and 32 bit alignment on x86 and 128 bit width and
alignment on x86_64. This differs from the ICC documented behavior (which states
128 bit for both with no special consideration for x86), since it would result
in significant changes to how Clang/LLVM handles x87 doubles. On Windows, 80
results in 128 bit width and alignment on x86 and x86_64.

Finally, when set to 128, the long double type will be a 128 bit size and
alignment, consistent with the IEEE quad-double implementation.

Note that this option is likely only to be exposed as /Qlong-double for the
short term, so only 80 and Windows will be used. However, GCC has similar
functionality to permit setting of these values, so support is left for the
other two options in case those are useful in the future.
  }];
  let UserContent = [{
The ``fintel-long-double-size`` CC1 option controls the size of a long-double.
This is used as a part of the ICC compatibility option /Qlong-double, which on
Windows sets the compiler to use 80 bit long doubles instead of 64 bit.

This option has 3 valid values: 64, 80 and 128.
  }];
}

def RelaxSpirCCNoProtoDiagDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker = "CMPLRLLVM-8625";
  let RegressionTest = "Sema/intel/spircc_noproto_error.c";
  let Owner = "mprice1";
  let Content = [{
This change relaxes the 'function with no prototype cannot use the X calling
convention' diagnostic from an error to a warning. This diagnostic is
triggered for unprototyped function declarations for calling conventions that
do not support variadic calls.

When offloading, code is compiled twice, once for the host (where the
diagnostic usually doesn't apply) and once for the target where
CC_SpirFunction is the default. While the diagnostic is technically valid it
isn't ideal that the code compiles okay on the host but fails during the
target compilation. Since the target compile might not even use this function
the value of the diagnostic is questionable. For now we'll make it a warning
which allows it to be quieted with the option -Wno-missing-prototype-for-cc.

.. code-block:: c

  void foo();  // unprototyped c function

.. code-block:: console

  $ clang -cc1 -triple spir64 t.c
  t.c:1:6: error: function with no prototype cannot use the spir_function calling convention
  void foo();
       ^

  $ clang -cc1 -triple spir64 -fintel-compatibility t.c
  t.c:1:6: warning: function with no prototype cannot use the spir_function calling convention
  void foo();
       ^
  1 warning generated.

  $ clang -cc1 -triple spir64 -fintel-compatibility -Wno-missing-prototype-for-cc t.c
  $
  }];
  let UserContent = [{
This change relaxes the 'function with no prototype cannot use the X calling
convention' diagnostic from an error to a warning. This diagnostic is
triggered for unprototyped function declarations for calling conventions that
do not support variadic calls.
  }];
}

def IntrinsicPromotionDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker = "CMPLRLLVM-8909";
  let RegressionTest = "CodeGen/intel/intrin-promotion.c";
  let Owner = "ekeane1";
  let Content = [{
This change automatically promotes the target features of functions that use intrinsics
or intrinsic builtins that would otherwise be disallowed due to always-inlining. The
error message that this should prevent is "<intrin-name> needs target feature <feature>."
Only intrinsics defined in the system headers will cause this promotion to happen
automatically.

This feature can be enabled by using the -Xclang -mintrinsic-promote flag.

Use of this feature is highly discouraged and can cause difficult to diagnose runtime
errors to occur. Using this flag will result in the promoted function containing
instructions not supported by the target architecture, so it should only be used
in combination with some sort of feature based manual dispatch system.

A less intrusive (generated-code wise) mechanism of accomplishing the same task
is to explicitly mark functions using the target attribute:

.. code-block:: c

  __attribute__((target("avx")))
  void uses_avx_intrin(__m128 a, __m128 b) {
    return _mm_cmp_ps(a, b, 0);
  }


  }];
  let UserContent = [{
This change automatically promotes the target features of functions that use
intrinsics or intrinsic builtins that would otherwise be disallowed due to
always-inlining. The error message that this should prevent is '<intrin-name>
needs target feature <feature>.' Only intrinsics defined in the system headers
will cause this promotion to happen automatically.
  }];
}

def CpuDispatchUseLibIrcDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let RegressionTest = "CodeGen/intel/attr-cpudispatch.c";
  let Owner = "ekeane1";
  let Content = [{
Typically, the cpu-dispatch functionality uses libgcc's (or compiler-rt's)
infrastructure to test the current cpu features list. However, due to some
customizations to libirc, including a highjacking feature that is useful for
validating multiversioning changes, using libirc is necessary and beneficial
in Intel Compat mode.
  }];
  let UserContent = [{
This change causes cpu-dispatch/cpu-specific multiversioning to use libirc
as its source of cpu-feature data. This library features finer grained feature
checking, as well as permits the highjacking of the feature list via environment
variable, which can be used to validate versions on newer hardware.
  }];
}

def DisplayFullFilePathDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker = "CMPLRLLVM-26876,CMPLRLLVM-28856";
  let RegressionTest = "CodeGenCXX/intel/display-filename-or-path.cpp";
  let Owner = "smanna";
  let Default = 0;
  let Content = [{
Clang currently shows the full build-machine paths of our program files as
specified on the command-line into the final assembly when using
``__builtin_FILE`` since CMake always uses full-paths. This change adds a new
command-line switch that is ON by default in -fintel-compatibility mode to
control the behavior that ``__builtin_FILE`` should only include the filename
(not the path) by default.

.. code-block:: c++

   void takes_const_char_ptr(const char * FN = __builtin_FILE());

   int main() {
     takes_const_char_ptr();
   }

.. code-block:: console

   $ clang -cc1 -fintel-compatibility -fintel-compatibility-enable=DisplayFullFilePath -emit-llvm -o - `pwd`/t.cpp

.. code-block:: llvm

   @.str = private unnamed_addr constant [48 x i8] c"`pwd`/t.cpp\00", align 1

.. code-block:: console

   $ clang -cc1 -fintel-compatibility -fintel-compatibility-disable=DisplayFullFilePath -emit-llvm -o - `pwd`/t.cpp

.. code-block:: llvm

   @.str = private unnamed_addr constant [6 x i8] c"t.cpp\00", align 1

  }];
  let UserContent = [{
Clang currently shows the full build-machine paths of our program files as
specified on the command-line into the final assembly when using
``__builtin_FILE`` since CMake always uses full-paths. New command-line
switch controls the behavior to include filename or full file path when
using ``__builtin_FILE``.
  }];
}

def UnderbarPragmaEnabledDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let Owner = "ekeane1";
  let RegressionTest = "Lexer/intel/underbar-pragma.cpp";
  let Default = 1;
  let Content = [{
Microsoft accepts `__pragma` as a keyword alternative for `#pragma`. ICC accepts
this in Linux mode as well, so this feature enables it in Intel compatibility
mode.

.. code-block:: c++

  __pragma(warning(push)); // MS/ICC Compat spelling
  _Pragma(warning(push)); // C99/C++11 spelling.
  #pragma(warning(push)); // Normal directive.

}];
let UserContent = [{
This change allows the use of the `__pragma` keyword as an alternative to
#pragma` or `_Pragma`.

.. code-block:: c++

  __pragma(warning(push)); // MS/ICC Compat spelling
  _Pragma(warning(push)); // C99/C++11 spelling.
  #pragma(warning(push)); // Normal directive.

}];
}

def GlobalDocumentation {
  code Intro =[{..
  -------------------------------------------------------------------
  NOTE: This file is automatically generated by running
  clang-tblgen -gen-intel-cust-docs. Do not edit this file by hand!!
  -------------------------------------------------------------------

===============================
Intel-only Clang Customizations
===============================
.. contents::
   :local:

Introduction
============

This page lists the Intel customizations we've added to Clang. All
customizations can be described here in a single location and the description
can be reviewed with the code implementing the customization.  Making this
part of the code review means we can ensure all changes are documented
and will never be lost in bug tracking or source control changes.

Each customization is associated with a tag both here and in the sources.  When
encountering this customization during development or resolving a merge
conflict we can go directly here for information.

It also allows us a means to enumerate all our changes so they can be reviewed
from time to time.  For example if we are ready to release a new version of
the compiler where support for X is no longer needed, we can look at these
and remove any related to X.

Each customization has an associated category and additional categories can
be added as needed.

Customizations will have whatever properties we decide we need.  Some
possibilities:

 - BugTracker - Jira number in case more investigation is needed. We want it
   to be rare to go here though.
 - Customer - If an important customer requested this.
 - RegressionTest - To easily find some real code for this feature.
 - Owner - Who implemented it or someone who knows something about it if it
   goes wrong or needs to be merged with community changes.
 - CompatVersions - If this is some compatibility fix which does it apply to.
   Possibly update this field when new MS/GCC compilers are released and
   remove if we don't care about it anymore.

The Content should describe the basic functionality, give a small
code snippet, and describe any unusual or tricky parts of the implementation.

Using Tags
==========

Each documented item has a tag.  The tag can be used in the sources to point
the reader to this documentation.

So instead of:

.. code-block:: c

  #if INTEL_CUSTOMIZATION
    <code implementing foobar extension>
  #endif // INTEL_CUSTOMIZATION

We can add the tag along with the code:

.. code-block:: c

  #if INTEL_CUSTOMIZATION
    // IntelFooBarExtension
    <code implementing foobar extension>
  #endif // INTEL_CUSTOMIZATION

If we come across this block reading code or resolving a merge conflict we can
easily open the docs and find out why it is here.

In addition when used with an option like IntelCompat we can use the tag as
part of the condition that checks the flag:

.. code-block:: c

  if (getContext().getLangOpts().isIntelCompat(LangOptions::IntelFooBarExtension))

Not only does this identify the customization it allows us to individually
control that customization.  If we are triaging a bug and think this one is
to blame we can easily test it with a command line option:

.. code-block:: console

  $ icx -Xclang -fintel-compatibility -fintel-compatibility-disable=IntelFooBarExtension

Or if we want to just enable that one item:

.. code-block:: console

  $ icx -Xclang -fintel-compatibility-enable=IntelFooBarExtension

These options are for internal use only.  They are for compiler developers to
more easily understand and maintain the code.  We don't want customers using
them and we don't want anyone relying on specific names.  We can expect to
remove or rename them when it makes sense.

A list of current tags can be shown with:

.. code-block:: console

  $ icx -Xclang -fintel-compatibility-help

Using these in normal development should be very easy.  When implementing
a new extension you add the documentation to IntelCustDocs.td and then just
use the tag in your code.  The documentation and code are developed and
reviewed together.

Building The Documentation
==========================

The clang documentation is not built by default.  To build it along with the
compiler use:

.. code-block:: console

  $ ics build -j<n> -sphinx

Important: Our builds use makefiles generated by cmake. The makefiles are not
regenerated every ics build.  To get documentation you need cmake to generate
new makefiles with the sphinx targets.  The easiest way to accomplish that is
to remove your builds directory and run the ics build command above.

Writing Documentation
============================
Clang/LLVM documentation is written in reStructuredText format and placed in
.rst files.  Sphinx is used to generate html from the .rst file.

You can just use the existing documentation as examples and start writing.
More details can be found in this documentation:

https://media.readthedocs.org/pdf/sphinx/stable/sphinx.pdf

There are also online sphinx editors that you can paste your rST into to see
how it looks.  One is here:

https://livesphinx.herokuapp.com/

Suggested Content / Template
============================
In order to make investigating merge conflicts or bugs in our customizations
easier we'd like to have a fully compilable, but very small test case for each
entry. It should also include the exact command line to compile that example
to view the AST and/or see the IR.  Assume the reader knows nothing about the
item and wants to quickly see what it does or step through it in the debugger.

Here is a suggested minimal template for cases that introduce new syntax
(such as pragmas and attributes).  Additional info can be added anywhere
it makes sense.  Extensions that do not add new syntax would likely omit
some of these or add different sections.

.. code-block:: text

  <A very high level user description of what this pragma/attribute does.>

  Syntax:

  .. code-block:: text

    <syntax diagram or other description of the syntax>

  Example:

  .. code-block:: c

    <fully-compilable small test case>

  AST:

  .. code-block:: console

    $ <full command line to display the AST for the above example>

  .. code-block:: text

    <snippet from the AST showing the new construct>

  IR:

  .. code-block:: console

    $ <full command line to displat the LLVM IR for the above example>

  .. code-block:: llvm

    <snippet from the IR dump showing the new construct>
  }];
}

//===--- IntelCustDocs.td - Documentation for intel-specific attrs --------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
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
// clang-tblgen -gen-intel-docs -I <root>/llvm/tools/clang/include
//   <root>/llvm/tools/clang/include/clang/Basic/intel/IntelCustDocs.td -o
//   <root>/llvm/tools/clang/docs/IntelCustomizations.rst
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
}

// New categories can be added by adding a new record here.
def DocCatIntelLoops : DocumentationCategory<"Intel Loop Pragmas">;
def DocCatIntelOptimization : DocumentationCategory<"Optimization">;
def DocCatIntelHLS : DocumentationCategory<"HLS/FPGA">;
def DocCatIntelOpenMP : DocumentationCategory<"OpenMP">;
def DocCatIntelCompat : DocumentationCategory<"Intel Compatibility">;
def DocCatMSCompat : DocumentationCategory<"Microsoft Compatibility">;

// Specifies the documentation to be associated with the given category.
class Documentation {
  DocumentationCategory Category;
  code Content;
  // The Option is optional.  If used it allows splitting based on a tag.
  // This works well for options like IntelCompat which control many small
  // "features".
  string Option = "";
  // The remaining fields are TBD. Items can be added (with an initialized
  // string) or removed here without the need to change the tblgen code.
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
  let Content = [{
Microsoft allows you to catch a string literal with catch(char \*).

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
}
def IntelTBAABFDocs : Documentation {
  let Category = DocCatIntelOptimization;
  let Option = "IntelCompat";
  let BugTracker= "CMPLRLLVM-2066";
  let RegressionTest = "test/CodeGen/intel/bitfield-tbaa1.c";
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
}

// Intel Compatibility

def AllowMissingTypenameDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker = "CQ#368310,CMPLRS-25573";
  let RegressionTest = "cfeCpp/cq117771";
  let Owner = "";
  let CompatVersions = "";
  let Content = [{
Microsoft allows you to leave off typename in some contexts.  Clang also
allows this in MSVCCompat mode.  See isMicrosoftMissingTypename in
SemaDecl.cpp.

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
}

def AllowExtraArgumentDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker = "CQ#364630,CMPLRS-15158";
  let RegressionTest = "c9xcC/c9x_7_24_4_5_8_10_a,...";
  let Owner = "";
  let CompatVersions = "";
  let Content = [{
Microsoft allows the call with extra arguments with warning in C. Clang and gcc emit error.

The Intel compiler allows this with warning in C.

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

  #pragma vector {always[assert]|aligned|unaligned|temporal|nontemporal|[no]vecremainder|[no]mask_readwrite}

``pragma vector`` with no other keyword will be treated as an additional
spelling of the community-supported ``pragma vectorize(enable)``.

Currently we only support ``pragma vector`` with no keywords, or with 'always'.

Example:

.. code-block:: c

  void foo(int lb, int ub, int *a, int *b) {
    #pragma vector
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
    #pragma vector always
    for(int j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
  }

AST:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -ast-dump t.c

.. code-block:: text

  AttributedStmt 0xed8d0f0 <<invalid sloc>, line:3:46>
  | |-LoopHintAttr 0xed8d0c8 <line:2:11> Implicit vector Vectorize Enable
  | | |-<<<NULL>>>
  | | `-<<<NULL>>>
  | `-ForStmt 0xed8d090 <line:3:3, col:46>

  AttributedStmt 0xed8d628 <<invalid sloc>, line:5:46>
  |-LoopHintAttr 0xed8d5d8 <line:4:11> Implicit vector Vectorize Enable
  | |-<<<NULL>>>
  | `-<<<NULL>>>
  |-LoopHintAttr 0xed8d600 <col:11> Implicit vector VectorizeAlways Enable
  | |-<<<NULL>>>
  | `-<<<NULL>>>
  `-ForStmt 0xed8d5a0 <line:5:3, col:46>

Note than ``#pragma vector always`` will be represented as two LoopHint
attributes.

IR:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2

  br label %for.cond6, !llvm.loop !4

  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.vectorize.enable", i1 true}
  !4 = distinct !{!4, !5, !3}
  !5 = !{!"llvm.loop.vectorize.ignore_profitability"}
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
}

def PragmaBlockLoopDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Option = "IntelCompat";
  let RegressionTest = "Parser/intel/block_loop.cpp, Sema/intel/block_loop.cpp, CodeGenCXX/intel/block_loop.cpp";
  let Owner = "jyu2";
  let Content = [{
The ``block_loop`` pragma lets you exert greater control over optimizations
on a specific loop inside a nested loop.

Syntax:

.. code-block:: text

  #pragma block_loop [level(levels)] [private(vars)] [factor(expr)]

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

AST:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -ast-dump -o - t.c

.. code-block:: text

  AttributedStmt 0xdb151f8 <<invalid sloc>, line:5:41>
  |-IntelBlockLoopAttr 0xdb15190 <line:3:11, col:56> Implicit 1 2
  | |-IntegerLiteral 0xdb14cb0 <col:40> 'int' 256
  | |-IntegerLiteral 0xdb14cb0 <col:40> 'int' 256
  | `-DeclRefExpr 0xdb14d30 <col:53> 'int' lvalue ParmVar 0xdab48c0 'var' 'int'
  `-ForStmt 0xdb15158 <line:4:3, line:5:41>

IR:

.. code-block:: console

  $ clang -cc1 -fintel-compatibility -emit-llvm -o - t.c

.. code-block:: llvm

  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.PRIVATE"(i32* %var.addr), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 256), "QUAL.PRAGMA.LEVEL"(i32 2), "QUAL.PRAGMA.FACTOR"(i32 256) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
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
}

def PragmaIVDepDocs : Documentation {
  let Category = DocCatIntelLoops;
  let Owner = "mprice1,pmrao";
  let Content = [{
The ``ivdep`` pragma instructs the compiler to ignore assumed vector
dependencies in a loop.

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

  AttributedStmt 0xe315d38 <<invalid sloc>, line:4:31>
  |-LoopHintAttr 0xe315d10 <line:3:11> Implicit ivdep IVDepHLS Enable

  AttributedStmt 0xe315f88 <<invalid sloc>, line:6:31>
  |-LoopHintAttr 0xe315f60 <line:5:11, col:27> Implicit ivdep IVDepHLS Numeric

  AttributedStmt 0xe3161e0 <<invalid sloc>, line:8:31>
  |-LoopHintAttr 0xe3161b8 <line:7:11, col:27> Implicit ivdep IVDepHLS LoopExpr

  AttributedStmt 0xe316458 <<invalid sloc>, line:10:31>
  |-LoopHintAttr 0xe316430 <line:9:11, col:28> Implicit ivdep IVDepHLS Full

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2

  br label %for.cond2, !llvm.loop !4

  %4 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"([16 x i32]* %Arr, i32 -1) ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.PRAGMA.END.IVDEP"() ]

  %7 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"([16 x i32]* %Arr, i32 4) ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.PRAGMA.END.IVDEP"() ]

  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.ivdep.enable"}
  !4 = distinct !{!4, !5}
  !5 = !{!"llvm.loop.ivdep.safelen", i32 4}

  }];
}

def HLSPragmaDocs : Documentation {
  let Category = DocCatIntelHLS;
  let BugTracker = "CMPLRLLVM-5584";
  let RegressionTest = "SemaIntelHLS/pragma.cpp, CodeGenIntelHLS/pragma.cpp";
  let Owner = "jyu2";
  let Content = [{
These are simple HLS/FPGA loop pragmas that either have one constant integral
argument,or no arguments.  ** Note: Let's add new topics for any additional
pragmas instead of adding onto to this topic. **

Syntax:

.. code-block:: text

  #pragma [ii|ii_at_most|ii_at_least] [<constant-int>]
  #pragma [speculated_iterations|max_concurrency] [<constant-int>]
  #pragma min_ii_at_target_fmax
  #pragma disable_loop_pipelining

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
  #pragma disable_loop_pipelining, DisableLoopPipelining, Enable, llvm.loop.intel.pipelining.disable

Example:

.. code-block:: c

  void foo() {
    int var = 0;
    #pragma ii_at_least 4
    for (int i = 0; i < 10; ++i) var++;
    #pragma disable_loop_pipelining
    for (int i = 0; i < 10; ++i) var++;
  }

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump  t.c

.. code-block:: text

  AttributedStmt 0xe4efd70 <<invalid sloc>, line:4:42>
  |-LoopHintAttr 0xe4efd48 <line:3:11, col:24> Implicit ii_at_least IIAtLeast Numeric
  | | |-IntegerLiteral 0xe4efa98 <col:23> 'int' 4
  | | `-<<<NULL>>>
  | `-ForStmt 0xe4efd10 <line:4:3, col:42>

  AttributedStmt 0xe4f0068 <<invalid sloc>, line:6:42>
  |-LoopHintAttr 0xe4f0040 <line:5:11> Implicit disable_loop_pipelining DisableLoopPipelining Enable
  | |-<<<NULL>>>
  | `-<<<NULL>>>
  `-ForStmt 0xe4f0008 <line:6:3, col:42>

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.c

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2

  br label %for.cond2, !llvm.loop !4

  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.intel.ii.at.least.count", i32 4}
  !4 = distinct !{!4, !3, !5}
  !5 = !{!"llvm.loop.intel.pipelining.disable"}
  }];
}

def HLSForceHyperoptPragmaDocs : Documentation {
  let Category = DocCatIntelHLS;
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
}

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
}

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
}

def MayIUseFeatureDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "None";
  let RegressionTest ="test/CodeGen/intel/may-i-use-cpu-feature.cpp";
  let Owner = "ekeane1";
  let Content = [{
The Intel Compiler implements a cpu-id like function to determine the
current processor's features called _may_i_use_cpu_feature. This function
accepts a compile-time constant unsigned 64 bit integer that contains a
bitmask to validate against the current processor's feature list.

This function returns a boolean 'true' if every feature in the bitmask
parameter is supported by the current host processor.

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
}

def ClusterDocs : Documentation {
  let Category = DocCatIntelHLS;
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
}

def StallEnableDocs : Documentation {
  let Category = DocCatIntelHLS;
  let BugTracker = "CMPLRLLVM-5585";
  let RegressionTest = "SemaIntelHLS/stall_enable.cpp, CodeGenIntelHLS/stall_enable.cpp, SemaOpenCL/intel-fpga-stall_enable.cl, CodeGenOpenCL/intel-fpga-stall_enable.cl";
  let Owner = "eandrews";
  let Content = [{
The ``stall_enable`` attribute is an FPGA function and lambda function
attribute used in HLS and OpenCL to generate FPGA hardware for the function
using stall/enable protocol. The attribute takes no arguments and adds
metadata to the function/lambda definition.

Syntax:

.. code-block:: text

  __attribute__((stall_enable))

C++11 attribute syntax is also supported on HLS:

.. code-block:: text

  [[clang::stall_enable]]

Example:

.. code-block:: c

  void __attribute__((stall_enable)) foo() {}

AST:

.. code-block:: console

  $ clang -cc1 -fhls -ast-dump t.c

.. code-block:: text

  FunctionDecl 0x235b69b8d98 <t.c:1:1, col:43> col:36 foo 'void ()'
  |-CompoundStmt 0x235b69b8eb0 <col:42, col:43>
  `-StallEnableAttr 0x235b69b8e30 <col:21>

IR:

.. code-block:: console

  $ clang -cc1 -fhls -emit-llvm -o - t.c

.. code-block:: llvm

  define void @_Z4foo1v() #0 !stall_enable !2 {}
  !2 = !{i32 1}
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

The Content should describe the basic functionality, give a small code snippet,
and describe any unusual or tricky parts of the implementation.

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

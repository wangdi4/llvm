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
def DocCatMSCompat : DocumentationCategory<"Microsoft Compatibility">;
def DocCatIntelCompat : DocumentationCategory<"Intel Compatibility">;
def DocCatIntelOptimization : DocumentationCategory<"Optimization">;

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
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker= "";
  let RegressionTest = "test/Sema/intel/vector.cpp,test/CodeGenCXX/intel/vector.cpp";
  let Owner = "jyu2";
  let CompatVersions = "";
  let Content = [{
The ``novector`` pragma specifies that a particular loop should never be
vectorized, even if it is legal to do so. When avoiding vectorization of a
loop is desirable (when vectorization results in a performance regression
rather than improvement), the ``novector`` pragma can be used in the source text
to disable vectorization of a loop. This behavior is in contrast to the ``vector
always`` pragma.

the ``pragma novector`` will be recognized and map to existing

.. code-block:: c

  #pragma vectorize(disable)

For :

.. code-block:: c

  #pragma novector
  for(;;);

AST:

AttributedStmt
      LoopHintAttr Implicit novector Vectorize Disable

Metadata:

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2
  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.vector.vectorize.width", i32 1}

A correct usage of this pragma:

.. code-block:: c

  void foo(int lb, int ub) {
  #pragma novector
  for(j=lb; j<ub; j++) { a[j]=a[j]+b[j]; }
  }

  }];

}

def PragmaVectorDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker= "";
  let RegressionTest = "test/Sema/intel/vector.cpp,test/CodeGenCXX/intel/vector.cpp";
  let Owner = "jyu2";
  let CompatVersions = "";
  let Content = [{
The ''pragma vector'' indicates to the compiler that the loop should be vectorized according to the argument keywords.
  "assert, aligned, unaligned, vecremainder, novecremainder">;

.. code-block:: c

  vector {always[assert]|aligned|unaligned|temporal|nontemporal|[no]vecremainder|[no]mask_readwrite}

``pragma vector`` will be represented like the existing:

.. code-block:: c

  #pragma vectorize(enable)

For :

.. code-block:: c

  #pragma vector
  for(;;);

AST:

AttributedStmt
      LoopHintAttr Implicit vector Vectorize Enable

Metadata:

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2
  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.vector.vectorize.enable", i1 true}

``#pragma vector always`` will be represented as two LoopHint attributes:

.. code-block:: c

  #pragma vector always
  for(;;);

AST:

AttributedStmt
      LoopHintAttr Implicit vector VectorizeAlways Enable

      LoopHintAttr Implicit vector Vectorize Enable

Metadata:

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2
  !2 = distinct !{!2, !3 !4}
  !3 = !{!"llvm.loop.ignore_profitability"}
  !4 = !{!"llvm.loop.vectorize.enable", i1 true}

  }];

}

def PragmaFusionDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Owner = "pmrao";
  let Content = [{
The ``fusion`` pragma instructs the compiler to fuse the loop with adjacent loops.

This is enabled with -fintel-compatibility as:

.. code-block:: c

  #pragma fusion

AST:

AttributedStmt
      LoopHintAttr Implicit fusion Fusion Enable

Metadata:

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2
  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.fusion.enable"}

  }];
}

def PragmaNoFusionDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Owner = "pmrao";
  let Content = [{
The ``nofusion`` pragma instructs the compiler not to fuse the loop with adjacent loops.

This is enabled with -fintel-compatibility as:

.. code-block:: c

  #pragma nofusion

AST:

AttributedStmt
      LoopHintAttr Implicit nofusion Fusion Disable

Metadata:

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2
  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.fusion.disable"}

  }];
}

def PragmaUnrollAndJamDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Owner = "pmrao";
  let Content = [{
The ``unroll_and_jam`` pragma instructs the compiler to enable unrolling and jamming loops.

This is enabled with -fintel-compatibility as:

.. code-block:: c

  #pragma unroll_and_jam

AST:

AttributedStmt
      LoopHintAttr Implicit unroll_and_jam UnrollAndJam Enable

Metadata:

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2
  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.unroll_and_jam.enable"}

The pragma accepts an optional integer argument as:

.. code-block:: c

  #pragma unroll_and_jam (n)

AST:

AttributedStmt
      LoopHintAttr Implicit unroll_and_jam UnrollAndJam Numeric
        -IntegerLiteral 'int' n


Metadata:

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2
  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.unroll_and_jam.count", i32 n}

  }];
}

def PragmaNoUnrollAndJamDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Owner = "pmrao";
  let Content = [{
The ``nounroll_and_jam`` pragma instructs the compiler to disable unrolling and jamming loops.

This is enabled with -fintel-compatibility as:

.. code-block:: c

  #pragma nounroll_and_jam

AST:

AttributedStmt
      LoopHintAttr Implicit nounroll_and_jam NoUnrollAndJam Disable

Metadata:

.. code-block:: llvm

  br label %for.cond, !llvm.loop !2
  !2 = distinct !{!2, !3}
  !3 = !{!"llvm.loop.unroll_and_jam.disable"}

  }];
}

def PragmaBlockLoopDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let Owner = "jyu2";
  let Content = [{
The ``block_loop`` pragma lets you exert greater control over optimizations
on a specific loop inside a nested loop.

.. code-block:: c

  #pragma block_loop [level(levels)] [private(vars)] [factor(expr)]

Adding new IntelBlockLoopAttr intead using LoopHint attribute framework. No
metadata are generated.  Directives are requested here.

For:

.. code-block:: c

  #pragma block_loop level(1:2) factor(256) private(var)
   for (...)
    for (...);

AST:

AttributedStmt

.. code-block:: c

   |-IntelBlockLoopAttr 0xcf13e98 <line:5:9, col:54> Implicit 1 2
      | |-IntegerLiteral 0xcee1508 <col:40> 'int' 256
      | |-IntegerLiteral 0xcee1538 <col:40> 'int' 256
      | `-DeclRefExpr 0xcee1588 <col:51> 'int' lvalue Var 0xcee1460 'var' 'int'

IR:

.. code-block:: llvm

  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.PRIVATE"(double* %var.addr), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 16), "QUAL.PRAGMA.LEVEL"(i32 2), "QUAL.PRAGMA.FACTOR"(i32 16) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]

  }];

}

def PragmaIVDepDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Owner = "mprice1,pmrao";
  let Content = [{
The ``ivdep`` pragma instructs the compiler to ignore assumed vector
dependencies in a loop.

The HLS form (enabled with -fhls) is:

.. code-block:: c

  #pragma ivdep [safelen(n)] [array(<arrayname>)]

The Intel form (enabled with -fintel-compatibility) is:

.. code-block:: c

  #pragma ivdep [back|loop]

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

  }];
}

def HLSPragmaDocs : Documentation {
  let Category = DocCatIntelCompat;
  let BugTracker = "Cmplrllvm-5584";
  let RegressionTest = "SemaIntelHLS/pragma.cpp, CodeGenIntelHLS/pragma.cpp";
  let Owner = "jyu2";
  let Content = [{
The ``ii, ii_at_most, ...`` pragmas are FPGA loop pragmas

The HLS form (enabled with -fhls) is:

.. code-block:: c

  #pragma ii <4>
  #pragma ii_at_most <4>
  #pragma ii_at_least <4>
  #pragma speculated_iterations <4>
  #pragma max_concurrency <4>
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

for example:
using command line:

.. code-block:: bash

  $ clang -cc1 -fhls -ast-dump -o - t.c
  $ clang -cc1 -fhls -emit-llvm -o - t.c

.. code-block:: c

  void foo() {
    int var = 0;
    #pragma max_concurrency  4
    #pragma ii_at_least 4
    for (int i = 0; i < 10; ++i) var = var+1;
  }

AST:

AttributedStmt

.. code-block:: c

    `-AttributedStmt 0xd2b86c0 <<invalid sloc>, line:21:18>
      |-LoopHintAttr 0xd2b8670 <line:15:11, col:30> Implicit max_concurrency MaxConcurrency Numeric
      | |-IntegerLiteral 0xd2b83a0 <col:28> 'int' 4
      | `-<<<NULL>>>
      |-LoopHintAttr 0xd2b8698 <line:16:11, col:26> Implicit ii_at_least IIAtLeast Numeric
      | |-IntegerLiteral 0xd2b83e0 <col:24> 'int' 4
      | `-<<<NULL>>>

.. code-block:: llvm

  !3 = !{!"llvm.loop.max_concurrency.count", i32 4}
  !4 = !{!"llvm.loop.intel.ii.at.least.count", i32 100}

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

.. code-block:: bash

  clang -cc1 -fintel-compatibility w.cpp -flto -flto-unit -fwhole-program-vtables -emit-llvm -o -

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
}];
}

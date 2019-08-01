// INTEL -- xmain inlining logic prefers cloning foo() to inlining, so this test
//          fails.  To work around that, disable xmain-specific inlining logic.
// INTEL -- loopopt significantly changes the pass pipeline and makes the test
//          fail so we disable it.
// INTEL -- Our jump threading customizations can cause the unroll routine to be optimized as if loop
//          unrolling had been done, so we disable them also.
// RUN: %clang_cc1 -O2 -fprofile-sample-use=%S/Inputs/pgo-sample-thinlto-summary.prof %s -emit-llvm -mllvm -inline-for-xmain=0 -mllvm -loopopt=0 -o - 2>&1 | FileCheck %s -check-prefix=SAMPLEPGO
// RUN: %clang_cc1 -O2 -fprofile-sample-use=%S/Inputs/pgo-sample-thinlto-summary.prof %s -emit-llvm -mllvm -jump-thread-loop-header=false -mllvm -inline-for-xmain=0 -mllvm -loopopt=0 -flto=thin -o - 2>&1 | FileCheck %s -check-prefix=THINLTO
// Checks if hot call is inlined by normal compile, but not inlined by
// thinlto compile.

int baz(int);
int g;

void foo(int n) {
  for (int i = 0; i < n; i++)
    g += baz(i);
}

// SAMPLEPGO-LABEL: define {{(dso_local )?}}void @bar
// THINLTO-LABEL: define {{(dso_local )?}}void @bar
// SAMPLEPGO-NOT: call{{.*}}foo
// THINLTO: call{{.*}}foo
void bar(int n) {
  for (int i = 0; i < n; i++)
    foo(i);
}

// Checks if loop unroll is invoked by normal compile, but not thinlto compile.
// SAMPLEPGO-LABEL: define {{(dso_local )?}}void @unroll
// THINLTO-LABEL: define {{(dso_local )?}}void @unroll
// SAMPLEPGO: call{{.*}}baz
// SAMPLEPGO: call{{.*}}baz
// THINLTO: call{{.*}}baz
// THINLTO-NOT: call{{.*}}baz
void unroll() {
  for (int i = 0; i < 2; i++)
    baz(i);
}

// Checks that icp is not invoked for ThinLTO, but invoked for normal samplepgo.
// SAMPLEPGO-LABEL: define {{(dso_local )?}}void @icp
// THINLTO-LABEL: define {{(dso_local )?}}void @icp
// SAMPLEPGO: if.true.direct_targ
// FIXME: the following condition needs to be reversed once
//        LTOPreLinkDefaultPipeline is customized.
// THINLTO-NOT: if.true.direct_targ
void icp(void (*p)()) {
  p();
}

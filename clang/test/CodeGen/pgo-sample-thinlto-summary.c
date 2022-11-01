// INTEL_CUSTOMIZATION
// INTEL -- xmain inlining logic prefers cloning foo() to inlining, so this test
//          fails.  To work around that, disable xmain-specific inlining logic.
// INTEL -- loopopt significantly changes the pass pipeline and makes the test
//          fail so we disable it.
// INTEL -- Our jump threading customizations can cause the unroll routine to be optimized as if loop
//          unrolling had been done, so we disable them also.
// Checks if hot call is inlined by normal compile, but not inlined by
// thinlto compile.
// RUN: %clang_cc1 -fdebug-pass-manager -O2 -fprofile-sample-use=%S/Inputs/pgo-sample-thinlto-summary.prof %s -emit-llvm -mllvm -loopopt=0 -o - 2>&1 | FileCheck %s -check-prefix=SAMPLEPGO
// RUN: %clang_cc1 -fdebug-pass-manager -O2 -fprofile-sample-use=%S/Inputs/pgo-sample-thinlto-summary.prof %s -emit-llvm -flto=thin -mllvm -loopopt=0 -o - 2>&1 | FileCheck %s -check-prefix=THINLTO
// end INTEL_CUSTOMIZATION

int baz(int);
int g;

void foo(int n) {
  for (int i = 0; i < n; i++)
    g += baz(i);
}

// Checks that loop unroll and icp are invoked by normal compile, but not thinlto compile.

// SAMPLEPGO:               Running pass: PGOIndirectCallPromotion on [module]
// SAMPLEPGO:               Running pass: LoopUnrollPass on bar

// SAMPLEPGO-OLDPM:         PGOIndirectCallPromotion
// SAMPLEPGO-OLDPM:         Unroll loops
// SAMPLEPGO-OLDPM:         Unroll loops

// THINLTO-NOT:             Running pass: PGOIndirectCallPromotion on [module]
// THINLTO-NOT:             Running pass: LoopUnrollPass on bar

// THINLTO-OLDPM-NOT:       PGOIndirectCallPromotion
// The first Unroll loop pass is the createSimpleLoopUnrollPass that unrolls and peels
// loops with small constant trip counts. The second one is skipped by ThinLTO.
// THINLTO-OLDPM:           Unroll loops
// THINLTO-OLDPM-NOT:       Unroll loops


// Checks if hot call is inlined by normal compile, but not inlined by
// thinlto compile.
// SAMPLEPGO-LABEL: define {{(dso_local )?}}void @bar
// THINLTO-LABEL: define {{(dso_local )?}}void @bar
// SAMPLEPGO-NOT: call{{.*}}foo
// THINLTO: call{{.*}}foo
void bar(int n) {
  for (int i = 0; i < n; i++)
    foo(i);
}

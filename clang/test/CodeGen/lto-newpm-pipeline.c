// REQUIRES: x86-registered-target

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=full -O0 %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-FULL-O0
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=thin -O0 %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-THIN-O0
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=full -O1 %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-FULL-OPTIMIZED
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=thin -O1 %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-THIN-OPTIMIZED
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=full -O2 %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-FULL-OPTIMIZED
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=thin -O2 %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-THIN-OPTIMIZED
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=full -O3 %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-FULL-OPTIMIZED
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=thin -O3 %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-THIN-OPTIMIZED
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=full -Os %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-FULL-OPTIMIZED
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=thin -Os %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-THIN-OPTIMIZED
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=full -Oz %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-FULL-OPTIMIZED
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -mllvm -verify-analysis-invalidation=0 -fdebug-pass-manager -flto=thin -Oz %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-THIN-OPTIMIZED

// INTEL_CUSTOMIZATION
// LoopVectorizePass is disabled by default.
// Execute CHECKs with -mllvm -enable-lv
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -fdebug-pass-manager -flto=full -O1 -mllvm -enable-lv %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-FULL-OPTIMIZED-LV
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -fdebug-pass-manager -flto=full -O2 -mllvm -enable-lv %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-FULL-OPTIMIZED-LV
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -fdebug-pass-manager -flto=full -O3 -mllvm -enable-lv %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-FULL-OPTIMIZED-LV
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -fdebug-pass-manager -flto=full -Os -mllvm -enable-lv %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-FULL-OPTIMIZED-LV
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc -o /dev/null -fdebug-pass-manager -flto=full -Oz -mllvm -enable-lv %s 2>&1 | FileCheck %s \
// RUN:   -check-prefix=CHECK-FULL-OPTIMIZED-LV

// CHECK-FULL-O0: Running analysis: InnerAnalysisManagerProxy
// CHECK-FULL-O0-NEXT: Running pass: LowerSubscriptIntrinsicPass
// CHECK-FULL-O0-NEXT: Running pass: InlineReportSetupPass
// CHECK-FULL-O0-NEXT: Running pass: InlineForceInlinePass
// CHECK-FULL-O0-NEXT: Running pass: InlineListsPass
// CHECK-FULL-O0-NEXT: Running pass: AlwaysInlinerPass
// end INTEL_CUSTOMIZATION

// CHECK-FULL-O0-NEXT: Running analysis: ProfileSummaryAnalysis
// CHECK-FULL-O0-NEXT: Running pass: CoroConditionalWrapper
// CHECK-FULL-O0-NEXT: Running pass: CanonicalizeAliasesPass
// CHECK-FULL-O0-NEXT: Running pass: NameAnonGlobalPass
// CHECK-FULL-O0-NEXT: Running pass: AnnotationRemarksPass
// CHECK-FULL-O0-NEXT: Running analysis: TargetLibraryAnalysis
// INTEL_CUSTOMIZATION
// CHECK-FULL-O0-NEXT: Running pass: Intel_DebugPass
// CHECK-FULL-O0-NEXT: Running pass: InlineReportMakeCurrentPass
// CHECK-FULL-O0-NEXT: Running pass: InlineReportEmitterPass
// end INTEL_CUSTOMIZATION
// CHECK-FULL-O0-NEXT: Running pass: VerifierPass
// CHECK-FULL-O0-NEXT: Running pass: BitcodeWriterPass

// INTEL_CUSTOMIZATION
// CHECK-THIN-O0: Running analysis: InnerAnalysisManagerProxy
// CHECK-THIN-O0-NEXT: Running pass: LowerSubscriptIntrinsicPass
// CHECK-THIN-O0-NEXT: Running pass: InlineReportSetupPass
// CHECK-THIN-O0-NEXT: Running pass: InlineForceInlinePass
// CHECK-THIN-O0-NEXT: Running pass: InlineListsPass
// CHECK-THIN-O0-NEXT: Running pass: AlwaysInlinerPass
// end INTEL_CUSTOMIZATION

// TODO: The LTO pre-link pipeline currently invokes
//       buildPerModuleDefaultPipeline(), which contains LoopVectorizePass.
//       This may change as the pipeline gets implemented.
// INTEL - LoopVectorizePass is disabled by default, so CHECKs are removed.
// CHECK-FULL-OPTIMIZED: Running pass: BitcodeWriterPass

// INTEL_CUSTOMIZATION
// CHECK-FULL-OPTIMIZED-LV: Running pass: LoopVectorizePass
// CHECK-FULL-OPTIMIZED-LV: Running pass: BitcodeWriterPass
// end INTEL_CUSTOMIZATION

// The ThinLTO pre-link pipeline shouldn't contain passes like
// LoopVectorizePass.
// CHECK-THIN-OPTIMIZED-NOT: Running pass: LoopVectorizePass
// CHECK-THIN-OPTIMIZED: Running pass: CanonicalizeAliasesPass
// CHECK-THIN-OPTIMIZED: Running pass: NameAnonGlobalPass
// CHECK-THIN-OPTIMIZED: Running pass: ThinLTOBitcodeWriterPass

void Foo(void) {}

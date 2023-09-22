; RUN: SATest -BUILD -llvm-option=-debug-pass-manager -config=%S/pipeline-sycl-O0.tst.cfg 2>&1 | FileCheck %s --check-prefix=CHECK-O0
; RUN: SATest -BUILD -llvm-option=-debug-pass-manager -config=%S/pipeline-sycl-O1.tst.cfg 2>&1 | FileCheck %s --check-prefix=CHECK-O1
; RUN: SATest -BUILD -llvm-option=-debug-pass-manager -config=%S/pipeline-sycl-O2.tst.cfg 2>&1 | FileCheck %s --check-prefix=CHECK-O2
; RUN: SATest -BUILD -llvm-option=-debug-pass-manager -config=%S/pipeline-sycl-O3.tst.cfg 2>&1 | FileCheck %s --check-prefix=CHECK-O3

; RUN: SATest -BUILD -llvm-option=-debug-pass=Structure -config=%S/pipeline-sycl-O0.tst.cfg 2>&1 | FileCheck %s --check-prefix=CHECK-CG-O0
; RUN: SATest -BUILD -llvm-option=-debug-pass=Structure -config=%S/pipeline-sycl-O1.tst.cfg 2>&1 | FileCheck %s --check-prefix=CHECK-CG-O1
; RUN: SATest -BUILD -llvm-option=-debug-pass=Structure -config=%S/pipeline-sycl-O2.tst.cfg 2>&1 | FileCheck %s --check-prefix=CHECK-CG-O2
; RUN: SATest -BUILD -llvm-option=-debug-pass=Structure -config=%S/pipeline-sycl-O3.tst.cfg 2>&1 | FileCheck %s --check-prefix=CHECK-CG-O3

; CHECK-O0-NOT: CallSiteSplittingPass
; CHECK-O1-NOT: CallSiteSplittingPass
; CHECK-O2-NOT: CallSiteSplittingPass
; CHECK-O3: CallSiteSplittingPass

; CHECK-O0-NOT: SpeculativeExecutionPass
; CHECK-O1-NOT: SpeculativeExecutionPass
; CHECK-O2: SpeculativeExecutionPass
; CHECK-O3: SpeculativeExecutionPass

; CHECK-CG-O0: Fast Register Allocator
; CHECK-CG-O1: Greedy Register Allocator
; CHECK-CG-O2: Greedy Register Allocator
; CHECK-CG-O3: Greedy Register Allocator

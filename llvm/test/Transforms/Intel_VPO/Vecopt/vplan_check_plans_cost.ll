; RUN: opt -S -VPlanDriver -vplan-print-after-evaluator -disable-output %s 2>&1 | FileCheck %s

; CHECK: There is no peel loop.
; CHECK-NEXT: The main loop is vectorized with vector factor 4. The vector cost is 128000(32 x 4000).
; CHECK-NEXT: The remainder loop is scalar with trip count 3. The scalar cost is 12000(3 x 4000).

; RUN: opt -S -VPlanDriver -vplan-print-after-evaluator -vplan-disable-vector-peel-and-vector-remainder=false -disable-output %s 2>&1 | FileCheck %s --check-prefix=VECTOR-CHECK

; VECTOR-CHECK: There is no peel loop.
; VECTOR-CHECK-NEXT: The main loop is vectorized with vector factor 4. The vector cost is 128000(32 x 4000).
; VECTOR-CHECK-NEXT: The remainder loop has trip count 2 and it is vectorized with vector factor 2. The vector cost is 8000.
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @main() {
entry:
  br label %preheader

preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header
header:
  %iv = phi i32 [ 0, %preheader ], [ %iv.next, %latch ]
  %iv.next = add nsw i32 %iv, 1
  br label %latch

latch:
  %x = add nsw i32 %iv, 1
  %bottom_test = icmp eq i32 %iv.next, 131
  br i1 %bottom_test, label %loopexit, label %header

loopexit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind


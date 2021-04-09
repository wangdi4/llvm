; RUN: opt -S -mattr=+avx512f -VPlanDriver -vplan-print-after-evaluator -disable-output %s 2>&1 | FileCheck %s --check-prefix=NO-PEEL-SCALAR-REMAINDER
; RUN: opt -S -mattr=+avx512f -VPlanDriver -vplan-print-after-evaluator -vplan-enable-non-masked-vectorized-remainder -disable-output %s 2>&1 | FileCheck %s --check-prefix=NON-MASKED-VECTOR-REMAINDER
; RUN: opt -S -mattr=+avx512f -VPlanDriver -vplan-print-after-evaluator -vplan-enable-masked-variant -vplan-enable-masked-vectorized-remainder -disable-output %s 2>&1 | FileCheck %s --check-prefix=MASKED-VECTOR-REMAINDER

; NO-PEEL-SCALAR-REMAINDER: Evaluators for VF=4
; NO-PEEL-SCALAR-REMAINDER-NEXT: There is no peel loop.
; NO-PEEL-SCALAR-REMAINDER-NEXT: The main loop is vectorized with vector factor 4. The vector cost is 194716000(97358 x 2000).
; NO-PEEL-SCALAR-REMAINDER-NEXT: The remainder loop is scalar with trip count 3. The scalar cost is 6000(3 x 2000).

; NO-PEEL-SCALAR-REMAINDER: Evaluators for VF=8
; NO-PEEL-SCALAR-REMAINDER-NEXT: There is no peel loop.
; NO-PEEL-SCALAR-REMAINDER-NEXT: The main loop is vectorized with vector factor 8. The vector cost is 97358000(48679 x 2000).
; NO-PEEL-SCALAR-REMAINDER-NEXT: The remainder loop is scalar with trip count 3. The scalar cost is 6000(3 x 2000).

; NO-PEEL-SCALAR-REMAINDER: Evaluators for VF=16
; NO-PEEL-SCALAR-REMAINDER-NEXT: There is no peel loop.
; NO-PEEL-SCALAR-REMAINDER-NEXT: The main loop is vectorized with vector factor 16. The vector cost is 48678000(24339 x 2000).
; NO-PEEL-SCALAR-REMAINDER-NEXT: The remainder loop is scalar with trip count 11. The scalar cost is 22000(11 x 2000).

; NON-MASKED-VECTOR-REMAINDER: Evaluators for VF=4
; NON-MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; NON-MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 4. The vector cost is 194716000(97358 x 2000).
; NON-MASKED-VECTOR-REMAINDER-NEXT:The remainder loop is scalar with trip count 3. The scalar cost is 6000(3 x 2000).

; NON-MASKED-VECTOR-REMAINDER: Evaluators for VF=8
; NON-MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; NON-MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 8. The vector cost is 97358000(48679 x 2000).
; NON-MASKED-VECTOR-REMAINDER-NEXT: The remainder loop is scalar with trip count 3. The scalar cost is 6000(3 x 2000).

; NON-MASKED-VECTOR-REMAINDER: Evaluators for VF=16
; NON-MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; NON-MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 16. The vector cost is 48678000(24339 x 2000).
; NON-MASKED-VECTOR-REMAINDER-NEXT: The remainder loop has trip count 3 and it is vectorized with vector factor 4. The vector cost is 12000.
; NON-MASKED-VECTOR-REMAINDER-NEXT: The remainder loop has a new remainder loop with trip count 3.

; MASKED-VECTOR-REMAINDER: Evaluators for VF=4
; MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 4. The vector cost is 194716000(97358 x 2000).
; MASKED-VECTOR-REMAINDER-NEXT: The remainder loop has trip count 3 and it is vectorized with a mask. The vector cost is 3000.

; MASKED-VECTOR-REMAINDER: Evaluators for VF=8
; MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 8. The vector cost is 97358000(48679 x 2000).
; MASKED-VECTOR-REMAINDER-NEXT: The remainder loop has trip count 3 and it is vectorized with a mask. The vector cost is 3000.

; MASKED-VECTOR-REMAINDER: Evaluators for VF=16
; MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 16. The vector cost is 48678000(24339 x 2000).
; MASKED-VECTOR-REMAINDER-NEXT: The remainder loop has trip count 11 and it is vectorized with a mask. The vector cost is 3000.

; target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "x86_64-unknown-linux-gnu"

define void @test1() {
entry:
  br label %preheader

preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header
header:
  %iv = phi i32 [ 0, %preheader ], [ %iv.next, %latch ]
  br label %latch

latch:
  %iv.next = add nsw i32 %iv, 1
  ; %t = sext i32 %iv to i64
  ; %x = add nsw i64 %t, 1
  %bottom_test = icmp eq i32 %iv.next, 389435
  br i1 %bottom_test, label %loopexit, label %header

loopexit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret void
}

; RUN: opt -S -mattr=+avx512f -VPlanDriver -vplan-print-after-evaluator -vplan-enable-peeling -vplan-enable-general-peeling-cost-model -disable-output %s 2>&1 | FileCheck %s --check-prefix=SCALAR-PEEL-REMAINDER

; RUN: opt -S -mattr=+avx512f -VPlanDriver -vplan-print-after-evaluator -vplan-enable-peeling -vplan-enable-general-peeling-cost-model -vplan-enable-masked-variant -vplan-enable-vectorized-peel -disable-output %s 2>&1 | FileCheck %s --check-prefix=VECTOR-PEEL

; RUN: opt -S -mattr=+avx512f -VPlanDriver -vplan-print-after-evaluator -vplan-enable-peeling -vplan-enable-general-peeling-cost-model -vplan-enable-masked-variant -vplan-enable-vectorized-peel -vplan-enable-masked-vectorized-remainder -disable-output %s 2>&1 | FileCheck %s --check-prefix=MASKED-PEEL-REMAINDER

;SCALAR-PEEL-REMAINDER: Evaluators for VF=2
;SCALAR-PEEL-REMAINDER-NEXT: The peel loop is scalar with trip count 1. The scalar cost is 3000(1 x 3000).
;SCALAR-PEEL-REMAINDER-NEXT: The main loop is vectorized with vector factor 2. The vector cost is 608490625(194717 x 3125).
;SCALAR-PEEL-REMAINDER-NEXT: There is no remainder loop.

;SCALAR-PEEL-REMAINDER: Evaluators for VF=4
;SCALAR-PEEL-REMAINDER-NEXT: The peel loop is scalar with trip count 3. The scalar cost is 9000(3 x 3000).
;SCALAR-PEEL-REMAINDER-NEXT: The main loop is vectorized with vector factor 4. The vector cost is 328583250(97358 x 3375).
;SCALAR-PEEL-REMAINDER-NEXT: There is no remainder loop.

;SCALAR-PEEL-REMAINDER: Evaluators for VF=8
;SCALAR-PEEL-REMAINDER-NEXT: The peel loop is scalar with trip count 7. The scalar cost is 21000(7 x 3000).
;SCALAR-PEEL-REMAINDER-NEXT: The main loop is vectorized with vector factor 8. The vector cost is 188627250(48678 x 3875).
;SCALAR-PEEL-REMAINDER-NEXT: The remainder loop is scalar with trip count 4. The scalar cost is 12000(4 x 3000).

; VECTOR-PEEL: Evaluators for VF=2
; VECTOR-PEEL-NEXT: The peel loop is scalar with trip count 1. The scalar cost is 3000(1 x 3000).
; VECTOR-PEEL-NEXT: The main loop is vectorized with vector factor 2. The vector cost is 608490625(194717 x 3125).
; VECTOR-PEEL-NEXT: There is no remainder loop.

; VECTOR-PEEL: Evaluators for VF=4
; VECTOR-PEEL-NEXT: The peel loop has trip count 3 and it is vectorized with a mask. The vector cost is 4000.
; VECTOR-PEEL-NEXT: The main loop is vectorized with vector factor 4. The vector cost is 328583250(97358 x 3375).
; VECTOR-PEEL-NEXT: There is no remainder loop.

; VECTOR-PEEL: Evaluators for VF=8
; VECTOR-PEEL-NEXT: The peel loop has trip count 7 and it is vectorized with a mask. The vector cost is 4000.
; VECTOR-PEEL-NEXT: The main loop is vectorized with vector factor 8. The vector cost is 188627250(48678 x 3875).
; VECTOR-PEEL-NEXT:The remainder loop is scalar with trip count 4. The scalar cost is 12000(4 x 3000).

; MASKED-PEEL-REMAINDER: Evaluators for VF=2
; MASKED-PEEL-REMAINDER-NEXT: The peel loop is scalar with trip count 1. The scalar cost is 3000(1 x 3000).
; MASKED-PEEL-REMAINDER-NEXT: The main loop is vectorized with vector factor 2. The vector cost is 608490625(194717 x 3125).
; MASKED-PEEL-REMAINDER-NEXT: There is no remainder loop.

; MASKED-PEEL-REMAINDER: Evaluators for VF=4
; MASKED-PEEL-REMAINDER-NEXT: The peel loop has trip count 3 and it is vectorized with a mask. The vector cost is 4000.
; MASKED-PEEL-REMAINDER-NEXT: The main loop is vectorized with vector factor 4. The vector cost is 328583250(97358 x 3375).
; MASKED-PEEL-REMAINDER-NEXT: There is no remainder loop.

; MASKED-PEEL-REMAINDER: Evaluators for VF=8
; MASKED-PEEL-REMAINDER-NEXT: The peel loop has trip count 7 and it is vectorized with a mask. The vector cost is 4000.
; MASKED-PEEL-REMAINDER-NEXT: The main loop is vectorized with vector factor 8. The vector cost is 188627250(48678 x 3875).
; MASKED-PEEL-REMAINDER-NEXT: The remainder loop has trip count 4 and it is vectorized with a mask. The vector cost is 4000.

define void @test2(i64* %array) {
entry:
  br label %preheader

preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header
header:
  %iv = phi i64 [ 0, %preheader ], [ %iv.next, %latch ]
  %ptr = getelementptr inbounds i64, i64* %array, i64 %iv
  store i64 %iv, i64* %ptr, align 8
  br label %latch

latch:
  %iv.next = add nsw i64 %iv, 1
  %bottom_test = icmp eq i64 %iv.next, 389435
  br i1 %bottom_test, label %loopexit, label %header

loopexit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind


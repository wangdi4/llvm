; RUN: opt -S -mattr=+avx512f -vplan-vec -vplan-print-after-evaluator -disable-output %s 2>&1 | FileCheck %s --check-prefix=NO-PEEL-SCALAR-REMAINDER
; RUN: opt -S -mattr=+avx512f -vplan-vec -vplan-print-after-evaluator -vplan-enable-non-masked-vectorized-remainder -disable-output %s 2>&1 | FileCheck %s --check-prefix=NON-MASKED-VECTOR-REMAINDER
; RUN: opt -S -mattr=+avx512f -vplan-vec -vplan-print-after-evaluator  -vplan-enable-masked-vectorized-remainder -disable-output %s 2>&1 | FileCheck %s --check-prefix=MASKED-VECTOR-REMAINDER

; NO-PEEL-SCALAR-REMAINDER: Evaluators for VF=4
; NO-PEEL-SCALAR-REMAINDER-NEXT: There is no peel loop.
; NO-PEEL-SCALAR-REMAINDER-NEXT: The main loop is vectorized with vector factor 4. The vector cost is 194716(97358 x 2).
; NO-PEEL-SCALAR-REMAINDER-NEXT: The remainder loop is scalar with known trip count 3. The scalar cost is 6(3 x 2).

; NO-PEEL-SCALAR-REMAINDER: Evaluators for VF=8
; NO-PEEL-SCALAR-REMAINDER-NEXT: There is no peel loop.
; NO-PEEL-SCALAR-REMAINDER-NEXT: The main loop is vectorized with vector factor 8. The vector cost is 97358(48679 x 2).
; NO-PEEL-SCALAR-REMAINDER-NEXT: The remainder loop is scalar with known trip count 3. The scalar cost is 6(3 x 2).

; NO-PEEL-SCALAR-REMAINDER: Evaluators for VF=16
; NO-PEEL-SCALAR-REMAINDER-NEXT: There is no peel loop.
; NO-PEEL-SCALAR-REMAINDER-NEXT: The main loop is vectorized with vector factor 16. The vector cost is 48678(24339 x 2).
; NO-PEEL-SCALAR-REMAINDER-NEXT: The remainder loop is scalar with known trip count 11. The scalar cost is 22(11 x 2).

; NON-MASKED-VECTOR-REMAINDER: Evaluators for VF=4
; NON-MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; NON-MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 4. The vector cost is 194716(97358 x 2).
; NON-MASKED-VECTOR-REMAINDER-NEXT:The remainder loop is scalar with known trip count 3. The scalar cost is 6(3 x 2).

; NON-MASKED-VECTOR-REMAINDER: Evaluators for VF=8
; NON-MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; NON-MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 8. The vector cost is 97358(48679 x 2).
; NON-MASKED-VECTOR-REMAINDER-NEXT: The remainder loop is scalar with known trip count 3. The scalar cost is 6(3 x 2).

; NON-MASKED-VECTOR-REMAINDER: Evaluators for VF=16
; NON-MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; NON-MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 16. The vector cost is 48678(24339 x 2).
; NON-MASKED-VECTOR-REMAINDER-NEXT: The remainder loop has known trip count 3 and it is vectorized with vector factor 4. The vector cost is 12.
; NON-MASKED-VECTOR-REMAINDER-NEXT: The remainder loop has a new remainder loop with known trip count 3.

; MASKED-VECTOR-REMAINDER: Evaluators for VF=4
; MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 4. The vector cost is 194716(97358 x 2).
; MASKED-VECTOR-REMAINDER-NEXT: The remainder loop has known trip count 3 and it is vectorized with a mask. The vector cost is 5.

; MASKED-VECTOR-REMAINDER: Evaluators for VF=8
; MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 8. The vector cost is 97358(48679 x 2).
; MASKED-VECTOR-REMAINDER-NEXT: The remainder loop has known trip count 3 and it is vectorized with a mask. The vector cost is 5.

; MASKED-VECTOR-REMAINDER: Evaluators for VF=16
; MASKED-VECTOR-REMAINDER-NEXT: There is no peel loop.
; MASKED-VECTOR-REMAINDER-NEXT: The main loop is vectorized with vector factor 16. The vector cost is 48678(24339 x 2).
; MASKED-VECTOR-REMAINDER-NEXT: The remainder loop has known trip count 11 and it is vectorized with a mask. The vector cost is 5.

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

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind


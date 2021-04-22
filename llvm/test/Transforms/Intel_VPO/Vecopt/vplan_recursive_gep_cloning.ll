; RUN: opt -VPlanDriver -S %s | FileCheck %s
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @main(<2 x i32>* %a) local_unnamed_addr {
entry:
  br label %DIR.OMP.SIMD.1

; CHECK: vector.ph:
; CHECK-NEXT: [[BSI:%.*]] = insertelement <4 x <2 x i32>*> poison, <2 x i32>* %a, i32 0
; CHECK-NEXT: [[BCAST:%.*]] = shufflevector <4 x <2 x i32>*> [[BSI]], <4 x <2 x i32>*> poison, <4 x i32> zeroinitializer

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %trunc = trunc i64 %indvars.iv to i32

  %cast = bitcast <2 x i32> * %a to i32 *
  ; Verify that %gep isn't just cloned because that will make the clone to use
  ; %cast which isn't defined in the vector loop.
  %gep = getelementptr i32, i32* %cast, i32 1
;; DA recognizes all operands of the GEP to be uniform and hence we generate scalar GEP followed by broadcast,
;; unlike IR legality.
; CHECK: [[BC1:%.*]] = bitcast <4 x <2 x i32>*> [[BCAST]] to <4 x i32*>
; CHECK-NEXT: [[E1:%.*]] = extractelement <4 x i32*> [[BC1]], i32 0
; CHECK-NEXT: [[GEP1:%.*]] = getelementptr i32, i32* [[E1]], i32 1
; CHECK-NEXT: [[S1:%.*]] = insertelement <4 x i32*> poison, i32* %mm_vectorGEP, i32 0
; CHECK-NEXT: shufflevector <4 x i32*> [[S1]], <4 x i32*> poison, <4 x i32> zeroinitializer
  %gep.vec = getelementptr i32, i32* %gep, i32 %trunc

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %omp.loop.exit
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

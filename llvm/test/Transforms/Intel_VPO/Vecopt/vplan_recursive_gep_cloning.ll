; RUN: opt -passes=vplan-vec -S %s | FileCheck %s
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @main(ptr %a) local_unnamed_addr {
entry:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %trunc = trunc i64 %indvars.iv to i32

; Verify that %gep isn't just cloned because that will make the clone to use
; %a which isn't defined in the vector loop.
  %gep = getelementptr i32, ptr %a, i32 1
; DA recognizes all operands of the GEP to be uniform and hence we generate scalar GEP.
; CHECK: [[GEP1:%.*]] = getelementptr i32, ptr [[A:%.*]], i32 1
  %gep.vec = getelementptr i32, ptr %gep, i32 %trunc

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

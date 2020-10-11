; RUN: opt -S -VPlanDriver %s | FileCheck %s
; RUN: opt -S -passes="vplan-driver" %s | FileCheck %s
;
; Test that we bail out on liveout private.
;
; CHECK-NOT: <4 x i64>
define void @simd_loop(i32* %A, i32* %B) #0 {
entry:
  %private = alloca i32, align 4
  br label %DIR.OMP.SIMD.3

DIR.OMP.SIMD.3:                                   ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LASTPRIVATE"(i32* %private) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.3, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.3 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %priv.outgoing = load i32, i32* %arrayidx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  %..lcssa = phi i32 [ %priv.outgoing, %omp.inner.for.body ]
  store i32 %..lcssa, i32* %private, align 8
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.END.SIMD.1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)


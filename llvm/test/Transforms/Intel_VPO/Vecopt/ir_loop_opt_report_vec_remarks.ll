; Test to check the functionality of vectorization opt-report for LLVM-IR based vectorizer.

; RUN: opt -VPlanDriver -vplan-force-vf=4 -intel-loop-optreport=low -intel-ir-optreport-emitter %s 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; Deprecated the llvm.intel.directive* representation.
; TODO: Update this test to use llvm.directive.region.entry/exit instead.
; XFAIL: *

; Check output from opt-report emitter
; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:    Remark: LOOP WAS VECTORIZED
; OPTREPORT-NEXT:    Remark: vectorization support: vector length {{.*}}
; OPTREPORT-NEXT: LOOP END

; RUN: opt -VPlanDriver -vplan-force-vf=4 -intel-loop-optreport=low %s 2>&1 < %s -S | FileCheck %s

; Check metadata for remarks from vectorized IR
; CHECK: [[M1:!.*]] = distinct !{!"llvm.loop.optreport", [[M2:!.*]]}
; CHECK-NEXT: [[M2]] = distinct !{!"intel.loop.optreport", [[M3:!.*]]}
; CHECK-NEXT: [[M3]] = !{!"intel.optreport.remarks", [[M4:!.*]], [[M5:!.*]]}
; CHECK-NEXT: [[M4]] = !{!"intel.optreport.remark", !"LOOP WAS VECTORIZED"}
; CHECK-NEXT: [[M5]] = !{!"intel.optreport.remark", !"vectorization support: vector length %s", {{.*}}}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %arr) local_unnamed_addr {
entry:
  %i2 = alloca i32, align 4
  %0 = bitcast i32* %i2 to i8*
  call void @llvm.lifetime.start(i64 4, i8* nonnull %0) #3
  store i32 0, i32* %i2, align 4, !tbaa !1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LINEAR", i32* nonnull %i2, i32 3)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %.omp.iv.06 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add1, %omp.inner.for.body ]
  call void (...) @baz() #3
  %1 = load i32, i32* %i2, align 4, !tbaa !1
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %idxprom
  store i32 %1, i32* %arrayidx, align 4, !tbaa !1
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* %i2, align 4, !tbaa !1
  %add1 = add nuw nsw i64 %.omp.iv.06, 1
  %exitcond = icmp eq i64 %add1, 300
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %omp.loop.exit
  call void @llvm.lifetime.end(i64 4, i8* nonnull %0) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare void @baz(...) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }


!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

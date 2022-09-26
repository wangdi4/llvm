; Test to check the functionality of vectorization opt-report for LLVM-IR based vectorizer.

; RUN: opt -vplan-vec -vplan-force-vf=4 -intel-opt-report=low -intel-ir-optreport-emitter %s 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; RUN: opt -vplan-vec -vplan-force-vf=4 -intel-opt-report=low %s 2>&1 < %s -S | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %arr) local_unnamed_addr {
; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
; OPTREPORT-NEXT:      remark #15305: vectorization support: vector length {{.*}}
; OPTREPORT-NEXT:  LOOP END

; CHECK-LABEL: define void @foo(
; CHECK: !llvm.loop [[FOO_LOOP_MD:!.*]]
entry:
  %i2 = alloca i32, align 4
  %0 = bitcast i32* %i2 to i8*
  call void @llvm.lifetime.start(i64 4, i8* nonnull %0) #3
  store i32 0, i32* %i2, align 4, !tbaa !1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR"(i32* %i2, i32 3) ]
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
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %omp.loop.exit
  call void @llvm.lifetime.end(i64 4, i8* nonnull %0) #3
  ret void
}

define void @test_outer([1024 x [1024 x i64]]* %a) local_unnamed_addr {
; OPTREPORT-LABEL:  Global optimization report for : test_outer
; OPTREPORT-EMPTY:
; OPTREPORT-NEXT:  LOOP BEGIN
; OPTREPORT-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
; OPTREPORT-NEXT:      remark #15305: vectorization support: vector length 4
; OPTREPORT-EMPTY:
; OPTREPORT-NEXT:      LOOP BEGIN
; OPTREPORT-NEXT:      LOOP END
; OPTREPORT-NEXT:  LOOP END
; CHECK-LABEL: define void @test_outer(
; CHECK: !llvm.loop [[TEST_OUTER_LOOP_MD:!.*]]
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %outer.loop

outer.loop:
  %outer.induction.phi = phi i64 [ 0, %entry ], [ %outer.induction, %outer.loop.latch ]
  br label %inner.loop

inner.loop:
  %inner.induction.phi = phi i64 [ 0, %outer.loop ], [ %inner.induction, %inner.loop ]
  %inner.induction = add nuw nsw i64 %inner.induction.phi, 1
  %exitcond = icmp eq i64 %inner.induction, 1024
  br i1 %exitcond, label %inner.exit, label %inner.loop

inner.exit:
  br label %outer.loop.latch

outer.loop.latch:
  %outer.induction = add nuw nsw i64 %outer.induction.phi, 1
  %exitcond26 = icmp eq i64 %outer.induction, 1024
  br i1 %exitcond26, label %exit, label %outer.loop

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Check metadata for remarks from vectorized IR
; CHECK: [[FOO_LOOP_MD:!.*]] = distinct !{[[FOO_LOOP_MD]], [[FOO_OPTRPT:![^,]+]]{{.*}}}
; CHECK: [[FOO_OPTRPT]] = distinct !{!"intel.optreport.rootnode", [[FOO_OPTRPT_INTEL:!.*]]}
; CHECK-NEXT: [[FOO_OPTRPT_INTEL]] = distinct !{!"intel.optreport", [[REMARKS:!.*]]}
; CHECK-NEXT: [[REMARKS]] = !{!"intel.optreport.remarks", [[R1:!.*]], [[R2:!.*]]}
; CHECK-NEXT: [[R1]] = !{!"intel.optreport.remark", i32 15301, !"SIMD LOOP WAS VECTORIZED"}
; CHECK-NEXT: [[R2]] = !{!"intel.optreport.remark", i32 15305, !"vectorization support: vector length %s", {{.*}}}
; CHECK: [[TEST_OUTER_LOOP_MD]] = distinct !{[[TEST_OUTER_LOOP_MD]], [[TEST_OUTER_OPTRPT:![^,]+]]{{.*}}}
; CHECK-NEXT: [[TEST_OUTER_OPTRPT]] = distinct !{!"intel.optreport.rootnode", [[TEST_OUTER_OPTRPT_INTEL:!.*]]}
; CHECK-NEXT: [[TEST_OUTER_OPTRPT_INTEL]] = distinct !{!"intel.optreport", [[REMARKS]]}


; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare void @baz(...) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: argmemonly nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }


!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

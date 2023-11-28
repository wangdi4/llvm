; Test to check the loop number is printed correctly in opt report.

; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-report-loop-number -vplan-force-vf=2 -intel-opt-report=high < %s -disable-output 2>&1 | FileCheck %s
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -vplan-report-loop-number -vplan-force-vf=2 -intel-opt-report=high < %s -disable-output 2>&1 | FileCheck %s

; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-report-loop-number -vplan-force-vf=2 -intel-opt-report=high -vplan-vec-range=1 < %s -disable-output 2>&1 | FileCheck %s --check-prefixes=ONLY1st
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -vplan-report-loop-number -vplan-force-vf=2 -intel-opt-report=high -vplan-vec-range=1 < %s -disable-output 2>&1 | FileCheck %s --check-prefixes=ONLY1st

; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-report-loop-number -vplan-force-vf=2 -intel-opt-report=high -vplan-vec-range="~1:2" < %s -disable-output 2>&1 | FileCheck %s --check-prefixes=NONE
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -vplan-report-loop-number -vplan-force-vf=2 -intel-opt-report=high -vplan-vec-range="~1:2" < %s -disable-output 2>&1 | FileCheck %s --check-prefixes=NONE


;For the vplan-vec-range=1. This is valid until bailout is not reported.
; ONLY1st: remark #15506: vplan loop number: 1
; ONLY1st-NOT: remark #15506: vplan loop number: 2

;For the vplan-vec-range="~1:2". This is valid until bailout is not reported.
; NONE-NOT: remark #15506: vplan loop number

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare void @serial_call_1() nounwind
declare double @llvm.sqrt.f64(double %val) #1

define void @test(i64 %n, ptr %arr, ptr %arr1) {
; CHECK-LABEL:  for : test
; CHECK:      LOOP BEGIN
; CHECK-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15506: vplan loop number: 1
; CHECK-NEXT:     remark #15305: vectorization support: vector length 2
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.latch, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  call void @serial_call_1()
  %inner.cond = icmp eq i64 %indvars.iv, 42
  br i1 %inner.cond, label %for.body2.preheader, label %for.latch

for.body2.preheader:                              ; preds = %for.body
  br label %for.body2

for.body2:                                        ; preds = %for.body2.preheader, %for.body2
  %indvars.iv2 = phi i64 [ %indvars.iv.next2, %for.body2 ], [ 0, %for.body2.preheader ]
  %d = sitofp i64 %indvars.iv2 to double
  %sqrt = call double @llvm.sqrt.f64(double %d)
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv2, 1
  %exitcond2 = icmp eq i64 %indvars.iv.next2, %n
  br i1 %exitcond2, label %for.latch.loopexit, label %for.body2

for.latch.loopexit:                               ; preds = %for.body2
  br label %for.latch

for.latch:                                        ; preds = %for.latch.loopexit, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.latch
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:                                             ; preds = %for.end, %entry
  ret void
}

define void @test2(i64 %n, ptr %arr, ptr %arr1) {
; CHECK-LABEL: for : test2
; CHECK:      LOOP BEGIN
; CHECK-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15506: vplan loop number: 2
; CHECK-NEXT:     remark #15305: vectorization support: vector length 2
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.latch, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  call void @serial_call_1()
  %inner.cond = icmp eq i64 %indvars.iv, 42
  br i1 %inner.cond, label %for.body2.preheader, label %for.latch

for.body2.preheader:                              ; preds = %for.body
  br label %for.body2

for.body2:                                        ; preds = %for.body2.preheader, %for.body2
  %indvars.iv2 = phi i64 [ %indvars.iv.next2, %for.body2 ], [ 0, %for.body2.preheader ]
  %d = sitofp i64 %indvars.iv2 to double
  %sqrt = call double @llvm.sqrt.f64(double %d)
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv2, 1
  %exitcond2 = icmp eq i64 %indvars.iv.next2, %n
  br i1 %exitcond2, label %for.latch.loopexit, label %for.body2

for.latch.loopexit:                               ; preds = %for.body2
  br label %for.latch

for.latch:                                        ; preds = %for.latch.loopexit, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.latch
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:                                             ; preds = %for.end, %entry
  ret void
}


attributes #1 = { nounwind readnone "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx,+avx2" }

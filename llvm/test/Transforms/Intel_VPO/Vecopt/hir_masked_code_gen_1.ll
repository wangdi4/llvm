; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPODriverHIR -hir-cg -print-after=VPODriverHIR -S  < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -S  < %s 2>&1 | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; check hir
; CHECK:     DO i1 = 0, 99, 4   <DO_LOOP>
; CHECK:         (<4 x i32>*)(%ip)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>; Mask = @{{{.*}}}
; CHECK:     END LOOP
; check llvm IR
; CHECK:     call void @llvm.masked.store.v4i32.p0v4i32(<4 x i32> {{.*}}, <4 x i32>* {{.*}}, {{.*}}, <4 x i1>
; ModuleID = 'tif.c'
source_filename = "tif.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %ip, i32 %N1, i32 %N2) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %0 = trunc i64 %indvars.iv to i32
  %add = add i32 %0, %N1
  %tobool = icmp eq i32 %add, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %ip, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20616)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

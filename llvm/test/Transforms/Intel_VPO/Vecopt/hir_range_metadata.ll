; LLVM IR generated from following source using
; icpx -O1 -S -emit-llvm r2.c
; bool arr1[1024];
; bool arr2[1024]; 
; 
; void foo()
; {
;   int index;
; 
;   for (index = 0; index < 1024; index++) {
;     arr2[index] = !arr1[index];
;   }
; }
;   
; RUN: opt -default-vpo-vf=4 -hir-ssa-deconstruction -hir-vec-dir-insert -VPODriverHIR -hir-cg -simplifycfg -print-after=simplifycfg -S < %s 2>&1 | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR
;   
; CHECK:   store <4 x i8>
; CHECK-NOT: !range
; ModuleID = 'r2.c'
source_filename = "r2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = local_unnamed_addr global [1024 x i8] zeroinitializer, align 16
@arr2 = local_unnamed_addr global [1024 x i8] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @_Z3foov() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i8], [1024 x i8]* @arr1, i64 0, i64 %indvars.iv
  %0 = load i8, i8* %arrayidx, align 1, !tbaa !1, !range !6
  %arrayidx2 = getelementptr inbounds [1024 x i8], [1024 x i8]* @arr2, i64 0, i64 %indvars.iv
  %1 = xor i8 %0, 1
  store i8 %1, i8* %arrayidx2, align 1, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17978) (llvm/branches/loopopt 20316)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA1024_b", !3, i64 0}
!3 = !{!"bool", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{i8 0, i8 2}

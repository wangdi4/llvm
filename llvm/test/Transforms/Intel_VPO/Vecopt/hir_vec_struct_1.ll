; ModuleID = 'tstr.c'
; struct S1 {
;   int arr1[100];
;   int arr2[100];
; } s1;
; 
; void foo()
; {
;   int index;
; 
;   for (index = 0; index < 100; index++)
;     s1.arr1[index] = index;
; }
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPODriverHIR -default-vpo-vf=4 -print-after=VPODriverHIR -S < %s 2>&1 | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; HIR Test.
; CHECK: DO i1 = 0, 99, 4   <DO_LOOP>
; CHECK: (<4 x i32>*)(@s1)[0].0[i1] = i1 + 
; CHECK: END LOOP
source_filename = "tstr.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S1 = type { [100 x i32], [100 x i32] }

@s1 = common local_unnamed_addr global %struct.S1 zeroinitializer, align 4

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds %struct.S1, %struct.S1* @s1, i64 0, i32 0, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20662) (llvm/branches/loopopt 20686)"}
!1 = !{!2, !4, i64 0}
!2 = !{!"struct@S1", !3, i64 0, !3, i64 400}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

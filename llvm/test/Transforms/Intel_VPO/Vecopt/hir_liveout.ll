; LLVM IR generated from following source using icx -O1 -S -emit-llvm
; float arr1[1024], arr2[1024];
; 
; float foo()
; {
;   float val = 0.0;
;   int index;
; 
;   for (index = 0; index < 1024; index++) {
;     val = arr1[index] + 1.0;
;     arr2[index] = val;
;   }
; 
;   return val;
; }
; 
;RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -default-vpo-vf=4 -VPODriverHIR -hir-cg -mem2reg -S %s -print-after=VPODriverHIR 2>&1 | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; CHECK:           BEGIN REGION { modified }
; CHECK:            + DO i1 = 0, 1023, 4   <DO_LOOP>
; CHECK:            |   %.vec = (<4 x float>*)(@arr1)[0][i1];
; CHECK:            |   %conv1.vec = %.vec  +  1.000000e+00;
; CHECK:            |   (<4 x float>*)(@arr2)[0][i1] = %conv1.vec;
; CHECK:            + END LOOP
; CHECK:            %conv1 = extractelement %conv1.vec,  3;
; CHECK:           END REGION

; CHECK: loop
; CHECK: load <4 x float>
; CHECK: add <4 x float>
; CHECK: store <4 x float>
; CHECK: afterloop
; CHECK: extractelement <4 x float> {{.*}}{{i64|i32}} 3
; ModuleID = 'f1.c'
source_filename = "f1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@arr2 = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define float @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @arr1, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %conv1 = fadd float %0, 1.000000e+00
  %arrayidx3 = getelementptr inbounds [1024 x float], [1024 x float]* @arr2, i64 0, i64 %indvars.iv
  store float %conv1, float* %arrayidx3, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %conv1.lcssa = phi float [ %conv1, %for.body ]
  ret float %conv1.lcssa
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17978) (llvm/branches/loopopt 20351)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA1024_f", !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

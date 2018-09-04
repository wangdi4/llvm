; Generated from following source using icx -O1 -S -emit-llvm 
; float arr[1024];
; 
; float foo()
; {
;   float sum = 0.0;
;   int index;
; 
;   for (index = 0; index < 1024; index++)
;     sum += arr[index];
; 
;   return sum;
; }
; 
; ModuleID = 'f1.c'
;RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPODriverHIR -hir-cg -mem2reg -S %s -print-after=VPODriverHIR 2>&1 | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; CHECK:           BEGIN REGION { modified }
; CHECK:           %RedOp = zeroinitializer;

; CHECK:           + DO i1 = 0, 1023, 4   <DO_LOOP>
; CHECK:           |   %RedOp = %RedOp  +  (<4 x float>*)(@arr)[0][i1];
; CHECK:           + END LOOP

; CHECK:           %Lo = shufflevector %RedOp,  %RedOp,  <i32 0, i32 1>;
; CHECK:           %Hi = shufflevector %RedOp,  %RedOp,  <i32 2, i32 3>;
; CHECK:           %reduce = %Lo  +  %Hi;
; CHECK:           %Lo1 = extractelement %reduce,  0;
; CHECK:           %Hi2 = extractelement %reduce,  1;
; CHECK:           %reduced = %Lo1  +  %Hi2;
; CHECK:           %sum.06 = %reduced  +  %sum.06;
; CHECK:           END REGION

; CHECK: loop
; CHECK: phi <4 x float> [ zeroinitializer
; CHECK: fadd{{.*}} <4 x float>
; CHECK: afterloop
; CHECK: shufflevector <4 x float>
; CHECK: shufflevector <4 x float>
; CHECK: fadd{{.*}} <2 x float>
; CHECK: extractelement <2 x float>
; CHECK: extractelement <2 x float>
; CHECK: fadd{{.*}} float
; CHECK: fadd{{.*}} float
source_filename = "f1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define float @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.06 = phi float [ 0.000000e+00, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @arr, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %add = fadd fast float %sum.06, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi float [ %add, %for.body ]
  ret float %add.lcssa
}

attributes #0 = { norecurse nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17978) (llvm/branches/loopopt 20351)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA1024_f", !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

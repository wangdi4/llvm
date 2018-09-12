; LLVM IR generated from testcase below using icx -O1 -S -emit-llvm
; int *ip[1024];
; int arr[1024];
; 
; void foo()
; {
;   int index;
; 
;   for (index = 0; index < 1024; index++) {
;     ip[index] = &arr[index];
;   }
; }
; 
; RUN: opt -default-vpo-vf=4 -hir-ssa-deconstruction -hir-vec-dir-insert -VPODriverHIR -hir-cg -print-after=VPODriverHIR -S < %s 2>&1 | FileCheck %s
; RUN: opt -default-vpo-vf=4 -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -S -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; CHECK:      DO i1 = 0, 1023, 4   <DO_LOOP>
; CHECK-NEXT:     (<4 x i32*>*)(@ip)[0][i1] = &((<4 x i32*>)(@arr)[0][i1 + <i64 0, i64 1, i64 2, i64 3>]);
; CHECK-NEXT: END LOOP
; CHECK:  %arrayIdx = getelementptr inbounds [1024 x i32*], [1024 x i32*]* @ip, i64 0, i64 %0
; CHECK-NEXT:  %1 = bitcast i32** %arrayIdx to <4 x i32*>*
; CHECK-NEXT:  %2 = load i64, i64* %i1.i64
; CHECK-NEXT:  %.splatinsert = insertelement <4 x i64> undef, i64 %2, i32 0
; CHECK-NEXT:  %.splat = shufflevector <4 x i64> %.splatinsert, <4 x i64> undef, <4 x i32> zeroinitializer
; CHECK-NEXT:  %3 = add <4 x i64> <i64 0, i64 1, i64 2, i64 3>, %.splat
; CHECK-NEXT:  %arrayIdx1 = getelementptr inbounds [1024 x i32], <4 x [1024 x i32]*> <[1024 x i32]* @arr, [1024 x i32]* @arr, [1024 x i32]* @arr, [1024 x i32]* @arr>, <4 x i64> zeroinitializer, <4 x i64> %3
; CHECK-NEXT:  store <4 x i32*> %arrayIdx1, <4 x i32*>* %1, align 8, !tbaa !2
; ModuleID = 'a.c'
source_filename = "a.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common global [1024 x i32] zeroinitializer, align 16
@ip = common local_unnamed_addr global [1024 x i32*] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %indvars.iv
  %arrayidx2 = getelementptr inbounds [1024 x i32*], [1024 x i32*]* @ip, i64 0, i64 %indvars.iv
  store i32* %arrayidx, i32** %arrayidx2, align 8, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20470) (llvm/branches/loopopt 20481)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA1024_Pi", !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

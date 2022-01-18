; Test HIR decomposition of memory operands for addressof operation with a bitcast.
; Input LLVM-IR generated for below C code with command: icx -O2 -print-module-before-loopopt

; float *ip[1024];
; int arr[1024];
;
; void foo()
; {
;     int index;
;
;     for (index = 0; index < 1024; index++) {
;         ip[index] = (float*)&arr[index];
;     }
; }

; Input HIR
; <12>      + DO i1 = 0, 1023, 1   <DO_LOOP>
; <5>       |   (i32**)(@ip)[0][i1] = &((@arr)[0][i1]);
; <12>      + END LOOP


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-dump-subscript-details -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-print-after-plain-cfg -vplan-dump-subscript-details -disable-output < %s 2>&1 | FileCheck %s

; Check decomposed VPInstructions
; CHECK: i64 [[I1:%vp.*]] = phi
; CHECK-NEXT: i32* [[ADDR1:%vp.*]] = subscript inbounds [1024 x i32]* @arr {i64 0 : i64 0 : i64 4096 : [1024 x i32]*([1024 x i32])} {i64 0 : i64 [[I1]] : i64 4 : [1024 x i32](i32)}
; CHECK-NEXT: float** [[ADDR2:%vp.*]] = subscript inbounds [1024 x float*]* @ip {i64 0 : i64 0 : i64 8192 : [1024 x float*]*([1024 x float*])} {i64 0 : i64 [[I1]] : i64 8 : [1024 x float*](float*)}
; CHECK-NEXT: i32** [[BITCAST:%vp.*]] = bitcast float** [[ADDR2]]
; CHECK-NEXT: store i32* [[ADDR1]] i32** [[BITCAST]]


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common dso_local global [1024 x i32] zeroinitializer, align 16
@ip = common dso_local local_unnamed_addr global [1024 x float*] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %arrayidx2 = getelementptr inbounds [1024 x float*], [1024 x float*]* @ip, i64 0, i64 %indvars.iv, !intel-tbaa !7
  %0 = bitcast float** %arrayidx2 to i32**
  store i32* %arrayidx, i32** %0, align 8, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}


!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !9, i64 0}
!8 = !{!"array@_ZTSA1024_Pf", !9, i64 0}
!9 = !{!"pointer@_ZTSPf", !5, i64 0}

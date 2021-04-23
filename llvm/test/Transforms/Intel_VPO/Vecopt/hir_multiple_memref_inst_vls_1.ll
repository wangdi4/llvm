; Check that correct vector code is generated for input HIR with multiple memrefs in the same HLInst.
; Input LLVM-IR generated for below C code with command: icx -O2 -mllvm -print-module-before-loopopt

; struct S1 {
;     int a;
;     int b;
; } arr[1024];
;
; int ip[1024];
;
; void foo()
; {
;     int i1 = 0;
;     for (i1 = 0; i1 < 1024; i1++) {
;         arr[i1].a = ip[i1];
;         arr[i1].b = i1;
;     }
; }

; Input HIR
; <15>    + DO i1 = 0, 1023, 1   <DO_LOOP>
; <5>     |   (@arr)[0][i1].0 = (@ip)[0][i1];
; <8>     |   (@arr)[0][i1].1 = i1;
; <15>    + END LOOP


; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -enable-vplan-vls-cg -enable-vp-value-codegen-hir -disable-output -print-after=VPlanDriverHIR -tbaa < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,vplan-driver-hir" -vplan-force-vf=4 -enable-vplan-vls-cg -enable-vp-value-codegen-hir -disable-output -print-after=vplan-driver-hir < %s 2>&1 | FileCheck %s


; CHECK:       DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:    %.vec = (<4 x i32>*)(@ip)[0][i1];
; CHECK-NEXT:    %comb.shuf = shufflevector %.vec,  i1 + <i64 0, i64 1, i64 2, i64 3>,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>;
; CHECK-NEXT:    %vls.interleave = shufflevector %comb.shuf,  undef,  <i32 0, i32 4, i32 1, i32 5, i32 2, i32 6, i32 3, i32 7>;
; CHECK-NEXT:    (<8 x i32>*)(@arr)[0][i1].0 = %vls.interleave;
; CHECK-NEXT:  END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S1 = type { i32, i32 }

@ip = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr = common dso_local local_unnamed_addr global [1024 x %struct.S1] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @ip, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %a = getelementptr inbounds [1024 x %struct.S1], [1024 x %struct.S1]* @arr, i64 0, i64 %indvars.iv, i32 0
  store i32 %0, i32* %a, align 8, !tbaa !7
  %b = getelementptr inbounds [1024 x %struct.S1], [1024 x %struct.S1]* @arr, i64 0, i64 %indvars.iv, i32 1
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %b, align 4, !tbaa !9
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
!7 = !{!8, !4, i64 0}
!8 = !{!"struct@S1", !4, i64 0, !4, i64 4}
!9 = !{!8, !4, i64 4}

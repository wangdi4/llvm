; Test to check HIR CG when VLS group instructions are used in a reduction sequence.
; Specifically we test mixed CG support when VPLoopEntities representation is used for the
; reduction variable.

; Incoming HIR
;   BEGIN REGION { }
;      %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;      + DO i1 = 0, 101, 1   <DO_LOOP>
;      |   %sum.022.out = %sum.022; <Safe Reduction>
;      |   %2 = (@arr1)[0][3 * i1 + 1];
;      |   %4 = (@arr1)[0][3 * i1 + 2];
;      |   %5 = (@arr1)[0][3 * i1];
;      |   %sum.022 = %2 + %4 + %sum.022.out  +  %5; <Safe Reduction>
;      + END LOOP
;
;      @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;   END REGION

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -enable-vplan-vls-cg -hir-cg -disable-output -print-after=hir-vplan-vec  < %s 2>&1  | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vplan-force-vf=4 -enable-vplan-vls-cg -disable-output < %s 2>&1 | FileCheck %s


; CHECK:             [[RED_INIT0:%red.*]] = 0;
; CHECK-NEXT:        [[RED_INIT_INSERT0:%.*]] = insertelement [[RED_INIT0]],  [[SUM_0220:%.*]],  0
; CHECK-NEXT:        [[PHI_TEMP30:%.*]] = [[RED_INIT_INSERT0]]
; CHECK:             + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   [[DOTVLS_LOAD0:%.*]] = undef
; CHECK-NEXT:        |   [[DOTCOPY50:%.*]] = [[PHI_TEMP30]]
; CHECK-NEXT:        |   [[DOTVLS_LOAD0]] = (<16 x i32>*)(@arr1)[0][3 * i1], Mask = @{<i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 false, i1 false, i1 false, i1 false>}
; CHECK-NEXT:        |   [[VLS_EXTRACT0:%.*]] = shufflevector [[DOTVLS_LOAD0]],  [[DOTVLS_LOAD0]],  <i32 0, i32 3, i32 6, i32 9>
; CHECK-NEXT:        |   [[VLS_EXTRACT60:%.*]] = shufflevector [[DOTVLS_LOAD0]],  [[DOTVLS_LOAD0]],  <i32 1, i32 4, i32 7, i32 10>
; CHECK-NEXT:        |   [[VLS_EXTRACT70:%.*]] = shufflevector [[DOTVLS_LOAD0]],  [[DOTVLS_LOAD0]],  <i32 2, i32 5, i32 8, i32 11>
; CHECK-NEXT:        |   [[DOTVEC80:%.*]] = [[VLS_EXTRACT60]] + [[VLS_EXTRACT70]]  +  [[DOTCOPY50]]
; CHECK-NEXT:        |   [[DOTVEC90:%.*]] = [[DOTVEC80]]  +  [[VLS_EXTRACT0]]
; CHECK-NEXT:        |   [[PHI_TEMP30]] = [[DOTVEC90]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[SUM_0220]] = @llvm.vector.reduce.add.v4i32([[DOTVEC90]])


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr2 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind readonly uwtable
define dso_local i32 @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.022 = phi i32 [ 0, %entry ], [ %add10, %for.body ]
  %0 = mul nuw nsw i64 %indvars.iv, 3
  %1 = add nuw nsw i64 %0, 1
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %1
  %2 = load i32, i32* %arrayidx, align 4
  %3 = add nuw nsw i64 %0, 2
  %arrayidx4 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %3
  %4 = load i32, i32* %arrayidx4, align 4
  %arrayidx8 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %0
  %5 = load i32, i32* %arrayidx8, align 4
  %add5 = add i32 %2, %sum.022
  %add9 = add i32 %add5, %4
  %add10 = add i32 %add9, %5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 102
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add10.lcssa = phi i32 [ %add10, %for.body ]
  ret i32 %add10.lcssa
}

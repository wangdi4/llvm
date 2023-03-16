;
; RUN: opt -disable-output -passes='vplan-vec,instcombine,print' -vplan-print-after-predicator < %s 2>&1 | FileCheck %s
;
; LIT test to demonstrate the issue with poison being used as the mask
; for a masked scatter due to the way we compute block predicates that
; do not take into account possible poison values.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(ptr noalias noundef %lp1, ptr noalias noundef %lp2, ptr noalias noundef %lp3) local_unnamed_addr #0 {
; CHECK-LABEL:  VPlan after predicator:
; CHECK-NEXT:  VPlan IR for: foo:for.body.#{{[0-9]+}}
; CHECK-NEXT:    [[BB0:BB[0-9]+]]: # preds:
; CHECK-NEXT:     [DA: Uni] br [[BB1:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB0]]
; CHECK-NEXT:     [DA: Div] i64 [[VP_IV_IND_INIT:%.*]] = induction-init{add} i64 live-in0 i64 1
; CHECK-NEXT:     [DA: Uni] i64 [[VP_IV_IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     [DA: Uni] i64 [[VP_VECTOR_TRIP_COUNT:%.*]] = vector-trip-count i64 8, UF = 1
; CHECK-NEXT:     [DA: Uni] br [[BB2:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB1]], [[BB3:BB[0-9]+]]
; CHECK-NEXT:     [DA: Div] i64 [[VP_IV:%.*]] = phi  [ i64 [[VP_IV_IND_INIT]], [[BB1]] ],  [ i64 [[VP_IV_NEXT:%.*]], [[BB3]] ]
; CHECK-NEXT:     [DA: Div] ptr [[VP_ARRAYIDX:%.*]] = getelementptr inbounds i64, ptr [[LP10:%.*]] i64 [[VP_IV]]
; CHECK-NEXT:     [DA: Div] store i64 [[VP_IV]] ptr [[VP_ARRAYIDX]]
; CHECK-NEXT:     [DA: Div] i64 [[VP0:%.*]] = load ptr [[VP_ARRAYIDX]]
;
; NOTE: Next compare will always return true and the not of the same will always be false.
;
; CHECK-NEXT:     [DA: Div] i1 [[VP_CMP2_NOT:%.*]] = icmp eq i64 [[VP0]] i64 [[VP_IV]]
; CHECK-NEXT:     [DA: Div] i1 [[VP_CMP2_NOT_NOT:%.*]] = not i1 [[VP_CMP2_NOT]]
; CHECK-NEXT:     [DA: Uni] br [[BB4:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB4]]: # preds: [[BB2]]
; CHECK-NEXT:     [DA: Div] i1 [[VP1:%.*]] = block-predicate i1 [[VP_CMP2_NOT_NOT]]
; CHECK-NEXT:     [DA: Div] ptr [[VP_ARRAYIDX3:%.*]] = getelementptr inbounds i64, ptr [[LP20:%.*]] i64 [[VP_IV]]
;
; NOTE: As noted above, the block predicate will always be false and if we are using
; poison as pass-thru value for masked load, VP2 will end up being poison. Subsequently,
; VP_TOBOOL_NOT and VP_TOBOOL_NOT_NOT will be poison as well.
;
; CHECK-NEXT:     [DA: Div] i64 [[VP2:%.*]] = load ptr [[VP_ARRAYIDX3]]
; CHECK-NEXT:     [DA: Div] i1 [[VP_TOBOOL_NOT:%.*]] = icmp eq i64 [[VP2]] i64 0
; CHECK-NEXT:     [DA: Div] i1 [[VP_TOBOOL_NOT_NOT:%.*]] = not i1 [[VP_TOBOOL_NOT]]
; CHECK-NEXT:     [DA: Uni] br [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB4]]
;
; NOTE: Since VP_TOBOOL_NOT_NOT is poison, VP_BB4_BR_VP_TOBOOL_NOT_NOT will end up as poison.
; This will cause the masked store in BB6 to use poison as the store mask. Instcombine
; is able to do these optimizations and we end up with a masked scatter using poison as
; the scatter mask leading to incorrect runtime behavior. Instead of using an and here
; we need to use a select instruction to avoid the poison mask.
;
; CHECK-NEXT:     [DA: Div] i1 [[VP_BB4_BR_VP_TOBOOL_NOT_NOT:%.*]] = select i1 [[VP_CMP2_NOT_NOT]] i1 [[VP_TOBOOL_NOT_NOT]] i1 false
; CHECK-NEXT:     [DA: Uni] br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     [DA: Div] i1 [[VP3:%.*]] = block-predicate i1 [[VP_BB4_BR_VP_TOBOOL_NOT_NOT]]
; CHECK-NEXT:     [DA: Div] i64 [[VP_MUL5:%.*]] = shl i64 [[VP_IV]] i64 1
; CHECK-NEXT:     [DA: Div] ptr [[VP_ARRAYIDX6:%.*]] = getelementptr inbounds i64, ptr [[LP30:%.*]] i64 [[VP_MUL5]]
; CHECK-NEXT:     [DA: Div] store i64 1 ptr [[VP_ARRAYIDX6]]
; CHECK-NEXT:     [DA: Uni] br [[BB3]]
;
; CHECK:  define void @foo
; CHECK:       VPlannedBB6:
; CHECK-NOT:     call void @llvm.masked.scatter.v8i64.v8p0({{.*}}, <8 x i1> poison)
;
entry:
  br label %for.body.lr.ph

for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.body.lr.ph ], [ %iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i64, ptr %lp1, i64 %iv
  store i64 %iv, ptr %arrayidx, align 8
  %1 = load i64, ptr %arrayidx, align 8
  %cmp2.not = icmp eq i64 %1, %iv
  br i1 %cmp2.not, label %for.inc, label %if.then

if.then:
  %arrayidx3 = getelementptr inbounds i64, ptr %lp2, i64 %iv
  %2 = load i64, ptr %arrayidx3, align 8
  %tobool.not = icmp eq i64 %2, 0
  br i1 %tobool.not, label %for.inc, label %if.then4

if.then4:
  %mul5 = shl nuw nsw i64 %iv, 1
  %arrayidx6 = getelementptr inbounds i64, ptr %lp3, i64 %mul5
  store i64 1, ptr %arrayidx6, align 8
  br label %for.inc

for.inc:
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %iv.next, 8
  br i1 %exitcond.not, label %for.exit, label %for.body

for.exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.119

DIR.OMP.END.SIMD.119:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

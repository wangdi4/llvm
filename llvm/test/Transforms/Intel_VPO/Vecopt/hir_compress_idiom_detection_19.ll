; RUN: opt %s -mattr=+avx512f,+avx512vl -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -disable-output -debug-only=parvec-analysis -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -vplan-force-vf=4 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
;       |   if ((%C)[i1] != 0)
;       |   {
;       |      if ((%D)[i1] != 0)
;       |      {
;       |         (%B)[%j.018] = (%A)[i1];
;       |         %j.018 = %j.018  +  1;
;       |      }
;       |   }
;       + END LOOP
; END REGION

; CHECK:       [Compress/Expand Idiom] Increment {sb:3}+1 detected: {{.*}} [[J_0180:%.*]] = [[J_0180]]  +  1
; CHECK-NEXT:  Idiom List
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[J_0180]] = [[J_0180]]  +  1
; CHECK-NEXT:    CEStore: {{.*}} ([[B0:%.*]])[%j.018] = ([[A0:%.*]])[i1]
; CHECK-NEXT:      CELdStIndex: [[J_0180]]

; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:         [[BB3:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB3]]
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     i32 [[VP_INIT:%.*]] = compress-expand-index-init i32 [[J_0180]]
; CHECK-NEXT:     br [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2:BB[0-9]+]]
; CHECK-NEXT:     i32 [[VP7:%.*]] = phi  [ i32 [[VP_INIT]], [[BB1]] ],  [ i32 [[VP14:%.*]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP6:%.*]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP5:%.*]], [[BB2]] ]
; CHECK-NEXT:     ptr [[VP_SUBSCRIPT_1:%.*]] = subscript inbounds ptr [[C0:%.*]] i64 [[VP6]]
; CHECK-NEXT:     i32 [[VP_LOAD_1:%.*]] = load ptr [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:     i1 [[VP11:%.*]] = icmp ne i32 [[VP_LOAD_1]] i32 0
; CHECK-NEXT:     br i1 [[VP11]], [[BB4:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[BB0]]
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_2:%.*]] = subscript inbounds ptr [[D0:%.*]] i64 [[VP6]]
; CHECK-NEXT:       i32 [[VP_LOAD_2:%.*]] = load ptr [[VP_SUBSCRIPT_2]]
; CHECK-NEXT:       i1 [[VP12:%.*]] = icmp ne i32 [[VP_LOAD_2]] i32 0
; CHECK-NEXT:       br i1 [[VP12]], [[BB5:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB5]]: # preds: [[BB4]]
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_3:%.*]] = subscript inbounds ptr [[A0]] i64 [[VP6]]
; CHECK-NEXT:       i32 [[VP_LOAD:%.*]] = load ptr [[VP_SUBSCRIPT_3]]
; CHECK-NEXT:       i64 [[VP10:%.*]] = sext i32 [[VP7]] to i64
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT:%.*]] = subscript inbounds ptr [[B0]] i64 [[VP10]]
; CHECK-NEXT:       compress-store i32 [[VP_LOAD]] ptr [[VP_SUBSCRIPT]]
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB5]], [[BB4]], [[BB0]]
; CHECK-NEXT:     i1 [[VP15:%.*]] = phi  [ i1 true, [[BB5]] ],  [ i1 false, [[BB4]] ],  [ i1 false, [[BB0]] ]
; CHECK-NEXT:     i32 [[VP14]] = compress-expand-index-inc {stride: 1} i32 [[VP7]] i1 [[VP15]]
; CHECK-NEXT:     i64 [[VP5]] = add i64 [[VP6]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP13:%.*]] = icmp slt i64 [[VP5]] i64 1024
; CHECK-NEXT:     br i1 [[VP13]], [[BB0]], [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     i32 [[VP_FINAL:%.*]] = compress-expand-index-final i32 [[VP14]]
; CHECK-NEXT:     br [[BB7:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB7]]: # preds: [[BB6]]
; CHECK-NEXT:     br <External Block>

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[J_0180]]
; CHECK:             + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC50:%.*]] = undef
; CHECK-NEXT:        |   [[DOTVEC20:%.*]] = undef
; CHECK-NEXT:        |   [[DOTVEC0:%.*]] = (<4 x i32>*)([[C0]])[i1]
; CHECK-NEXT:        |   [[DOTVEC10:%.*]] = [[DOTVEC0]] != 0
; CHECK-NEXT:        |   [[DOTVEC20]] = (<4 x i32>*)([[D0]])[i1], Mask = @{[[DOTVEC10]]}
; CHECK-NEXT:        |   [[DOTVEC30:%.*]] = [[DOTVEC20]] != 0
; CHECK-NEXT:        |   [[DOTVEC40:%.*]] = ([[DOTVEC0]] != 0) ? [[DOTVEC30]] : 0
; CHECK-NEXT:        |   [[DOTVEC50]] = (<4 x i32>*)([[A0]])[i1], Mask = @{[[DOTVEC40]]}
; CHECK-NEXT:        |   @llvm.masked.compressstore.v4i32([[DOTVEC50]],  &((i32*)([[B0]])[%phi.temp]),  [[DOTVEC40]])
; CHECK-NEXT:        |   [[SELECT0:%.*]] = ([[DOTVEC40]] == <i1 true, i1 true, i1 true, i1 true>) ? -1 : 0
; CHECK-NEXT:        |   [[CAST0:%.*]] = bitcast.<4 x i1>.i4([[SELECT0]])
; CHECK-NEXT:        |   [[POPCNT0:%.*]] = @llvm.ctpop.i4([[CAST0]])
; CHECK-NEXT:        |   [[ZEXT0:%.*]] = zext.i4.i32([[POPCNT0]])
; CHECK-NEXT:        |   [[MUL0:%.*]] = [[ZEXT0]]  *  1
; CHECK-NEXT:        |   [[ADD0:%.*]] = [[PHI_TEMP0]]  +  [[MUL0]]
; CHECK-NEXT:        |   [[PHI_TEMP0]] = [[ADD0]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[J_0180]] = [[ADD0]]
; CHECK-NEXT:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_Z3fooPiS_S_S_i(ptr noalias nocapture noundef readonly %A, ptr noalias nocapture noundef writeonly %B, ptr noalias nocapture noundef readonly %C, ptr noalias nocapture noundef readonly %D) {
for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %j.018 = phi i32 [ 0, %for.body.preheader ], [ %j.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %C, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx
  %cmp1.not = icmp eq i32 %0, 0
  br i1 %cmp1.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, ptr %D, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx3
  %cmp4.not = icmp eq i32 %1, 0
  br i1 %cmp4.not, label %for.inc, label %if.then5

if.then5:                                         ; preds = %if.then
  %arrayidx7 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx7
  %idxprom8 = sext i32 %j.018 to i64
  %arrayidx9 = getelementptr inbounds i32, ptr %B, i64 %idxprom8
  store i32 %2, ptr %arrayidx9
  %inc = add nsw i32 %j.018, 1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then5, %if.then
  %j.1 = phi i32 [ %inc, %if.then5 ], [ %j.018, %if.then ], [ %j.018, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  ret void
}

; RUN: opt %s -mattr=+avx512f,+avx512vl -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -disable-output -debug-only=parvec-analysis -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -vplan-force-vf=4 -vplan-force-uf=1 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 31, 1   <DO_LOOP>
;       |   if ((%c)[i1] != 0)
;       |   {
;       |      if ((%d)[i1] != 0)
;       |      {
;       |         %2 = (%a.addr.017)[0];
;       |         %a.addr.017 = &((%a.addr.017)[1]);
;       |         %conv = sitofp.i32.float(%2);
;       |         (%e)[i1] = %conv;
;       |      }
;       |      %3 = (%a2.018)[0];
;       |      %a2.018 = &((%a2.018)[1]);
;       |      (%b)[i1] = %3;
;       |   }
;       + END LOOP
; END REGION

; CHECK:       [Compress/Expand Idiom] Increment {sb:4}+1 detected: {{.*}} [[A_ADDR_0170:%.*]] = &(([[A_ADDR_0170]])[1])
; CHECK:       [Compress/Expand Idiom] Increment {sb:3}+1 detected: {{.*}} [[A2_0180:%.*]] = &(([[A2_0180]])[1])
; CHECK:       Idiom List
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[A_ADDR_0170]] = &(([[A_ADDR_0170]])[1])
; CHECK-NEXT:    CELoad: ([[A_ADDR_0170]])[0]
; CHECK-NEXT:      CELdStIndex: [[A_ADDR_0170]]
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[A2_0180]] = &(([[A2_0180]])[1])
; CHECK-NEXT:    CELoad: ([[A2_0180]])[0]
; CHECK-NEXT:      CELdStIndex: [[A2_0180]]

; CHECK-LABEL: VPlan after importing plain CFG:
; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 31 BinOp: i64 [[VP6:%.*]] = add i64 [[VP7:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP7]], i64 [[VP6]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: ptr [[VP8:%.*]] = phi  [ ptr [[A_ADDR_0170]], [[BB1:BB[0-9]+]] ],  [ ptr [[VP9:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: ptr [[A_ADDR_0170]]
; CHECK-NEXT:    TotalStride: 1
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      ptr [[VP_SUBSCRIPT:%.*]] = subscript inbounds ptr [[VP8]] i64 1
; CHECK-NEXT:    Loads:
; CHECK-NEXT:      i32 [[VP_LOAD:%.*]] = load ptr [[VP_SUBSCRIPT_1:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      ptr [[VP8]] = phi  [ ptr [[A_ADDR_0170]], [[BB1]] ],  [ ptr [[VP9]], [[BB2]] ]
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: ptr [[VP8]], ptr [[A_ADDR_0170]], ptr [[VP_SUBSCRIPT]], i32 [[VP_LOAD]],
; CHECK-EMPTY:
; CHECK-NEXT:    Phi: ptr [[VP10:%.*]] = phi  [ ptr [[A2_0180]], [[BB1]] ],  [ ptr [[VP11:%.*]], [[BB2]] ]
; CHECK-NEXT:    LiveIn: ptr [[A2_0180]]
; CHECK-NEXT:    TotalStride: 1
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      ptr [[VP_SUBSCRIPT_2:%.*]] = subscript inbounds ptr [[VP10]] i64 1
; CHECK-NEXT:    Loads:
; CHECK-NEXT:      i32 [[VP_LOAD_1:%.*]] = load ptr [[VP_SUBSCRIPT_3:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      ptr [[VP10]] = phi  [ ptr [[A2_0180]], [[BB1]] ],  [ ptr [[VP11]], [[BB2]] ]
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: ptr [[VP10]], ptr [[A2_0180]], ptr [[VP_SUBSCRIPT_2]], i32 [[VP_LOAD_1]],

; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:         [[BB3:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB3]]
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     ptr [[VP_INIT:%.*]] = compress-expand-index-init ptr [[A_ADDR_0170]]
; CHECK-NEXT:     ptr [[VP_INIT_1:%.*]] = compress-expand-index-init ptr [[A2_0180]]
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2]]
; CHECK-NEXT:     ptr [[VP10]] = phi  [ ptr [[VP_INIT_1]], [[BB1]] ],  [ ptr [[VP18:%.*]], [[BB2]] ]
; CHECK-NEXT:     ptr [[VP8]] = phi  [ ptr [[VP_INIT]], [[BB1]] ],  [ ptr [[VP17:%.*]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP7]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP6]], [[BB2]] ]
; CHECK-NEXT:     ptr [[VP_SUBSCRIPT_4:%.*]] = subscript inbounds ptr [[C0:%.*]] i64 [[VP7]]
; CHECK-NEXT:     i32 [[VP_LOAD_2:%.*]] = load ptr [[VP_SUBSCRIPT_4]]
; CHECK-NEXT:     i1 [[VP12:%.*]] = icmp ne i32 [[VP_LOAD_2]] i32 0
; CHECK-NEXT:     br i1 [[VP12]], [[BB4:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[BB0]]
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_5:%.*]] = subscript inbounds ptr [[D0:%.*]] i64 [[VP7]]
; CHECK-NEXT:       i8 [[VP_LOAD_3:%.*]] = load ptr [[VP_SUBSCRIPT_5]]
; CHECK-NEXT:       i1 [[VP13:%.*]] = icmp ne i8 [[VP_LOAD_3]] i8 0
; CHECK-NEXT:       br i1 [[VP13]], [[BB5:BB[0-9]+]], [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:        [[BB5]]: # preds: [[BB4]]
; CHECK-NEXT:         ptr [[VP_SUBSCRIPT_1]] = subscript inbounds ptr [[VP8]]
; CHECK-NEXT:         i32 [[VP19:%.*]] = expand-load ptr [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:         float [[VP14:%.*]] = sitofp i32 [[VP19]] to float
; CHECK-NEXT:         ptr [[VP_SUBSCRIPT_6:%.*]] = subscript inbounds ptr [[E0:%.*]] i64 [[VP7]]
; CHECK-NEXT:         store float [[VP14]] ptr [[VP_SUBSCRIPT_6]]
; CHECK-NEXT:         br [[BB6]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB6]]: # preds: [[BB5]], [[BB4]]
; CHECK-NEXT:       i1 [[VP20:%.*]] = phi  [ i1 true, [[BB5]] ],  [ i1 false, [[BB4]] ]
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_3]] = subscript inbounds ptr [[VP10]]
; CHECK-NEXT:       i32 [[VP21:%.*]] = expand-load ptr [[VP_SUBSCRIPT_3]]
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_7:%.*]] = subscript inbounds ptr [[B0:%.*]] i64 [[VP7]]
; CHECK-NEXT:       store i32 [[VP21]] ptr [[VP_SUBSCRIPT_7]]
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB6]], [[BB0]]
; CHECK-NEXT:     i1 [[VP22:%.*]] = phi  [ i1 true, [[BB6]] ],  [ i1 false, [[BB0]] ]
; CHECK-NEXT:     i1 [[VP23:%.*]] = phi  [ i1 [[VP20]], [[BB6]] ],  [ i1 false, [[BB0]] ]
; CHECK-NEXT:     ptr [[VP18]] = compress-expand-index-inc {i32, stride: 1} ptr [[VP10]] i1 [[VP22]]
; CHECK-NEXT:     ptr [[VP17]] = compress-expand-index-inc {i32, stride: 1} ptr [[VP8]] i1 [[VP23]]
; CHECK-NEXT:     i64 [[VP6]] = add i64 [[VP7]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP16:%.*]] = icmp slt i64 [[VP6]] i64 32
; CHECK-NEXT:     br i1 [[VP16]], [[BB0]], [[BB7:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB7]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     ptr [[VP_FINAL:%.*]] = compress-expand-index-final ptr [[VP17]]
; CHECK-NEXT:     ptr [[VP_FINAL_1:%.*]] = compress-expand-index-final ptr [[VP18]]
; CHECK-NEXT:     br [[BB8:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB8]]: # preds: [[BB7]]
; CHECK-NEXT:     br <External Block>

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[A2_0180]]
; CHECK-NEXT:        [[PHI_TEMP10:%.*]] = [[A_ADDR_0170]]
; CHECK:             + DO i1 = 0, 31, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC40:%.*]] = undef
; CHECK-NEXT:        |   [[DOTVEC0:%.*]] = (<4 x i32>*)([[C0]])[i1]
; CHECK-NEXT:        |   [[DOTVEC30:%.*]] = [[DOTVEC0]] != 0
; CHECK-NEXT:        |   [[DOTVEC40]] = (<4 x i8>*)([[D0]])[i1], Mask = @{[[DOTVEC30]]}
; CHECK-NEXT:        |   [[DOTVEC50:%.*]] = [[DOTVEC40]] != 0
; CHECK-NEXT:        |   [[DOTVEC60:%.*]] = ([[DOTVEC0]] != 0) ? [[DOTVEC50]] : 0
; CHECK-NEXT:        |   [[EXP_LOAD0:%.*]] = @llvm.masked.expandload.v4i32([[PHI_TEMP10]],  [[DOTVEC60]],  poison)
; CHECK-NEXT:        |   [[DOTVEC70:%.*]] = sitofp.<4 x i32>.<4 x float>([[EXP_LOAD0]])
; CHECK-NEXT:        |   (<4 x float>*)([[E0]])[i1] = [[DOTVEC70]], Mask = @{[[DOTVEC60]]}
; CHECK-NEXT:        |   [[SELECT0:%.*]] = ([[DOTVEC60]] == <i1 true, i1 true, i1 true, i1 true>) ? -1 : 0
; CHECK-NEXT:        |   [[EXP_LOAD80:%.*]] = @llvm.masked.expandload.v4i32([[PHI_TEMP0]],  [[DOTVEC30]],  poison)
; CHECK-NEXT:        |   (<4 x i32>*)([[B0]])[i1] = [[EXP_LOAD80]], Mask = @{[[DOTVEC30]]}
; CHECK-NEXT:        |   [[SELECT90:%.*]] = ([[DOTVEC30]] == <i1 true, i1 true, i1 true, i1 true>) ? -1 : 0
; CHECK-NEXT:        |   [[SELECT100:%.*]] = ([[DOTVEC30]] == <i1 true, i1 true, i1 true, i1 true>) ? [[SELECT0]] : 0
; CHECK-NEXT:        |   [[CAST0:%.*]] = bitcast.<4 x i1>.i4([[SELECT90]])
; CHECK-NEXT:        |   [[POPCNT0:%.*]] = @llvm.ctpop.i4([[CAST0]])
; CHECK-NEXT:        |   [[ZEXT0:%.*]] = zext.i4.i64([[POPCNT0]])
; CHECK-NEXT:        |   [[MUL0:%.*]] = [[ZEXT0]]  *  1
; CHECK-NEXT:        |   [[COPY0:%.*]] = &(([[PHI_TEMP0]])[%mul])
; CHECK-NEXT:        |   [[CAST110:%.*]] = bitcast.<4 x i1>.i4([[SELECT100]])
; CHECK-NEXT:        |   [[POPCNT120:%.*]] = @llvm.ctpop.i4([[CAST110]])
; CHECK-NEXT:        |   [[ZEXT130:%.*]] = zext.i4.i64([[POPCNT120]])
; CHECK-NEXT:        |   [[MUL140:%.*]] = [[ZEXT130]]  *  1
; CHECK-NEXT:        |   [[COPY150:%.*]] = &(([[PHI_TEMP10]])[%mul14])
; CHECK-NEXT:        |   [[PHI_TEMP0]] = [[COPY0]]
; CHECK-NEXT:        |   [[PHI_TEMP10]] = [[COPY150]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[A_ADDR_0170]] = [[COPY150]]
; CHECK-NEXT:        [[A2_0180]] = [[COPY0]]
; CHECK-NEXT:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr noalias nocapture noundef readonly %a, ptr noalias nocapture noundef writeonly %b, ptr noalias nocapture noundef readonly %c, ptr noalias nocapture noundef readonly %d, ptr noalias nocapture noundef writeonly %e) {
entry:
  %add.ptr = getelementptr inbounds i32, ptr %a, i64 2
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %a2.018 = phi ptr [ %add.ptr, %entry ], [ %a2.1, %for.inc ]
  %a.addr.017 = phi ptr [ %a, %entry ], [ %a.addr.2, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %c, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx2 = getelementptr inbounds i8, ptr %d, i64 %indvars.iv
  %1 = load i8, ptr %arrayidx2
  %tobool3.not = icmp eq i8 %1, 0
  br i1 %tobool3.not, label %if.end, label %if.then4

if.then4:                                         ; preds = %if.then
  %incdec.ptr = getelementptr inbounds i32, ptr %a.addr.017, i64 1
  %2 = load i32, ptr %a.addr.017
  %conv = sitofp i32 %2 to float
  %arrayidx6 = getelementptr inbounds float, ptr %e, i64 %indvars.iv
  store float %conv, ptr %arrayidx6
  br label %if.end

if.end:                                           ; preds = %if.then4, %if.then
  %a.addr.1 = phi ptr [ %incdec.ptr, %if.then4 ], [ %a.addr.017, %if.then ]
  %incdec.ptr7 = getelementptr inbounds i32, ptr %a2.018, i64 1
  %3 = load i32, ptr %a2.018
  %arrayidx9 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  store i32 %3, ptr %arrayidx9
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.end
  %a.addr.2 = phi ptr [ %a.addr.1, %if.end ], [ %a.addr.017, %for.body ]
  %a2.1 = phi ptr [ %incdec.ptr7, %if.end ], [ %a2.018, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 32
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void
}

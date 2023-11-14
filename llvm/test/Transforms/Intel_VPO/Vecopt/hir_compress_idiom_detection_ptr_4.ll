; RUN: opt %s -mattr=+avx512f,+avx512vl -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -disable-output -debug-only=parvec-analysis -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -vplan-force-vf=4 -vplan-force-uf=1 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 511, 1   <DO_LOOP>
;       |   if ((%c)[2 * i1] != 0)
;       |   {
;       |      (%b)[2 * i1] = (%a.addr.019)[1];
;       |      (%b)[2 * i1 + 1] = (%a.addr.019)[3];
;       |      %a.addr.019 = &((%a.addr.019)[5]);
;       |   }
;       + END LOOP
; END REGION

; CHECK:       [Compress/Expand Idiom] Increment {sb:3}+5 detected:  {{.*}} [[A_ADDR_0190:%.*]] = &(([[A_ADDR_0190]])[5])
; CHECK:       Idiom List
; CHECK-NEXT:  CEIndexIncFirst:  {{.*}} [[A_ADDR_0190]] = &(([[A_ADDR_0190]])[5])
; CHECK-NEXT:    CELoad: ([[A_ADDR_0190]])[1]
; CHECK-NEXT:      CELdStIndex: [[A_ADDR_0190]]
; CHECK-NEXT:    CELoad: ([[A_ADDR_0190]])[3]
; CHECK-NEXT:      CELdStIndex: [[A_ADDR_0190]]

; CHECK-LABEL: VPlan after importing plain CFG:
; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 511 BinOp: i64 [[VP3:%.*]] = add i64 [[VP4:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP4]], i64 [[VP3]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: ptr [[VP5:%.*]] = phi  [ ptr [[A_ADDR_0190]], [[BB1:BB[0-9]+]] ],  [ ptr [[VP6:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: ptr [[A_ADDR_0190]]
; CHECK-NEXT:    TotalStride: 5
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      ptr [[VP_SUBSCRIPT:%.*]] = subscript inbounds ptr [[VP5]] i64 5
; CHECK-NEXT:    Loads:
; CHECK-NEXT:      i32 [[VP_LOAD:%.*]] = load ptr [[VP_SUBSCRIPT_1:%.*]]
; CHECK-NEXT:      i32 [[VP_LOAD_1:%.*]] = load ptr [[VP_SUBSCRIPT_2:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      ptr [[VP5]] = phi  [ ptr [[A_ADDR_0190]], [[BB1]] ],  [ ptr [[VP6]], [[BB2]] ]
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: ptr [[VP5]], ptr [[A_ADDR_0190]], ptr [[VP_SUBSCRIPT]], i32 [[VP_LOAD]], i32 [[VP_LOAD_1]],

; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:         [[BB3:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB3]]
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     ptr [[VP_INIT:%.*]] = compress-expand-index-init ptr [[A_ADDR_0190]]
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2]]
; CHECK-NEXT:     ptr [[VP5]] = phi  [ ptr [[VP_INIT]], [[BB1]] ],  [ ptr [[VP12:%.*]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP4]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP3]], [[BB2]] ]
; CHECK-NEXT:     ptr [[VP13:%.*]] = compress-expand-index {i32, stride: 5} ptr [[VP5]]
; CHECK-NEXT:     i64 [[VP7:%.*]] = mul i64 2 i64 [[VP4]]
; CHECK-NEXT:     ptr [[VP_SUBSCRIPT_3:%.*]] = subscript inbounds ptr [[C0:%.*]] i64 [[VP7]]
; CHECK-NEXT:     i32 [[VP_LOAD_2:%.*]] = load ptr [[VP_SUBSCRIPT_3]]
; CHECK-NEXT:     i1 [[VP8:%.*]] = icmp ne i32 [[VP_LOAD_2]] i32 0
; CHECK-NEXT:     br i1 [[VP8]], [[BB4:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[BB0]]
; CHECK-NEXT:       i64 [[VP14:%.*]] = compress-expand-mask
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_1]] = subscript inbounds ptr [[VP13]] i64 1
; CHECK-NEXT:       i32 [[VP15:%.*]] = expand-load-nonu ptr [[VP_SUBSCRIPT_1]] i64 [[VP14]]
; CHECK-NEXT:       i64 [[VP9:%.*]] = mul i64 2 i64 [[VP4]]
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_4:%.*]] = subscript inbounds ptr [[B0:%.*]] i64 [[VP9]]
; CHECK-NEXT:       store i32 [[VP15]] ptr [[VP_SUBSCRIPT_4]]
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_2]] = subscript inbounds ptr [[VP13]] i64 3
; CHECK-NEXT:       i32 [[VP16:%.*]] = expand-load-nonu ptr [[VP_SUBSCRIPT_2]] i64 [[VP14]]
; CHECK-NEXT:       i64 [[VP10:%.*]] = add i64 [[VP9]] i64 1
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_5:%.*]] = subscript inbounds ptr [[B0]] i64 [[VP10]]
; CHECK-NEXT:       store i32 [[VP16]] ptr [[VP_SUBSCRIPT_5]]
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB4]], [[BB0]]
; CHECK-NEXT:     i1 [[VP17:%.*]] = phi  [ i1 true, [[BB4]] ],  [ i1 false, [[BB0]] ]
; CHECK-NEXT:     ptr [[VP12]] = compress-expand-index-inc {i32, stride: 5} ptr [[VP5]] i1 [[VP17]]
; CHECK-NEXT:     i64 [[VP3]] = add i64 [[VP4]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP11:%.*]] = icmp slt i64 [[VP3]] i64 512
; CHECK-NEXT:     br i1 [[VP11]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     ptr [[VP_FINAL:%.*]] = compress-expand-index-final ptr [[VP12]]
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[A_ADDR_0190]]
; CHECK:             + DO i1 = 0, 511, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC0:%.*]] = (<4 x i32>*)([[C0]])[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3>]
; CHECK-NEXT:        |   [[DOTVEC10:%.*]] = [[DOTVEC0]] != 0
; CHECK-NEXT:        |   [[CAST0:%.*]] = bitcast.<4 x i1>.i4([[DOTVEC10]])
; CHECK-NEXT:        |   [[POPCNT0:%.*]] = @llvm.ctpop.i4([[CAST0]])
; CHECK-NEXT:        |   [[SHL0:%.*]] = -1  <<  [[POPCNT0]]
; CHECK-NEXT:        |   [[XOR0:%.*]] = [[SHL0]]  ^  -1
; CHECK-NEXT:        |   [[CAST20:%.*]] = bitcast.i4.<4 x i1>([[XOR0]])
; CHECK-NEXT:        |   [[NSBGEPCOPY0:%.*]] = &((<4 x ptr>)([[PHI_TEMP0]])[<i64 0, i64 5, i64 10, i64 15>])
; CHECK-NEXT:        |   [[GATHER0:%.*]] = @llvm.masked.gather.v4i32.v4p0(&((<4 x ptr>)([[NSBGEPCOPY0]])[1]),  0,  [[CAST20]],  poison)
; CHECK-NEXT:        |   [[EXPAND0:%.*]] = @llvm.x86.avx512.mask.expand.v4i32([[GATHER0]],  poison,  [[DOTVEC10]])
; CHECK-NEXT:        |   [[NSBGEPCOPY30:%.*]] = &((<4 x ptr>)([[PHI_TEMP0]])[<i64 0, i64 5, i64 10, i64 15>])
; CHECK-NEXT:        |   [[GATHER40:%.*]] = @llvm.masked.gather.v4i32.v4p0(&((<4 x ptr>)([[NSBGEPCOPY30]])[3]),  0,  [[CAST20]],  poison)
; CHECK-NEXT:        |   [[EXPAND50:%.*]] = @llvm.x86.avx512.mask.expand.v4i32([[GATHER40]],  poison,  [[DOTVEC10]])
; CHECK-NEXT:        |   [[DOTEXTENDED0:%.*]] = shufflevector [[EXPAND0]],  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:        |   [[SHUFFLE0:%.*]] = shufflevector undef,  [[DOTEXTENDED0]],  <i32 8, i32 1, i32 9, i32 3, i32 10, i32 5, i32 11, i32 7>
; CHECK-NEXT:        |   [[DOTEXTENDED60:%.*]] = shufflevector [[EXPAND50]],  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:        |   [[SHUFFLE70:%.*]] = shufflevector [[SHUFFLE0]],  [[DOTEXTENDED60]],  <i32 0, i32 8, i32 2, i32 9, i32 4, i32 10, i32 6, i32 11>
; CHECK-NEXT:        |   [[VLS_MASK0:%.*]] = shufflevector [[DOTVEC10]],  zeroinitializer,  <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:        |   (<8 x i32>*)([[B0]])[2 * i1] = [[SHUFFLE70]], Mask = @{%vls.mask}
; CHECK-NEXT:        |   [[SELECT0:%.*]] = ([[DOTVEC10]] == <i1 true, i1 true, i1 true, i1 true>) ? -1 : 0
; CHECK-NEXT:        |   [[CAST80:%.*]] = bitcast.<4 x i1>.i4([[SELECT0]])
; CHECK-NEXT:        |   [[POPCNT90:%.*]] = @llvm.ctpop.i4([[CAST80]])
; CHECK-NEXT:        |   [[ZEXT0:%.*]] = zext.i4.i64([[POPCNT90]])
; CHECK-NEXT:        |   [[MUL0:%.*]] = [[ZEXT0]]  *  5
; CHECK-NEXT:        |   [[COPY0:%.*]] = &(([[PHI_TEMP0]])[%mul])
; CHECK-NEXT:        |   [[PHI_TEMP0]] = [[COPY0]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[A_ADDR_0190]] = [[COPY0]]
; CHECK-NEXT:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr noalias nocapture noundef readonly %a, ptr noalias nocapture noundef writeonly %b, ptr noalias nocapture noundef readonly %c) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %a.addr.019 = phi ptr [ %a, %entry ], [ %a.addr.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %c, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %incdec.ptr = getelementptr inbounds i32, ptr %a.addr.019, i64 1
  %1 = load i32, ptr %incdec.ptr
  %arrayidx3 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  store i32 %1, ptr %arrayidx3
  %incdec.ptr4 = getelementptr inbounds i32, ptr %a.addr.019, i64 3
  %2 = load i32, ptr %incdec.ptr4
  %3 = or i64 %indvars.iv, 1
  %arrayidx7 = getelementptr inbounds i32, ptr %b, i64 %3
  store i32 %2, ptr %arrayidx7
  %incdec.ptr8 = getelementptr inbounds i32, ptr %a.addr.019, i64 5
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %a.addr.1 = phi ptr [ %incdec.ptr8, %if.then ], [ %a.addr.019, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv, 1022
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.inc
  ret void
}

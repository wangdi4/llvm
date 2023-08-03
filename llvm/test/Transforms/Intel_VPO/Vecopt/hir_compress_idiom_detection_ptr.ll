; RUN: opt %s -mattr=+avx512f,+avx512vl -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -disable-output -debug-only=parvec-analysis -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -vplan-force-vf=4 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 1023, 1   <DO_LOOP> <vectorize>
;       |   %0 = (%keys)[i1];
;       |   %1 = (%lookup)[%0];
;       |   if (%1 != 0)
;       |   {
;       |      (%rowp.addr.014)[0] = i1;
;       |      %rowp.addr.014 = &((%rowp.addr.014)[1]);
;       |      (%outp.015)[0] = %1;
;       |      %outp.015 = &((%outp.015)[1]);
;       |   }
;       + END LOOP
; END REGION

; CHECK:       [Compress/Expand Idiom] Increment {sb:4}+1 detected: {{.*}} [[ROWP_ADDR_0140:%.*]] = &(([[ROWP_ADDR_0140]])[1])
; CHECK:       [Compress/Expand Idiom] Increment {sb:3}+1 detected: {{.*}} [[OUTP_0150:%.*]] = &(([[OUTP_0150]])[1])
; CHECK-NEXT:  Idiom List
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[ROWP_ADDR_0140]] = &(([[ROWP_ADDR_0140]])[1])
; CHECK-NEXT:    CEStore: {{.*}} ([[ROWP_ADDR_0140]])[0] = i1
; CHECK-NEXT:      CELdStIndex: [[ROWP_ADDR_0140]]
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[OUTP_0150]] = &(([[OUTP_0150]])[1])
; CHECK-NEXT:    CEStore: {{.*}} ([[OUTP_0150]])[0] = [[TMP1:%.*]]
; CHECK-NEXT:      CELdStIndex: [[OUTP_0150]]

; CHECK-LABEL: VPlan after importing plain CFG:
; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 1023 BinOp: i64 [[VP4:%.*]] = add i64 [[VP5:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP5]], i64 [[VP4]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: ptr [[VP6:%.*]] = phi  [ ptr [[ROWP_ADDR_0140]], [[BB1:BB[0-9]+]] ],  [ ptr [[VP7:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: ptr [[ROWP_ADDR_0140]]
; CHECK-NEXT:    TotalStride: 1
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      ptr [[VP_SUBSCRIPT:%.*]] = subscript inbounds ptr [[VP6]] i64 1
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store i32 [[VP8:%.*]] ptr [[VP_SUBSCRIPT_1:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      ptr [[VP6]] = phi  [ ptr [[ROWP_ADDR_0140]], [[BB1]] ],  [ ptr [[VP7]], [[BB2]] ]
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: ptr [[VP6]], ptr [[ROWP_ADDR_0140]], ptr [[VP_SUBSCRIPT]], void [[VP_STORE:%.*]],

; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:         [[BB4:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB4]]
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     ptr [[VP_INIT:%.*]] = compress-expand-index-init ptr [[ROWP_ADDR_0140]]
; CHECK-NEXT:     ptr [[VP_INIT_1:%.*]] = compress-expand-index-init ptr [[OUTP_0150]]
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2]]
; CHECK-NEXT:     ptr [[VP9:%.*]] = phi  [ ptr [[VP_INIT_1]], [[BB1]] ],  [ ptr [[VP16:%.*]], [[BB2]] ]
; CHECK-NEXT:     ptr [[VP6]] = phi  [ ptr [[VP_INIT]], [[BB1]] ],  [ ptr [[VP15:%.*]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP5]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP4]], [[BB2]] ]
; CHECK-NEXT:     ptr [[VP_SUBSCRIPT_4:%.*]] = subscript inbounds ptr [[KEYS0:%.*]] i64 [[VP5]]
; CHECK-NEXT:     i32 [[VP_LOAD_1:%.*]] = load ptr [[VP_SUBSCRIPT_4]]
; CHECK-NEXT:     i64 [[VP11:%.*]] = zext i32 [[VP_LOAD_1]] to i64
; CHECK-NEXT:     ptr [[VP_SUBSCRIPT_5:%.*]] = subscript inbounds ptr [[LOOKUP0:%.*]] i64 [[VP11]]
; CHECK-NEXT:     i64 [[VP_LOAD:%.*]] = load ptr [[VP_SUBSCRIPT_5]]
; CHECK-NEXT:     i1 [[VP12:%.*]] = icmp ne i64 [[VP_LOAD]] i64 0
; CHECK-NEXT:     br i1 [[VP12]], [[BB3:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB3]]: # preds: [[BB0]]
; CHECK-NEXT:       i32 [[VP8]] = trunc i64 [[VP5]] to i32
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_1]] = subscript inbounds ptr [[VP6]]
; CHECK-NEXT:       compress-store i32 [[VP8]] ptr [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_3:%.*]] = subscript inbounds ptr [[VP9]]
; CHECK-NEXT:       compress-store i64 [[VP_LOAD]] ptr [[VP_SUBSCRIPT_3]]
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB3]], [[BB0]]
; CHECK-NEXT:     i1 [[VP17:%.*]] = phi  [ i1 true, [[BB3]] ],  [ i1 false, [[BB0]] ]
; CHECK-NEXT:     i1 [[VP18:%.*]] = phi  [ i1 true, [[BB3]] ],  [ i1 false, [[BB0]] ]
; CHECK-NEXT:     ptr [[VP16]] = compress-expand-index-inc {i64, stride: 1} ptr [[VP9]] i1 [[VP17]]
; CHECK-NEXT:     ptr [[VP15]] = compress-expand-index-inc {i32, stride: 1} ptr [[VP6]] i1 [[VP18]]
; CHECK-NEXT:     i64 [[VP4]] = add i64 [[VP5]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP13:%.*]] = icmp slt i64 [[VP4]] i64 1024
; CHECK-NEXT:     br i1 [[VP13]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     ptr [[VP_FINAL:%.*]] = compress-expand-index-final ptr [[VP15]]
; CHECK-NEXT:     ptr [[VP_FINAL_1:%.*]] = compress-expand-index-final ptr [[VP16]]
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[OUTP_0150]]
; CHECK-NEXT:        [[PHI_TEMP20:%.*]] = [[ROWP_ADDR_0140]]
; CHECK:             + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC0:%.*]] = (<4 x i32>*)([[KEYS0]])[i1]
; CHECK-NEXT:        |   [[DOTVEC40:%.*]] = (<4 x i64>*)([[LOOKUP0]])[[[DOTVEC0:%.*]]]
; CHECK-NEXT:        |   [[DOTVEC50:%.*]] = [[DOTVEC40]] != 0
; CHECK-NEXT:        |   @llvm.masked.compressstore.v4i32(i1 + <i64 0, i64 1, i64 2, i64 3>,  [[PHI_TEMP20]],  [[DOTVEC50]])
; CHECK-NEXT:        |   @llvm.masked.compressstore.v4i64([[DOTVEC40]],  [[PHI_TEMP0]],  [[DOTVEC50]])
; CHECK-NEXT:        |   [[SELECT0:%.*]] = ([[DOTVEC50]] == <i1 true, i1 true, i1 true, i1 true>) ? -1 : 0
; CHECK-NEXT:        |   [[SELECT60:%.*]] = ([[DOTVEC50]] == <i1 true, i1 true, i1 true, i1 true>) ? -1 : 0
; CHECK-NEXT:        |   [[CAST0:%.*]] = bitcast.<4 x i1>.i4([[SELECT0]])
; CHECK-NEXT:        |   [[POPCNT0:%.*]] = @llvm.ctpop.i4([[CAST0]])
; CHECK-NEXT:        |   [[ZEXT0:%.*]] = zext.i4.i64([[POPCNT0]])
; CHECK-NEXT:        |   [[MUL0:%.*]] = [[ZEXT0]]  *  1
; CHECK-NEXT:        |   [[COPY0:%.*]] = &(([[PHI_TEMP0]])[%mul])
; CHECK-NEXT:        |   [[CAST70:%.*]] = bitcast.<4 x i1>.i4([[SELECT60]])
; CHECK-NEXT:        |   [[POPCNT80:%.*]] = @llvm.ctpop.i4([[CAST70]])
; CHECK-NEXT:        |   [[ZEXT90:%.*]] = zext.i4.i64([[POPCNT80]])
; CHECK-NEXT:        |   [[MUL100:%.*]] = [[ZEXT90]]  *  1
; CHECK-NEXT:        |   [[COPY110:%.*]] = &(([[PHI_TEMP20]])[%mul10])
; CHECK-NEXT:        |   [[PHI_TEMP0]] = [[COPY0]]
; CHECK-NEXT:        |   [[PHI_TEMP20]] = [[COPY110]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[ROWP_ADDR_0140]] = [[COPY110]]
; CHECK-NEXT:        [[OUTP_0150]] = [[COPY0]]
; CHECK-NEXT:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i64 @foo(ptr noalias noundef %output_0, ptr noalias nocapture noundef writeonly %rowp, ptr nocapture noundef readonly %keys, ptr nocapture noundef readonly %lookup) {
entry:
  %cmp13.not = icmp eq i64 1024, 0
  br i1 %cmp13.not, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %n.016 = phi i64 [ %inc, %for.inc ], [ 0, %for.body.preheader ]
  %outp.015 = phi ptr [ %outp.1, %for.inc ], [ %output_0, %for.body.preheader ]
  %rowp.addr.014 = phi ptr [ %rowp.addr.1, %for.inc ], [ %rowp, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %keys, i64 %n.016
  %0 = load i32, ptr %arrayidx
  %idxprom = zext i32 %0 to i64
  %arrayidx1 = getelementptr inbounds i64, ptr %lookup, i64 %idxprom
  %1 = load i64, ptr %arrayidx1
  %cmp2.not = icmp eq i64 %1, 0
  br i1 %cmp2.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %conv = trunc i64 %n.016 to i32
  %incdec.ptr = getelementptr inbounds i32, ptr %rowp.addr.014, i64 1
  store i32 %conv, ptr %rowp.addr.014, !tbaa !3
  %incdec.ptr5 = getelementptr inbounds i64, ptr %outp.015, i64 1
  store i64 %1, ptr %outp.015, !tbaa !7
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %rowp.addr.1 = phi ptr [ %incdec.ptr, %if.then ], [ %rowp.addr.014, %for.body ]
  %outp.1 = phi ptr [ %incdec.ptr5, %if.then ], [ %outp.015, %for.body ]
  %inc = add nuw i64 %n.016, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %outp.1.lcssa = phi ptr [ %outp.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %outp.0.lcssa = phi ptr [ %output_0, %entry ], [ %outp.1.lcssa, %for.cond.cleanup.loopexit ]
  %sub.ptr.lhs.cast = ptrtoint ptr %outp.0.lcssa to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %output_0 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %sub.ptr.div = ashr exact i64 %sub.ptr.sub, 3
  ret i64 %sub.ptr.div
}

!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"long", !5, i64 0}

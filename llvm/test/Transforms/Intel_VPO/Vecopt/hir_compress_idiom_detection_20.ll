; RUN: opt %s -mattr=+avx512f,+avx512vl -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -disable-output -debug-only=parvec-analysis -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -vplan-force-vf=4 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
;       |   %0 = (%a)[i1];
;       |   if (%0 != 0)
;       |   {
;       |      %1 = (%f)[i1];
;       |      if (%1 > 1.000000e+01)
;       |      {
;       |         %2 = (%f)[%c.032];
;       |         %c.032 = %c.032  +  1;
;       |         %.pn = %2;
;       |      }
;       |      else
;       |      {
;       |         %conv9 = sitofp.i32.float(%0);
;       |         %.pn = %conv9;
;       |      }
;       |      %r.1 = %1  +  %r.033;
;       |      %r.033 = %r.1  +  %.pn;
;       |   }
;       + END LOOP
; END REGION

; CHECK:  [Compress/Expand Idiom] Increment {sb:4}+1 detected: {{.*}} [[C_0320:%.*]] = [[C_0320]]  +  1
; CHECK-NEXT:  Idiom List
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[C_0320]] = [[C_0320]]  +  1
; CHECK-NEXT:    CELoad: ([[F0:%.*]])[%c.032]
; CHECK-NEXT:      CELdStIndex: [[C_0320]]

; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:         [[BB4:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB4]]
; CHECK-NEXT:     float [[VP_RED_INIT:%.*]] = reduction-init float 0.000000e+00
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     i32 [[VP_INIT:%.*]] = compress-expand-index-init i32 [[C_0320]]
; CHECK-NEXT:     br [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2:BB[0-9]+]]
; CHECK-NEXT:     float [[VP6:%.*]] = phi  [ float [[VP_RED_INIT]], [[BB1]] ],  [ float [[VP5:%.*]], [[BB2]] ]
; CHECK-NEXT:     i32 [[VP11:%.*]] = phi  [ i32 [[VP_INIT]], [[BB1]] ],  [ i32 [[VP25:%.*]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP10:%.*]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP9:%.*]], [[BB2]] ]
; CHECK-NEXT:     ptr [[VP_SUBSCRIPT_1:%.*]] = subscript inbounds ptr [[A0:%.*]] i64 [[VP10]]
; CHECK-NEXT:     i32 [[VP_LOAD_1:%.*]] = load ptr [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:     i1 [[VP16:%.*]] = icmp ne i32 [[VP_LOAD_1]] i32 0
; CHECK-NEXT:     br i1 [[VP16]], [[BB5:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB5]]: # preds: [[BB0]]
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_2:%.*]] = subscript inbounds ptr [[F0]] i64 [[VP10]]
; CHECK-NEXT:       float [[VP_LOAD_2:%.*]] = load ptr [[VP_SUBSCRIPT_2]]
; CHECK-NEXT:       i1 [[VP17:%.*]] = fcmp ogt float [[VP_LOAD_2]] float 1.000000e+01
; CHECK-NEXT:       br i1 [[VP17]], [[BB6:BB[0-9]+]], [[BB7:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:        [[BB7]]: # preds: [[BB5]]
; CHECK-NEXT:         float [[VP18:%.*]] = sitofp i32 [[VP_LOAD_1]] to float
; CHECK-NEXT:         float [[VP19:%.*]] = hir-copy float [[VP18]] , OriginPhiId: -1
; CHECK-NEXT:         br [[BB3:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:        [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:         i64 [[VP15:%.*]] = sext i32 [[VP11]] to i64
; CHECK-NEXT:         ptr [[VP_SUBSCRIPT:%.*]] = subscript inbounds ptr [[F0]] i64 [[VP15]]
; CHECK-NEXT:         float [[VP26:%.*]] = expand-load ptr [[VP_SUBSCRIPT]]
; CHECK-NEXT:         float [[VP20:%.*]] = hir-copy float [[VP26]] , OriginPhiId: -1
; CHECK-NEXT:         br [[BB3]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB3]]: # preds: [[BB6]], [[BB7]]
; CHECK-NEXT:       i1 [[VP27:%.*]] = phi  [ i1 true, [[BB6]] ],  [ i1 false, [[BB7]] ]
; CHECK-NEXT:       float [[VP21:%.*]] = phi  [ float [[VP20]], [[BB6]] ],  [ float [[VP19]], [[BB7]] ]
; CHECK-NEXT:       float [[VP7:%.*]] = fadd float [[VP_LOAD_2]] float [[VP6]]
; CHECK-NEXT:       float [[VP8:%.*]] = fadd float [[VP7]] float [[VP21]]
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB3]], [[BB0]]
; CHECK-NEXT:     float [[VP5]] = phi  [ float [[VP8]], [[BB3]] ],  [ float [[VP6]], [[BB0]] ]
; CHECK-NEXT:     i1 [[VP28:%.*]] = phi  [ i1 [[VP27]], [[BB3]] ],  [ i1 false, [[BB0]] ]
; CHECK-NEXT:     i32 [[VP25]] = compress-expand-index-inc {stride: 1} i32 [[VP11]] i1 [[VP28]]
; CHECK-NEXT:     i64 [[VP9]] = add i64 [[VP10]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP22:%.*]] = icmp slt i64 [[VP9]] i64 1024
; CHECK-NEXT:     br i1 [[VP22]], [[BB0]], [[BB8:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB8]]: # preds: [[BB2]]
; CHECK-NEXT:     float [[VP_RED_FINAL:%.*]] = reduction-final{fadd} float [[VP5]] float [[R_0330:%.*]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     i32 [[VP_FINAL:%.*]] = compress-expand-index-final i32 [[VP25]]
; CHECK-NEXT:     br [[BB9:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB9]]: # preds: [[BB8]]
; CHECK-NEXT:     br <External Block>

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[RED_INIT0:%.*]] = 0.000000e+00
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[RED_INIT0]]
; CHECK-NEXT:        [[PHI_TEMP40:%.*]] = [[C_0320]]
; CHECK:             + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC70:%.*]] = undef
; CHECK-NEXT:        |   [[DOTVEC0:%.*]] = (<4 x i32>*)([[A0]])[i1]
; CHECK-NEXT:        |   [[DOTVEC60:%.*]] = [[DOTVEC0]] != 0
; CHECK-NEXT:        |   [[DOTVEC70]] = (<4 x float>*)([[F0]])[i1], Mask = @{[[DOTVEC60]]}
; CHECK-NEXT:        |   [[DOTVEC80:%.*]] = [[DOTVEC70]] > 1.000000e+01
; CHECK-NEXT:        |   [[DOTVEC90:%.*]] = [[DOTVEC80]]  ^  -1
; CHECK-NEXT:        |   [[DOTVEC100:%.*]] = ([[DOTVEC0]] != 0) ? [[DOTVEC90]] : 0
; CHECK-NEXT:        |   [[DOTVEC110:%.*]] = ([[DOTVEC0]] != 0) ? [[DOTVEC80]] : 0
; CHECK-NEXT:        |   [[DOTVEC120:%.*]] = sitofp.<4 x i32>.<4 x float>([[DOTVEC0]])
; CHECK-NEXT:        |   [[DOTCOPY130:%.*]] = [[DOTVEC120]]
; CHECK-NEXT:        |   [[EXP_LOAD0:%.*]] = @llvm.masked.expandload.v4f32(&((float*)([[F0]])[%phi.temp4]),  [[DOTVEC110]],  poison)
; CHECK-NEXT:        |   [[DOTCOPY140:%.*]] = [[EXP_LOAD0]]
; CHECK-NEXT:        |   [[SELECT0:%.*]] = ([[DOTVEC110]] == <i1 true, i1 true, i1 true, i1 true>) ? -1 : 0
; CHECK-NEXT:        |   [[SELECT150:%.*]] = ([[DOTVEC110]] == <i1 true, i1 true, i1 true, i1 true>) ? [[DOTCOPY140]] : [[DOTCOPY130]]
; CHECK-NEXT:        |   [[DOTVEC160:%.*]] = [[DOTVEC70]]  +  [[PHI_TEMP0]]
; CHECK-NEXT:        |   [[DOTVEC170:%.*]] = [[DOTVEC160]]  +  [[SELECT150]]
; CHECK-NEXT:        |   [[SELECT180:%.*]] = ([[DOTVEC60]] == <i1 true, i1 true, i1 true, i1 true>) ? [[DOTVEC170]] : [[PHI_TEMP0]]
; CHECK-NEXT:        |   [[SELECT190:%.*]] = ([[DOTVEC60]] == <i1 true, i1 true, i1 true, i1 true>) ? [[SELECT0]] : 0
; CHECK-NEXT:        |   [[CAST0:%.*]] = bitcast.<4 x i1>.i4([[SELECT190]])
; CHECK-NEXT:        |   [[POPCNT0:%.*]] = @llvm.ctpop.i4([[CAST0]])
; CHECK-NEXT:        |   [[ZEXT0:%.*]] = zext.i4.i32([[POPCNT0]])
; CHECK-NEXT:        |   [[MUL0:%.*]] = [[ZEXT0]]  *  1
; CHECK-NEXT:        |   [[ADD0:%.*]] = [[PHI_TEMP40]]  +  [[MUL0]]
; CHECK-NEXT:        |   [[PHI_TEMP0]] = [[SELECT180]]
; CHECK-NEXT:        |   [[PHI_TEMP40]] = [[ADD0]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[R_0330]] = @llvm.vector.reduce.fadd.v4f32([[R_0330]],  [[SELECT180]])
; CHECK-NEXT:        [[C_0320]] = [[ADD0]]
; CHECK-NEXT:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(ptr nocapture noundef readonly %f, ptr nocapture noundef readonly %a) {
for.body.preheader:
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %r.033 = phi float [ 0.000000e+00, %for.body.preheader ], [ %r.2, %for.inc ]
  %c.032 = phi i32 [ 0, %for.body.preheader ], [ %c.2, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx2 = getelementptr inbounds float, ptr %f, i64 %indvars.iv
  %1 = load float, ptr %arrayidx2
  %cmp3 = fcmp fast ogt float %1, 1.000000e+01
  br i1 %cmp3, label %if.then4, label %if.else

if.then4:                                         ; preds = %if.then
  %inc = add nsw i32 %c.032, 1
  %idxprom5 = sext i32 %c.032 to i64
  %arrayidx6 = getelementptr inbounds float, ptr %f, i64 %idxprom5
  %2 = load float, ptr %arrayidx6
  br label %if.end

if.else:                                          ; preds = %if.then
  %conv9 = sitofp i32 %0 to float
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then4
  %c.1 = phi i32 [ %inc, %if.then4 ], [ %c.032, %if.else ]
  %.pn = phi float [ %2, %if.then4 ], [ %conv9, %if.else ]
  %r.1 = fadd fast float %1, %r.033
  %add13 = fadd fast float %r.1, %.pn
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.end
  %c.2 = phi i32 [ %c.1, %if.end ], [ %c.032, %for.body ]
  %r.2 = phi float [ %add13, %if.end ], [ %r.033, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %conv16 = sitofp i32 %c.2 to float
  %add17 = fadd fast float %r.2, %conv16
  %conv18 = fptosi float %add17 to i32
  ret i32 %conv18
}

; XFAIL:*
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,hir-temp-array-transpose" -hir-create-function-level-region -print-after=hir-temp-array-transpose -disable-output 2>&1 | FileCheck %s

; Check that Temp Array Transpose successfully kicks in for this use case. Test is a reduced version of GB6
; benchmark. Note the if (-1 * ptrtoint.i8*.i64(%24) + ptrtoint.i8*.i64(%23) == %11) which affects
; the loopnest that contains one of the uses of %20.

; HIR Before
;        BEGIN REGION { modified }
;              %10 = (null)[0];
;              %11 = (null)[0];
;              %12 = @llvm.umul.with.overflow.i64(0,  0);
;              %18 = (null)[0];
;              %20 = (%B)[0].2;
;              %23 = &((%.pre222)[0]);
;              %24 = null;
;              if (%18 > 0)
;              {
;                 + DO i1 = 0, sext.i32.i64(%11) + -1, 1
;                 |   + DO i2 = 0, zext.i32.i64(%18) + -1, 1
;                 |   |   %34 = (%20)[i1 + sext.i32.i64(%11) * i2];
;                 |   |   (null)[0] = %34;
;                 |   + END LOOP
;                 + END LOOP
;              }
;              else
;              {
;                 ret ;
;              }
;              if (-1 * ptrtoint.i8*.i64(%24) + ptrtoint.i8*.i64(%23) == %11)
;              {
;                 if (%18 > 0)
;                 {
;                    + DO i1 = 0, sext.i32.i64(%10) + -1, 1
;                    |   + DO i2 = 0, -1 * ptrtoint.i8*.i64(%24) + ptrtoint.i8*.i64(%23) + -1, 1
;                    |   |   + DO i3 = 0, sext.i32.i64(%18) + -1, 1
;                    |   |   |   %45 = (%20)[i2 + (-1 * ptrtoint.i8*.i64(%24) + ptrtoint.i8*.i64(%23)) * i3];
;                    |   |   + END LOOP
;                    |   |
;                    |   |   (null)[0] = %45;
;                    |   + END LOOP
;                    + END LOOP
;                 }
;              }
;              else
;              {
;                 if (%18 <= 0)
;                 {
;                    goto for.body49.i.preheader;
;                 }
;                 else
;                 {
;                    + DO i1 = 0, sext.i32.i64(%10) + -1, 1
;                    |   + DO i2 = 0, sext.i32.i64(%11) + -1, 1
;                    |   |   + DO i3 = 0, sext.i32.i64(%18) + -1, 1
;                    |   |   |   %58 = (%20)[i2 + sext.i32.i64(%11) * i3];
;                    |   |   + END LOOP
;                    |   |
;                    |   |   (null)[0] = %58;
;                    |   + END LOOP
;                    + END LOOP
;                 }
;              }
;              ret ;
;              for.body49.i.preheader:
;              ret ;
;        END REGION

; HIR After
; CHECK: BEGIN REGION { modified }
;              %10 = (null)[0];
;              %11 = (null)[0];
;              %12 = @llvm.umul.with.overflow.i64(0,  0);
;              %18 = (null)[0];
; CHECK:       %20 = (%B)[0].2;
;              %23 = &((%.pre222)[0]);
;              %24 = null;
; CHECK:       %call = @llvm.stacksave();
; CHECK:       %TranspTmpArr = alloca (zext.i32.i64(%18) * sext.i32.i64(%11));
; CHECK:       + DO i1 = 0, sext.i32.i64(%11) + -1, 1
; CHECK:       |   + DO i2 = 0, zext.i32.i64(%18) + -1, 1
; CHECK:       |   |   (%TranspTmpArr)[i1][i2] = (%20)[i2][i1];
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
;              if (%18 > 0)
;              {
; CHECK:          + DO i1 = 0, sext.i32.i64(%11) + -1, 1
; CHECK:          |   + DO i2 = 0, zext.i32.i64(%18) + -1, 1
; CHECK:          |   |   %34 = (%TranspTmpArr)[i1][i2];
;                 |   |   (null)[0] = %34;
;                 |   + END LOOP
;                 + END LOOP
;              }
;              else
;              {
;                 ret ;
;              }
; CHECK:       if (-1 * ptrtoint.i8*.i64(%24) + ptrtoint.i8*.i64(%23) == %11)
;              {
;                 if (%18 > 0)
;                 {
; CHECK:             + DO i1 = 0, sext.i32.i64(%10) + -1, 1
; CHECK:             |   + DO i2 = 0, -1 * ptrtoint.i8*.i64(%24) + ptrtoint.i8*.i64(%23) + -1, 1
; CHECK:             |   |   + DO i3 = 0, sext.i32.i64(%18) + -1, 1
; CHECK:             |   |   |   %45 = (%TranspTmpArr)[i2][i3];
;                    |   |   + END LOOP
;                    |   |
;                    |   |   (null)[0] = %45;
;                    |   + END LOOP
;                    + END LOOP
;                 }
;              }
;              else
;              {
;                 if (%18 <= 0)
;                 {
;                    goto for.body49.i.preheader;
;                 }
;                 else
;                 {
; CHECK:             + DO i1 = 0, sext.i32.i64(%10) + -1, 1
; CHECK:             |   + DO i2 = 0, sext.i32.i64(%11) + -1, 1
; CHECK:             |   |   + DO i3 = 0, sext.i32.i64(%18) + -1, 1
; CHECK:             |   |   |   %58 = (%TranspTmpArr)[i2][i3];
;                    |   |   + END LOOP
;                    |   |
;                    |   |   (null)[0] = %58;
;                    |   + END LOOP
;                    + END LOOP
;                 }
;              }
; CHECK:       @llvm.stackrestore(&((%call)[0]));
;              ret ;
;              for.body49.i.preheader:
;              ret ;
;        END REGION


%"struct.ml::cpu::Matrix" = type { i32, i32, i8* }

define void @_ZN2ml3cpu15gemm_lowp_naiveERKNS0_6MatrixIaEES4_RKNS1_IiEERS2_NS0_8BiasTypeESt6vectorIiSaIiEESC_aSA_IaSaIaEESC_a(%"struct.ml::cpu::Matrix"* %B, i8* %.pre222) {
entry:
  %0 = load i32, i32* null, align 8
  br i1 false, label %for.cond1.preheader.lr.ph, label %delete.end33

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %1 = load i32, i32* null, align 4
  %2 = icmp sgt i32 0, 0
  br i1 false, label %for.cond1.preheader.preheader, label %delete.end33

for.cond1.preheader.preheader:                    ; preds = %for.cond1.preheader.lr.ph
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %for.cond1.preheader.preheader
  %3 = phi i32 [ 0, %for.cond.cleanup3 ], [ 0, %for.cond1.preheader.preheader ]
  %4 = phi i32 [ 0, %for.cond.cleanup3 ], [ 0, %for.cond1.preheader.preheader ]
  br i1 false, label %for.body4.preheader, label %for.cond.cleanup3

for.body4.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body4

for.cond.cleanup3.loopexit:                       ; preds = %for.body4
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %5 = phi i32 [ 0, %for.cond.cleanup3.loopexit ], [ 0, %for.cond1.preheader ]
  %6 = phi i32 [ 0, %for.cond.cleanup3.loopexit ], [ 0, %for.cond1.preheader ]
  %cmp = icmp slt i32 0, 0
  br i1 %cmp, label %for.cond1.preheader, label %delete.end33.loopexit

for.body4:                                        ; preds = %for.body4, %for.body4.preheader
  %7 = phi i32 [ 0, %for.body4 ], [ 0, %for.body4.preheader ]
  %8 = load i8*, i8** null, align 8
  %9 = load i32, i32* null, align 4
  %cmp2 = icmp slt i32 0, 0
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3.loopexit

delete.end33.loopexit:                            ; preds = %for.cond.cleanup3
  br label %delete.end33

delete.end33:                                     ; preds = %delete.end33.loopexit, %for.cond1.preheader.lr.ph, %entry
  %10 = load i32, i32* null, align 8
  %11 = load i32, i32* null, align 4
  %12 = tail call { i64, i1 } @llvm.umul.with.overflow.i64(i64 0, i64 0)
  %13 = extractvalue { i64, i1 } zeroinitializer, 1
  %14 = extractvalue { i64, i1 } zeroinitializer, 0
  %15 = select i1 false, i64 0, i64 0
  %16 = bitcast i8* null to i32*
  %17 = bitcast i8* null to i32*
  %18 = load i32, i32* null, align 8
  %19 = load i8*, i8** null, align 8
  %data21 = getelementptr inbounds %"struct.ml::cpu::Matrix", %"struct.ml::cpu::Matrix"* %B, i64 0, i32 2
  %20 = load i8*, i8** %data21, align 8
  %21 = load i8*, i8** null, align 8
  %22 = load i8*, i8** null, align 8
  br label %_ZNSt12_Vector_baseIaSaIaEEC2EmRKS0_.exit.i

_ZNSt16allocator_traitsISaIaEE8allocateERS0_m.exit.i.i.i.i: ; No predecessors!
  %.pre2221 = load i8*, i8** null, align 8
  %.pre223 = load i8*, i8** null, align 8
  br label %_ZNSt12_Vector_baseIaSaIaEEC2EmRKS0_.exit.i

_ZNSt12_Vector_baseIaSaIaEEC2EmRKS0_.exit.i:      ; preds = %_ZNSt16allocator_traitsISaIaEE8allocateERS0_m.exit.i.i.i.i, %delete.end33
  %23 = phi i8* [ null, %_ZNSt16allocator_traitsISaIaEE8allocateERS0_m.exit.i.i.i.i ], [ %.pre222, %delete.end33 ]
  %24 = phi i8* [ %.pre222, %_ZNSt16allocator_traitsISaIaEE8allocateERS0_m.exit.i.i.i.i ], [ null, %delete.end33 ]
  %sub.ptr.lhs.cast.i.i.i.i.i.i.i.i.i = ptrtoint i8* %23 to i64
  %sub.ptr.rhs.cast.i.i.i.i.i.i.i.i.i = ptrtoint i8* %24 to i64
  %sub.ptr.sub.i.i.i.i.i.i.i.i.i = sub i64 %sub.ptr.lhs.cast.i.i.i.i.i.i.i.i.i, %sub.ptr.rhs.cast.i.i.i.i.i.i.i.i.i
  br label %_ZNSt6vectorIaSaIaEEC2ERKS1_.exit

_ZNSt6vectorIaSaIaEEC2ERKS1_.exit:                ; preds = %_ZNSt12_Vector_baseIaSaIaEEC2EmRKS0_.exit.i
  %conv.i = sext i32 %11 to i64
  br label %_ZNSt6vectorIiSaIiEE17_S_check_init_lenEmRKS0_.exit.i.i

_ZNSt6vectorIiSaIiEE17_S_check_init_lenEmRKS0_.exit.i.i: ; preds = %_ZNSt6vectorIaSaIaEEC2ERKS1_.exit
  br i1 false, label %_ZNSt6vectorIiSaIiEEC2EmRKS0_.exit.i, label %if.end.i.i.i.i.i.i.i.i

if.end.i.i.i.i.i.i.i.i:                           ; preds = %_ZNSt6vectorIiSaIiEE17_S_check_init_lenEmRKS0_.exit.i.i
  %25 = bitcast i8* null to i32*
  br label %_ZNSt6vectorIiSaIiEEC2EmRKS0_.exit.i

_ZNSt6vectorIiSaIiEEC2EmRKS0_.exit.i:             ; preds = %if.end.i.i.i.i.i.i.i.i, %_ZNSt6vectorIiSaIiEE17_S_check_init_lenEmRKS0_.exit.i.i
  %conv1.i = sext i32 %10 to i64
  br label %_ZNSt6vectorIiSaIiEE17_S_check_init_lenEmRKS0_.exit.i174.i

_ZNSt6vectorIiSaIiEE17_S_check_init_lenEmRKS0_.exit.i174.i: ; preds = %_ZNSt6vectorIiSaIiEEC2EmRKS0_.exit.i
  br i1 false, label %for.cond15.preheader.i, label %for.body.lr.ph.i

for.body.lr.ph.i:                                 ; preds = %_ZNSt6vectorIiSaIiEE17_S_check_init_lenEmRKS0_.exit.i174.i
  %26 = bitcast i8* null to i32*
  br label %for.body.us.preheader.i

for.body.us.preheader.i:                          ; preds = %for.body.lr.ph.i
  %27 = zext i32 0 to i64
  br label %for.body.us.i

for.body.us.i:                                    ; preds = %for.cond4.for.cond.cleanup6_crit_edge.us.i, %for.body.us.preheader.i
  %28 = mul nuw nsw i64 0, 0
  br label %for.body7.us.i

for.body7.us.i:                                   ; preds = %for.body7.us.i, %for.body.us.i
  %29 = add nuw nsw i64 0, 0
  %30 = load i8, i8* null, align 1
  %31 = load i32, i32* null, align 4
  %exitcond336.not.i = icmp eq i64 0, 0
  br i1 %exitcond336.not.i, label %for.cond4.for.cond.cleanup6_crit_edge.us.i, label %for.body7.us.i

for.cond4.for.cond.cleanup6_crit_edge.us.i:       ; preds = %for.body7.us.i
  %exitcond341.not.i = icmp eq i64 0, 0
  br i1 %exitcond341.not.i, label %for.cond15.preheader.i.loopexit, label %for.body.us.i

for.cond15.preheader.i.loopexit:                  ; preds = %for.cond4.for.cond.cleanup6_crit_edge.us.i
  br label %for.cond15.preheader.i

for.cond15.preheader.i:                           ; preds = %for.cond15.preheader.i.loopexit, %_ZNSt6vectorIiSaIiEE17_S_check_init_lenEmRKS0_.exit.i174.i
  %cmp16274.i = icmp sgt i32 1, 0
  br i1 true, label %for.body18.lr.ph.i, label %for.cond41.preheader.i

for.body18.lr.ph.i:                               ; preds = %for.cond15.preheader.i
  %cmp23272.i = icmp sgt i32 %18, 0
  br i1 %cmp23272.i, label %for.body18.us.preheader.i, label %for.body18.i.preheader

for.body18.i.preheader:                           ; preds = %for.body18.lr.ph.i
  ret void

for.body18.us.preheader.i:                        ; preds = %for.body18.lr.ph.i
  %wide.trip.count322.i175 = zext i32 %18 to i64
  br label %for.body18.us.i

for.body18.us.i:                                  ; preds = %for.cond22.for.cond.cleanup24_crit_edge.us.i, %for.body18.us.preheader.i
  %indvars.iv324.i = phi i64 [ 0, %for.body18.us.preheader.i ], [ %indvars.iv.next325.i, %for.cond22.for.cond.cleanup24_crit_edge.us.i ]
  br label %for.body25.us.i

for.body25.us.i:                                  ; preds = %for.body25.us.i, %for.body18.us.i
  %indvars.iv318.i = phi i64 [ 0, %for.body18.us.i ], [ %indvars.iv.next319.i, %for.body25.us.i ]
  %32 = mul nsw i64 %indvars.iv318.i, %conv.i
  %33 = add nsw i64 %32, %indvars.iv324.i
  %arrayidx29.us.i = getelementptr inbounds i8, i8* %20, i64 %33
  %34 = load i8, i8* %arrayidx29.us.i, align 1
  %conv30.us.i = sext i8 %34 to i32
  %35 = load i32, i32* null, align 4
  %add33.us.i = add nsw i32 0, 2
  store i32 %conv30.us.i, i32* null, align 4
  %indvars.iv.next319.i = add nuw nsw i64 %indvars.iv318.i, 1
  %exitcond323.not.i = icmp eq i64 %indvars.iv.next319.i, %wide.trip.count322.i175
  br i1 %exitcond323.not.i, label %for.cond22.for.cond.cleanup24_crit_edge.us.i, label %for.body25.us.i

for.cond22.for.cond.cleanup24_crit_edge.us.i:     ; preds = %for.body25.us.i
  %indvars.iv.next325.i = add nuw nsw i64 %indvars.iv324.i, 1
  %exitcond327.not.i = icmp eq i64 %indvars.iv.next325.i, %conv.i
  br i1 %exitcond327.not.i, label %for.cond41.preheader.i.loopexit, label %for.body18.us.i

for.cond41.preheader.i.loopexit:                  ; preds = %for.cond22.for.cond.cleanup24_crit_edge.us.i
  br label %for.cond41.preheader.i

for.cond41.preheader.i:                           ; preds = %for.cond41.preheader.i.loopexit, %for.cond15.preheader.i
  br label %for.cond46.preheader.lr.ph.i

for.cond46.preheader.lr.ph.i:                     ; preds = %for.cond41.preheader.i
  %cmp62215.i = icmp sgt i32 %18, 0
  %36 = sext i32 %18 to i64
  %cmp52.i = icmp eq i64 %sub.ptr.sub.i.i.i.i.i.i.i.i.i, %conv.i
  br label %for.cond46.preheader.i

for.cond46.preheader.i:                           ; preds = %for.cond.cleanup48.i, %for.cond46.preheader.lr.ph.i
  %indvars.iv312.i = phi i64 [ 0, %for.cond46.preheader.lr.ph.i ], [ %indvars.iv.next313.i, %for.cond.cleanup48.i ]
  br i1 true, label %for.body49.lr.ph.i, label %for.cond.cleanup48.i

for.body49.lr.ph.i:                               ; preds = %for.cond46.preheader.i
  %37 = mul nsw i64 0, 0
  %38 = mul nsw i64 0, 0
  br i1 %cmp52.i, label %for.body49.us.i.preheader, label %for.body49.lr.ph.split.i

for.body49.us.i.preheader:                        ; preds = %for.body49.lr.ph.i
  br i1 %cmp62215.i, label %for.body49.us.i.us.preheader, label %for.body49.us.i.preheader247

for.body49.us.i.preheader247:                     ; preds = %for.body49.us.i.preheader
  br label %for.body49.us.i

for.body49.us.i.us.preheader:                     ; preds = %for.body49.us.i.preheader
  br label %for.body49.us.i.us

for.body49.us.i.us:                               ; preds = %for.cond.cleanup63.us.i.loopexit.us, %for.body49.us.i.us.preheader
  %indvars.iv307.i.us = phi i64 [ %indvars.iv.next308.i.us, %for.cond.cleanup63.us.i.loopexit.us ], [ 0, %for.body49.us.i.us.preheader ]
  %39 = add nsw i64 0, 0
  %40 = load i32, i32* null, align 4
  br label %for.body64.us.i.us

for.body64.us.i.us:                               ; preds = %for.body64.us.i.us, %for.body49.us.i.us
  %indvars.iv300.i.us = phi i64 [ %indvars.iv.next301.i.us, %for.body64.us.i.us ], [ 0, %for.body49.us.i.us ]
  %41 = add nsw i64 0, 0
  %42 = load i8, i8* null, align 1
  %43 = mul nsw i64 %indvars.iv300.i.us, undef
  %44 = add nsw i64 %43, %indvars.iv307.i.us
  %arrayidx73.us.i.us = getelementptr inbounds i8, i8* %20, i64 %44
  %45 = load i8, i8* %arrayidx73.us.i.us, align 1
  %conv74.us.i.us = sext i8 %45 to i32
  %mul75.us.i.us = mul nsw i32 2, 1
  %add76.us.i.us = add nsw i32 1, 0
  %indvars.iv.next301.i.us = add nuw nsw i64 %indvars.iv300.i.us, 1
  %exitcond306.not.i.us = icmp eq i64 %indvars.iv.next301.i.us, %36
  br i1 %exitcond306.not.i.us, label %for.cond.cleanup63.us.i.loopexit.us, label %for.body64.us.i.us

for.cond.cleanup63.us.i.loopexit.us:              ; preds = %for.body64.us.i.us
  %add76.us.i.us.lcssa = phi i32 [ 0, %for.body64.us.i.us ]
  %46 = load i32, i32* null, align 4
  %47 = load i32, i32* null, align 4
  %add93.us.i.us = add i32 0, 0
  %add98.us.i.us = sub i32 0, 0
  store i32 %conv74.us.i.us, i32* null, align 4
  %indvars.iv.next308.i.us = add nuw nsw i64 %indvars.iv307.i.us, 1
  %exitcond311.not.i.us = icmp eq i64 %indvars.iv.next308.i.us, %sub.ptr.sub.i.i.i.i.i.i.i.i.i
  br i1 %exitcond311.not.i.us, label %for.cond.cleanup48.i.loopexit, label %for.body49.us.i.us

for.body49.us.i:                                  ; preds = %for.body49.us.i, %for.body49.us.i.preheader247
  %48 = add nsw i64 0, 0
  %49 = load i32, i32* null, align 4
  %50 = load i32, i32* null, align 4
  %51 = load i32, i32* null, align 4
  %exitcond311.not.i = icmp eq i64 0, 0
  br i1 %exitcond311.not.i, label %for.cond.cleanup48.i.loopexit248, label %for.body49.us.i

for.body49.lr.ph.split.i:                         ; preds = %for.body49.lr.ph.i
  br i1 %cmp62215.i, label %for.body49.us221.i.preheader, label %for.body49.i.preheader

for.body49.i.preheader:                           ; preds = %for.body49.lr.ph.split.i
  ret void

for.body49.us221.i.preheader:                     ; preds = %for.body49.lr.ph.split.i
  br label %for.body49.us221.i

for.body49.us221.i:                               ; preds = %for.cond61.for.cond.cleanup63_crit_edge.us267.i, %for.body49.us221.i.preheader
  %indvars.iv295.i = phi i64 [ %indvars.iv.next296.i, %for.cond61.for.cond.cleanup63_crit_edge.us267.i ], [ 0, %for.body49.us221.i.preheader ]
  %52 = add nsw i64 0, 0
  %53 = load i32, i32* null, align 4
  br label %for.body64.us250.i

for.body64.us250.i:                               ; preds = %for.body64.us250.i, %for.body49.us221.i
  %indvars.iv288.i = phi i64 [ 0, %for.body49.us221.i ], [ %indvars.iv.next289.i, %for.body64.us250.i ]
  %54 = add nsw i64 0, 0
  %55 = load i8, i8* null, align 1
  %56 = mul nsw i64 %indvars.iv288.i, undef
  %57 = add nsw i64 %56, %indvars.iv295.i
  %arrayidx73.us260.i = getelementptr inbounds i8, i8* %20, i64 %57
  %58 = load i8, i8* %arrayidx73.us260.i, align 1
  %conv74.us261.i = sext i8 %58 to i32
  %mul75.us262.i = mul nsw i32 2, 1
  %add76.us263.i = add nsw i32 1, 0
  %indvars.iv.next289.i = add nuw nsw i64 %indvars.iv288.i, 1
  %exitcond294.not.i = icmp eq i64 %indvars.iv.next289.i, %36
  br i1 %exitcond294.not.i, label %for.cond61.for.cond.cleanup63_crit_edge.us267.i, label %for.body64.us250.i

for.cond61.for.cond.cleanup63_crit_edge.us267.i:  ; preds = %for.body64.us250.i
  %add76.us263.i.lcssa = phi i32 [ 0, %for.body64.us250.i ]
  %add93.us246.i = add i32 0, 0
  %add98.us247.i = sub i32 0, 0
  store i32 %conv74.us261.i, i32* null, align 4
  %indvars.iv.next296.i = add nuw nsw i64 %indvars.iv295.i, 1
  %exitcond299.not.i = icmp eq i64 %indvars.iv.next296.i, %conv.i
  br i1 %exitcond299.not.i, label %for.cond.cleanup48.i.loopexit249, label %for.body49.us221.i

for.cond.cleanup43.i.loopexit:                    ; preds = %for.cond.cleanup48.i
  ret void

for.cond.cleanup48.i.loopexit:                    ; preds = %for.cond.cleanup63.us.i.loopexit.us
  br label %for.cond.cleanup48.i

for.cond.cleanup48.i.loopexit248:                 ; preds = %for.body49.us.i
  br label %for.cond.cleanup48.i

for.cond.cleanup48.i.loopexit249:                 ; preds = %for.cond61.for.cond.cleanup63_crit_edge.us267.i
  br label %for.cond.cleanup48.i

for.cond.cleanup48.i:                             ; preds = %for.cond.cleanup48.i.loopexit249, %for.cond.cleanup48.i.loopexit248, %for.cond.cleanup48.i.loopexit, %for.cond46.preheader.i
  %indvars.iv.next313.i = add nuw nsw i64 %indvars.iv312.i, 1
  %exitcond317.not.i = icmp eq i64 %indvars.iv.next313.i, %conv1.i
  br i1 %exitcond317.not.i, label %for.cond.cleanup43.i.loopexit, label %for.cond46.preheader.i
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare { i64, i1 } @llvm.umul.with.overflow.i64(i64, i64) #0

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memmove.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1 immarg) #2

attributes #0 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #1 = { argmemonly nofree nounwind willreturn writeonly }
attributes #2 = { argmemonly nofree nounwind willreturn }

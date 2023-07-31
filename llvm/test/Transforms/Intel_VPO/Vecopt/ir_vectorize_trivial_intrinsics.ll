; Test to check correctness of CallVecDecisions analysis and codegen for trivially vectorizable intrinsics.

; RUN: opt < %s -passes=vplan-vec -vplan-force-vf=2 -vplan-print-after-call-vec-decisions -disable-output | FileCheck %s --check-prefix=VEC-PROP
; RUNX: opt < %s -passes=vplan-vec -vplan-force-vf=2 -S | FileCheck %s --check-prefix=CG-CHECK

declare double @llvm.powi.f64.i32(double %Val, i32 %power) nounwind readnone

define void @powi_f64(i32 %n, ptr noalias nocapture readonly %y, ptr noalias nocapture %x, i32 %P) local_unnamed_addr #2 {
; VEC-PROP-LABEL:  VPlan after CallVecDecisions analysis for merged CFG:
; VEC-PROP:            [DA: Div] double [[VP2:%.*]] = load ptr [[VP_ARRAYIDX:%.*]]
; VEC-PROP-NEXT:       [DA: Div] double [[VP_CALL1:%.*]] = call double [[VP2]] i32 [[P0:%.*]] llvm.powi.v2f64 [x 1]
; VEC-PROP:            [DA: Div] i32 [[VP_EXPONENT:%.*]] = trunc i64 [[VP_INDVARS_IV:%.*]] to i32
; VEC-PROP-NEXT:       [DA: Div] double [[VP_CALL2:%.*]] = call double [[VP2]] i32 [[VP_EXPONENT]] ptr @llvm.powi.f64.i32 [Serial]

; CG-CHECK-LABEL: @powi_f64(
; CG-CHECK:       vector.body:
; CG-CHECK:         [[WIDE_LOAD:%.*]] = load <2 x double>, ptr [[PTR:%.*]], align 8
; CG-CHECK-NEXT:    [[WIDE_LOAD_EXTRACT_1_:%.*]] = extractelement <2 x double> [[WIDE_LOAD]], i32 1
; CG-CHECK-NEXT:    [[WIDE_LOAD_EXTRACT_0_:%.*]] = extractelement <2 x double> [[WIDE_LOAD]], i32 0
; CG-CHECK-NEXT:    [[VEC_CALL:%.*]] = call <2 x double> @llvm.powi.v2f64.i32(<2 x double> [[WIDE_LOAD]], i32 [[P:%.*]])
; CG-CHECK:         [[IV_TRUNC:%.*]] = trunc <2 x i64> [[VEC_PHI:%.*]] to <2 x i32>
; CG-CHECK-NEXT:    [[DOTEXTRACT_1_:%.*]] = extractelement <2 x i32> [[IV_TRUNC]], i32 1
; CG-CHECK-NEXT:    [[DOTEXTRACT_0_:%.*]] = extractelement <2 x i32> [[IV_TRUNC]], i32 0
; CG-CHECK-NEXT:    [[SERIAL_CALL_1:%.*]] = call double @llvm.powi.f64.i32(double [[WIDE_LOAD_EXTRACT_0_]], i32 [[DOTEXTRACT_0_]])
; CG-CHECK-NEXT:    [[TMP8:%.*]] = insertelement <2 x double> undef, double [[SERIAL_CALL_1]], i32 0
; CG-CHECK-NEXT:    [[SERIAL_CALL_2:%.*]] = call double @llvm.powi.f64.i32(double [[WIDE_LOAD_EXTRACT_1_]], i32 [[DOTEXTRACT_1_]])
; CG-CHECK-NEXT:    [[TMP10:%.*]] = insertelement <2 x double> [[TMP8]], double [[SERIAL_CALL_2]], i32 1
;
entry:
  %cmp9 = icmp sgt i32 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds double, ptr %y, i64 %indvars.iv
  %0 = load double, ptr %arrayidx, align 8

  ; powi call with loop invariant always scalar operand (can be vectorized).
  %call1 = tail call double @llvm.powi.f64.i32(double %0, i32 %P)
  %arrayidx4 = getelementptr inbounds double, ptr %x, i64 %indvars.iv
  store double %call1, ptr %arrayidx4, align 8

  ; powi call with loop variant always scalar operand (should be serialized).
  %exponent = trunc i64 %indvars.iv to i32
  %call2 = tail call double @llvm.powi.f64.i32(double %0, i32 %exponent)
  store double %call2, ptr %arrayidx4, align 8

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare double @llvm.fmuladd.f64(double %a, double %b, double %c) nounwind readnone

define void @fmuladd_f64(i32 %n, ptr %a_arr, ptr %b_arr, ptr %c_arr) local_unnamed_addr #2 {
; VEC-PROP-LABEL:  VPlan after CallVecDecisions analysis for merged CFG:
; VEC-PROP:            [DA: Div] double [[VP_CALL1:%.*]] = call double [[VP_A:%.*]] double [[VP_B:%.*]] double [[VP_C:%.*]] llvm.fmuladd.v2f64 [x 1]
; VEC-PROP-NEXT:       [DA: Div] i1 [[VP_COND:%.*]] = fcmp oeq double [[VP_CALL1]] double 4.200000e+01
; VEC-PROP-NEXT:       [DA: Uni] br [[BB5:BB[0-9]+]]
; VEC-PROP-EMPTY:
; VEC-PROP-NEXT:      [[BB5]]: # preds: [[BB3:BB.*]]
; VEC-PROP-NEXT:       [DA: Div] i1 [[VP2:%.*]] = block-predicate i1 [[VP_COND]]
; VEC-PROP-NEXT:       [DA: Div] double [[VP_CALL2:%.*]] = call double [[VP_A]] double [[VP_B]] double [[VP_C]] llvm.fmuladd.v2f64 [x 1]
; VEC-PROP-NEXT:     [DA: Uni] br [[BB4:BB[0-9]+]]
;
; CG-CHECK-LABEL: @fmuladd_f64(
; CG-CHECK:       vector.body:
; CG-CHECK:         [[CALL1:%.*]] = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> [[WIDE_LOAD:%.*]], <2 x double> [[WIDE_LOAD3:%.*]], <2 x double> [[WIDE_LOAD5:%.*]])
; CG-CHECK-NEXT:    [[TMP7:%.*]] = fcmp oeq <2 x double> [[CALL1]], <double 4.200000e+01, double 4.200000e+01>
; CG-CHECK-NEXT:    [[CALL2:%.*]] = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> [[WIDE_LOAD]], <2 x double> [[WIDE_LOAD3]], <2 x double> [[WIDE_LOAD5]])
;
entry:
  %cmp9 = icmp sgt i32 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %if.merge ], [ 0, %for.body.preheader ]
  %a_idx = getelementptr inbounds double, ptr %a_arr, i64 %indvars.iv
  %a = load double, ptr %a_idx, align 8
  %b_idx = getelementptr inbounds double, ptr %b_arr, i64 %indvars.iv
  %b = load double, ptr %b_idx, align 8
  %c_idx = getelementptr inbounds double, ptr %c_arr, i64 %indvars.iv
  %c = load double, ptr %c_idx, align 8
  ; Unmasked fmuladd call
  %call1 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
  %cond = fcmp oeq double %call1, 42.0
  br i1 %cond, label %if.then, label %if.merge

if.then:
  ; Unmasked fmuladd call
  %call2 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
  br label %if.merge

if.merge:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind

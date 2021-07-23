; RUN: opt %s -S -enable-vp-value-codegen-hir=1 -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -hir-memory-reduction-sinking -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -disable-output 2>&1 | FileCheck %s --check-prefix=VPCHECK

; CHECK:        + DO i1 = 0, -1 * [[DOTPR0:%.*]] + 3, 1   <DO_LOOP>
; CHECK:        |   [[MUL_VEC0:%.*]] = (<2 x double>*)([[TMP2:%.*]])[<i64 0, i64 4>]  *  [[TMP5:%.*]]
; CHECK-NEXT:   |   [[DOTVEC0:%.*]] = [[RED_VAR0:%.*]]  +  [[MUL_VEC0]]
; CHECK-NEXT:   |   [[MUL_VEC0]] = (<2 x double>*)([[TMP2]])[<i64 0, i64 4> + 1]  *  [[TMP5]]
; CHECK-NEXT:   |   [[DOTVEC200:%.*]] = [[RED_VAR130:%.*]]  +  [[MUL_VEC0]]
; CHECK-NEXT:   |   [[MUL_VEC0]] = (<2 x double>*)([[TMP2]])[<i64 0, i64 4> + 2]  *  [[TMP5]]
; CHECK-NEXT:   |   [[DOTVEC220:%.*]] = [[RED_VAR140:%.*]]  +  [[MUL_VEC0]]
; CHECK-NEXT:   |   [[MUL_VEC0]] = (<2 x double>*)([[TMP2]])[<i64 0, i64 4> + 3]  *  [[TMP5]]
; CHECK-NEXT:   |   [[DOTVEC240:%.*]] = [[RED_VAR150:%.*]]  +  [[MUL_VEC0]]
; CHECK-NEXT:   |   [[MUL_VEC0]] = (<2 x double>*)([[TMP2]])[<i64 0, i64 4> + 8]  *  [[TMP5]]
; CHECK-NEXT:   |   [[RED_VAR0]] = [[DOTVEC0]]  +  [[MUL_VEC0]]
; CHECK-NEXT:   |   [[MUL_VEC0]] = (<2 x double>*)([[TMP2]])[<i64 0, i64 4> + 9]  *  [[TMP5]]
; CHECK-NEXT:   |   [[RED_VAR130]] = [[DOTVEC200]]  +  [[MUL_VEC0]]
; CHECK-NEXT:   |   [[MUL_VEC0]] = (<2 x double>*)([[TMP2]])[<i64 0, i64 4> + 10]  *  [[TMP5]]
; CHECK-NEXT:   |   [[RED_VAR140]] = [[DOTVEC220]]  +  [[MUL_VEC0]]
; CHECK-NEXT:   |   [[MUL_VEC0]] = (<2 x double>*)([[TMP2]])[<i64 0, i64 4> + 11]  *  [[TMP5]]
; CHECK-NEXT:   |   [[RED_VAR150]] = [[DOTVEC240]]  +  [[MUL_VEC0]]
; CHECK:        + END LOOP

; VPCHECK:      + DO i1 = 0, -1 * [[DOTPR0:%.*]] + 3, 1   <DO_LOOP>
; VPCHECK:      |   [[DOTVEC0:%.*]] = (<2 x double>*)([[TMP2:%.*]])[4 * <i64 0, i64 1>]
; VPCHECK-NEXT: |   [[DOTVEC190:%.*]] = [[DOTVEC0]]  *  [[TMP5:%.*]]
; VPCHECK-NEXT: |   [[DOTVEC200:%.*]] = [[RED_VAR0:%.*]]  +  [[DOTVEC190]]
; VPCHECK-NEXT: |   [[DOTVEC210:%.*]] = (<2 x double>*)([[TMP2]])[4 * <i64 0, i64 1> + 1]
; VPCHECK-NEXT: |   [[DOTVEC220:%.*]] = [[DOTVEC210]]  *  [[TMP5]]
; VPCHECK-NEXT: |   [[DOTVEC230:%.*]] = [[RED_VAR130:%.*]]  +  [[DOTVEC220]]
; VPCHECK-NEXT: |   [[DOTVEC240:%.*]] = (<2 x double>*)([[TMP2]])[4 * <i64 0, i64 1> + 2]
; VPCHECK-NEXT: |   [[DOTVEC250:%.*]] = [[DOTVEC240]]  *  [[TMP5]]
; VPCHECK-NEXT: |   [[DOTVEC260:%.*]] = [[RED_VAR140:%.*]]  +  [[DOTVEC250]]
; VPCHECK-NEXT: |   [[DOTVEC270:%.*]] = (<2 x double>*)([[TMP2]])[4 * <i64 0, i64 1> + 3]
; VPCHECK-NEXT: |   [[DOTVEC280:%.*]] = [[DOTVEC270]]  *  [[TMP5]]
; VPCHECK-NEXT: |   [[DOTVEC290:%.*]] = [[RED_VAR150:%.*]]  +  [[DOTVEC280]]
; VPCHECK-NEXT: |   [[DOTVEC300:%.*]] = (<2 x double>*)([[TMP2]])[4 * <i64 0, i64 1> + 8]
; VPCHECK-NEXT: |   [[DOTVEC310:%.*]] = [[DOTVEC300]]  *  [[TMP5]]
; VPCHECK-NEXT: |   [[RED_VAR0]] = [[DOTVEC200]]  +  [[DOTVEC310]]
; VPCHECK-NEXT: |   [[DOTVEC330:%.*]] = (<2 x double>*)([[TMP2]])[4 * <i64 0, i64 1> + 9]
; VPCHECK-NEXT: |   [[DOTVEC340:%.*]] = [[DOTVEC330]]  *  [[TMP5]]
; VPCHECK-NEXT: |   [[RED_VAR130]] = [[DOTVEC230]]  +  [[DOTVEC340]]
; VPCHECK-NEXT: |   [[DOTVEC360:%.*]] = (<2 x double>*)([[TMP2]])[4 * <i64 0, i64 1> + 10]
; VPCHECK-NEXT: |   [[DOTVEC370:%.*]] = [[DOTVEC360]]  *  [[TMP5]]
; VPCHECK-NEXT: |   [[RED_VAR140]] = [[DOTVEC260]]  +  [[DOTVEC370]]
; VPCHECK-NEXT: |   [[DOTVEC390:%.*]] = (<2 x double>*)([[TMP2]])[4 * <i64 0, i64 1> + 11]
; VPCHECK-NEXT: |   [[DOTVEC400:%.*]] = [[DOTVEC390]]  *  [[TMP5]]
; VPCHECK-NEXT: |   [[RED_VAR150]] = [[DOTVEC290]]  +  [[DOTVEC400]]
; VPCHECK:      + END LOOP

@c = external global i32

define i32 @main() {
entry:
  %0 = bitcast i8* undef to double*
  br label %for.cond5.preheader

for.cond5.preheader:                              ; preds = %entry
  br label %for.body7

for.cond.cleanup:                                 ; preds = %for.body7
  %1 = load double, double* undef
  %.pr = load i32, i32* @c
  br label %for.body15.lr.ph

for.body15.lr.ph:                                 ; preds = %for.cond.cleanup
  br label %for.body15

for.body7:                                        ; preds = %for.body7, %for.cond5.preheader
  br i1 undef, label %for.cond.cleanup, label %for.body7

for.cond13.for.cond36.preheader_crit_edge:        ; preds = %for.end29
  ret i32 undef

for.body15:                                       ; preds = %for.end29, %for.body15.lr.ph
  %inc3265 = phi i32 [ %.pr, %for.body15.lr.ph ], [ %inc32, %for.end29 ]
  br label %for.cond19.preheader

for.cond19.preheader:                             ; preds = %for.inc27, %for.body15
  %incdec.ptr.lcssa63 = phi double* [ %0, %for.body15 ], [ %scevgep, %for.inc27 ]
  %storemerge57 = phi i32 [ 0, %for.body15 ], [ %inc28, %for.inc27 ]
  br label %for.body21

for.body21:                                       ; preds = %for.body21, %for.cond19.preheader
  %indvars.iv67 = phi i64 [ 0, %for.cond19.preheader ], [ %indvars.iv.next68, %for.body21 ]
  %incdec.ptr61 = phi double* [ %incdec.ptr.lcssa63, %for.cond19.preheader ], [ %incdec.ptr, %for.body21 ]
  %incdec.ptr = getelementptr inbounds double, double* %incdec.ptr61, i64 1
  %2 = load double, double* %incdec.ptr61
  %mul = fmul fast double %2, %1
  %add.ptr23 = getelementptr inbounds double, double* undef, i64 %indvars.iv67
  %3 = load double, double* %add.ptr23
  %add = fadd fast double %3, %mul
  store double %add, double* %add.ptr23
  %indvars.iv.next68 = add nuw nsw i64 %indvars.iv67, 1
  %exitcond69.not = icmp eq i64 %indvars.iv.next68, 4
  br i1 %exitcond69.not, label %for.inc27, label %for.body21

for.inc27:                                        ; preds = %for.body21
  %scevgep = getelementptr double, double* %incdec.ptr.lcssa63, i64 4
  %inc28 = add nuw nsw i32 %storemerge57, 1
  %exitcond71.not = icmp eq i32 %inc28, 4
  br i1 %exitcond71.not, label %for.end29, label %for.cond19.preheader, !llvm.loop !0

for.end29:                                        ; preds = %for.inc27
  %inc32 = add nsw i32 %inc3265, 1
  %exitcond73.not = icmp eq i32 %inc32, 4
  br i1 %exitcond73.not, label %for.cond13.for.cond36.preheader_crit_edge, label %for.body15
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 2}

; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" < %s -disable-output -xmain-opt-level=3 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate" -aa-pipeline="basic-aa" -debug-only=hir-opt-predicate < %s -disable-output -xmain-opt-level=3 < %s 2>&1 | FileCheck -check-prefix=CHECK-DBG %s

; This change set checks that the two select instructions %slct.8 and %slct.9
; were unswitched since they share the same condition. It was created from
; the following Fortran code:

;  subroutine loop_merge_compute(nproma, nlev, nblk, &
;          i_startblk, i_endblk, &
;          i_startlev, i_endlev, &
;          i_startidx, i_endidx, &
;          blkidx, array)
;    implicit none
;    integer, intent(in) :: nproma, nlev, nblk
;    integer, intent(in) :: i_startblk, i_endblk
;    integer, intent(in) :: i_startlev, i_endlev
;    integer, intent(in) :: i_startidx, i_endidx
;
;    integer, intent(in) :: blkidx(nlev,nblk)
;    real(kind=real64) :: array(nproma, nlev, nblk)
;
;    logical :: blkidx_pos
;
;    integer :: jb, jk, jc, jbind
;    real(kind=real64) :: add1, add2, add3
;
;    do jb = i_startblk, i_endblk
;      ! Vectorize along the grid levels, indices blkidx(:,jb) are identical
;      do jk = i_startlev, i_endlev
;        do jc = i_startidx, i_endidx
;          blkidx_pos = blkidx(jk,jb) > 0
;          jbind = MERGE(blkidx(jk,jb), 1, blkidx_pos)
;          add1 = MERGE(array(jc,jk,jb), real(0,real64), blkidx_pos)
;          add2 = MERGE(-array(jc,jk,jb), real(0,real64), blkidx_pos)
;          add3 = MERGE(real(1.0,real64)*array(jc,jk,jb), real(0,real64), blkidx_pos)
;          array(jc,jk,jbind) = add1 + add2 + add3
;        end do
;      end do
;    end do
;
;  end subroutine loop_merge_compute

; HIR before transformation

;   BEGIN REGION { }
;         + DO i1 = 0, sext.i32.i64((1 + %I_ENDBLK_fetch.104)) + -1 * sext.i32.i64(%I_STARTBLK_fetch.103) + -1, 1   <DO_LOOP>
;         |   if (%I_ENDLEV_fetch.107 >= %I_STARTLEV_fetch.106)
;         |   {
;         |      + DO i2 = 0, sext.i32.i64((1 + %I_ENDLEV_fetch.107)) + -1 * sext.i32.i64(%I_STARTLEV_fetch.106) + -1, 1   <DO_LOOP>
;         |      |   if (%I_ENDIDX_fetch.110 >= %I_STARTIDX_fetch.109)
;         |      |   {
;         |      |      %"BLKIDX[][]_fetch.118" = (%BLKIDX)[i1 + sext.i32.i64(%I_STARTBLK_fetch.103) + -1][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1];
;         |      |      %slct.7 = (%"BLKIDX[][]_fetch.118" > 0) ? %"BLKIDX[][]_fetch.118" : 1;
;         |      |
;         |      |      + DO i3 = 0, sext.i32.i64((1 + %I_ENDIDX_fetch.110)) + -1 * sext.i32.i64(%I_STARTIDX_fetch.109) + -1, 1   <DO_LOOP>
;         |      |      |   %"ARRAY[][][]_fetch.137" = (%ARRAY)[i1 + sext.i32.i64(%I_STARTBLK_fetch.103)][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1];
;         |      |      |   %slct.8 = (%"BLKIDX[][]_fetch.118" > 0) ? %"ARRAY[][][]_fetch.137" : 0.000000e+00;
;         |      |      |   %neg.1 =  - %"ARRAY[][][]_fetch.137";
;         |      |      |   %slct.9 = (%"BLKIDX[][]_fetch.118" > 0) ? %neg.1 : 0.000000e+00;
;         |      |      |   %add.26 = %slct.8  +  %slct.9;
;         |      |      |   %add.27 = %slct.8  +  %add.26;
;         |      |      |   (%ARRAY)[%slct.7][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1] = %add.27;
;         |      |      + END LOOP
;         |      |   }
;         |      + END LOOP
;         |   }
;         + END LOOP
;   END REGION

; HIR after transformation

; CHECK:   BEGIN REGION { modified }
; CHECK:         if (%I_ENDLEV_fetch.107 >= %I_STARTLEV_fetch.106)
; CHECK:         {
; CHECK:            if (%I_ENDIDX_fetch.110 >= %I_STARTIDX_fetch.109)
; CHECK:            {
; CHECK:               + DO i1 = 0, sext.i32.i64((1 + %I_ENDBLK_fetch.104)) + -1 * sext.i32.i64(%I_STARTBLK_fetch.103) + -1, 1   <DO_LOOP>
; CHECK:               |   + DO i2 = 0, sext.i32.i64((1 + %I_ENDLEV_fetch.107)) + -1 * sext.i32.i64(%I_STARTLEV_fetch.106) + -1, 1   <DO_LOOP>
; CHECK:               |   |   %"BLKIDX[][]_fetch.118" = (%BLKIDX)[i1 + sext.i32.i64(%I_STARTBLK_fetch.103) + -1][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1];
; CHECK:               |   |   %slct.7 = (%"BLKIDX[][]_fetch.118" > 0) ? %"BLKIDX[][]_fetch.118" : 1;
; CHECK:               |   |   if (%"BLKIDX[][]_fetch.118" > 0)
; CHECK:               |   |   {
; CHECK:               |   |      + DO i3 = 0, sext.i32.i64((1 + %I_ENDIDX_fetch.110)) + -1 * sext.i32.i64(%I_STARTIDX_fetch.109) + -1, 1   <DO_LOOP>
; CHECK:               |   |      |   %"ARRAY[][][]_fetch.137" = (%ARRAY)[i1 + sext.i32.i64(%I_STARTBLK_fetch.103)][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1];
; CHECK:               |   |      |   %slct.8 = %"ARRAY[][][]_fetch.137";
; CHECK:               |   |      |   %neg.1 =  - %"ARRAY[][][]_fetch.137";
; CHECK:               |   |      |   %slct.9 = %neg.1;
; CHECK:               |   |      |   %add.26 = %slct.8  +  %slct.9;
; CHECK:               |   |      |   %add.27 = %slct.8  +  %add.26;
; CHECK:               |   |      |   (%ARRAY)[%slct.7][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1] = %add.27;
; CHECK:               |   |      + END LOOP
; CHECK:               |   |   }
; CHECK:               |   |   else
; CHECK:               |   |   {
; CHECK:               |   |      + DO i3 = 0, sext.i32.i64((1 + %I_ENDIDX_fetch.110)) + -1 * sext.i32.i64(%I_STARTIDX_fetch.109) + -1, 1   <DO_LOOP>
; CHECK:               |   |      |   %"ARRAY[][][]_fetch.137" = (%ARRAY)[i1 + sext.i32.i64(%I_STARTBLK_fetch.103)][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1];
; CHECK:               |   |      |   %slct.8 = 0.000000e+00;
; CHECK:               |   |      |   %neg.1 =  - %"ARRAY[][][]_fetch.137";
; CHECK:               |   |      |   %slct.9 = 0.000000e+00;
; CHECK:               |   |      |   %add.26 = %slct.8  +  %slct.9;
; CHECK:               |   |      |   %add.27 = %slct.8  +  %add.26;
; CHECK:               |   |      |   (%ARRAY)[%slct.7][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1] = %add.27;
; CHECK:               |   |      + END LOOP
; CHECK:               |   |   }
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:            }
; CHECK:         }
; CHECK:   END REGION

; Verify the debug information

; CHECK-DBG: Opt Predicate for Function: loop_merge_mod_mp_loop_merge_compute_
; CHECK-DBG: Candidates, count: 4
; CHECK-DBG:          %slct.9 = (%"BLKIDX[][]_fetch.118" > 0) ? %neg.1 : 0.000000e+00;
; CHECK-DBG: , L: 2, PU: [ F/F ]}
; CHECK-DBG:          %slct.8 = (%"BLKIDX[][]_fetch.118" > 0) ? %"ARRAY[][][]_fetch.137" : 0.000000e+00;
; CHECK-DBG: , L: 2, PU: [ F/F ]}
; CHECK-DBG: Candidate for converting Select:
; CHECK-DBG:         %slct.8 = (%"BLKIDX[][]_fetch.118" > 0) ? %"ARRAY[][][]_fetch.137" : 0.000000e+00;
; CHECK-DBG: Into If/Else:
; CHECK-DBG:        if (%"BLKIDX[][]_fetch.118" > 0)
; CHECK-DBG:        {
; CHECK-DBG:           %slct.8 = %"ARRAY[][][]_fetch.137";
; CHECK-DBG:        }
; CHECK-DBG:        else
; CHECK-DBG:        {
; CHECK-DBG:           %slct.8 = 0.000000e+00;
; CHECK-DBG:        }

; CHECK-DBG: Candidate for converting Select:
; CHECK-DBG:         %slct.9 = (%"BLKIDX[][]_fetch.118" > 0) ? %neg.1 : 0.000000e+00;
; CHECK-DBG: Into If/Else:
; CHECK-DBG:        if (%"BLKIDX[][]_fetch.118" > 0)
; CHECK-DBG:        {
; CHECK-DBG:           %slct.9 = %neg.1;
; CHECK-DBG:        }
; CHECK-DBG:        else
; CHECK-DBG:        {
; CHECK-DBG:           %slct.9 = 0.000000e+00;
; CHECK-DBG:        }

define void @loop_merge_mod_mp_loop_merge_compute_(i32* noalias nocapture readonly dereferenceable(4) %NPROMA, i32* noalias nocapture readonly dereferenceable(4) %NLEV, i32* noalias nocapture readonly dereferenceable(4) %NBLK, i32* noalias nocapture readonly dereferenceable(4) %I_STARTBLK, i32* noalias nocapture readonly dereferenceable(4) %I_ENDBLK, i32* noalias nocapture readonly dereferenceable(4) %I_STARTLEV, i32* noalias nocapture readonly dereferenceable(4) %I_ENDLEV, i32* noalias nocapture readonly dereferenceable(4) %I_STARTIDX, i32* noalias nocapture readonly dereferenceable(4) %I_ENDIDX, i32* noalias nocapture readonly dereferenceable(4) %BLKIDX, double* noalias nocapture dereferenceable(8) %ARRAY) {
alloca_3:
  %NLEV_fetch.100 = load i32, i32* %NLEV
  %NPROMA_fetch.102 = load i32, i32* %NPROMA
  %int_sext5 = sext i32 %NLEV_fetch.100 to i64
  %mul.14 = shl nsw i64 %int_sext5, 2
  %int_sext13 = sext i32 %NPROMA_fetch.102 to i64
  %mul.16 = shl nsw i64 %int_sext13, 3
  %mul.17 = mul nsw i64 %mul.16, %int_sext5
  %I_STARTBLK_fetch.103 = load i32, i32* %I_STARTBLK
  %I_ENDBLK_fetch.104 = load i32, i32* %I_ENDBLK
  %rel.25 = icmp slt i32 %I_ENDBLK_fetch.104, %I_STARTBLK_fetch.103
  br i1 %rel.25, label %bb23, label %bb22.preheader

bb22.preheader:                                   ; preds = %alloca_3
  %I_STARTLEV_fetch.106 = load i32, i32* %I_STARTLEV
  %I_ENDLEV_fetch.107 = load i32, i32* %I_ENDLEV
  %rel.26 = icmp slt i32 %I_ENDLEV_fetch.107, %I_STARTLEV_fetch.106
  %I_STARTIDX_fetch.109 = load i32, i32* %I_STARTIDX
  %I_ENDIDX_fetch.110 = load i32, i32* %I_ENDIDX
  %rel.27 = icmp slt i32 %I_ENDIDX_fetch.110, %I_STARTIDX_fetch.109
  %0 = sext i32 %I_STARTIDX_fetch.109 to i64
  %1 = add nsw i32 %I_ENDIDX_fetch.110, 1
  %2 = sext i32 %I_STARTLEV_fetch.106 to i64
  %3 = add nsw i32 %I_ENDLEV_fetch.107, 1
  %4 = sext i32 %I_STARTBLK_fetch.103 to i64
  %5 = add nsw i32 %I_ENDBLK_fetch.104, 1
  %wide.trip.count87 = sext i32 %5 to i64
  %wide.trip.count83 = sext i32 %3 to i64
  %wide.trip.count = sext i32 %1 to i64
  br label %bb22

bb22:                                             ; preds = %bb22.preheader, %bb27
  %indvars.iv85 = phi i64 [ %4, %bb22.preheader ], [ %indvars.iv.next86, %bb27 ]
  br i1 %rel.26, label %bb27, label %bb26.preheader

bb26.preheader:                                   ; preds = %bb22
  %"BLKIDX[]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %mul.14, i32* nonnull elementtype(i32) %BLKIDX, i64 %indvars.iv85)
  %"ARRAY[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul.17, double* nonnull elementtype(double) %ARRAY, i64 %indvars.iv85)
  br label %bb26

bb26:                                             ; preds = %bb26.preheader, %bb31
  %indvars.iv81 = phi i64 [ %2, %bb26.preheader ], [ %indvars.iv.next82, %bb31 ]
  br i1 %rel.27, label %bb31, label %bb30.preheader

bb30.preheader:                                   ; preds = %bb26
  %"BLKIDX[][]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"BLKIDX[]", i64 %indvars.iv81)
  %"BLKIDX[][]_fetch.118" = load i32, i32* %"BLKIDX[][]"
  %rel.28 = icmp sgt i32 %"BLKIDX[][]_fetch.118", 0
  %slct.7 = select i1 %rel.28, i32 %"BLKIDX[][]_fetch.118", i32 1
  %"ARRAY[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.16, double* nonnull elementtype(double) %"ARRAY[]", i64 %indvars.iv81)
  %int_sext33 = zext i32 %slct.7 to i64
  %"ARRAY[]34" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul.17, double* nonnull elementtype(double) %ARRAY, i64 %int_sext33)
  %"ARRAY[][]35" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.16, double* nonnull elementtype(double) %"ARRAY[]34", i64 %indvars.iv81)
  br label %bb30

bb30:                                             ; preds = %bb30.preheader, %bb30
  %indvars.iv = phi i64 [ %0, %bb30.preheader ], [ %indvars.iv.next, %bb30 ]
  %"ARRAY[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"ARRAY[][]", i64 %indvars.iv)
  %"ARRAY[][][]_fetch.137" = load double, double* %"ARRAY[][][]"
  %slct.8 = select i1 %rel.28, double %"ARRAY[][][]_fetch.137", double 0.000000e+00
  %neg.1 = fneg double %"ARRAY[][][]_fetch.137"
  %slct.9 = select i1 %rel.28, double %neg.1, double 0.000000e+00
  %add.26 = fadd double %slct.8, %slct.9
  %add.27 = fadd double %slct.8, %add.26
  %"ARRAY[][][]36" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"ARRAY[][]35", i64 %indvars.iv)
  store double %add.27, double* %"ARRAY[][][]36"
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb31.loopexit, label %bb30

bb31.loopexit:                                    ; preds = %bb30
  br label %bb31

bb31:                                             ; preds = %bb31.loopexit, %bb26
  %indvars.iv.next82 = add nsw i64 %indvars.iv81, 1
  %exitcond84 = icmp eq i64 %indvars.iv.next82, %wide.trip.count83
  br i1 %exitcond84, label %bb27.loopexit, label %bb26

bb27.loopexit:                                    ; preds = %bb31
  br label %bb27

bb27:                                             ; preds = %bb27.loopexit, %bb22
  %indvars.iv.next86 = add nsw i64 %indvars.iv85, 1
  %exitcond88 = icmp eq i64 %indvars.iv.next86, %wide.trip.count87
  br i1 %exitcond88, label %bb23.loopexit, label %bb22

bb23.loopexit:                                    ; preds = %bb27
  br label %bb23

bb23:                                             ; preds = %bb23.loopexit, %alloca_3
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #0

; Function Attrs: nofree nosync nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

attributes #0 = { nofree nosync nounwind readnone speculatable }
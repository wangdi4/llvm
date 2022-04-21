; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec  -vplan-force-vf=2 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=2 < %s 2>&1 | FileCheck %s
;
; LIT test for assertion fail reported in CMPLRLLVM-36658. The test was generated using:
;   ifx -O2 -S -xcore-avx2 -mllvm -print-module-before-loopopt test.f90
;
;  function check(a)
;  implicit none
;  logical, dimension(3,4) :: a
;  integer :: i
;  integer :: check
;
;  i = 1
;  if (all(all(a,i))) then
;     check = 0
;  else
;     check = 1
;  endif
;  end function check
;
; The Fortran all function elides the specified dimension(1) of array a. This generates
; incoming HIR that looks like the following:
;
;         + DO i2 = 0, 2, 1   <DO_LOOP>
;         |   %"check_$A_entry[][]_fetch.4" = (%"check_$A")[0:i1:12(i32*:0)][0:i2:4(i32*:3)];
;         |   %"var$1110[][]_fetch.24" = (%"var$111034.sub")[0:i1:4(i32*:0)][0:i2:0(i32*:0)];
;         |   %and.1 = %"var$1110[][]_fetch.24"  &  %"check_$A_entry[][]_fetch.4";
;         |   (%"var$111034.sub")[0:i1:4(i32*:0)][0:i2:0(i32*:0)] = %and.1;
;         + END LOOP
;
; Note the stride value of zero (0:i2:0). This causes an assertion fail in DA. If the stride
; value is zero, DA fix effectively treats the corresponding dimension as uniform. Test checks
; for successful vectorization.
;
; CHECK: DO i2 = 0, 1, 2   <DO_LOOP> <auto-vectorized> <novectorize>
;
define i32 @check_(i32* noalias nocapture readonly dereferenceable(4) %"check_$A") {
bb9:
  %"var$5" = alloca [1 x i64], align 16
  %"(i64*)var$5$4" = getelementptr inbounds [1 x i64], [1 x i64]* %"var$5", i64 0, i64 0
  %"var$5[]5" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 0, i64 8, i64* nonnull elementtype(i64) %"(i64*)var$5$4", i64 0)
  store i64 4, i64* %"var$5[]5", align 8
  %"var$111034" = alloca [4 x i32], align 4
  %"var$111034.sub" = getelementptr inbounds [4 x i32], [4 x i32]* %"var$111034", i64 0, i64 0
  br label %bb11

bb11:                                             ; preds = %bb9, %bb11
  %"var$12.039" = phi i64 [ 1, %bb9 ], [ %add.5, %bb11 ]
  %"var$1110[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"var$111034.sub", i64 %"var$12.039")
  store i32 -1, i32* %"var$1110[]", align 4
  %add.5 = add nuw nsw i64 %"var$12.039", 1
  %exitcond42 = icmp eq i64 %add.5, 5
  br i1 %exitcond42, label %loop_test11.preheader.preheader, label %bb11

loop_test11.preheader.preheader:                  ; preds = %bb11
  br label %loop_test11.preheader

loop_body12:                                      ; preds = %loop_test11.preheader, %loop_body12
  %"$loop_ctr1.037" = phi i64 [ 1, %loop_test11.preheader ], [ %add.6, %loop_body12 ]
  %"check_$A_entry[][]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"check_$A_entry[]", i64 %"$loop_ctr1.037")
  %"check_$A_entry[][]_fetch.4" = load i32, i32* %"check_$A_entry[][]", align 1
  %"var$1110[][]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 0, i32* nonnull elementtype(i32) %"var$1110[]11", i64 %"$loop_ctr1.037")
  %"var$1110[][]_fetch.24" = load i32, i32* %"var$1110[][]", align 4
  %and.1 = and i32 %"var$1110[][]_fetch.24", %"check_$A_entry[][]_fetch.4"
  store i32 %and.1, i32* %"var$1110[][]", align 4
  %add.6 = add nuw nsw i64 %"$loop_ctr1.037", 1
  %exitcond40.not = icmp eq i64 %add.6, 4
  br i1 %exitcond40.not, label %loop_exit13, label %loop_body12

loop_exit13:                                      ; preds = %loop_body12
  %add.7 = add nuw nsw i64 %"$loop_ctr2.038", 1
  %exitcond41.not = icmp eq i64 %add.7, 5
  br i1 %exitcond41.not, label %loop_body20.preheader, label %loop_test11.preheader

loop_body20.preheader:                            ; preds = %loop_exit13
  br label %loop_body20

loop_test11.preheader:                            ; preds = %loop_test11.preheader.preheader, %loop_exit13
  %"$loop_ctr2.038" = phi i64 [ %add.7, %loop_exit13 ], [ 1, %loop_test11.preheader.preheader ]
  %"check_$A_entry[]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 12, i32* nonnull elementtype(i32) %"check_$A", i64 %"$loop_ctr2.038")
  %"var$1110[]11" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 4, i32* nonnull elementtype(i32) %"var$111034.sub", i64 %"$loop_ctr2.038")
  br label %loop_body12

loop_body20:                                      ; preds = %loop_body20.preheader, %loop_body20
  %"var$13.036" = phi i32 [ %and.2, %loop_body20 ], [ -1, %loop_body20.preheader ]
  %"$loop_ctr.035" = phi i64 [ %add.8, %loop_body20 ], [ 1, %loop_body20.preheader ]
  %"var$1110[]12" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"var$111034.sub", i64 %"$loop_ctr.035")
  %"var$1110[]_fetch.30" = load i32, i32* %"var$1110[]12", align 4
  %and.2 = and i32 %"var$1110[]_fetch.30", %"var$13.036"
  %add.8 = add nuw nsw i64 %"$loop_ctr.035", 1
  %exitcond = icmp eq i64 %add.8, 5
  br i1 %exitcond, label %loop_exit21, label %loop_body20

loop_exit21:                                      ; preds = %loop_body20
  %and.2.lcssa = phi i32 [ %and.2, %loop_body20 ]
  %and.3 = and i32 %and.2.lcssa, 1
  %0 = xor i32 %and.3, 1
  ret i32 %0
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8, i64, i64, i64*, i64) #1

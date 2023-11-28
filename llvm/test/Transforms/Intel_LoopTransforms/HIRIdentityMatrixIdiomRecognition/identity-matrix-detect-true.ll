; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-identity-matrix-idiom-recognition,print<hir>" -enable-alt-identity-matrix-detection -debug-only=hir-hlnode-utils -aa-pipeline=basic-aa -disable-output %s 2>&1 | FileCheck %s

; Verify that cases of valid identity matrix are detected. We can accept false
; negatives, but not false positives

; FORTRAN Source code examples
;    real A(5,5)
;    real B(5,5), C(5,5)
;    do i=1,5
;      do j=1,5
;        A(j,i) = 0.0
;      enddo
;      A(i,i) = 1.0
;    enddo
;    do i=1,5
;      do j=1,5
;        B(j,i) = 0.0
;        C(j,i) = 0.0
;      enddo
;      B(i,i) = 1.0
;      C(i,i) = 1.0
;    enddo

; CHECK: Found Diag Inst in OuterLp: <{{[0-9]+}}>         (@A)[0][i1][i1] = 1.000000e+00;
; CHECK: Found Zero Instruction: <{{[0-9]+}}>          (@A)[0][i1][i2] = 0.000000e+00;
; CHECK: Found Ident Matrix, DiagInst: <{{[0-9]+}}>         (@A)[0][i1][i1] = 1.000000e+00;

; CHECK: Found Diag Inst in OuterLp: <{{[0-9]+}}>         (@C)[0][i1][i1] = 1.000000e+00;
; CHECK: Found Diag Inst in OuterLp: <{{[0-9]+}}>         (@B)[0][i1][i1] = 1.000000e+00;
; CHECK: Found Zero Instruction: <{{[0-9]+}}>         (@C)[0][i1][i2] = 0.000000e+00;
; CHECK: Found Zero Instruction: <{{[0-9]+}}>         (@B)[0][i1][i2] = 0.000000e+00;
; CHECK: Found Ident Matrix, DiagInst: <{{[0-9]+}}>         (@C)[0][i1][i1] = 1.000000e+00;
; CHECK: Found Ident Matrix, DiagInst: <{{[0-9]+}}>         (@B)[0][i1][i1] = 1.000000e+00;


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"C" = internal unnamed_addr global [5 x [5 x float]] zeroinitializer, align 16
@"B" = internal unnamed_addr global [5 x [5 x float]] zeroinitializer, align 16
@"A" = internal unnamed_addr global [5 x [5 x float]] zeroinitializer, align 16
@0 = internal unnamed_addr constant i32 65536, align 4
@1 = internal unnamed_addr constant i32 2, align 4

; Function Attrs: nounwind uwtable
define void @MAIN__() local_unnamed_addr #0 {
alloca_0:
  %func_result = tail call i32 @for_set_fpe_(ptr nonnull @0) #3
  %func_result2 = tail call i32 @for_set_reentrancy(ptr nonnull @1) #3
  br label %do.body6

do.body6:                                         ; preds = %do.epilog13, %alloca_0
  %indvars.iv38 = phi i64 [ %indvars.iv.next39, %do.epilog13 ], [ 1, %alloca_0 ]
  %"A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20, ptr nonnull elementtype(float) @"A", i64 %indvars.iv38), !ifx.array_extent !0
  br label %do.body10

do.body10:                                        ; preds = %do.body10, %do.body6
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body10 ], [ 1, %do.body6 ]
  %"A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"A[]", i64 %indvars.iv)
  store float 0.000000e+00, ptr %"A[][]", align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond.not, label %do.epilog13, label %do.body10

do.epilog13:                                      ; preds = %do.body10
  %"A[][]7" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"A[]", i64 %indvars.iv38)
  store float 1.000000e+00, ptr %"A[][]7", align 4
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %exitcond40.not = icmp eq i64 %indvars.iv.next39, 6
  br i1 %exitcond40.not, label %do.body16.preheader, label %do.body6

do.body16.preheader:                              ; preds = %do.epilog13
  br label %do.body16

do.body16:                                        ; preds = %do.body16.preheader, %do.epilog23
  %indvars.iv44 = phi i64 [ %indvars.iv.next45, %do.epilog23 ], [ 1, %do.body16.preheader ]
  %"B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20, ptr nonnull elementtype(float) @"B", i64 %indvars.iv44), !ifx.array_extent !0
  %"C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20, ptr nonnull elementtype(float) @"C", i64 %indvars.iv44), !ifx.array_extent !0
  br label %do.body20

do.body20:                                        ; preds = %do.body20, %do.body16
  %indvars.iv41 = phi i64 [ %indvars.iv.next42, %do.body20 ], [ 1, %do.body16 ]
  %"B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"B[]", i64 %indvars.iv41)
  store float 0.000000e+00, ptr %"B[][]", align 4
  %"C[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"C[]", i64 %indvars.iv41)
  store float 0.000000e+00, ptr %"C[][]", align 4
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond43.not = icmp eq i64 %indvars.iv.next42, 6
  br i1 %exitcond43.not, label %do.epilog23, label %do.body20

do.epilog23:                                      ; preds = %do.body20
  %"B[][]15" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"B[]", i64 %indvars.iv44)
  store float 1.000000e+00, ptr %"B[][]15", align 4
  %"C[][]19" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"C[]", i64 %indvars.iv44)
  store float 1.000000e+00, ptr %"C[][]19", align 4
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond46.not = icmp eq i64 %indvars.iv.next45, 6
  br i1 %exitcond46.not, label %do.epilog19, label %do.body16

do.epilog19:                                      ; preds = %do.epilog23
  ret void
}

declare i32 @for_set_fpe_(ptr nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nounwind }

!0 = !{i64 5}

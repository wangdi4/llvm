; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-identity-matrix-idiom-recognition,print<hir>" -enable-alt-identity-matrix-detection -debug-only=hir-hlnode-utils -disable-output %s 2>&1 | FileCheck %s

; Verify that cases of invalid identity matrix are detected. We can accept false
; negatives, but not false positives

; FORTRAN Source code examples
;   real A(5,5), D(5,5,5)
;   real B(5,5), C(5,5)
;   do i=1,5
;     do j=1,5
;       A(j,i) = 0.0
;     enddo
;     A(i,0) = 1.0
;     A(i,i) = 1.0
;   enddo
;   do i=1,4
;     do j=1,4
;       B(j,i) = 0.0
;     enddo
;     B(i,i) = 1.0
;   enddo
;   do i=1,5
;     do j=1,5
;       C(j,i) = 0.0
;     enddo
;     call foo()
;     C(i,i) = 1.0
;   enddo
;   do i=1,5
;     A(i,i) = 1.0
;     do j=1,5
;       A(j,i) = 0.0
;     enddo
;   enddo

; CHECK: Found Diag Inst in OuterLp: <18>         (@A)[0][i1][i1] = 1.000000e+00;
; CHECK-NOT: Found Identity Matrix for Loop

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
  %"A[]5" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20, ptr nonnull elementtype(float) @"A", i64 0), !ifx.array_extent !0
  br label %do.body6

do.body6:                                         ; preds = %do.epilog13, %alloca_0
  %indvars.iv62 = phi i64 [ %indvars.iv.next63, %do.epilog13 ], [ 1, %alloca_0 ]
  %"A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20, ptr nonnull elementtype(float) @"A", i64 %indvars.iv62), !ifx.array_extent !0
  br label %do.body10

do.body10:                                        ; preds = %do.body10, %do.body6
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body10 ], [ 1, %do.body6 ]
  %"A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"A[]", i64 %indvars.iv)
  store float 0.000000e+00, ptr %"A[][]", align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond.not, label %do.epilog13, label %do.body10

do.epilog13:                                      ; preds = %do.body10
  %"A[][]6" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"A[]5", i64 %indvars.iv62)
  store float 1.000000e+00, ptr %"A[][]6", align 4
  %"A[][]10" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"A[]", i64 %indvars.iv62)
  store float 1.000000e+00, ptr %"A[][]10", align 4
  %indvars.iv.next63 = add nuw nsw i64 %indvars.iv62, 1
  %exitcond64.not = icmp eq i64 %indvars.iv.next63, 6
  br i1 %exitcond64.not, label %do.body17.preheader, label %do.body6

do.body17.preheader:                              ; preds = %do.epilog13
  br label %do.body17

do.body17:                                        ; preds = %do.body17.preheader, %do.epilog24
  %indvars.iv68 = phi i64 [ %indvars.iv.next69, %do.epilog24 ], [ 1, %do.body17.preheader ]
  %"B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20, ptr nonnull elementtype(float) @"B", i64 %indvars.iv68), !ifx.array_extent !0
  br label %do.body21

do.body21:                                        ; preds = %do.body21, %do.body17
  %indvars.iv65 = phi i64 [ %indvars.iv.next66, %do.body21 ], [ 1, %do.body17 ]
  %"B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"B[]", i64 %indvars.iv65)
  store float 0.000000e+00, ptr %"B[][]", align 4
  %indvars.iv.next66 = add nuw nsw i64 %indvars.iv65, 1
  %exitcond67.not = icmp eq i64 %indvars.iv.next66, 5
  br i1 %exitcond67.not, label %do.epilog24, label %do.body21

do.epilog24:                                      ; preds = %do.body21
  %"B[][]16" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"B[]", i64 %indvars.iv68)
  store float 1.000000e+00, ptr %"B[][]16", align 4
  %indvars.iv.next69 = add nuw nsw i64 %indvars.iv68, 1
  %exitcond70.not = icmp eq i64 %indvars.iv.next69, 5
  br i1 %exitcond70.not, label %do.body27.preheader, label %do.body17

do.body27.preheader:                              ; preds = %do.epilog24
  br label %do.body27

do.body27:                                        ; preds = %do.body27.preheader, %do.epilog34
  %indvars.iv74 = phi i64 [ %indvars.iv.next75, %do.epilog34 ], [ 1, %do.body27.preheader ]
  %"C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20, ptr nonnull elementtype(float) @"C", i64 %indvars.iv74), !ifx.array_extent !0
  br label %do.body31

do.body31:                                        ; preds = %do.body31, %do.body27
  %indvars.iv71 = phi i64 [ %indvars.iv.next72, %do.body31 ], [ 1, %do.body27 ]
  %"C[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"C[]", i64 %indvars.iv71)
  store float 0.000000e+00, ptr %"C[][]", align 4
  %indvars.iv.next72 = add nuw nsw i64 %indvars.iv71, 1
  %exitcond73.not = icmp eq i64 %indvars.iv.next72, 6
  br i1 %exitcond73.not, label %do.epilog34, label %do.body31

do.epilog34:                                      ; preds = %do.body31
  tail call void (...) @foo_() #3
  %"C[][]22" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"C[]", i64 %indvars.iv74)
  store float 1.000000e+00, ptr %"C[][]22", align 4
  %indvars.iv.next75 = add nuw nsw i64 %indvars.iv74, 1
  %exitcond76.not = icmp eq i64 %indvars.iv.next75, 6
  br i1 %exitcond76.not, label %do.body43.preheader, label %do.body27

do.body43.preheader:                              ; preds = %do.epilog34
  br label %do.body43

do.body43:                                        ; preds = %do.body43.preheader, %do.epilog51
  %indvars.iv80 = phi i64 [ %indvars.iv.next81, %do.epilog51 ], [ 1, %do.body43.preheader ]
  %"A[]25" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20, ptr nonnull elementtype(float) @"A", i64 %indvars.iv80), !ifx.array_extent !0
  %"A[][]26" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"A[]25", i64 %indvars.iv80)
  store float 1.000000e+00, ptr %"A[][]26", align 4
  br label %do.body48

do.body48:                                        ; preds = %do.body48, %do.body43
  %indvars.iv77 = phi i64 [ %indvars.iv.next78, %do.body48 ], [ 1, %do.body43 ]
  %"A[][]30" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"A[]25", i64 %indvars.iv77)
  store float 0.000000e+00, ptr %"A[][]30", align 4
  %indvars.iv.next78 = add nuw nsw i64 %indvars.iv77, 1
  %exitcond79.not = icmp eq i64 %indvars.iv.next78, 6
  br i1 %exitcond79.not, label %do.epilog51, label %do.body48

do.epilog51:                                      ; preds = %do.body48
  %indvars.iv.next81 = add nuw nsw i64 %indvars.iv80, 1
  %exitcond82.not = icmp eq i64 %indvars.iv.next81, 6
  br i1 %exitcond82.not, label %do.epilog46, label %do.body43

do.epilog46:                                      ; preds = %do.epilog51
  ret void
}

declare i32 @for_set_fpe_(ptr nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

declare void @foo_(...) local_unnamed_addr

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nounwind }

!0 = !{i64 5}

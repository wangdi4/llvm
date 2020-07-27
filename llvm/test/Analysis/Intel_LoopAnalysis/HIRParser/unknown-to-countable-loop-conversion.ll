; RUN: opt < %s -hir-ssa-deconstruction -xmain-opt-level=3 -analyze -hir-framework -hir-framework-debug=loop-formation 2>&1 | FileCheck %s --check-prefix=LOOP-FORMATION
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -xmain-opt-level=3 -hir-framework-debug=loop-formation 2>&1 | FileCheck %s --check-prefix=LOOP-FORMATION

; RUN: opt < %s -hir-ssa-deconstruction -xmain-opt-level=3 -analyze -hir-framework -hir-framework-debug=parser 2>&1 | FileCheck %s --check-prefix=PARSER
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -xmain-opt-level=3 -hir-framework-debug=parser 2>&1 | FileCheck %s --check-prefix=PARSER

; Verify that the loop in the second region is parsed as a DO loop by
; eliminatiing the label/bottom test in parsing phase.
; The loop is recognized by unknown in loop formation phase but once %t4 is
; parsed and nsw is applied to (1 + %t1)<nsw> during parsing of first region,
; the second loop becomes countable using the additional information that %t1
; is not signed max value.

; LOOP-FORMATION:     BEGIN REGION

; LOOP-FORMATION:     + UNKNOWN LOOP i1
; LOOP-FORMATION-NOT: loop1:

; LOOP-FORMATION:     + END LOOP

; LOOP-FORMATION:     BEGIN REGION

; LOOP-FORMATION:     + UNKNOWN LOOP i1
; LOOP-FORMATION:     |   loop2:

; LOOP-FORMATION:     |   if (0x0 true 0x0)
; LOOP-FORMATION:     |   {
; LOOP-FORMATION:     |   }
; LOOP-FORMATION:     |   else
; LOOP-FORMATION:     |   {
; LOOP-FORMATION:     |      goto loop2;
; LOOP-FORMATION:     |   }
; LOOP-FORMATION:     + END LOOP

; PARSER: BEGIN REGION

; PARSER: + DO i1 = 0, %t3 + -2, 1   <DO_LOOP>
; PARSER: |   (%t2)[sext.i32.i64(%t1)][i1] = 0.000000e+00;
; PARSER: + END LOOP

; PARSER: BEGIN REGION

; PARSER: + DO i1 = 0, sext.i32.i64((-1 + %t1)), 1   <DO_LOOP>
; PARSER: |   @prtri_(&((%t2)[i1][0]));
; PARSER: + END LOOP


define void @foo(i32 %t1, double* %t2, i64 %t3) "intel-lang"="fortran" {
entry:
  br label %dom.block

dom.block:                                             ; preds = %t3228, %t3226
  %t4 = add nsw i32 %t1, 1
  %sxt = sext i32 %t4 to i64
  %gep1 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* %t2, i64 %sxt)
  %gep2 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* %gep1, i64 1)
  br label %loop1

loop1:                                             ; preds = %loop1, %dom.block
  %iv1 = phi i64 [ 1, %dom.block ], [ %t3242, %loop1 ]
  %gep3 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* %gep2, i64 %iv1)
  store double 0.000000e+00, double* %gep3, align 1
  %t3242 = add nuw nsw i64 %iv1, 1
  %t3243 = icmp eq i64 %t3242, %t3
  br i1 %t3243, label %exit1, label %loop1

exit1:
  %t3235 = icmp slt i32 %t1, 1
  br i1 %t3235, label %exit3, label %loop2.pre

loop2.pre:
  br label %loop2

loop2:                                             ; preds = %loop2.pre, %loop2
  %iv2 = phi i64 [ 1, %loop2.pre ], [ %t3257, %loop2 ]
  %t3253 = trunc i64 %iv2 to i32
  %t3255 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* %t2, i64 %iv2)
  %t3256 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* %t3255, i64 1)
  call fastcc void @prtri_(double* %t3256) #4
  %t3257 = add nuw i64 %iv2, 1
  %t3258 = trunc i64 %t3257 to i32
  %t3259 = icmp slt i32 %t1, %t3258
  br i1 %t3259, label %exit2, label %loop2

exit2:                                   ; preds = %loop2
  br label %exit3

exit3:
  ret void
}

declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

declare hidden fastcc void @prtri_(double* noalias) unnamed_addr #4

attributes #0 = { nounwind readnone speculatable }
attributes #4 = { nofree nounwind uwtable willreturn }

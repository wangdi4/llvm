; RUN: opt < %s -hir-ssa-deconstruction -analyze -hir-framework | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output 2>&1 | FileCheck %s

; Check that %p will not be merged with the rest of the chain.

; BEGIN REGION { }
;       + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK:|   %p = &((@A)[0][0]);
; CHECK:|   (%sp)[0] = &((%p)[1][0]);
;       + END LOOP
; END REGION

@A = global [256 x double] zeroinitializer

define void @foo(double** %sp) {
entry:
  br label %loop

loop:
  %i = phi i32 [0, %entry], [%ip, %loop]

  %p = getelementptr inbounds [256 x double], [256 x double]* @A, i64 0, i64 0
  %p1 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 0, i64 512, double* nonnull %p, i64 1)
  %p2 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 0, i64 128, double* nonnull %p1, i64 0)
  store double* %p2, double** %sp

  %ip = add nsw nuw i32 %i, 1
  %cmp = icmp ult i32 %ip, 2
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}

declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64)


; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details-dims -disable-output 2>&1 | FileCheck %s

; Check that (%p) with IV LB and (%q) with IV stride are parsed with LB/Stride as a blob.

; BEGIN REGION { }
;       + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK:|   (%p)[%i:i1:8(double:0)] = 1.000000e+00;
; CHECK:|   (%q)[0:i1:%i(double:0)] = 1.000000e+00;
; CHECK:|   %i = i1 + 1;
;       + END LOOP
; END REGION

define void @foo(ptr %p, ptr %q) {
entry:
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%ip, %loop]

  %p4 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i, i64 8, ptr elementtype(double) nonnull %p, i64 %i)
  store double 1.0, ptr %p4

  %q4 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 %i, ptr elementtype(double) nonnull %q, i64 %i)
  store double 1.0, ptr %q4

  %ip = add nsw nuw i64 %i, 1
  %cmp = icmp ult i64 %ip, 2
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)


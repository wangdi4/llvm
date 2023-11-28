;RUN: opt -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S %s | FileCheck %s

; Verify that CG successfully generates code for the load ref.

; Formed HIR-
; + DO i1 = 0, 1, 1   <DO_LOOP>
; |   %ld = (%ld.ptr)[0].2[0][i1];
; |   (%st.ptr)[0] = %ld;
; + END LOOP

; CHECK: region.0:
; Loop invariant GEP is generated in loop preheader which is also the region
; entry block.
; CHECK: [[GEP:%.*]] = getelementptr inbounds %struct, {{.*}} %ld.ptr, i64 0, i32 2, i64 0

; CHECK: loop.{{[0-9]+}}:
; CHECK-NEXT: [[IV:%.*]] = load i64, {{.*}} %i1.i64, align 4
; CHECK-NEXT: = getelementptr inbounds double, ptr [[GEP]], i64 [[IV]]


%struct = type { i32, i32, [4 x double] }

define void @foo(ptr %ld.ptr, ptr %st.ptr) {
entry:
  %gep = getelementptr inbounds %struct, ptr %ld.ptr, i64 0, i32 2, i64 0
  %base.ld.ptr = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 32, ptr nonnull elementtype(double) %gep, i64 1)
  br label %loop

loop:                                              ; preds = %loop, %entry
  %iv = phi i64 [ 1, %entry ], [ %iv.inc, %loop ]
  %subs = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %base.ld.ptr, i64 %iv)
  %ld = load double, ptr %subs
  store double %ld, ptr %st.ptr
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 3
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #1 = { nounwind readnone speculatable }

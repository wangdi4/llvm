;RUN: opt -hir-ssa-deconstruction -hir-cg -force-hir-cg -S %s | FileCheck %s
;RUN: opt -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S %s | FileCheck %s
;RUN: opt -opaque-pointers -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S %s | FileCheck %s

; Verify that CG successfully generates code for the load ref.

; Formed HIR-
; + DO i1 = 0, 1, 1   <DO_LOOP>
; |   %ld = (%ld.ptr)[0].2[0][i1];
; |   (%st.ptr)[0] = %ld;
; + END LOOP
 
; CHECK: region.0:
; CHECK: loop.{{[0-9]+}}:
; CHECK: = getelementptr inbounds %struct, {{.*}} %ld.ptr, i64 0, i32 2, i64 0
; CHECK-NEXT: = load i64, {{.*}} %i1.i64, align 4
; CHECK-NEXT: = getelementptr inbounds double, {{.*}} %0, i64 %1


%struct = type { i32, i32, [4 x double] }

define void @foo(%struct* %ld.ptr, double* %st.ptr) {
entry:
  %gep = getelementptr inbounds %struct, %struct* %ld.ptr, i64 0, i32 2, i64 0
  %base.ld.ptr = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 32, double* nonnull elementtype(double) %gep, i64 1)
  br label %loop

loop:                                              ; preds = %loop, %entry
  %iv = phi i64 [ 1, %entry ], [ %iv.inc, %loop ]
  %subs = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %base.ld.ptr, i64 %iv) 
  %ld = load double, double* %subs
  store double %ld, double* %st.ptr
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 3
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}

declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #1 = { nounwind readnone speculatable }

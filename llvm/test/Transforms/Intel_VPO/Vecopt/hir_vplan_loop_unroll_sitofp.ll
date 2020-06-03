; RUN: opt -S < %s -hir-ssa-deconstruction -hir-temp-cleanup -VPlanDriverHIR -vplan-force-vf=4 -vplan-force-uf=3 -print-after=VPlanDriverHIR -enable-vp-value-codegen-hir=0 -disable-output 2>&1 | FileCheck %s
; RUN: opt -S < %s -hir-ssa-deconstruction -hir-temp-cleanup -VPlanDriverHIR -vplan-force-vf=4 -vplan-force-uf=3 -print-after=VPlanDriverHIR -enable-vp-value-codegen-hir -disable-output 2>&1 | FileCheck %s --check-prefix=VPCHECK

define dso_local void @foo(i64 %n) local_unnamed_addr {
; CHECK:      [[CONV_VEC0:%.*]] = sitofp.<4 x i32>.<4 x float>(i1 + <i64 0, i64 1, i64 2, i64 3>)
; CHECK-NEXT: [[CONV_VEC0]] = sitofp.<4 x i32>.<4 x float>(i1 + <i64 4, i64 5, i64 6, i64 7>)
; CHECK-NEXT: [[CONV_VEC0]] = sitofp.<4 x i32>.<4 x float>(i1 + <i64 8, i64 9, i64 10, i64 11>)
; VPCHECK:      [[VEC:%.*]] = sitofp.<4 x i32>.<4 x float>(i1 + <i64 0, i64 1, i64 2, i64 3>);
; VPCHECK-NEXT: [[VEC2:%.*]] = sitofp.<4 x i32>.<4 x float>(i1 + <i64 0, i64 1, i64 2, i64 3> + 4);
; VPCHECK-NEXT: [[VEC3:%.*]] = sitofp.<4 x i32>.<4 x float>(i1 + <i64 0, i64 1, i64 2, i64 3> + 8);
;
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %body

body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %body ]
  %1 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %1 to float
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %exit, label %body

exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind


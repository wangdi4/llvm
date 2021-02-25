; HIR vector code generation currently does not support in memory entities. Test
; checks that we bail out during vectorization for such cases.
;
; RUN: opt -hir-framework -VPlanDriverHIR -print-after=VPlanDriverHIR -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="vplan-driver-hir" -print-after=vplan-driver-hir -disable-output < %s 2>&1 | FileCheck %s

; CHECK-LABEL: IR Dump After{{.+}}VPlan{{.*}}Driver{{.*}}HIR{{.*}}
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP> <simd> <vectorize>
; CHECK-NEXT:    |   %val = (%arr)[i1];
; CHECK-NEXT:    |   %retval = (%ret)[0];
; CHECK-NEXT:    |   (%ret)[0] = %val + %retval;
; CHECK-NEXT:    + END LOOP
;

define dso_local i64 @foo(i64* %arr) {
entry:
  %ret = alloca i64, align 8
  store i64 0, i64* %ret, align 8
  br label %preheader

preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.REDUCTION.ADD"(i64* %ret) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.04 = phi i64 [ 0, %preheader ], [ %inc, %for.body ]
  %ptridx = getelementptr inbounds i64, i64* %arr, i64 %i.04
  %val = load i64, i64* %ptridx, align 8
  %retval = load i64, i64* %ret, align 8
  %add = add nsw i64 %retval, %val
  store i64 %add, i64* %ret, align 8
  %inc = add nuw nsw i64 %i.04, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  %0 = load i64, i64* %ret, align 8
  ret i64 %0
}

declare dso_local i64 @baz(i64*)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
;
; Code generation of Induction Final instruction had an issue where we were trying to
; use an already attached RegDDRef to create new HLInsts. The issue is fixed by cloning
; the needed RegDDRefs. Check for successful vectorization.
;
; CHECK: DO i1 = 0, 1023, 4   <DO_LOOP>
;
define void @foo() {
entry:
  %i1.linear = alloca i32, align 4
  store i32 0, ptr %i1.linear, align 4
  br label %preheader

preheader:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:TYPED"(ptr %i1.linear, i32 0, i32 1, i32 1) ]
  br label %for.body

for.body:                               ; preds = %preheader, %for.body
  %.omp.iv.local.011 = phi i64 [ 0, %preheader ], [ %add2, %for.body ]
  %1 = load i32, ptr %i1.linear, align 4
  %add1 = add nsw i32 %1, 1
  store i32 %add1, ptr %i1.linear, align 4
  %add2 = add nuw nsw i64 %.omp.iv.local.011, 1
  %exitcond.not = icmp eq i64 %add2, 1024
  br i1 %exitcond.not, label %ret, label %for.body


ret:                               ; preds = %for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

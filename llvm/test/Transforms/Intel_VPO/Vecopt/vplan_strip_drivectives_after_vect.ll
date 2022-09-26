; Verify that directices are removed after VPlan vectorizer.

; We don't have auto-vectorization for the LLVM-IR path yet, so we need an
; explicit SIMD region which in turn requires hir-cg/VPODirectiveCleanup for the
; HIR-path to re-use the exact same checks.

; TODO: We can remove the VPODirectiveCleanup once we add support for operand bundle
; representation in HIR.

; RUN: opt -enable-new-pm=0 -vplan-vec -vplan-force-vf=8 -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes='vplan-vec' -vplan-force-vf=8 -S < %s 2>&1 | FileCheck %s
; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg  -vplan-force-vf=8 -VPODirectiveCleanup -S < %s 2>&1 | FileCheck %s
; RUN: opt -vplan-force-vf=8 -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg,vpo-directive-cleanup' -S < %s 2>&1 | FileCheck %s

@arr.i32.1 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.2 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

define void @doit() local_unnamed_addr #0 {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  %ld.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.1, i64 0, i64 %indvars.iv
  %ld = load i32, i32* %ld.idx
  %st.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.2, i64 0, i64 %indvars.iv
  store i32 %ld, i32* %st.idx

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

attributes #1 = { nounwind }

; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
; CHECK-NOT: call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]

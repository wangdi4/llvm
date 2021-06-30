; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-print-after-plain-cfg -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-print-after-plain-cfg -disable-output < %s 2>&1 | FileCheck %s

; Verify that decomposer is able to handle metadata.
; IR generated from IR in hir_vplan_decomp_term_refs.ll.

; CHECK: i32 %vp{{[0-9]+}} = load i32* %vp{{[0-9]+}}
; CHECK-NEXT: call metadata !"MD.TEST"
; CHECK-NEXT: i32* %vp{{[0-9]+}} = subscript
; CHECK-NEXT: store i32 %vp{{[0-9]+}} i32* %vp{{[0-9]+}}

target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@c = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@b = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16

define void @foo(i32 %factor) local_unnamed_addr {
entry:
  %mul3 = mul i32 %factor, 15
  %t4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds [1600 x i32], [1600 x i32]* @a, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx
  call void @llvm.intel.directive(metadata !"MD.TEST")
  %arrayidx8 = getelementptr inbounds [1600 x i32], [1600 x i32]* @c, i64 0, i64 %indvars.iv
  store i32 %0, i32* %arrayidx8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1600
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:
  call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare void @llvm.intel.directive(metadata)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

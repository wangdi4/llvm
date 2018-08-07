; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s

; Verify that decomposer is able to handle metadata.
; IR generated from IR in hir_vplan_decomp_term_refs.ll.

; CHECK: %vp{{[0-9]+}} = load %vp{{[0-9]+}}
; CHECK-NEXT: %vp{{[0-9]+}} = call %vp{{[0-9]+}}
; CHECK-NEXT: store %vp{{[0-9]+}} %vp{{[0-9]+}}

target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@c = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@b = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16

define void @foo(i32 %factor) local_unnamed_addr {
entry:
  %mul3 = mul i32 %factor, 15
  tail call void @llvm.intel.directive(metadata !40)
  call void @llvm.intel.directive.qual.opnd.i32(metadata !42, i32 8)
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
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

declare void @llvm.intel.directive(metadata) #1
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32)

!40 = !{!"DIR.OMP.SIMD"}
!42 = !{!"QUAL.OMP.SIMDLEN"}

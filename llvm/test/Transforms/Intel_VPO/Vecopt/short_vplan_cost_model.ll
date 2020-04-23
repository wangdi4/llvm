; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -S -VPlanDriverHIR -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     | FileCheck %s --check-prefix=CHECK-HIR
; TODO: Move -hir-vec-dir-insert under new PM
; RUN: opt < %s -S -passes="hir-ssa-deconstruction,hir-vec-dir-insert,vplan-driver-hir" -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     | FileCheck %s --check-prefix=CHECK-HIR

; RUN: opt < %s -S -VPlanDriver -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: opt < %s -S -passes="vplan-driver" -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     | FileCheck %s --check-prefix=CHECK-LLVM

; Test that VPlan Cost Model drives the VF selection.

@arr.i32.1 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.2 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.3 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.4 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

@arr.float.1 = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@arr.float.2 = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@arr.float.3 = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16

define void @test_do_not_vectorize() local_unnamed_addr #0 {
; CHECK-LLVM-LABEL: test_do_not_vectorize
; CHECK-LLVM-NOT: load <{{.*}} x i32>
; CHECK-LLVM-NOT: store <{{.*}} x i32>
; CHECK-HIR-LABEL: test_do_not_vectorize
; CHECK-HIR-NOT: load <{{.*}} x i32>
; CHECK-HIR-NOT: store <{{.*}} x i32>
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  %ld.i32.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.1, i64 0, i64 %indvars.iv
  %ld.i32 = load i32, i32* %ld.i32.idx

  %ld.i32.idx.2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.2, i64 0, i64 %indvars.iv
  %ld.i32.2 = load i32, i32* %ld.i32.idx.2

  %ld.i32.idx.3 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.3, i64 0, i64 %indvars.iv
  %ld.i32.3 = load i32, i32* %ld.i32.idx.3

  %div.1 = sdiv i32 %ld.i32, %ld.i32.2
  %div.2 = sdiv i32 %div.1, %ld.i32.3


  %st.i32.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.4, i64 0, i64 %indvars.iv
  store i32 %div.2, i32* %st.i32.idx

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }


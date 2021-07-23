; RUN: opt < %s -S -vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: opt < %s -S -passes="vplan-vec" -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN: | FileCheck %s --check-prefix=CHECK-LLVM

; RUN: opt < %s -S -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     | FileCheck %s --check-prefix=CHECK-HIR
; RUN: opt < %s -S -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" \
; RUN: -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN: | FileCheck %s --check-prefix=CHECK-HIR


; Test that VPlan Cost Model drives the VF selection.

@arr.i32.1 = external local_unnamed_addr global [1024 x i32], align 16
@arr.i32.2 = external local_unnamed_addr global [1024 x i32], align 16
@arr.i32.3 = external local_unnamed_addr global [1024 x i32], align 16
@arr.i32.4 = external local_unnamed_addr global [1024 x i32], align 16

@arr.float.1 = external local_unnamed_addr global [1024 x float], align 16
@arr.float.2 = external local_unnamed_addr global [1024 x float], align 16
@arr.float.3 = external local_unnamed_addr global [1024 x float], align 16

define void @test_vectorize() local_unnamed_addr #0 {
; CHECK-LLVM-LABEL: test_vectorize
; CHECK-LLVM: fadd fast <8 x float>
; CHECK-HIR-LABEL: test_vectorize
; CHECK-HIR: fadd fast <8 x float>
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  %float.ld.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.1, i64 0, i64 %indvars.iv
  %float.ld = load float, float* %float.ld.idx
  %float2.ld.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.2, i64 0, i64 %indvars.iv
  %float2.ld = load float, float* %float2.ld.idx

  %float.add = fadd fast float %float.ld, %float2.ld
  %float.add1 = fadd fast float %float.ld, %float.add
  %float.add2 = fadd fast float %float.ld, %float.add1
  %float.st.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.3, i64 0, i64 %indvars.iv
  store float %float.add2, float* %float.st.idx

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

define void @test_do_not_vectorize() local_unnamed_addr #0 {
; CHECK-LLVM-LABEL: test_do_not_vectorize
; CHECK-HIR-LABEL: test_do_not_vectorize
; CHECK-HIR-NOT: load <{{.*}} x i32>
; CHECK-HIR-NOT: store <{{.*}} x i32>
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %idx = mul i64 %indvars.iv, %indvars.iv

  %ld.i32.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.1, i64 0, i64 %idx
  %ld.i32 = load i32, i32* %ld.i32.idx

  %ld.i32.idx.2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.2, i64 0, i64 %idx
  %ld.i32.2 = load i32, i32* %ld.i32.idx.2

  %ld.i32.idx.3 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.3, i64 0, i64 %idx
  %ld.i32.3 = load i32, i32* %ld.i32.idx.3

  %div.1 = sdiv i32 %ld.i32, %ld.i32.2
  %div.2 = sdiv i32 %div.1, %ld.i32.3


  %st.i32.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.4, i64 0, i64 %idx
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

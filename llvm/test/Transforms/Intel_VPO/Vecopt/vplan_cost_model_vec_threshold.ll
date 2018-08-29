; REQUIRES: asserts
; RUN: opt < %s -S -VPlanDriver  -mtriple=x86_64-unknown-unknown -mattr=+avx2 -debug \
; RUN:     -vec-threshold=50 \
; RUN:     2>&1 | FileCheck %s

; REQUIRES: asserts
; RUN: opt < %s -S -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vec-threshold=50 -debug \
; RUN:     2>&1 | FileCheck %s

; REQUIRES: asserts
; RUN: opt < %s -S -VPlanDriver  -mtriple=x86_64-unknown-unknown -mattr=+avx2 -debug \
; RUN:     -vec-threshold=0 \
; RUN:     2>&1 | FileCheck %s --check-prefix=VEC-ALWAYS

; REQUIRES: asserts
; RUN: opt < %s -S -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vec-threshold=0 -debug \
; RUN:     2>&1 | FileCheck %s --check-prefix=VEC-ALWAYS


; Test that VPlan Cost Model drives the VF selection.

@arr.i32.1 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.2 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.3 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.4 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

@arr.float.1 = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@arr.float.2 = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@arr.float.3 = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16

define void @test_vectorize() local_unnamed_addr #0 {
; CHECK: Applying threshold
; VEC-ALWAYS: vector always is used for the given loop
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

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

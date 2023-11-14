; Test support for llvm.smax, llvm.umax, llvm.smin, llvm.umin instrinsics
; in reduction handling.

; RUN: opt -S -passes=vplan-vec -vplan-force-vf=4 < %s 2>&1 | FileCheck %s

; RUN: opt -S -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -disable-output -vplan-force-vf=4 -hir-details-no-verbose-indent < %s 2>&1 | FileCheck %s -check-prefix=HIR

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; CHECK-LABEL: @foo_smax
; CHECK: vector.body:
; CHECK: %{{.*}} = call <4 x i32> @llvm.smax.v4i32
; CHECK: VPlannedBB{{.*}}:
; CHECK: %{{.*}} = call i32 @llvm.vector.reduce.smax.v4i32

; HIR: Function: foo_smax
; HIR: + DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR: |   {{.*}} = @llvm.smax.v4i32
; HIR: + END LOOP
; HIR: {{.*}} = @llvm.vector.reduce.smax.v4i32

define dso_local i32 @foo_smax() local_unnamed_addr {
entry:
  %max.red = alloca i32, align 4
  store i32 0, ptr %max.red, align 4
  br label %for.entry

for.entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MAX:TYPED"(ptr %max.red, i32 0, i32 1) ]
  br label %for.ph

for.ph:
  %max.red.promoted = load i32, ptr %max.red, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %for.ph ], [ %indvars.iv.next, %for.body ]
  %max.011 = phi i32 [ %max.red.promoted, %for.ph ], [ %1, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], ptr @a, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %1 = tail call i32 @llvm.smax.i32(i32 %0, i32 %max.011)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %cmp.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:
  %.lcssa = phi i32 [ %1, %for.body ]
  store i32 %.lcssa, ptr %max.red, align 4
  br label %endsimd

endsimd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %theend

theend:
  ret i32 %.lcssa
}

; CHECK-LABEL: @foo_smin
; CHECK: vector.body:
; CHECK: %{{.*}} = call <4 x i32> @llvm.smin.v4i32
; CHECK: VPlannedBB{{.*}}:
; CHECK: %{{.*}} = call i32 @llvm.vector.reduce.smin.v4i32

; HIR: Function: foo_smin
; HIR: + DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR: |   {{.*}} = @llvm.smin.v4i32
; HIR: + END LOOP
; HIR: {{.*}} = @llvm.vector.reduce.smin.v4i32

define dso_local i32 @foo_smin() local_unnamed_addr {
entry:
  %min.red = alloca i32, align 4
  store i32 100000, ptr %min.red, align 4
  br label %for.entry

for.entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MIN:TYPED"(ptr %min.red, i32 0, i32 1) ]
  br label %for.ph

for.ph:
  %min.red.promoted = load i32, ptr %min.red, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %for.ph ], [ %indvars.iv.next, %for.body ]
  %min.011 = phi i32 [ %min.red.promoted, %for.ph ], [ %1, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], ptr @a, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %1 = tail call i32 @llvm.smin.i32(i32 %0, i32 %min.011)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %cmp.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:
  %.lcssa = phi i32 [ %1, %for.body ]
  store i32 %.lcssa, ptr %min.red, align 4
  br label %endsimd

endsimd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %theend

theend:
  ret i32 %.lcssa
}

; CHECK-LABEL: @foo_umax
; CHECK: vector.body:
; CHECK: %{{.*}} = call <4 x i32> @llvm.umax.v4i32
; CHECK: VPlannedBB{{.*}}:
; CHECK: %{{.*}} = call i32 @llvm.vector.reduce.umax.v4i32

; HIR: Function: foo_umax
; HIR: + DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR: |   {{.*}} = @llvm.umax.v4i32
; HIR: + END LOOP
; HIR: {{.*}} = @llvm.vector.reduce.umax.v4i32

define dso_local i32 @foo_umax() local_unnamed_addr {
entry:
  %max.red = alloca i32, align 4
  store i32 0, ptr %max.red, align 4
  br label %for.entry

for.entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MAX:UNSIGNED.TYPED"(ptr %max.red, i32 0, i32 1) ]
  br label %for.ph

for.ph:
  %max.red.promoted = load i32, ptr %max.red, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %for.ph ], [ %indvars.iv.next, %for.body ]
  %max.011 = phi i32 [ %max.red.promoted, %for.ph ], [ %1, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], ptr @a, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %1 = tail call i32 @llvm.umax.i32(i32 %0, i32 %max.011)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %cmp.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:
  %.lcssa = phi i32 [ %1, %for.body ]
  store i32 %.lcssa, ptr %max.red, align 4
  br label %endsimd

endsimd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %theend

theend:
  ret i32 %.lcssa
}

; CHECK-LABEL: @foo_umin
; CHECK: vector.body:
; CHECK: %{{.*}} = call <4 x i32> @llvm.umin.v4i32
; CHECK: VPlannedBB{{.*}}:
; CHECK: %{{.*}} = call i32 @llvm.vector.reduce.umin.v4i32

; HIR: Function: foo_umin
; HIR: + DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR: |   {{.*}} = @llvm.umin.v4i32
; HIR: + END LOOP
; HIR: {{.*}} = @llvm.vector.reduce.umin.v4i32

define dso_local i32 @foo_umin() local_unnamed_addr {
entry:
  %min.red = alloca i32, align 4
  store i32 100000, ptr %min.red, align 4
  br label %for.entry

for.entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MIN:UNSIGNED.TYPED"(ptr %min.red, i32 0, i32 1) ]
  br label %for.ph

for.ph:
  %min.red.promoted = load i32, ptr %min.red, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %for.ph ], [ %indvars.iv.next, %for.body ]
  %min.011 = phi i32 [ %min.red.promoted, %for.ph ], [ %1, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], ptr @a, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %1 = tail call i32 @llvm.umin.i32(i32 %0, i32 %min.011)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %cmp.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:
  %.lcssa = phi i32 [ %1, %for.body ]
  store i32 %.lcssa, ptr %min.red, align 4
  br label %endsimd

endsimd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %theend

theend:
  ret i32 %.lcssa
}

declare i32 @llvm.smax.i32(i32, i32)
declare i32 @llvm.smin.i32(i32, i32)
declare i32 @llvm.umax.i32(i32, i32)
declare i32 @llvm.umin.i32(i32, i32)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

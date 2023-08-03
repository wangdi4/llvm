; This test checks that the cost model correctly computes cost zero for calls
; to '@llvm.assume', and for instructions which only feed into an assumption
; operand. These instructions are dead: they are not executed and are removed
; just before LLVM-IR is translated into MIR, hence they should have zero cost.

; RUN: opt < %s -passes='vplan-vec' -disable-output \
; RUN:     -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 \
; RUN:     | FileCheck %s --check-prefixes=CHECK,IR
; RUN: opt < %s -passes='hir-ssa-deconstruction,hir-vplan-vec' -disable-output \
; RUN:     -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 \
; RUN:     | FileCheck %s --check-prefixes=CHECK,HIR

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = external dso_local local_unnamed_addr global [256 x i64], align 16

define void @single_root_fully_contained(i64 %N) local_unnamed_addr {
; CHECK-LABEL: Cost Model for VPlan single_root_fully_contained:{{.*}} with VF = 4:
; HIR before VPlan:
; <20>         + DO i1 = 0, 255, 1   <DO_LOOP> <simd>
; <7>          |   %offset.in.bounds = i1 + %N <u 256;
; <8>          |   @llvm.assume(%offset.in.bounds);
; <10>         |   (@A)[0][i1] = i1;
; <20>         + END LOOP

; HIR: Cost 0 for i64 [[ADD1:%vp.*]] = add i64 %N i64 {{%vp.*}}
; HIR: Cost 0 for i1 [[CMP1:%vp.*]] = icmp ult i64 [[ADD1]] i64 256
; HIR: Cost 0 for call i1 [[CMP1]] ptr @llvm.assume
for.preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.preheader ], [ %iv.next, %for.body ]

  ; IR: Cost 0 for i64 [[ADD:%vp.*]] = add i64 %N i64 {{%vp.*}}
  %offset = add i64 %N, %iv
  ; IR: Cost 0 for i1 [[CMP1:%vp.*]] = icmp ult i64 [[ADD]] i64 256
  %offset.in.bounds = icmp ult i64 %offset, 256
  ; IR: Cost 0 for call i1 [[CMP1]] ptr @llvm.assume [Serial]
  call void @llvm.assume(i1 %offset.in.bounds)

  %A.elem = getelementptr [256 x i64], ptr @A, i64 0, i64 %iv
  store i64 %iv, ptr %A.elem, align 8

  %iv.next = add nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 256
  br i1 %exitcond, label %for.exit, label %for.body

for.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define void @single_root_not_fully_contained(i64 %N) local_unnamed_addr {
; CHECK-LABEL: Cost Model for VPlan single_root_not_fully_contained:{{.*}} with VF = 4:
; HIR before VPlan:
; <22>         + DO i1 = 0, 255, 1   <DO_LOOP> <simd>
; <8>          |   %offset.in.bounds = i1 + %N <u 256;
; <9>          |   @llvm.assume(%offset.in.bounds);
; <12>         |   (@A)[0][i1 + %N] = i1;
; <22>         + END LOOP

; HIR: Cost 2 for i64 [[ADD1:%vp.*]] = add i64 %N i64 {{%vp.*}}
; HIR: Cost 0 for i1 [[CMP1:%vp.*]] = icmp ult i64 [[ADD1]] i64 256
; HIR: Cost 0 for call i1 [[CMP1]] ptr @llvm.assume
for.preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.preheader ], [ %iv.next, %for.body ]

  ; IR: Cost 2 for i64 [[ADD:%vp.*]] = add i64 %N i64 {{%vp.*}}
  %offset = add i64 %N, %iv
  ; IR: Cost 0 for i1 [[CMP1:%vp.*]] = icmp ult i64 [[ADD]] i64 256
  %offset.in.bounds = icmp ult i64 %offset, 256
  ; IR: Cost 0 for call i1 [[CMP1]] ptr @llvm.assume [Serial]
  call void @llvm.assume(i1 %offset.in.bounds)

  %A.elem = getelementptr [256 x i64], ptr @A, i64 0, i64 %offset
  store i64 %iv, ptr %A.elem, align 8

  %iv.next = add nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 256
  br i1 %exitcond, label %for.exit, label %for.body

for.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define void @multiple_roots_fully_contained(i64 %N, i64 %N2) local_unnamed_addr {
; CHECK-LABEL: Cost Model for VPlan multiple_roots_fully_contained:{{.*}} with VF = 4:
; HIR before VPlan:
; <23>         + DO i1 = 0, 255, 1   <DO_LOOP> <simd>
; <7>          |   %offset.in.bounds = i1 + %N <u 256;
; <8>          |   @llvm.assume(%offset.in.bounds);
; <10>         |   %offset.mul.in.bounds = %N2 * i1 + (%N * %N2) < 2048;
; <11>         |   @llvm.assume(%offset.mul.in.bounds);
; <13>         |   (@A)[0][i1] = i1;
; <23>         + END LOOP

; HIR: Cost 0 for i64 [[ADD1:%vp.*]] = add i64 %N i64 {{%vp.*}}
; HIR: Cost 0 for i1 [[CMP1:%vp.*]] = icmp ult i64 [[ADD1]] i64 256
; HIR: Cost 0 for call i1 [[CMP1]] ptr @llvm.assume
; HIR: Cost 0 for i64 [[MUL:%vp.*]] = mul i64 %N2 i64 {{%vp.*}}
; HIR: Cost 0 for i64 [[ADD2:%vp.*]] = add i64 {{%vp.*}} i64 [[MUL]]
; HIR: Cost 0 for i1 [[CMP2:%vp.*]] = icmp slt i64 [[ADD2]] i64 2048
; HIR: Cost 0 for call i1 [[CMP2]] ptr @llvm.assume
for.preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.preheader ], [ %iv.next, %for.body ]

  ; IR: Cost 0 for i64 [[ADD:%vp.*]] = add i64 %N i64 {{%vp.*}}
  %offset = add i64 %N, %iv
  ; IR: Cost 0 for i1 [[CMP1:%vp.*]] = icmp ult i64 [[ADD]] i64 256
  %offset.in.bounds = icmp ult i64 %offset, 256
  ; IR: Cost 0 for call i1 [[CMP1]] ptr @llvm.assume [Serial]
  call void @llvm.assume(i1 %offset.in.bounds)

  ; IR: Cost 0 for i64 [[MUL:%vp.*]] = mul i64 %N2 i64 [[ADD]]
  %offset.mul = mul i64 %N2, %offset
  ; IR: Cost 0 for i1 [[CMP2:%vp.*]] = icmp slt i64 [[MUL]] i64 2048
  %offset.mul.in.bounds = icmp slt i64 %offset.mul, 2048
  ; IR: Cost 0 for call i1 [[CMP2]] ptr @llvm.assume [Serial]
  call void @llvm.assume(i1 %offset.mul.in.bounds)

  %A.elem = getelementptr [256 x i64], ptr @A, i64 0, i64 %iv
  store i64 %iv, ptr %A.elem, align 8

  %iv.next = add nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 256
  br i1 %exitcond, label %for.exit, label %for.body

for.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define void @multiple_roots_not_fully_contained(i64 %N, i64 %N2) local_unnamed_addr {
; CHECK-LABEL: Cost Model for VPlan multiple_roots_not_fully_contained:{{.*}} with VF = 4:
; HIR before VPlan:
; <23>         + DO i1 = 0, 255, 1   <DO_LOOP> <simd>
; <7>          |   %offset.in.bounds = i1 + %N <u 256;
; <8>          |   @llvm.assume(%offset.in.bounds);
; <10>         |   %offset.mul.in.bounds = %N2 * i1 + (%N * %N2) < 2048;
; <11>         |   @llvm.assume(%offset.mul.in.bounds);
; <13>         |   (@A)[0][i1 + %N] = i1;
; <23>         + END LOOP

; HIR: Cost 0 for i64 [[ADD1:%vp.*]] = add i64 %N i64 {{%vp.*}}
; HIR: Cost 0 for i1 [[CMP1:%vp.*]] = icmp ult i64 [[ADD1]] i64 256
; HIR: Cost 0 for call i1 [[CMP1]] ptr @llvm.assume
; HIR: Cost 14 for i64 [[MUL:%vp.*]] = mul i64 %N2 i64 {{%vp.*}}
; HIR: Cost 2 for i64 [[ADD2:%vp.*]] = add i64 {{%vp.*}} i64 [[MUL]]
; HIR: Cost 0 for i1 [[CMP2:%vp.*]] = icmp slt i64 [[ADD2]] i64 2048
; HIR: Cost 0 for call i1 [[CMP2]] ptr @llvm.assume
for.preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.preheader ], [ %iv.next, %for.body ]

  ; IR: Cost 2 for i64 [[ADD:%vp.*]] = add i64 %N i64 {{%vp.*}}
  %offset = add i64 %N, %iv
  ; IR: Cost 0 for i1 [[CMP1:%vp.*]] = icmp ult i64 [[ADD]] i64 256
  %offset.in.bounds = icmp ult i64 %offset, 256
  ; IR: Cost 0 for call i1 [[CMP1]] ptr @llvm.assume [Serial]
  call void @llvm.assume(i1 %offset.in.bounds)

  ; IR: Cost 14 for i64 [[MUL:%vp.*]] = mul i64 %N2 i64 [[ADD]]
  %offset.mul = mul i64 %N2, %offset
  ; IR: Cost 0 for i1 [[CMP2:%vp.*]] = icmp slt i64 [[MUL]] i64 2048
  %offset.mul.in.bounds = icmp slt i64 %offset.mul, 2048
  ; IR: Cost 0 for call i1 [[CMP2]] ptr @llvm.assume [Serial]
  call void @llvm.assume(i1 %offset.mul.in.bounds)

  %A.elem = getelementptr [256 x i64], ptr @A, i64 0, i64 %offset.mul
  store i64 %iv, ptr %A.elem, align 8

  %iv.next = add nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 256
  br i1 %exitcond, label %for.exit, label %for.body

for.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.assume(i1 noundef) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() nounwind

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) nounwind

attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }

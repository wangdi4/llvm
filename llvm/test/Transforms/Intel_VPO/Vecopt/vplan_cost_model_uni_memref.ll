; RUN: opt < %s -disable-output -vplan-vec -vplan-cost-model-print-analysis-for-vf=4 -mattr=avx2 | FileCheck %s
; RUN: opt < %s -disable-output -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-cost-model-print-analysis-for-vf=4 -mattr=avx2 -enable-intel-advanced-opts | FileCheck %s

; The test cements the costs of uniform loads/stores.
; Uniform loads may have extra cost comparing to uniform stores if their
; results are broadcast.

target triple = "x86_64-unknown-linux-gnu"

@a = external local_unnamed_addr global [1024 x i32], align 16

define dso_local void @foo(i32* nocapture noalias readonly %ptr1,
; CHECK:    Cost 2 for i32 [[VP_LOAD:%.*]] = load i32* [[PTR10:%.*]]
; TODO: the second load is not broadcast but CM doesn't check SVA yet.
; The cost should be 1 for this load when CM is fixed.
; CHECK:    Cost 2 for i32 [[VP_LOAD_1:%.*]] = load i32* [[PTR20:%.*]]
; CHECK:    Cost 2 for store i32 [[VP_LOAD_1]] i32* [[PTR40:%.*]]
; CHECK:    Cost 2 for store i32 [[VP4:%.*]] i32* [[PTR30:%.*]]
;
  i32* nocapture noalias readonly %ptr2,
  i32* nocapture noalias writeonly %ptr3,
  i32* nocapture noalias writeonly %ptr4) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %else.fi ]
  %idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %iv
  %cnst.vec = load i32, i32* %ptr1, align 8
  %cnst.scal = load i32, i32* %ptr2, align 8
  %wvar = load i32, i32* %idx, align 16
  %cond = icmp ult i32 %cnst.vec, 0
  br i1 %cond, label %if.cond, label %else.cond

if.cond:
  store i32 %cnst.scal, i32* %ptr4, align 8
  br label %else.fi

else.cond:
  %res = add nsw nuw i32 %cnst.vec, %wvar
  store i32 %res, i32* %ptr3, align 8
  br label %else.fi

else.fi:
  %iv.next = add nuw nsw i64 %iv, 1
  %cmp = icmp ult i64 %iv, 1022
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

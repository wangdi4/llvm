;
; RUN: opt -disable-output -passes='vplan-vec,print' -vplan-print-after-transformed-soa-geps -vplan-force-uf=2 -vplan-dump-soa-info < %s 2>&1 | FileCheck %s
;
; LIT test to demonstrate issue with SOA transform when we force unrolling. While transforming
; SOA GEPs, the const-step vector is incorrectly being created with (VF * UF) number of steps.
; it should be created with just VF number of steps. This causes LLVM IR verification errors.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @baz() {
;
; CHECK-LABEL:  SOA profitability:
; CHECK-NEXT:  SOASafe = [[VP_ARR_PRIV:%.*]] (arr.priv) Profitable = 1
; CHECK-NEXT:  SOA profitability:
; CHECK-NEXT:  SOASafe = [[VP0:%.*]] Profitable = 1
; CHECK-NEXT:  VPlan after Dump Transformed SOA GEPs:
; CHECK-NEXT:  VPlan IR for: baz:for.body.#{{[0-9]+}}
;
; The number of steps in const-step-vector needs to be 4(VF) and not 8(VF*UF).
; The test uses simdlen(4) and vplan-forced-uf(2).
;
; CHECK:          [DA: Div] i32 [[VP_CONST_STEP:%.*]] = const-step-vector: { Start:0, Step:1, NumSteps:4}
; CHECK-NEXT:     [DA: Div] ptr [[VP1:%.*]] = getelementptr i64, ptr [[VP_ARRAYIDX2:%.*]] i32 0 i32 [[VP_CONST_STEP]]
; CHECK-NEXT:     [DA: Div] store i64 1111 ptr [[VP1]]
;
; CHECK:  define void @baz() {
; CHECK:       vector.body:
; CHECK:       VPlannedBB4:
;
; The indices all need to be vectors of size 4.
;
; CHECK:         [[SOA_VECTORGEP70:%.*]] = getelementptr <4 x i64>, <4 x ptr> [[SOA_VECTORGEP60:%.*]], <4 x i32> zeroinitializer, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i64.v4p0(<4 x i64> <i64 1111, i64 1111, i64 1111, i64 1111>, <4 x ptr> [[SOA_VECTORGEP70]], i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
;
entry:
  %arr.priv = alloca [4 x i64], align 16
  br label %for.ph

for.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr.priv, i64 0, i64 4) ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.ph ], [ %iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [4 x i64], ptr %arr.priv, i64 0, i64 3
  store i64 0, ptr %arrayidx, align 8
  %rem = urem i64 %iv, 4
  %arrayidx2 = getelementptr inbounds [4 x i64], ptr %arr.priv, i64 0, i64 %rem
  store i64 1111, ptr %arrayidx2, align 8
  %iv.next = add nuw nsw i64 %iv, 1
  %cmp = icmp eq i64 %iv.next, 16
  br i1 %cmp, label %exit, label %for.body

exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

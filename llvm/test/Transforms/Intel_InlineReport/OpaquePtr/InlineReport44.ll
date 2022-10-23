; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers < %s -passes='cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe807 -inline-threshold=10 -inline-for-array-struct-arg-min-uses=12 -pre-lto-inline-cost -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; RUN: opt -opaque-pointers < %s -passes='cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe807 -inline-threshold=10 -inline-for-array-struct-arg-min-uses=12 -inline-for-array-struct-arg-min-caller-args=2 -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-threshold=10 -inline-for-array-struct-arg-min-uses=12 -pre-lto-inline-cost -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-threshold=10 -inline-for-array-struct-arg-min-uses=12 -inline-for-array-struct-arg-min-caller-args=2 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD

; Negative checks for the "struct array args" heuristic.

; Old pass manager checks
; Both pass managers with metadata-based inline report

; CHECK-OLD: foo{{.*}}Inlining is not profitable
; CHECK-OLD: foo{{.*}}Inlining is not profitable
; CHECK-OLD: baz{{.*}}Inlining is not profitable
; CHECK-OLD: baz{{.*}}Inlining is not profitable
; CHECK-OLD: bam{{.*}}Inlining is not profitable
; CHECK-OLD: bam{{.*}}Inlining is not profitable
; CHECK-OLD: call i32 @foo
; CHECK-OLD: call i32 @foo
; CHECK-OLD: call i32 @baz
; CHECK-OLD: call i32 @baz
; CHECK-OLD: call i32 @bam
; CHECK-OLD: call i32 @bam

; New pass manager checks

; CHECK-NEW: call i32 @foo
; CHECK-NEW: call i32 @foo
; CHECK-NEW: call i32 @baz
; CHECK-NEW: call i32 @baz
; CHECK-NEW: call i32 @bam
; CHECK-NEW: call i32 @bam
; CHECK-NEW: foo{{.*}}Inlining is not profitable
; CHECK-NEW: foo{{.*}}Inlining is not profitable
; CHECK-NEW: baz{{.*}}Inlining is not profitable
; CHECK-NEW: baz{{.*}}Inlining is not profitable
; CHECK-NEW: bam{{.*}}Inlining is not profitable
; CHECK-NEW: bam{{.*}}Inlining is not profitable

%struct.MYBOX = type { [3 x i32], [3 x float] }

@box1 = common dso_local global %struct.MYBOX zeroinitializer, align 4
@box2 = common dso_local global %struct.MYBOX zeroinitializer, align 4
@hisarg3 = common dso_local global i32 0, align 4
@hisarg4 = common dso_local global float 0.000000e+00, align 4

; Check that "foo" is NOT inlined by the "array struct arg" heuristic because
; its array struct args do not have enough uses.

define internal fastcc i32 @foo(ptr nocapture readonly %arg1, ptr nocapture readonly %arg2, i32 %arg3, float %arg4) unnamed_addr #1 {
entry:
  %conv = sitofp i32 %arg3 to float
  %cmp = fcmp ogt float %conv, %arg4
  %arrayidx = getelementptr inbounds %struct.MYBOX, ptr %arg1, i64 0, i32 0, i64 0
  %0 = load i32, ptr %arrayidx, align 4
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %arrayidx3 = getelementptr inbounds %struct.MYBOX, ptr %arg1, i64 0, i32 0, i64 1
  %1 = load i32, ptr %arrayidx3, align 4
  %add = add nsw i32 %1, %0
  %arrayidx5 = getelementptr inbounds %struct.MYBOX, ptr %arg2, i64 0, i32 0, i64 0
  %2 = load i32, ptr %arrayidx5, align 4
  %add6 = add nsw i32 %add, %2
  %arrayidx8 = getelementptr inbounds %struct.MYBOX, ptr %arg2, i64 0, i32 0, i64 1
  %3 = load i32, ptr %arrayidx8, align 4
  %add9 = add nsw i32 %add6, %3
  br label %return

if.end:                                           ; preds = %entry
  %arrayidx13 = getelementptr inbounds %struct.MYBOX, ptr %arg2, i64 0, i32 0, i64 0
  %4 = load i32, ptr %arrayidx13, align 4
  %arrayidx19 = getelementptr inbounds %struct.MYBOX, ptr %arg1, i64 0, i32 0, i64 1
  %5 = load i32, ptr %arrayidx19, align 4
  %factor = shl i32 %0, 1
  %add17 = add i32 %4, %factor
  %add20 = add i32 %add17, %5
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ %add9, %if.then ], [ %add20, %if.end ]
  ret i32 %retval.0
}

; Check that "baz" is NOT inlined by the "array struct arg" heuristic because
; it does not have any array struct args.

define internal i32 @baz(i32 %arg3, float %arg4) #0 {
entry:
  %retval = alloca i32, align 4
  %arg3.addr = alloca i32, align 4
  %arg4.addr = alloca float, align 4
  store i32 %arg3, ptr %arg3.addr, align 4
  store float %arg4, ptr %arg4.addr, align 4
  %0 = load i32, ptr %arg3.addr, align 4
  %conv = sitofp i32 %0 to float
  %1 = load float, ptr %arg4.addr, align 4
  %cmp = fcmp olt float %conv, %1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = load i32, ptr %arg3.addr, align 4
  %conv2 = sitofp i32 %2 to float
  %3 = load float, ptr %arg4.addr, align 4
  %mul = fmul float %conv2, %3
  %4 = load float, ptr %arg4.addr, align 4
  %sub = fsub float %mul, %4
  %5 = load i32, ptr %arg3.addr, align 4
  %mul3 = mul nsw i32 %5, 20
  %conv4 = sitofp i32 %mul3 to float
  %add = fadd float %sub, %conv4
  %conv5 = fptosi float %add to i32
  store i32 %conv5, ptr %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  %6 = load i32, ptr %arg3.addr, align 4
  %conv6 = sitofp i32 %6 to float
  %7 = load float, ptr %arg4.addr, align 4
  %mul7 = fmul float %conv6, %7
  %8 = load float, ptr %arg4.addr, align 4
  %sub8 = fsub float %mul7, %8
  %9 = load i32, ptr %arg3.addr, align 4
  %mul9 = mul nsw i32 %9, 20
  %conv10 = sitofp i32 %mul9 to float
  %add11 = fadd float %sub8, %conv10
  %conv12 = fptosi float %add11 to i32
  store i32 %conv12, ptr %retval, align 4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %10 = load i32, ptr %retval, align 4
  ret i32 %10
}

; Check that "bam" is NOT inlined by the "array struct arg" heuristic because
; it does not have any args.

define internal fastcc i32 @bam() unnamed_addr #0 {
entry:
  %0 = load i32, ptr @hisarg3, align 4
  %conv = sitofp i32 %0 to float
  %1 = load float, ptr @hisarg4, align 4
  %mul = fmul float %1, %conv
  %cmp = fcmp ogt float %mul, %conv
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %sub = fsub float %mul, %1
  %mul5 = mul nsw i32 %0, 20
  %conv6 = sitofp i32 %mul5 to float
  %add = fadd float %sub, %conv6
  br label %return

if.end:                                           ; preds = %entry
  %mul8 = mul nsw i32 %0, 20
  %conv9 = sitofp i32 %mul8 to float
  %mul10 = fmul float %1, %conv9
  %sub11 = fsub float %mul10, %1
  %add13 = fadd float %sub11, %conv
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0.in = phi float [ %add, %if.then ], [ %add13, %if.end ]
  %retval.0 = fptosi float %retval.0.in to i32
  ret i32 %retval.0
}

define dso_local i32 @bar(i32 %myarg3, float %myarg4) #0 {
entry:
  %call = call i32 @foo(ptr @box1, ptr @box2, i32 %myarg3, float %myarg4)
  %call1 = call i32 @foo(ptr @box2, ptr @box1, i32 %myarg3, float %myarg4)
  %add = add nsw i32 %call, %call1
  %call2 = call i32 @baz(i32 %myarg3, float %myarg4)
  %add3 = add nsw i32 %add, %call2
  %mul = mul nsw i32 2, %myarg3
  %sub = fsub float 5.000000e+00, %myarg4
  %call4 = call i32 @baz(i32 %mul, float %sub)
  %add5 = add nsw i32 %add3, %call4
  %call6 = call i32 @bam()
  %sub7 = sub nsw i32 %add5, %call6
  %call8 = call i32 @bam()
  %mul9 = mul nsw i32 2, %call8
  %add10 = add nsw i32 %sub7, %mul9
  ret i32 %add10
}

; end INTEL_FEATURE_SW_ADVANCED

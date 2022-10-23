; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers < %s -passes='cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe807 -inline-threshold=10 -inline-for-array-struct-arg-min-uses=12 -pre-lto-inline-cost -S 2>&1 | FileCheck --check-prefix=CHECK-OLD %s
; RUN: opt -opaque-pointers < %s -passes='cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe807 -inline-threshold=10 -inline-for-array-struct-arg-min-uses=12 -inline-for-array-struct-arg-min-caller-args=2 -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-threshold=10 -inline-for-array-struct-arg-min-uses=12 -pre-lto-inline-cost -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-MD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-threshold=10 -inline-for-array-struct-arg-min-uses=12 -inline-for-array-struct-arg-min-caller-args=2 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-MD

; Check that the "array struct args" heuristic is applied for "foo", both in
; the compile and link step.

; Both pass managers with metadata based inline report

; CHECK-MD: COMPILE FUNC: bar
; CHECK-MD: INLINE: foo{{.*}}<<Callee has callsites with array struct args>>
; CHECK-MD: INLINE: foo{{.*}}<<Callee has single callsite and local linkage>>
; CHECK-MD: DEAD STATIC FUNC: foo
; CHECK-MD-NOT: call i32 @foo

; Old pass manager checks

; CHECK-OLD: DEAD STATIC FUNC: foo
; CHECK-OLD: COMPILE FUNC: bar
; CHECK-OLD: INLINE: foo{{.*}}<<Callee has callsites with array struct args>>
; CHECK-OLD: INLINE: foo{{.*}}<<Callee has single callsite and local linkage>>
; CHECK-OLD-NOT: call i32 @foo

; New pass manager checks

; CHECK-NEW-NOT: call i32 @foo
; CHECK-NEW: DEAD STATIC FUNC: foo
; CHECK-NEW: COMPILE FUNC: bar
; CHECK-NEW: INLINE: foo{{.*}}<<Callee has callsites with array struct args>>
; CHECK-NEW: INLINE: foo{{.*}}<<Callee has single callsite and local linkage>>

target triple = "x86_64-unknown-linux-gnu"

%struct.MYBOX = type { [3 x i32], [3 x float] }

@box1 = common dso_local global %struct.MYBOX zeroinitializer, align 4
@box2 = common dso_local global %struct.MYBOX zeroinitializer, align 4

define dso_local i32 @bar(i32 %myarg3, float %myarg4) {
entry:
  %call = call i32 @foo(ptr @box1, ptr @box2, i32 %myarg3, float %myarg4)
  %call1 = call i32 @foo(ptr @box2, ptr @box1, i32 %myarg3, float %myarg4)
  %add = add nsw i32 %call, %call1
  ret i32 %add
}

define internal i32 @foo(ptr %arg1, ptr %arg2, i32 %arg3, float %arg4) {
entry:
  %conv = sitofp i32 %arg3 to float
  %cmp = fcmp ogt float %conv, %arg4
  %x = getelementptr inbounds %struct.MYBOX, ptr %arg1, i32 0, i32 0
  %arrayidx = getelementptr inbounds [3 x i32], ptr %x, i64 0, i64 0
  %i = load i32, ptr %arrayidx, align 4
  %x2 = getelementptr inbounds %struct.MYBOX, ptr %arg1, i32 0, i32 0
  %arrayidx3 = getelementptr inbounds [3 x i32], ptr %x2, i64 0, i64 1
  %i1 = load i32, ptr %arrayidx3, align 4
  %add = add nsw i32 %i, %i1
  %x4 = getelementptr inbounds %struct.MYBOX, ptr %arg1, i32 0, i32 0
  %arrayidx5 = getelementptr inbounds [3 x i32], ptr %x4, i64 0, i64 2
  %i2 = load i32, ptr %arrayidx5, align 4
  %add6 = add nsw i32 %add, %i2
  %conv7 = sitofp i32 %add6 to float
  %y = getelementptr inbounds %struct.MYBOX, ptr %arg1, i32 0, i32 1
  %arrayidx8 = getelementptr inbounds [3 x float], ptr %y, i64 0, i64 0
  %i3 = load float, ptr %arrayidx8, align 4
  %add9 = fadd float %conv7, %i3
  %y10 = getelementptr inbounds %struct.MYBOX, ptr %arg1, i32 0, i32 1
  %arrayidx11 = getelementptr inbounds [3 x float], ptr %y10, i64 0, i64 1
  %i4 = load float, ptr %arrayidx11, align 4
  %add12 = fadd float %add9, %i4
  %y13 = getelementptr inbounds %struct.MYBOX, ptr %arg1, i32 0, i32 1
  %arrayidx14 = getelementptr inbounds [3 x float], ptr %y13, i64 0, i64 2
  %i5 = load float, ptr %arrayidx14, align 4
  %add15 = fadd float %add12, %i5
  %x16 = getelementptr inbounds %struct.MYBOX, ptr %arg2, i32 0, i32 0
  %arrayidx17 = getelementptr inbounds [3 x i32], ptr %x16, i64 0, i64 0
  %i6 = load i32, ptr %arrayidx17, align 4
  %conv18 = sitofp i32 %i6 to float
  %add19 = fadd float %add15, %conv18
  %x20 = getelementptr inbounds %struct.MYBOX, ptr %arg2, i32 0, i32 0
  %arrayidx21 = getelementptr inbounds [3 x i32], ptr %x20, i64 0, i64 1
  %i7 = load i32, ptr %arrayidx21, align 4
  %conv22 = sitofp i32 %i7 to float
  %add23 = fadd float %add19, %conv22
  %x24 = getelementptr inbounds %struct.MYBOX, ptr %arg2, i32 0, i32 0
  %arrayidx25 = getelementptr inbounds [3 x i32], ptr %x24, i64 0, i64 2
  %i8 = load i32, ptr %arrayidx25, align 4
  %conv26 = sitofp i32 %i8 to float
  %add27 = fadd float %add23, %conv26
  %y28 = getelementptr inbounds %struct.MYBOX, ptr %arg2, i32 0, i32 1
  %arrayidx29 = getelementptr inbounds [3 x float], ptr %y28, i64 0, i64 0
  %i9 = load float, ptr %arrayidx29, align 4
  %add30 = fadd float %add27, %i9
  %y31 = getelementptr inbounds %struct.MYBOX, ptr %arg2, i32 0, i32 1
  %arrayidx32 = getelementptr inbounds [3 x float], ptr %y31, i64 0, i64 1
  %i10 = load float, ptr %arrayidx32, align 4
  %add33 = fadd float %add30, %i10
  %y34 = getelementptr inbounds %struct.MYBOX, ptr %arg2, i32 0, i32 1
  %arrayidx35 = getelementptr inbounds [3 x float], ptr %y34, i64 0, i64 2
  %i11 = load float, ptr %arrayidx35, align 4
  %add36 = fadd float %add33, %i11
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %conv37 = sitofp i32 %arg3 to float
  %add38 = fadd float %add36, %conv37
  %add39 = fadd float %add38, %arg4
  %conv40 = fptosi float %add39 to i32
  br label %return

if.end:                                           ; preds = %entry
  %conv80 = fptosi float %add36 to i32
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ %conv40, %if.then ], [ %conv80, %if.end ]
  ret i32 %retval.0
}
; end INTEL_FEATURE_SW_ADVANCED

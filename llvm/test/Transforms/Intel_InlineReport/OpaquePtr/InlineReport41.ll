; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers < %s -passes='cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe807 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-NEW %s
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-OLD

; Check that foo is NOT inlined into bar, in accord with the "dummy
; args" heuristic, because it does not have enough callsites with a
; sufficiently long sub-series of matching arguments. (Include an alternating
; pattern of two dummy values in one callsite.)

; CHECK-OLD: COMPILE FUNC: foo
; CHECK-OLD: COMPILE FUNC: bar
; CHECK-OLD-NOT: INLINE: foo{{.*}}<<Callee has callsites with dummy args>>
; CHECK: define{{.*}}@foo
; CHECK: define{{.*}}@bar
; CHECK-NEW: COMPILE FUNC: foo
; CHECK-NEW: COMPILE FUNC: bar
; CHECK-NEW-NOT: INLINE: foo{{.*}}<<Callee has callsites with dummy args>>

define dso_local i32 @foo(i32 %arg1, i32 %arg2, ptr %arg3, ptr %arg4, ptr %arg5, ptr %arg6, ptr %arg7, i32 %arg8) #0 {
entry:
  %sub = sub nsw i32 %arg1, %arg2
  %idx.ext = sext i32 %sub to i64
  %add.ptr = getelementptr inbounds i32, ptr %arg3, i64 %idx.ext
  %cmp = icmp ugt ptr %add.ptr, inttoptr (i64 1 to ptr)
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i32 15, ptr %arg3, align 4
  store i32 20, ptr %arg4, align 4
  store i32 25, ptr %arg5, align 4
  store i32 30, ptr %arg6, align 4
  store i32 35, ptr %arg7, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %add = add nsw i32 %arg1, %arg2
  %add1 = add nsw i32 %add, %arg8
  ret i32 %add1
}

define dso_local i32 @bar() local_unnamed_addr #1 {
entry:
  %dummy = alloca i32, align 4
  %silly = alloca i32, align 4
  %temp = alloca i32, align 4
  %forreal = alloca i32, align 4
  %forfake = alloca i32, align 4
  store i32 0, ptr %dummy, align 4
  store i32 0, ptr %temp, align 4
  store i32 1, ptr %forreal, align 4
  store i32 2, ptr %forfake, align 4
  %call = call i32 @foo(i32 2, i32 3, ptr nonnull %forreal, ptr nonnull %dummy, ptr nonnull %dummy, ptr nonnull %dummy, ptr nonnull %dummy, i32 4)
  %call1 = call i32 @foo(i32 2, i32 4, ptr nonnull %forfake, ptr nonnull %dummy, ptr nonnull %dummy, ptr nonnull %dummy, ptr nonnull %dummy, i32 5)
  %add = add nsw i32 %call, %call1
  %call2 = call i32 @foo(i32 2, i32 4, ptr nonnull %forreal, ptr nonnull %temp, ptr nonnull %temp, ptr nonnull %temp, ptr nonnull %temp, i32 5)
  %add3 = add nsw i32 %add, %call2
  %call4 = call i32 @foo(i32 2, i32 4, ptr nonnull %forfake, ptr nonnull %dummy, ptr nonnull %dummy, ptr %silly, ptr nonnull %silly, i32 5)
  %add5 = add nsw i32 %add3, %call4
  %call6 = call i32 @foo(i32 2, i32 4, ptr nonnull %forfake, ptr nonnull %forreal, ptr nonnull %forreal, ptr nonnull %forfake, ptr nonnull %dummy, i32 5)
  %add7 = add nsw i32 %add5, %call6
  ret i32 %add7
}
; end INTEL_FEATURE_SW_ADVANCED

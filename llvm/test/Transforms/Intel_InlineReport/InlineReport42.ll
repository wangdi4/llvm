; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt < %s -inline -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe807 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-NEW %s
; RUN: opt < %s -passes='cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe807 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-NEW %s
; Inline report via metadata
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -inline -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-OLD
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-OLD

; Check that foo is NOT inlined into bar , in accord with the "dummy
; args" heuristic, because its callsites do not feed a switch statement.

; CHECK-OLD: COMPILE FUNC: foo
; CHECK-OLD: COMPILE FUNC: bar
; CHECK-OLD-NOT: INLINE: foo{{.*}}<<Callee has callsites with dummy args>>
; CHECK: define{{.*}}@foo
; CHECK: define{{.*}}@bar
; CHECK-NEW: COMPILE FUNC: foo
; CHECK-NEW: COMPILE FUNC: bar
; CHECK-NEW-NOT: INLINE: foo{{.*}}<<Callee has callsites with dummy args>>

define dso_local i32 @foo(i32 %arg1, i32 %arg2, i32* %arg3, i32* %arg4, i32* %arg5, i32* %arg6, i32* %arg7, i32 %arg8) #0 {
entry:
  %sub = sub nsw i32 %arg1, %arg2
  %idx.ext = sext i32 %sub to i64
  %add.ptr = getelementptr inbounds i32, i32* %arg3, i64 %idx.ext
  %cmp = icmp ugt i32* %add.ptr, inttoptr (i64 1 to i32*)
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i32 15, i32* %arg3, align 4
  store i32 20, i32* %arg4, align 4
  store i32 25, i32* %arg5, align 4
  store i32 30, i32* %arg6, align 4
  store i32 35, i32* %arg7, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %add = add nsw i32 %arg1, %arg2
  %add1 = add nsw i32 %add, %arg8
  ret i32 %add1
}

define dso_local i32 @bar() local_unnamed_addr #1 {
entry:
  %dummy = alloca i32, align 4
  %temp = alloca i32, align 4
  %forreal = alloca i32, align 4
  %forfake = alloca i32, align 4
  %0 = bitcast i32* %dummy to i8*
  store i32 0, i32* %dummy, align 4
  %1 = bitcast i32* %temp to i8*
  store i32 0, i32* %temp, align 4
  %2 = bitcast i32* %forreal to i8*
  store i32 1, i32* %forreal, align 4
  %3 = bitcast i32* %forfake to i8*
  store i32 2, i32* %forfake, align 4
  %call = call i32 @foo(i32 2, i32 3, i32* nonnull %forreal, i32* nonnull %dummy, i32* nonnull %dummy, i32* nonnull %dummy, i32* nonnull %dummy, i32 4)
  %call1 = call i32 @foo(i32 2, i32 4, i32* nonnull %forfake, i32* nonnull %dummy, i32* nonnull %dummy, i32* nonnull %dummy, i32* nonnull %dummy, i32 5)
  %add = add nsw i32 %call, %call1
  %call2 = call i32 @foo(i32 2, i32 4, i32* nonnull %forreal, i32* nonnull %temp, i32* nonnull %temp, i32* nonnull %temp, i32* nonnull %temp, i32 5)
  %add3 = add nsw i32 %add, %call2
  %call4 = call i32 @foo(i32 2, i32 4, i32* nonnull %forfake, i32* nonnull %dummy, i32* nonnull %dummy, i32* nonnull %dummy, i32* nonnull %dummy, i32 5)
  %add5 = add nsw i32 %add3, %call4
  %call6 = call i32 @foo(i32 2, i32 4, i32* nonnull %forfake, i32* nonnull %forreal, i32* nonnull %forreal, i32* nonnull %forfake, i32* nonnull %dummy, i32 5)
  %add7 = add nsw i32 %add5, %call6
  ret i32 %add7
}
; end INTEL_FEATURE_SW_ADVANCED

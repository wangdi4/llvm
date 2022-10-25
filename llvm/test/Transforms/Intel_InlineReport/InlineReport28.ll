; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -inline -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; Inline report via metadata
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -inline -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD

; This test checks cases where the key switch heuristic does not trigger.
; The test does this by noting that the inline report does not produce the
; 'key switch' message and that the functions @f and @g within the various
; versions of @mycaller are inlined.

; Inlining report output for old pass manager

; CHECK-OLD-LABEL: COMPILE FUNC: mycaller1
; CHECK-OLD-NOT:{{.*}}Callsite has key switch computations
; CHECK-OLD-LABEL: COMPILE FUNC: mycaller2
; CHECK-OLD-NOT:{{.*}}Callsite has key switch computations
; CHECK-OLD-LABEL: COMPILE FUNC: mycaller3
; CHECK-OLD-NOT:{{.*}}Callsite has key switch computations
; CHECK-OLD-LABEL: COMPILE FUNC: mycaller4
; CHECK-OLD-NOT:{{.*}}Callsite has key switch computations
; CHECK-OLD-LABEL: COMPILE FUNC: mycaller5
; CHECK-OLD-NOT:{{.*}}Callsite has key switch computations
; CHECK-OLD-LABEL: COMPILE FUNC: mycaller6
; CHECK-OLD-NOT:{{.*}}Callsite has key switch computations
; CHECK-OLD-LABEL: COMPILE FUNC: mycaller7
; CHECK-OLD-NOT:{{.*}}Callsite has key switch computations

%struct.ss = type { i32, i64, i32 (%struct.ss*)* }
%struct.tt = type { %struct.ss, i32 (%struct.ss*)* }

define internal void @f(%struct.ss* byval(%struct.ss)  %b) nounwind  {
entry:
  %tmp = getelementptr %struct.ss, %struct.ss* %b, i32 0, i32 0
  %tmp1 = load i32, i32* %tmp, align 4
  %tmp2 = add i32 %tmp1, 1
  store i32 %tmp2, i32* %tmp, align 4
  ret void
}

define internal void @g(%struct.ss* byval(%struct.ss) align 32 %b) nounwind {
entry:
  %tmp = getelementptr %struct.ss, %struct.ss* %b, i32 0, i32 0
  %tmp1 = load i32, i32* %tmp, align 4
  %tmp2 = sub i32 %tmp1, 1
  store i32 %tmp2, i32* %tmp, align 4
  ret void
}

define internal i32 @h(i32 %mine) nounwind {
  ret i32 %mine
}

define linkonce_odr dso_local i32 @mycallee(%struct.ss* %this, %struct.ss* %s0) align 2 {
  %tmp1 = getelementptr %struct.ss, %struct.ss* %this, i32 0, i32 2
  %tmp2 = load i32 (%struct.ss*)*, i32 (%struct.ss*)** %tmp1, align 4
  %call = call i32 %tmp2(%struct.ss* %s0)
  ret i32 %call
}

define linkonce_odr dso_local i32 @mycallee5(%struct.ss* %this, %struct.ss* %s0) align 2 {
  ret i32 5
}

; Check that the inlining of @g within @mycaller1 is not inhibited,
; because @mycaller1 has more than 3 arguments.

; CHECK-OLD-LABEL: @mycaller1
; CHECK-OLD-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller1
; CHECK-NEW-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller1(%struct.tt* %this, %struct.ss* %theXObject, i1 zeroext %fInReset, i32 %myfour) align 2 {
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  ret i1 0
}

; Check that the inlining of @g within @mycaller2 is not inhibited,
; because @mycaller2 does not have a switch statement.

; CHECK-OLD-LABEL: @mycaller2
; CHECK-OLD-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller2
; CHECK-NEW-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller2(%struct.tt* %this, %struct.ss* %theXObject, i1 zeroext %fInReset) align 2 {
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  ret i1 0
}

; Check that the inlining of @f and @g within @mycaller3 is not inhibited,
; because @mycaller3 has a switch statement without enough cases.

; CHECK-OLD-LABEL: @mycaller3
; CHECK-OLD-NOT: call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-OLD-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller3
; CHECK-NEW-NOT: call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller3(%struct.tt* %this, %struct.ss* %theXObject, i1 zeroext %fInReset) align 2 {
entry:
  %t0 = getelementptr inbounds %struct.tt, %struct.tt* %this, i64 0, i32 0
  %call = call i32 @mycallee(%struct.ss* %t0, %struct.ss* %theXObject)
  switch i32 %call, label %sw.default [
    i32 11, label %sw.bb5
    i32 12, label %sw.bb8
    i32 4, label %sw.bb12
    i32 10, label %sw.bb17
    i32 8, label %sw.bb20
    i32 3, label %sw.bb23
    i32 5, label %sw.bb32
    i32 13, label %sw.bb41
  ]

sw.bb5: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb8: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb12: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb17: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb20: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb23: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb32: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb41: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.default: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.epilog: ; preds = %sw.default, %sw.bb5, %sw.bb8, %sw.bb8, %sw.bb12, %sw.bb17, %sw.bb20, %sw.bb23, %sw.bb32, %sw.bb41
  ret i1 0
}

; Check that the inlining of @f and @g within @mycaller4 is not inhibited,
; because @mycaller4 has a switch statement that does not depend on
; value returned by a call.

; CHECK-OLD-LABEL: @mycaller4
; CHECK-OLD-NOT: call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-OLD-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller4
; CHECK-NEW-NOT: call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller4(%struct.tt* %this, %struct.ss* %theXObject, i32* %tmp) align 2 {
entry:
  %tmp1 = load i32, i32* %tmp, align 4
  switch i32 %tmp1, label %sw.default [
    i32 2, label %sw.bb2
    i32 0, label %sw.bb2
    i32 9, label %sw.bb2
    i32 11, label %sw.bb5
    i32 12, label %sw.bb8
    i32 4, label %sw.bb12
    i32 10, label %sw.bb17
    i32 8, label %sw.bb20
    i32 3, label %sw.bb23
    i32 5, label %sw.bb32
    i32 13, label %sw.bb41
  ]

sw.bb2: ; preds = %entry, %entry, %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb5: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb8: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb12: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb17: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb20: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb23: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb32: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb41: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.default: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.epilog: ; preds = %sw.default, %sw.bb2 %sw.bb5, %sw.bb8, %sw.bb8, %sw.bb12, %sw.bb17, %sw.bb20, %sw.bb23, %sw.bb32, %sw.bb41
  ret i1 0
}

; Check that the inlining of @f and @g within @mycaller5 is not inhibited,
; because @mycaller5 has a switch statement that depends on a call that
; does not get its return value from an indirect function call.

; CHECK-OLD-LABEL: @mycaller5
; CHECK-OLD-NOT: call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-OLD-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller5
; CHECK-NEW-NOT: call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller5(%struct.tt* %this, %struct.ss* %theXObject, i1 zeroext %fInReset) align 2 {
entry:
  %t0 = getelementptr inbounds %struct.tt, %struct.tt* %this, i64 0, i32 0
  %call = call i32 @mycallee5(%struct.ss* %t0, %struct.ss* %theXObject)
  switch i32 %call, label %sw.default [
    i32 2, label %sw.bb2
    i32 0, label %sw.bb2
    i32 9, label %sw.bb2
    i32 11, label %sw.bb5
    i32 12, label %sw.bb8
    i32 4, label %sw.bb12
    i32 10, label %sw.bb17
    i32 8, label %sw.bb20
    i32 3, label %sw.bb23
    i32 5, label %sw.bb32
    i32 13, label %sw.bb41
  ]

sw.bb2: ; preds = %entry, %entry, %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb5: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb8: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb12: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb17: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb20: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb23: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb32: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb41: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.default: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.epilog: ; preds = %sw.default, %sw.bb2 %sw.bb5, %sw.bb8, %sw.bb8, %sw.bb12, %sw.bb17, %sw.bb20, %sw.bb23, %sw.bb32, %sw.bb41
  ret i1 0
}

; Check that the inlining of @f and @g within @mycaller6 is not inhibited,
; because @mycaller6 has a block structure that does not consist as a
; series of independent switch targets, connected together into a
; single join block.

; CHECK-OLD-LABEL: @mycaller6
; CHECK-OLD-NOT: call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-OLD-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller6
; CHECK-NEW-NOT: call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller6(%struct.tt* %this, %struct.ss* %theXObject, i1 zeroext %fInReset) align 2 {
entry:
  %t0 = getelementptr inbounds %struct.tt, %struct.tt* %this, i64 0, i32 0
  %call = call i32 @mycallee(%struct.ss* %t0, %struct.ss* %theXObject)
  switch i32 %call, label %sw.default [
    i32 2, label %sw.bb2
    i32 0, label %sw.bb2
    i32 9, label %sw.bb2
    i32 11, label %sw.bb5
    i32 12, label %sw.bb8
    i32 4, label %sw.bb12
    i32 10, label %sw.bb17
    i32 8, label %sw.bb20
    i32 3, label %sw.bb23
    i32 5, label %sw.bb32
    i32 13, label %sw.bb41
  ]

sw.bb2: ; preds = %entry, %entry, %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.bb20

sw.bb5: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb8: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb12: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb17: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb20: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb23: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb32: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb41: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.default: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog1

sw.epilog: ; preds = %sw.bb2 %sw.bb5, %sw.bb8, %sw.bb8, %sw.bb12, %sw.bb17, %sw.bb20, %sw.bb23, %sw.bb32, %sw.bb41
  ret i1 0

sw.epilog1: ; preds = sw.default
  ret i1 1
}

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local i8* @__cxa_begin_catch(i8*)

define linkonce_odr hidden void @__clang_call_terminate(i8*) {
  %2 = call i8* @__cxa_begin_catch(i8* %0) #4
  unreachable
}

; Check that the inlining of @f and @g within @mycaller7 is not inhibited,
; because @mycaller7 has an invoke instruction.

; CHECK-OLD-LABEL: @mycaller7
; CHECK-OLD-NOT: call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-OLD-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller7
; CHECK-NEW-NOT: call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-NOT: call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller7(%struct.tt* %this, %struct.ss* %theXObject, i1 zeroext %fInReset) align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %t0 = getelementptr inbounds %struct.tt, %struct.tt* %this, i64 0, i32 0
  %call = call i32 @mycallee(%struct.ss* %t0, %struct.ss* %theXObject)
  switch i32 %call, label %sw.default [
    i32 2, label %sw.bb2
    i32 0, label %sw.bb2
    i32 9, label %sw.bb2
    i32 11, label %sw.bb5
    i32 12, label %sw.bb8
    i32 4, label %sw.bb12
    i32 10, label %sw.bb17
    i32 8, label %sw.bb20
    i32 3, label %sw.bb23
    i32 5, label %sw.bb32
    i32 13, label %sw.bb41
  ]

sw.bb2: ; preds = %entry, %entry, %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb5: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb8: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb12: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb17: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb20: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb23: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb32: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.bb41: ; preds = %entry
  call void @f(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.default: ; preds = %entry
  call void @g(%struct.ss* byval(%struct.ss) %theXObject) nounwind
  br label %sw.epilog

sw.epilog: ; preds = %sw.default, %sw.bb2 %sw.bb5, %sw.bb8, %sw.bb8, %sw.bb12, %sw.bb17, %sw.bb20, %sw.bb23, %sw.bb32, %sw.bb41
  %inv = invoke i32 @h(i32 7) to label %ilabel1 unwind label %lpad
  ret i1 2

ilabel1:
  ret i1 1

lpad:
  %t29 = landingpad { i8*, i32 }
          catch i8* null
  %t30 = extractvalue { i8*, i32 } %t29, 0
  call void @__clang_call_terminate(i8* %t30)
  unreachable
}

@myglobal1 = dso_local global %struct.tt zeroinitializer, align 16
@myglobal2 = dso_local global %struct.ss zeroinitializer, align 16

define i1 @main() nounwind  {
  %t1 = call i1 @mycaller1(%struct.tt* @myglobal1, %struct.ss* @myglobal2, i1 0, i32 0)
  %t2 = call i1 @mycaller2(%struct.tt* @myglobal1, %struct.ss* @myglobal2, i1 0)
  %t3 = call i1 @mycaller3(%struct.tt* @myglobal1, %struct.ss* @myglobal2, i1 0)
  %t4 = call i1 @mycaller4(%struct.tt* @myglobal1, %struct.ss* @myglobal2, i32* null)
  %t5 = call i1 @mycaller5(%struct.tt* @myglobal1, %struct.ss* @myglobal2, i1 0)
  %t6 = call i1 @mycaller6(%struct.tt* @myglobal1, %struct.ss* @myglobal2, i1 0)
  %t7 = call i1 @mycaller7(%struct.tt* @myglobal1, %struct.ss* @myglobal2, i1 0)
  ret i1 %t1
}

; Inlining report output for new pass manager

; CHECK-NEW-LABEL: COMPILE FUNC: mycaller1
; CHECK-NEW-NOT:{{.*}}Callsite has key switch computations
; CHECK-NEW-LABEL: COMPILE FUNC: mycaller2
; CHECK-NEW-NOT:{{.*}}Callsite has key switch computations
; CHECK-NEW-LABEL: COMPILE FUNC: mycaller3
; CHECK-NEW-NOT:{{.*}}Callsite has key switch computations
; CHECK-NEW-LABEL: COMPILE FUNC: mycaller4
; CHECK-NEW-NOT:{{.*}}Callsite has key switch computations
; CHECK-NEW-LABEL: COMPILE FUNC: mycaller5
; CHECK-NEW-NOT:{{.*}}Callsite has key switch computations
; CHECK-NEW-LABEL: COMPILE FUNC: mycaller6
; CHECK-NEW-NOT:{{.*}}Callsite has key switch computations
; CHECK-NEW-LABEL: COMPILE FUNC: mycaller7
; CHECK-NEW-NOT:{{.*}}Callsite has key switch computations
; end INTEL_FEATURE_SW_ADVANCED

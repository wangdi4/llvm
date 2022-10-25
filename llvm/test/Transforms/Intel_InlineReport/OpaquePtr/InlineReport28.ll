; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD

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

%struct.tt = type { %struct.ss, ptr }
%struct.ss = type { i32, i64, ptr }

@myglobal1 = dso_local global %struct.tt zeroinitializer, align 16
@myglobal2 = dso_local global %struct.ss zeroinitializer, align 16

; Function Attrs: nounwind
define internal void @f(ptr byval(%struct.ss) %b) #0 {
entry:
  %tmp = getelementptr %struct.ss, ptr %b, i32 0, i32 0
  %tmp1 = load i32, ptr %tmp, align 4
  %tmp2 = add i32 %tmp1, 1
  store i32 %tmp2, ptr %tmp, align 4
  ret void
}

; Function Attrs: nounwind
define internal void @g(ptr byval(%struct.ss) align 32 %b) #0 {
entry:
  %tmp = getelementptr %struct.ss, ptr %b, i32 0, i32 0
  %tmp1 = load i32, ptr %tmp, align 4
  %tmp2 = sub i32 %tmp1, 1
  store i32 %tmp2, ptr %tmp, align 4
  ret void
}

; Function Attrs: nounwind
define internal i32 @h(i32 %mine) #0 {
bb:
  ret i32 %mine
}

define linkonce_odr dso_local i32 @mycallee(ptr %this, ptr %s0) align 2 {
bb:
  %tmp1 = getelementptr %struct.ss, ptr %this, i32 0, i32 2
  %tmp2 = load ptr, ptr %tmp1, align 4
  %call = call i32 %tmp2(ptr %s0)
  ret i32 %call
}

define linkonce_odr dso_local i32 @mycallee5(ptr %this, ptr %s0) align 2 {
bb:
  ret i32 5
}

; Check that the inlining of @g within @mycaller1 is not inhibited,
; because @mycaller1 has more than 3 arguments.

; CHECK-OLD-LABEL: @mycaller1
; CHECK-OLD-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller1
; CHECK-NEW-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller1(ptr %this, ptr %theXObject, i1 zeroext %fInReset, i32 %myfour) align 2 {
bb:
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  ret i1 false
}

; Check that the inlining of @g within @mycaller2 is not inhibited,
; because @mycaller2 does not have a switch statement.

; CHECK-OLD-LABEL: @mycaller2
; CHECK-OLD-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller2
; CHECK-NEW-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller2(ptr %this, ptr %theXObject, i1 zeroext %fInReset) align 2 {
bb:
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  ret i1 false
}

; Check that the inlining of @f and @g within @mycaller3 is not inhibited,
; because @mycaller3 has a switch statement without enough cases.

; CHECK-OLD-LABEL: @mycaller3
; CHECK-OLD-NOT: call void @f(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-OLD-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller3
; CHECK-NEW-NOT: call void @f(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller3(ptr %this, ptr %theXObject, i1 zeroext %fInReset) align 2 {
entry:
  %t0 = getelementptr inbounds %struct.tt, ptr %this, i64 0, i32 0
  %call = call i32 @mycallee(ptr %t0, ptr %theXObject)
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

sw.bb5:                                           ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb8:                                           ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb12:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb17:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb20:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb23:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb32:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb41:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb41, %sw.bb32, %sw.bb23, %sw.bb20, %sw.bb17, %sw.bb12, %sw.bb8, %sw.bb5
  ret i1 false
}

; Check that the inlining of @f and @g within @mycaller4 is not inhibited,
; because @mycaller4 has a switch statement that does not depend on
; value returned by a call.

; CHECK-OLD-LABEL: @mycaller4
; CHECK-OLD-NOT: call void @f(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-OLD-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller4
; CHECK-NEW-NOT: call void @f(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller4(ptr %this, ptr %theXObject, ptr %tmp) align 2 {
entry:
  %tmp1 = load i32, ptr %tmp, align 4
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

sw.bb2:                                           ; preds = %entry, %entry, %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb5:                                           ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb8:                                           ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb12:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb17:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb20:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb23:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb32:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb41:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb41, %sw.bb32, %sw.bb23, %sw.bb20, %sw.bb17, %sw.bb12, %sw.bb8, %sw.bb5, %sw.bb2
  ret i1 false
}

; Check that the inlining of @f and @g within @mycaller5 is not inhibited,
; because @mycaller5 has a switch statement that depends on a call that
; does not get its return value from an indirect function call.

; CHECK-OLD-LABEL: @mycaller5
; CHECK-OLD-NOT: call void @f(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-OLD-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller5
; CHECK-NEW-NOT: call void @f(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller5(ptr %this, ptr %theXObject, i1 zeroext %fInReset) align 2 {
entry:
  %t0 = getelementptr inbounds %struct.tt, ptr %this, i64 0, i32 0
  %call = call i32 @mycallee5(ptr %t0, ptr %theXObject)
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

sw.bb2:                                           ; preds = %entry, %entry, %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb5:                                           ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb8:                                           ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb12:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb17:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb20:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb23:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb32:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb41:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb41, %sw.bb32, %sw.bb23, %sw.bb20, %sw.bb17, %sw.bb12, %sw.bb8, %sw.bb5, %sw.bb2
  ret i1 false
}

; Check that the inlining of @f and @g within @mycaller6 is not inhibited,
; because @mycaller6 has a block structure that does not consist as a
; series of independent switch targets, connected together into a
; single join block.

; CHECK-OLD-LABEL: @mycaller6
; CHECK-OLD-NOT: call void @f(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-OLD-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller6
; CHECK-NEW-NOT: call void @f(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller6(ptr %this, ptr %theXObject, i1 zeroext %fInReset) align 2 {
entry:
  %t0 = getelementptr inbounds %struct.tt, ptr %this, i64 0, i32 0
  %call = call i32 @mycallee(ptr %t0, ptr %theXObject)
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

sw.bb2:                                           ; preds = %entry, %entry, %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.bb20

sw.bb5:                                           ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb8:                                           ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb12:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb17:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb20:                                          ; preds = %sw.bb2, %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb23:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb32:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb41:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog1

sw.epilog:                                        ; preds = %sw.bb41, %sw.bb32, %sw.bb23, %sw.bb20, %sw.bb17, %sw.bb12, %sw.bb8, %sw.bb5
  ret i1 false

sw.epilog1:                                       ; preds = %sw.default
  ret i1 true
}

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local ptr @__cxa_begin_catch(ptr)

define linkonce_odr hidden void @__clang_call_terminate(ptr %arg) {
bb:
  %i = call ptr @__cxa_begin_catch(ptr %arg)
  unreachable
}

; Check that the inlining of @f and @g within @mycaller7 is not inhibited,
; because @mycaller7 has an invoke instruction.

; CHECK-OLD-LABEL: @mycaller7
; CHECK-OLD-NOT: call void @f(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-OLD-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-LABEL: @mycaller7
; CHECK-NEW-NOT: call void @f(ptr byval(%struct.ss) %theXObject) nounwind
; CHECK-NEW-NOT: call void @g(ptr byval(%struct.ss) %theXObject) nounwind

define dso_local zeroext i1 @mycaller7(ptr %this, ptr %theXObject, i1 zeroext %fInReset) align 2 personality ptr @__gxx_personality_v0 {
entry:
  %t0 = getelementptr inbounds %struct.tt, ptr %this, i64 0, i32 0
  %call = call i32 @mycallee(ptr %t0, ptr %theXObject)
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

sw.bb2:                                           ; preds = %entry, %entry, %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb5:                                           ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb8:                                           ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb12:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb17:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb20:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb23:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb32:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb41:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb41, %sw.bb32, %sw.bb23, %sw.bb20, %sw.bb17, %sw.bb12, %sw.bb8, %sw.bb5, %sw.bb2
  %inv = invoke i32 @h(i32 7)
          to label %ilabel1 unwind label %lpad

bb:                                               ; No predecessors!
  ret i1 false

ilabel1:                                          ; preds = %sw.epilog
  ret i1 true

lpad:                                             ; preds = %sw.epilog
  %t29 = landingpad { ptr, i32 }
          catch ptr null
  %t30 = extractvalue { ptr, i32 } %t29, 0
  call void @__clang_call_terminate(ptr %t30)
  unreachable
}

; Function Attrs: nounwind
define i1 @main() #0 {
bb:
  %t1 = call i1 @mycaller1(ptr @myglobal1, ptr @myglobal2, i1 false, i32 0)
  %t2 = call i1 @mycaller2(ptr @myglobal1, ptr @myglobal2, i1 false)
  %t3 = call i1 @mycaller3(ptr @myglobal1, ptr @myglobal2, i1 false)
  %t4 = call i1 @mycaller4(ptr @myglobal1, ptr @myglobal2, ptr null)
  %t5 = call i1 @mycaller5(ptr @myglobal1, ptr @myglobal2, i1 false)
  %t6 = call i1 @mycaller6(ptr @myglobal1, ptr @myglobal2, i1 false)
  %t7 = call i1 @mycaller7(ptr @myglobal1, ptr @myglobal2, i1 false)
  ret i1 %t1
}

attributes #0 = { nounwind }

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

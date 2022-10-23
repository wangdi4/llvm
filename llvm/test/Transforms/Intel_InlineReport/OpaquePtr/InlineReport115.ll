; RUN: opt -opaque-pointers -S -passes='cgscc(inline)' -inline-report=0xe807 -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that the inline report can be produced without dying because an active
; inlined call site is replaced without another during inlining. The check that
; DELETE does not appear below indicates that the active inlined call site being
; replaced was recognized and the replacing call site was substituted for it.

; CHECK-CL-LABEL: COMPILE FUNC: ?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z
; CHECK-LABEL: COMPILE FUNC: ??$?6U?$char_traits@D@std@@@std@@YAAAV?$basic_ostream@DU?$char_traits@D@std@@@0@AAV10@PBD@Z
; CHECK-NEXT: INLINE: ?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z <<Callee is single basic block>>
; CHECK-NOT: DELETE
; CHECK-MD-LABEL: COMPILE FUNC: ?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z

$"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z" = comdat any

define ptr @"??$?6U?$char_traits@D@std@@@std@@YAAAV?$basic_ostream@DU?$char_traits@D@std@@@0@AAV10@PBD@Z"() personality ptr @__CxxFrameHandler3 {
if.end87:
  ret ptr null

catch.dispatch:                                   ; No predecessors!
  %i = catchswitch within none [label %catch] unwind label %ehcleanup105

catch:                                            ; preds = %catch.dispatch
  %i1 = catchpad within %i [ptr null, i32 0, ptr null]
  invoke x86_thiscallcc void @"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z"(ptr null, i32 0, i1 false) [ "funclet"(token %i1) ]
          to label %invoke.cont98 unwind label %ehcleanup105

invoke.cont98:                                    ; preds = %catch
  ret ptr null

ehcleanup105:                                     ; preds = %catch, %catch.dispatch
  %i2 = cleanuppad within none []
  cleanupret from %i2 unwind to caller
}

declare i32 @__CxxFrameHandler3(...)

define x86_thiscallcc void @"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z"(ptr %this, i32 %_State, i1 %_Reraise) comdat {
entry:
  %call = call x86_thiscallcc i32 undef(ptr null)
  ret void
}

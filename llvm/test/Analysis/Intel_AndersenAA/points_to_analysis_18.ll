; CMPLRLLVM-9114: Verifies that points-to info for _Z4testv:%1 is not
; incorrectly computed as empty.
; RUN: opt < %s  -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; Andersens Analysis shouldn't ignore non-pointer landingpad instruction.
; Return type of landingpad instruction may be struct type that can have
; pointer fields. If landingpad instruction is ignored, the pointers in
; the return struct are not modeled correctly by the analysis.

; CHECK: [1] _Z4testv:%0   --> (0): <universal>
; CHECK: _Z4testv:%1       --> same as
; CHECK-SAME: _Z4testv:%0
; CHECK-NOT: [0] _Z4testv:%1        -->

@_ZTIPi = external constant ptr
define void @_Z4testv() personality ptr bitcast (ptr @__gxx_personality_v0 to ptr) {
entry:
  invoke void @_Z3foov()
     to label %invoke.cont unwind label %lpad
 lpad:
   %0 = landingpad {ptr, i32}
        catch ptr bitcast (ptr @_ZTIPi to ptr)
   %1 = extractvalue { ptr, i32 } %0, 0
   br label %invoke.cont
 invoke.cont:
  ret void
}

declare void @_Z3foov()
declare i32 @__gxx_personality_v0(...)

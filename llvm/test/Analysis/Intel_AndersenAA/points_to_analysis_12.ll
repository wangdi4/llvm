; RUN: opt < %s -passes='require<anders-aa>' -disable-output 2>/dev/null
; RUN: opt < %s -passes='require<anders-aa>' -disable-output -skip-anders-unreachable-asserts=false 2> /dev/null
; This test has different WinEH related instruction.
; It tests Andersens analysis doesn't crash in presence of WinEH, and that it
; doesn't just silently ignore the instructions.

declare void @_Z3quxv()

declare void @"\01?foo@@YAXXZ"()

declare i32 @__C_specific_handler(...)

declare void @"\01?bar@@YAXXZ"()

declare i32 @__gxx_personality_v0(...)

define i32 @main() personality ptr @__C_specific_handler {
entry:
  invoke void @"\01?foo@@YAXXZ"()
          to label %__try.cont unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %entry
  %cs = catchswitch within none [label %__except.ret] unwind to caller

__except.ret:                                     ; preds = %catch.dispatch
  %i = catchpad within %cs [ptr @"\01?filt$0@0@main@@"]
  catchret from %i to label %__except

__except:                                         ; preds = %__except.ret
  tail call void @"\01?bar@@YAXXZ"()
  br label %__try.cont

__try.cont:                                       ; preds = %__except, %entry
  ret i32 0
}

define internal i32 @"\01?filt$0@0@main@@"(ptr nocapture readnone %exception_pointers, ptr nocapture readnone %frame_pointer) {
entry:
  ret i32 20
}

define void @cleanupret0() personality ptr @__gxx_personality_v0 {
entry:
  invoke void @_Z3quxv() #0
          to label %exit unwind label %pad

pad:                                              ; preds = %entry
  %cp = cleanuppad within none [i7 4]
  cleanupret from %cp unwind to caller

exit:                                             ; preds = %entry
  ret void
}

attributes #0 = { optsize }

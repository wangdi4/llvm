; RUN: opt < %s -anders-aa -disable-output 2>/dev/null
; This test has different WinEH related instruction.
; It tests Andersens analysis doesn't crash in presence of WinEH.

declare void @_Z3quxv()
declare void @"\01?foo@@YAXXZ"()
declare i32 @__C_specific_handler(...)
declare void @"\01?bar@@YAXXZ"() 
declare i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind uwtable
define i32 @main() #0 personality i8* bitcast (i32 (...)* @__C_specific_handler to i8*) {
entry:
  invoke void @"\01?foo@@YAXXZ"()
          to label %__try.cont unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %entry
  %cs = catchswitch within none [label %__except.ret] unwind to caller

__except.ret:                                     ; preds = %catch.dispatch
  %0 = catchpad within %cs [i8* bitcast (i32 (i8*, i8*)* @"\01?filt$0@0@main@@" to i8*)]
  catchret from %0 to label %__except

__except:                                         ; preds = %__except.ret
  tail call void @"\01?bar@@YAXXZ"() 
  br label %__try.cont

__try.cont:                                       ; preds = %entry, %__except
  ret i32 0
}

; Function Attrs: nounwind readnone
define internal i32 @"\01?filt$0@0@main@@"(i8* nocapture readnone %exception_pointers, i8* nocapture readnone %frame_pointer) {
entry:
  ret i32 20
}

define void @cleanupret0() personality i32 (...)* @__gxx_personality_v0 {
entry:
  invoke void @_Z3quxv() optsize
          to label %exit unwind label %pad
pad:
  %cp = cleanuppad within none [i7 4]
  cleanupret from %cp unwind to caller
exit:
  ret void
}

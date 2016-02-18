; RUN: opt < %s -anders-aa -disable-output 2>/dev/null
; Test Andersens analysis that caused crash when calloc/malloc is called
; in invoke instruction. 

; Function Attrs: nounwind uwtable
define noalias i8* @"\01?foo@@YAPEADH@Z"(i32 %size) #0 personality i8* bitcast (i32 (...)* @__C_specific_handler to i8*) {
entry:
  %conv = sext i32 %size to i64
  %call = invoke noalias i8* @calloc(i64 %conv, i64 4)
          to label %__try.cont unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %entry
  %cs = catchswitch within none [label %__except] unwind to caller

__except:                                         ; preds = %catch.dispatch
  %0 = catchpad within %cs [i8* null]
  catchret from %0 to label %__try.cont

__try.cont:                                       ; preds = %entry, %__except
  %ptr.0 = phi i8* [ undef, %__except ], [ %call, %entry ]
  ret i8* %ptr.0

}

; Function Attrs: nounwind
declare noalias i8* @calloc(i64, i64) 

declare i32 @__C_specific_handler(...)




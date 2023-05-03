; RUN: opt < %s -passes='require<anders-aa>'  -disable-output 2>/dev/null
; Test Andersens analysis that caused crash when calloc/malloc is called
; in invoke instruction. 

define noalias ptr @"\01?foo@@YAPEADH@Z"(i32 %size) personality ptr @__C_specific_handler {
entry:
  %conv = sext i32 %size to i64
  %call = invoke noalias ptr @calloc(i64 %conv, i64 4)
          to label %__try.cont unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %entry
  %cs = catchswitch within none [label %__except] unwind to caller

__except:                                         ; preds = %catch.dispatch
  %i = catchpad within %cs [ptr null]
  catchret from %i to label %__try.cont

__try.cont:                                       ; preds = %__except, %entry
  %ptr.0 = phi ptr [ undef, %__except ], [ %call, %entry ]
  ret ptr %ptr.0
}

declare noalias ptr @calloc(i64, i64)

declare i32 @__C_specific_handler(...)

; RUN: llc < %s -mtriple=x86_64-pc-windows-msvc | FileCheck %s

; CHECK-LABEL: .Llsda_begin0:
; CHECK-NEXT:        .long   .Ltmp0@IMGREL                   # LabelStart
; CHECK-NEXT:        .long   .Ltmp1@IMGREL+1                 # LabelEnd
; CHECK-NEXT:        .long   1                               # CatchAll
; CHECK-NEXT:        .long   .LBB0_4@IMGREL                  # ExceptionHandler
; CHECK-NEXT:        .long   .Ltmp0@IMGREL                   # LabelStart
; CHECK-NEXT:        .long   .Ltmp1@IMGREL+1                 # LabelEnd
; CHECK-NEXT:        .long   1                               # CatchAll
; CHECK-NEXT:        .long   .LBB0_6@IMGREL                  # ExceptionHandler
; CHECK-NEXT:        .long   .Ltmp0@IMGREL                   # LabelStart
; CHECK-NEXT:        .long   .Ltmp1@IMGREL+1                 # LabelEnd
; CHECK-NEXT:        .long   "?dtor$7@?0?main@4HA"@IMGREL    # FinallyFunclet
; CHECK-NEXT:        .long   0                               # Null

define dso_local void @main() personality ptr @__C_specific_handler {
entry:
  invoke void @llvm.seh.scope.begin()
          to label %invoke.cont unwind label %ehcleanup

invoke.cont:                                      ; preds = %entry
  invoke void @llvm.seh.try.begin()
          to label %invoke.cont1 unwind label %ehcleanup

invoke.cont1:                                     ; preds = %invoke.cont
  invoke void @llvm.seh.try.begin()
          to label %invoke.cont2 unwind label %catch.dispatch10

invoke.cont2:                                     ; preds = %invoke.cont1
  invoke void @llvm.seh.try.begin()
          to label %invoke.cont4 unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %invoke.cont2
  %0 = catchswitch within none [label %__except] unwind label %catch.dispatch10

__except:                                         ; preds = %catch.dispatch
  %1 = catchpad within %0 [ptr null]
  unreachable

invoke.cont4:                                     ; preds = %invoke.cont2
  store volatile i32 1, ptr poison, align 4
  unreachable

catch.dispatch10:                                 ; preds = %catch.dispatch, %invoke.cont1
  %2 = catchswitch within none [label %__except11] unwind label %ehcleanup

__except11:                                       ; preds = %catch.dispatch10
  %3 = catchpad within %2 [ptr null]
  unreachable

ehcleanup:                                        ; preds = %catch.dispatch10, %invoke.cont, %entry
  %4 = cleanuppad within none []
  cleanupret from %4 unwind to caller
}

declare dso_local void @llvm.seh.scope.begin()
declare dso_local i32 @__C_specific_handler(...)
declare dso_local void @llvm.seh.try.begin()

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"eh-asynch", i32 1}

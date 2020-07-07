; RUN: opt < %s -simplifycfg -S | FileCheck %s
;
; CHECK:  %call = call i32 @_Z3foov(), !prof !1
; CHECK: !1 = !{!"branch_weights", i32 7}

; invoke with 2-edge !prof branch_weights, is changed to a call.
; The call should only have 1 edge in its branch_weights.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @_Z3foov() #0 {
entry:
  ret i32 0
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %exn.slot = alloca i8*, align 8
  %ehselector.slot = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = bitcast i32* %i to i8*
  %call = invoke i32 @_Z3foov()
          to label %invoke.cont unwind label %lpad, !prof !1

invoke.cont:                                      ; preds = %for.body
  br label %try.cont

lpad:                                             ; preds = %for.body
  %1 = landingpad { i8*, i32 }
          catch i8* null
  %2 = extractvalue { i8*, i32 } %1, 0
  store i8* %2, i8** %exn.slot, align 8
  %3 = extractvalue { i8*, i32 } %1, 1
  store i32 %3, i32* %ehselector.slot, align 4
  br label %catch

catch:                                            ; preds = %lpad
  %exn = load i8*, i8** %exn.slot, align 8
  %4 = call i8* @__cxa_begin_catch(i8* %exn) #3
  call void @__cxa_end_catch()
  br label %try.cont

try.cont:                                         ; preds = %catch, %invoke.cont
  br label %block2

block2:                                          ; preds = %try.cont
  ret i32 0
}

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local i8* @__cxa_begin_catch(i8*)

declare dso_local void @__cxa_end_catch()

attributes #0 = { noinline nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"branch_weights", i32 7, i32 0}

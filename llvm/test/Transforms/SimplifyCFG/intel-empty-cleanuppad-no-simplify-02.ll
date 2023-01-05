; Test verifies that SimplifyCFG doesn't remove empty cleanuppad
; BB (%ehcleanup) when the BB is the loop exit. %ehcleanup has
; multiple predecessors (%invoke.cont1, %invoke.cont, %for.body),
; which are in a loop.

; RUN: opt < %s -passes=simplifycfg -S -hoist-common-insts=true | FileCheck %s

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

; Check that ehcleanup block is not removed.
; CHECK: ehcleanup:
; CHECK:  %0 = cleanuppad within none []
; CHECK: cleanupret from %0 unwind label %catch.dispatch

%rtti.TypeDescriptor2 = type { i8**, i8*, [3 x i8] }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

$"??_R0H@8" = comdat any

@"??_7type_info@@6B@" = external constant i8*
@"??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { i8** @"??_7type_info@@6B@", i8* null, [3 x i8] c".H\00" }, comdat

; Function Attrs: noinline uwtable
define dso_local void @"?foo@@YAXXZ"() local_unnamed_addr personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %e = alloca i32, align 4
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %i.06 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  invoke void @"?bar@@YAXXZ"()
          to label %invoke.cont unwind label %ehcleanup

invoke.cont:                                      ; preds = %for.body
  invoke void @"?box@@YAXXZ"()
          to label %invoke.cont1 unwind label %ehcleanup

invoke.cont1:                                     ; preds = %invoke.cont
  invoke void @"?baz@@YAXXZ"()
          to label %for.inc unwind label %ehcleanup

for.inc:                                          ; preds = %invoke.cont1
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

ehcleanup:                                        ; preds = %invoke.cont1, %invoke.cont, %for.body
  %0 = cleanuppad within none []
  cleanupret from %0 unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %ehcleanup
  %1 = catchswitch within none [label %catch] unwind to caller

catch:                                            ; preds = %catch.dispatch
  %2 = catchpad within %1 [%rtti.TypeDescriptor2* @"??_R0H@8", i32 0, i32* %e]
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null) [ "funclet"(token %2) ]
  unreachable
}

declare dso_local void @"?bar@@YAXXZ"()

; Function Attrs: nofree
declare dso_local i32 @__CxxFrameHandler3(...)

declare dso_local void @"?box@@YAXXZ"()

declare dso_local void @"?baz@@YAXXZ"()

declare dso_local void @_CxxThrowException(i8*, %eh.ThrowInfo*)






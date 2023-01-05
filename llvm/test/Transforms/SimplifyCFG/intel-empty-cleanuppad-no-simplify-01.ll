; Test verifies that SimplifyCFG doesn't remove empty cleanuppad
; BB (%ehcleanup) when the BB is the loop exit. %ehcleanup has only
; one predecessor (%for.body), which is in loop.

; RUN: opt < %s -passes=simplifycfg -S | FileCheck %s

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%rtti.TypeDescriptor2 = type { i8**, i8*, [3 x i8] }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

; Check that ehcleanup block is not removed.
; CHECK: ehcleanup:
; CHECK:  %0 = cleanuppad within none []
; CHECK: cleanupret from %0 unwind label %catch.dispatch

$"??_R0H@8" = comdat any

@"??_7type_info@@6B@" = external constant i8*
@"??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { i8** @"??_7type_info@@6B@", i8* null, [3 x i8] c".H\00" }, comdat

define dso_local void @"?foo@@YAXXZ"() local_unnamed_addr #0 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %e = alloca i32, align 4
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %i.04 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  invoke void @"?bar@@YAXXZ"()
          to label %for.inc unwind label %ehcleanup

for.inc:                                          ; preds = %for.body
  %inc = add nuw nsw i32 %i.04, 1
  %exitcond = icmp eq i32 %inc, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

ehcleanup:                                        ; preds = %for.body
  %0 = cleanuppad within none []
  cleanupret from %0 unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %ehcleanup
  %1 = catchswitch within none [label %catch] unwind to caller

catch:                                            ; preds = %catch.dispatch
  %2 = catchpad within %1 [%rtti.TypeDescriptor2* @"??_R0H@8", i32 0, i32* %e]
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null) [ "funclet"(token %2) ]
  unreachable
}

declare dso_local void @"?bar@@YAXXZ"() local_unnamed_addr

; Function Attrs: nofree
declare dso_local i32 @__CxxFrameHandler3(...)

; Function Attrs: nofree
declare dso_local void @_CxxThrowException(i8*, %eh.ThrowInfo*)

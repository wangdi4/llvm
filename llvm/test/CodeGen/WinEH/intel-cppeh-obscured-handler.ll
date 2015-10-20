; RUN: opt -mtriple=x86_64-pc-windows-msvc -winehprepare -S -o - < %s | FileCheck %s

; This test is based on the following code:
;
; void f() {
;   try {
;     try {
;       may_throw();
;     } catch (...) { // f.catch.1
;       throw 1;
;     }
;   } catch (int x) { // f.catch
;     handle_exception(x);
;   }
;   do_something();
; }
;
; The purpose of the test is to verify that WinEHPrepare identifies the outer
; catch block as a potential target for the invoke of may_throw().  Although
; the outer catch handler is only reachable from the inner catch handler, it is
; necessary to identify the nesting properly in order for the EH table
; generation to compute the correct exception states.

; ModuleID = 'intel-cppeh-obscured-handler.cpp'
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc18.0.0"

%rtti.TypeDescriptor2 = type { i8**, i8*, [3 x i8] }
%eh.CatchHandlerType = type { i32, i8* }
%eh.CatchableType = type { i32, i32, i32, i32, i32, i32, i32 }
%eh.CatchableTypeArray.1 = type { i32, [1 x i32] }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

$"\01??_R0H@8" = comdat any

$"_CT??_R0H@84" = comdat any

$_CTA1H = comdat any

$_TI1H = comdat any

@"\01??_7type_info@@6B@" = external constant i8*
@"\01??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { i8** @"\01??_7type_info@@6B@", i8* null, [3 x i8] c".H\00" }, comdat
@llvm.eh.handlertype.H.0 = private unnamed_addr constant %eh.CatchHandlerType { i32 0, i8* bitcast (%rtti.TypeDescriptor2* @"\01??_R0H@8" to i8*) }, section "llvm.metadata"
@__ImageBase = external constant i8
@"_CT??_R0H@84" = linkonce_odr unnamed_addr constant %eh.CatchableType { i32 1, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor2* @"\01??_R0H@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 0, i32 -1, i32 0, i32 4, i32 0 }, section ".xdata", comdat
@_CTA1H = linkonce_odr unnamed_addr constant %eh.CatchableTypeArray.1 { i32 1, [1 x i32] [i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%eh.CatchableType* @"_CT??_R0H@84" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32)] }, section ".xdata", comdat
@_TI1H = linkonce_odr unnamed_addr constant %eh.ThrowInfo { i32 0, i32 0, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%eh.CatchableTypeArray.1* @_CTA1H to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, section ".xdata", comdat

; Function Attrs: uwtable
define void @"\01?f@@YAXXZ"() #0 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %exn.slot = alloca i8*
  %ehselector.slot = alloca i32
  %tmp = alloca i32, align 4
  %x = alloca i32, align 4
  invoke void @"\01?may_throw@@YAXXZ"()
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  br label %try.cont

lpad:                                             ; preds = %entry
  %0 = landingpad { i8*, i32 }
          catch i8* null
  %1 = extractvalue { i8*, i32 } %0, 0
  store i8* %1, i8** %exn.slot
  %2 = extractvalue { i8*, i32 } %0, 1
  store i32 %2, i32* %ehselector.slot
  br label %catch

catch:                                            ; preds = %lpad
  %exn = load i8*, i8** %exn.slot
  call void @llvm.eh.begincatch(i8* %exn, i8* null) #2
  store i32 1, i32* %tmp
  %3 = bitcast i32* %tmp to i8*
  invoke void @_CxxThrowException(i8* %3, %eh.ThrowInfo* @_TI1H) #4
          to label %unreachable unwind label %lpad.1

lpad.1:                                           ; preds = %catch
  %4 = landingpad { i8*, i32 }
          catch %eh.CatchHandlerType* @llvm.eh.handlertype.H.0
  %5 = extractvalue { i8*, i32 } %4, 0
  store i8* %5, i8** %exn.slot
  %6 = extractvalue { i8*, i32 } %4, 1
  store i32 %6, i32* %ehselector.slot
  br label %catch.dispatch

catch.dispatch:                                   ; preds = %lpad.1
  %sel = load i32, i32* %ehselector.slot
  %7 = call i32 @llvm.eh.typeid.for(i8* bitcast (%eh.CatchHandlerType* @llvm.eh.handlertype.H.0 to i8*)) #2
  %matches = icmp eq i32 %sel, %7
  br i1 %matches, label %catch.2, label %eh.resume

catch.2:                                          ; preds = %catch.dispatch
  %exn3 = load i8*, i8** %exn.slot
  %8 = bitcast i32* %x to i8*
  call void @llvm.eh.begincatch(i8* %exn3, i8* %8) #2
  %9 = load i32, i32* %x, align 4
  call void @"\01?handle_exception@@YAXH@Z"(i32 %9)
  call void @llvm.eh.endcatch() #2
  br label %try.cont.5

try.cont.5:                                       ; preds = %catch.2, %try.cont
  call void @"\01?do_something@@YAXXZ"()
  ret void

try.cont:                                         ; preds = %invoke.cont
  br label %try.cont.5

eh.resume:                                        ; preds = %catch.dispatch
  %exn.6 = load i8*, i8** %exn.slot
  %sel.7 = load i32, i32* %ehselector.slot
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.6, 0
  %lpad.val.8 = insertvalue { i8*, i32 } %lpad.val, i32 %sel.7, 1
  resume { i8*, i32 } %lpad.val.8

unreachable:                                      ; preds = %catch
  unreachable
}

; Expected form of prepared IR for f() with non-EH parts omitted.
;
; CHECK: define void @"\01?f@@YAXXZ"()
; CHECK: entry:
; CHECK:   [[TMP:\%.+]] = alloca i32, align 4
; CHECK:   [[X:\%.+]] = alloca i32, align 4
; CHECK:   call void (...) @llvm.localescape(i32* [[X]], i32* [[TMP]])
; CHECK:   invoke void @"\01?may_throw@@YAXXZ"()
; CHECK:           to label %invoke.cont unwind label %[[LPAD_LABEL:.+]]
;
; CHECK: [[LPAD_LABEL]]:
; CHECK:   landingpad { i8*, i32 }
; CHECK:                catch i8* null
; CHECK:   [[RECOVER:\%.+]] = call i8* (...) @llvm.eh.actions(i32 1, i8* null, i32 -1, i8* (i8*, i8*)* @"\01?f@@YAXXZ.catch.1", i32 1, i8* bitcast (%eh.CatchHandlerType* @llvm.eh.handlertype.H.0 to i8*), i32 0, i8* (i8*, i8*)* @"\01?f@@YAXXZ.catch")
; CHECK:   indirectbr i8* [[RECOVER]], [label %[[TRY_CONT_LABEL:.+]]]
;
; CHECK: [[TRY_CONT_LABEL]]:
; CHECK:   call void @"\01?do_something@@YAXXZ"()
; CHECK:   ret void
; CHECK: }

; Expected form of the outlined outer catch handler with stub blocks omitted.
;
; CHECK: define internal i8* @"\01?f@@YAXXZ.catch"(i8*, i8*)
; CHECK: entry:
; CHECK:   [[X_PTR:\%.+]] = call i8* @llvm.localrecover(i8* bitcast (void ()* @"\01?f@@YAXXZ" to i8*), i8* %1, i32 0)
; CHECK:   [[X1:\%.+]] = bitcast i8* [[X_PTR]] to i32*
; CHECK:   [[TMP_X:\%.+]] = load i32, i32* [[X1]], align 4
; CHECK:   call void @"\01?handle_exception@@YAXH@Z"(i32 [[TMP_X]])
; CHECK: }

; Expected form of the outlined inner catch handler.
;
; CHECK: define internal i8* @"\01?f@@YAXXZ.catch.1"(i8*, i8*)
; CHECK: entry:
; CHECK:   invoke void @_CxxThrowException
; CHECK:           to label %[[UNREACHABLE_LABEL:.+]] unwind label %[[LPAD1_LABEL:.+]]
;
; CHECK: [[LPAD1_LABEL]]:
; CHECK:   landingpad { i8*, i32 }
; CHECK:           catch %eh.CatchHandlerType* @llvm.eh.handlertype.H.0
; CHECK:   [[RECOVER1:\%.+]] = call i8* (...) @llvm.eh.actions(i32 1, i8* bitcast (%eh.CatchHandlerType* @llvm.eh.handlertype.H.0 to i8*), i32 0, i8* (i8*, i8*)* @"\01?f@@YAXXZ.catch")
;   Note that the indirectbr target list below is empty because the handler
;   does not return to any block in this function.
; CHECK:   indirectbr i8* [[RECOVER1]], []
;
; CHECK: [[UNREACHABLE_LABEL]]:
; CHECK:   unreachable
; CHECK: }

declare void @"\01?may_throw@@YAXXZ"() #1

declare i32 @__CxxFrameHandler3(...)

; Function Attrs: nounwind
declare void @llvm.eh.begincatch(i8* nocapture, i8* nocapture) #2

declare void @_CxxThrowException(i8*, %eh.ThrowInfo*)

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #3

declare void @"\01?handle_exception@@YAXH@Z"(i32) #1

; Function Attrs: nounwind
declare void @llvm.eh.endcatch() #2

declare void @"\01?do_something@@YAXXZ"() #1

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone }
attributes #4 = { noreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 3.8.0 (trunk 1200)"}

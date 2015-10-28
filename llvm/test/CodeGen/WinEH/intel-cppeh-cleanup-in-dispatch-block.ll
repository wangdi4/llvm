; RUN: opt -mtriple=x86_64-pc-windows-msvc -winehprepare -S -o - < %s | FileCheck %s

; ModuleID = 'cppeh15.cpp'
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc18.0.0"

; This tests a scenario where there is cleanup code in the block from which a
; catch handler is dispatched.  There was a bug in WinEHPrepare where this
; cleanup code was being inserted in both the catch handler and a cleanup
; handler.

define void @test() personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  invoke void @will_throw()
          to label %unreachable unwind label %lpad

lpad:                                             ; preds = %entry
  %lp = landingpad { i8*, i32 }
		  cleanup
          catch %eh.CatchHandlerType* @llvm.eh.handlertype.H.8
  %exn = extractvalue { i8*, i32 } %lp, 0
  %sel = extractvalue { i8*, i32 } %lp, 1
  call void @do_cleanup()
  %int.type = call i32 @llvm.eh.typeid.for(i8* bitcast (%eh.CatchHandlerType* @llvm.eh.handlertype.H.8 to i8*))
  %matches = icmp eq i32 %sel, %int.type
  br i1 %matches, label %catch, label %eh.resume

try.cont:                                         ; preds = %catch
  ret void

catch:                                            ; preds = %lpad
  call void @llvm.eh.begincatch(i8* %exn, i8* null)
  call void @handle_int()
  call void @llvm.eh.endcatch()
  br label %try.cont

eh.resume:                                        ; preds = %lpad
  resume { i8*, i32 } %lp

unreachable:                                      ; preds = %entry
  unreachable
}

; CHECK-LABEL: define internal void @test.cleanup(i8*, i8*)
; CHECK: entry:
; CHECK:   call void @do_cleanup()
; CHECK: }

; CHECK-LABEL: define internal i8* @test.catch(i8*, i8*)
; CHECK: entry:
; CHECK-NOT:   call void @do_cleanup()
; CHECK:   call void @handle_int()
; CHECK: }

%rtti.TypeDescriptor2 = type { i8**, i8*, [3 x i8] }
%eh.CatchHandlerType = type { i32, i8* }

$"\01??_R0H@8" = comdat any

@"\01??_7type_info@@6B@" = external constant i8*
@"\01??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { i8** @"\01??_7type_info@@6B@", i8* null, [3 x i8] c".H\00" }, comdat
@llvm.eh.handlertype.H.8 = private unnamed_addr constant %eh.CatchHandlerType { i32 8, i8* bitcast (%rtti.TypeDescriptor2* @"\01??_R0H@8" to i8*) }, section "llvm.metadata"


declare void @do_cleanup()
declare void @handle_int()
declare void @will_throw()

declare i32 @__CxxFrameHandler3(...)
declare i32 @llvm.eh.typeid.for(i8*)
declare void @llvm.eh.begincatch(i8* nocapture, i8* nocapture)
declare void @llvm.eh.endcatch()


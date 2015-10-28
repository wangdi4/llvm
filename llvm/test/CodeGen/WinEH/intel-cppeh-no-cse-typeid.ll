; RUN: opt -mtriple=x86_64-pc-windows-msvc -O2 -S -o - < %s | FileCheck %s

; This test is based on the following code:
;
; void test()
; {
;   try {
;     throw 1;
;   } catch(int) {}
;   try {
;     throw 2;
;   } catch(int) {}
; }
;
; The purpose of the test is to verify that the typeid intrinsic, which
; the WinEHPrepare pass relies upon for catch handler outlining, is not
; eliminated by optimization passes.
;
; The 'llvm.eh.typeid.for' intrinsic is defined as having no side effects
; and accessing no memory, because when compiling for non-MSVC targets we
; want this intrinsic to be optimized away when possible.  However, we can't
; allow that to happen for MSVC targets.
;
; The call to 'llvm.eh.typeid.for' in 'lpad.2' is dominated by the 'true'
; destination of the '%matches' compare in 'lpad' and so it is known that both
; '%sel' and '%typeid' hold the value that will be returned by
; 'llvm.eh.typeid.for' and stored as '%typeid2'.  Multiple optimization passes
; will want to eliminate this second call to the intrinsic as redundant, but
; they must be updated to check for WinEH intrinsic use.

; ModuleID = 'intel-cppeh-no-cse-typeid.cpp'
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc18.0.0"

; Function Attrs: uwtable
define void @"\01?test@@YAXXZ"() #0 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %tmp = alloca i32, align 4
  store i32 1, i32* %tmp, align 4
  %0 = bitcast i32* %tmp to i8*
  invoke void @_CxxThrowException(i8* %0, %eh.ThrowInfo* @_TI1H) #3
          to label %unreachable unwind label %lpad

; CHECK: lpad:
; CHECK:   [[LP:\%.+]] = landingpad { i8*, i32 }
; CHECK:   [[SEL:\%.+]] = extractvalue { i8*, i32 } [[LP]], 1
; CHECK:   [[TYPEID:\%.+]] = call i32 @llvm.eh.typeid.for(i8* bitcast (%eh.CatchHandlerType* @llvm.eh.handlertype.H.0 to i8*))
; CHECK:   [[MATCHES:\%.+]] = icmp eq i32 [[SEL]], [[TYPEID]]
lpad:                                             ; preds = %entry
  %lp = landingpad { i8*, i32 }
          catch %eh.CatchHandlerType* @llvm.eh.handlertype.H.0
  %exn = extractvalue { i8*, i32 } %lp, 0
  %sel = extractvalue { i8*, i32 } %lp, 1
  %typeid = call i32 @llvm.eh.typeid.for(i8* bitcast (%eh.CatchHandlerType* @llvm.eh.handlertype.H.0 to i8*)) #2
  %matches = icmp eq i32 %sel, %typeid
  br i1 %matches, label %catch, label %eh.resume

catch:                                            ; preds = %lpad
  call void @llvm.eh.begincatch(i8* %exn, i8* null) #2
  call void @llvm.eh.endcatch() #2
  br label %try.cont

try.cont:                                         ; preds = %catch
  store i32 2, i32* %tmp, align 4
  %1 = bitcast i32* %tmp to i8*
  invoke void @_CxxThrowException(i8* %1, %eh.ThrowInfo* @_TI1H) #3
          to label %unreachable unwind label %lpad.2

; CHECK: lpad.2:
; CHECK:   [[LP2:\%.+]] = landingpad { i8*, i32 }
; CHECK:   [[SEL2:\%.+]] = extractvalue { i8*, i32 } [[LP2]], 1
; CHECK:   [[TYPEID2:\%.+]] = call i32 @llvm.eh.typeid.for(i8* bitcast (%eh.CatchHandlerType* @llvm.eh.handlertype.H.0 to i8*))
; CHECK:   [[MATCHES2:\%.+]] = icmp eq i32 [[SEL2]], [[TYPEID2]]
lpad.2:                                           ; preds = %try.cont
  %lp2 = landingpad { i8*, i32 }
          catch i8* bitcast (%eh.CatchHandlerType* @llvm.eh.handlertype.H.0 to i8*)
  %exn2 = extractvalue { i8*, i32 } %lp2, 0
  %sel2 = extractvalue { i8*, i32 } %lp2, 1
  %typeid2 = call i32 @llvm.eh.typeid.for(i8* bitcast (%eh.CatchHandlerType* @llvm.eh.handlertype.H.0 to i8*)) #2
  %matches5 = icmp eq i32 %sel2, %typeid2
  br i1 %matches5, label %catch.6, label %eh.resume

catch.6:                                          ; preds = %lpad.2
  call void @llvm.eh.begincatch(i8* %exn2, i8* null) #2
  call void @llvm.eh.endcatch() #2
  br label %try.cont.8

try.cont.8:                                       ; preds = %catch.6
  ret void

eh.resume:                                        ; preds = %lpad.2, %lpad
  resume { i8*, i32 } undef

unreachable:                                      ; preds = %try.cont, %entry
  unreachable
}


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

declare void @_CxxThrowException(i8*, %eh.ThrowInfo*)

declare i32 @__CxxFrameHandler3(...)

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #1

; Function Attrs: nounwind
declare void @llvm.eh.begincatch(i8* nocapture, i8* nocapture) #2

; Function Attrs: nounwind
declare void @llvm.eh.endcatch() #2

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }
attributes #3 = { noreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 3.8.0 (trunk 1312)"}

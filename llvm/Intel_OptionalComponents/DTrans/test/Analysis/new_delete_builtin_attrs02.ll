; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -debug-only=dtransanalysis -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtransanalysis -disable-output 2>&1 | FileCheck %s

; This test checks that user-defined function allocate() is not recognized as
; AK_UserMalloc function since its first parameter is used inside the function.

; CHECK: Analyzing for MallocPostDom _ZN10MemManager8allocateEl
; CHECK-NOT: Is MallocPostDom _ZN10MemManager8allocateEl

%class.MemManager = type { i32 (...)** }

$__clang_call_terminate = comdat any

$_ZTS2Ex = comdat any

$_ZTI2Ex = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS2Ex = linkonce_odr dso_local constant [4 x i8] c"2Ex\00", comdat, align 1
@_ZTI2Ex = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @_ZTS2Ex, i32 0, i32 0) }, comdat, align 8

define dso_local noalias nonnull i8* @_ZN10MemManager8allocateEl(%class.MemManager* nocapture %this, i64 %size) unnamed_addr #0 align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %bc = bitcast %class.MemManager* %this to i8*
; invoke instruction has implicit 'nobuiltin' attribute.
  %call = invoke i8* @_Znwm(i64 %size)
          to label %if.then unwind label %lpad

lpad:                                             ; preds = %entry
  %0 = landingpad { i8*, i32 }
          catch i8* null
  %1 = extractvalue { i8*, i32 } %0, 0
  %2 = tail call i8* @__cxa_begin_catch(i8* %1)
  %exception = tail call i8* @__cxa_allocate_exception(i64 1)
  invoke void @__cxa_throw(i8* nonnull %exception, i8* bitcast ({ i8*, i8* }* @_ZTI2Ex to i8*), i8* null)
          to label %unreachable unwind label %lpad2

lpad2:                                            ; preds = %lpad
  %3 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %invoke.cont3 unwind label %terminate.lpad

invoke.cont3:                                     ; preds = %lpad2
  %exn.obj = extractvalue { i8*, i32 } %3, 0
  call void @_Unwind_Resume(i8* %exn.obj)
  unreachable

if.then:                                          ; preds = %entry
  ret i8* %call

terminate.lpad:                                   ; preds = %lpad2
  %4 = landingpad { i8*, i32 }
          catch i8* null
  %5 = extractvalue { i8*, i32 } %4, 0
  tail call void @__clang_call_terminate(i8* %5)
  unreachable

unreachable:                                      ; preds = %lpad
  unreachable
}

declare dso_local noalias nonnull i8* @_Znwm(i64) local_unnamed_addr #1

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr

declare dso_local noalias nonnull i8* @__cxa_allocate_exception(i64) local_unnamed_addr

declare dso_local void @__cxa_throw(i8*, i8*, i8*) local_unnamed_addr

declare dso_local void @__cxa_end_catch() local_unnamed_addr

define linkonce_odr hidden void @__clang_call_terminate(i8*) local_unnamed_addr comdat {
  %2 = tail call i8* @__cxa_begin_catch(i8* %0)
  tail call void @_ZSt9terminatev()
  unreachable
}

declare dso_local void @_ZSt9terminatev() local_unnamed_addr

declare void @_Unwind_Resume(i8*)

attributes #1 = { nobuiltin }


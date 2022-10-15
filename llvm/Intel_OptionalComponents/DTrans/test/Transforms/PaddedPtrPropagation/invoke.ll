; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -internalize -disable-output -padded-pointer-prop -padded-pointer-info < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="internalize,padded-pointer-prop" < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; The test checks if padding is propagated through the InvokeInst

;CHECK:      ==== INITIAL FUNCTION SET ====
;CHECK:      Function info(callee):
;CHECK:        HasUnknownCallSites: 0
;CHECK:        Return Padding: -1
;CHECK:        Value paddings:
;CHECK-NEXT:     %2 = tail call i32* @llvm.ptr.annotation.p0i32
;CHECK-SAME:          {{:: 32$}}
;CHECK:      ==== END OF INITIAL FUNCTION SET ====
;CHECK:      ==== TRANSFORMED FUNCTION SET ====
;CHECK:      Function info(callee):
;CHECK:        HasUnknownCallSites: 0
;CHECK:        Return Padding: 32
;CHECK:        Value paddings:
;CHECK-NEXT:     %2 = tail call i32* @llvm.ptr.annotation.p0i32
;CHECK-SAME:          {{:: 32$}}
;CHECK:      Function info(caller):
;CHECK:        HasUnknownCallSites: 0
;CHECK:        Return Padding: -1
;CHECK:        Value paddings:
;CHECK-NEXT:     %call = invoke i32* @callee()
;CHECK-NEXT:             to label
;CHECK-SAME:             {{:: 32$}}
;CHECK:      ==== END OF TRANSFORMED FUNCTION SET ====

; C code
; extern int* IP;

; extern "C" int* callee() {
;     if (IP)
;         throw 0;
;     return __builtin_intel_padded(IP, 32);
; }

; extern "C" int* caller(void) {
;     try {
;         int* p = callee();
;         return p;
;     }
;     catch(...) {
;         return IP;
;     }
; }

@IP = external global i32*
@_ZTIi = external constant i8*
@.str = private constant [13 x i8] c"return_4.cpp\00"
@0 = private constant [16 x i8] c"padded 32 bytes\00"

define i32* @callee() {
entry:
  %0 = load i32*, i32** @IP
  %tobool = icmp eq i32* %0, null
  br i1 %tobool, label %if.end, label %if.then

if.then:
  %exception = tail call i8* @__cxa_allocate_exception(i64 4)
  %1 = bitcast i8* %exception to i32*
  store i32 0, i32* %1
  tail call void @__cxa_throw(i8* %exception, i8* bitcast (i8** @_ZTIi to i8*), i8* null)
  unreachable

if.end:
  %2 = tail call i32* @llvm.ptr.annotation.p0i32(i32* null, i8* getelementptr ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32 6, i8* null)
  ret i32* %2
}

define i32* @caller() personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = invoke i32* @callee()
          to label %try.cont unwind label %lpad

lpad:
  %0 = landingpad { i8*, i32 }
          catch i8* null
  %1 = extractvalue { i8*, i32 } %0, 0
  %2 = tail call i8* @__cxa_begin_catch(i8* %1)
  %3 = load i32*, i32** @IP
  tail call void @__cxa_end_catch()
  br label %try.cont

try.cont:
  %retval.0 = phi i32* [ %3, %lpad ], [ %call, %entry ]
  ret i32* %retval.0
}

declare i8* @__cxa_allocate_exception(i64)
declare void @__cxa_throw(i8*, i8*, i8*)
declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)
declare i8* @__cxa_begin_catch(i8*)
declare void @__cxa_end_catch()
declare i32 @__gxx_personality_v0(...)



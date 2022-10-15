; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -internalize -disable-output -padded-pointer-prop -padded-pointer-info < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="internalize,padded-pointer-prop" < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Checks propagation of the return padding to the caller
; C code
; extern int* IP;
;
; int* callee() {
;   return __builtin_intel_padded(IP, 32);
; }
;
; int* caller(void) {
;   int* p = callee();
;   return p;
; }

;CHECK: ==== INITIAL FUNCTION SET ====
;CHECK:      Function info(callee):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Return Padding: -1
;CHECK-NEXT:   Value paddings:
;CHECK-NEXT:      %1 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %0, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 4, i8* null) :: 32
;CHECK: ==== END OF INITIAL FUNCTION SET ====

;CHECK: ==== TRANSFORMED FUNCTION SET ====
;CHECK:      Function info(callee):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Return Padding: 32
;CHECK-NEXT:   Value paddings:
;CHECK-NEXT:      %1 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %0, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 4, i8* null) :: 32

;CHECK:      Function info(caller):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Return Padding: 32
;CHECK-NEXT:   Value paddings:
;CHECK: ==== END OF TRANSFORMED FUNCTION SET ====


@IP = external global i32*
@0 = private constant [16 x i8] c"padded 32 bytes\00"
@.str = private constant [11 x i8] c"return_3.c\00", section "llvm.metadata"

define i32* @callee() {
entry:
  %0 = load i32*, i32** @IP
  %1 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %0, i8* getelementptr ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 4, i8* null)
  ret i32* %1
}

define i32* @caller() {
entry:
  %call = tail call i32* @callee()
  ret i32* %call
}

declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)




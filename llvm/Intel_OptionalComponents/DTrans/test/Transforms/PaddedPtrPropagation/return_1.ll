; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -internalize -disable-output -padded-pointer-prop -padded-pointer-info < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="internalize,padded-pointer-prop" < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Checks that return value is propagated to FuncInfo

;CHECK:      ==== INITIAL FUNCTION SET ====
;CHECK:      Function info(foo):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Return Padding: -1
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:     i32* %p : -1
;CHECK:      ==== END OF INITIAL FUNCTION SET ====

;CHECK:      ==== TRANSFORMED FUNCTION SET ====
;CHECK:      Function info(foo):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Return Padding: 4
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:     i32* %p : -1
;CHECK:      ==== END OF TRANSFORMED FUNCTION SET ====

@0 = private unnamed_addr constant [15 x i8] c"padded 4 bytes\00"
@.str = private unnamed_addr constant [16 x i8] c"test_return.cpp\00"

define i32* @foo(i32* %p) {
  %1 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str, i64 0, i64 0), i32 2, i8* null)
  ret i32* %1
}

declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)


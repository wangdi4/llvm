; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="internalize,padded-pointer-prop" < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Checks padding propagation for simple recursion case
; C code
; int* foo(int* P) {
;   return __builtin_intel_padded(foo(P), 32);
; }

;CHECK:      ==== INITIAL FUNCTION SET ====
;CHECK:      Function info(foo):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Return Padding: -1
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:     i32* %P : -1
;CHECK-NEXT:   Value paddings:
;CHECK-NEXT:     %0 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %call, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i64 0, i64 0), i32 2, i8* null) :: 32
;CHECK:      ==== END OF INITIAL FUNCTION SET ====

;CHECK:      ==== TRANSFORMED FUNCTION SET ====
;CHECK:      Function info(foo):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Return Padding: 32
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:     i32* %P : -1
;CHECK-NEXT:   Value paddings:
;CHECK-NEXT:      %0 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %call, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i64 0, i64 0), i32 2, i8* null) :: 32
;CHECK-NEXT:      %call = tail call i32* @foo(i32* %P) :: 32
;CHECK:      ==== END OF TRANSFORMED FUNCTION SET ====

@0 = private constant [16 x i8] c"padded 32 bytes\00"
@.str = private constant [14 x i8] c"recursion_1.c\00", section "llvm.metadata"

define i32* @foo(i32* %P) {
entry:
  %call = tail call i32* @foo(i32* %P)
  %0 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %call, i8* getelementptr ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr ([14 x i8], [14 x i8]* @.str, i64 0, i64 0), i32 2, i8* null)
  ret i32* %0
}

declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)




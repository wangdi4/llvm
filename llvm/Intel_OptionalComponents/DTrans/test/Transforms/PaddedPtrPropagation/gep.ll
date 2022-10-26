; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="internalize,padded-pointer-prop" < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Checks padding propagation for GetElementPtr

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
;CHECK-NEXT:   Return Padding: 16
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:     i32* %p : -1
;CHECK-NEXT:   Value paddings:
;CHECK:          %add.ptr = getelementptr inbounds i32, i32* %0, i64 1 :: 16
;CHECK:      ==== END OF TRANSFORMED FUNCTION SET ====

@.str = private unnamed_addr constant [6 x i8] c"gep.c\00", section "llvm.metadata"
@0 = private unnamed_addr constant [16 x i8] c"padded 16 bytes\00"

define i32* @foo(i32* %p) {
entry:
  %0 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %p, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), i32 2, i8* null)
  %add.ptr = getelementptr inbounds i32, i32* %0, i64 1
  ret i32* %add.ptr
}

declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)


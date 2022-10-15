; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -internalize -disable-output -padded-pointer-prop -padded-pointer-info < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="internalize,padded-pointer-prop" < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Checks merging of padding for ReturnInst

;CHECK:       ==== INITIAL FUNCTION SET ====
;CHECK:       Function info(foo):
;CHECK-NEXT:    HasUnknownCallSites: 0
;CHECK-NEXT:    Return Padding: -1
;CHECK-NEXT:    Arguments' Padding:
;CHECK-NEXT:      i32* %p : -1
;CHECK:       ==== END OF INITIAL FUNCTION SET ====

;CHECK:       ==== TRANSFORMED FUNCTION SET ====
;CHECK:       Function info(foo):
;CHECK-NEXT:    HasUnknownCallSites: 0
;CHECK-NEXT:    Return Padding: 1
;CHECK-NEXT:    Arguments' Padding:
;CHECK-NEXT:      i32* %p : -1
;CHECK:       ==== END OF TRANSFORMED FUNCTION SET ====

@0 = private unnamed_addr constant [15 x i8] c"padded 1 bytes\00"
@.str = private unnamed_addr constant [18 x i8] c"test_return_2.cpp\00", section "llvm.metadata"
@1 = private unnamed_addr constant [15 x i8] c"padded 2 bytes\00"
@2 = private unnamed_addr constant [15 x i8] c"padded 3 bytes\00"
@3 = private unnamed_addr constant [15 x i8] c"padded 4 bytes\00"
@4 = private unnamed_addr constant [15 x i8] c"padded 4 bytes\00"

define i32* @foo(i32* %p, i32 %i) {
entry:
  switch i32 %i, label %sw.epilog [
    i32 1, label %sw.bb
    i32 2, label %sw.bb1
    i32 3, label %sw.bb2
    i32 4, label %sw.bb3
  ]

sw.bb:                                            ; preds = %entry
  %0 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @2, i64 0, i64 0), i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i64 0, i64 0), i32 8, i8* null)
  ret i32* %0

sw.bb1:                                           ; preds = %entry
  %1 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @1, i64 0, i64 0), i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i64 0, i64 0), i32 6, i8* null)
  ret i32* %1

sw.bb2:                                           ; preds = %entry
  %2 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i64 0, i64 0), i32 4, i8* null)
  ret i32* %2

sw.bb3:                                           ; preds = %entry
  %3 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @3, i64 0, i64 0), i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i64 0, i64 0), i32 10, i8* null)
  ret i32* %3

sw.epilog:                                        ; preds = %entry
  %4 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @4, i64 0, i64 0), i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i64 0, i64 0), i32 12, i8* null)
   ret i32* %4
}

declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)

; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -internalize -disable-output -padded-pointer-prop -padded-pointer-info < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="internalize,padded-pointer-prop" < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Checks merging of padding for SelectInst

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
;CHECK-NEXT:   Return Padding: 1
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:     i32* %p : -1
;CHECK-NEXT:   Value paddings:
;CHECK:           %4 = select i1 %cmp, i32* %0, i32* %1 :: 4
;CHECK-NEXT:      %5 = select i1 %cmp1, i32* %1, i32* %2 :: 1
;CHECK-NEXT:      %6 = select i1 %tobool, i32* %5, i32* %4 :: 1
;CHECK:      ==== END OF TRANSFORMED FUNCTION SET ====

@0 = private unnamed_addr constant [15 x i8] c"padded 4 bytes\00"
@.str = private unnamed_addr constant [9 x i8] c"select.c\00"
@1 = private unnamed_addr constant [15 x i8] c"padded 8 bytes\00"
@2 = private unnamed_addr constant [15 x i8] c"padded 1 bytes\00"

define i32* @foo(i32* %p) {
entry:
  %0 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i32 2, i8* null)
  %1 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @1, i64 0, i64 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i32 3, i8* null)
  %2 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @2, i64 0, i64 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i32 4, i8* null)
  %3 = load i32, i32* %p, align 4
  %cmp = icmp eq i32 %3, 0
  %4 = select i1 %cmp, i32* %0, i32* %1
  %cmp1 = icmp eq i32 %3, 1
  %5 = select i1 %cmp1, i32* %1, i32* %2
  %tobool = icmp eq i32* %p, null
  %6 = select i1 %tobool, i32* %5, i32* %4
  ret i32* %6
}

declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*) #1


; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="internalize,padded-pointer-prop" < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Checks correct processing of implicit call conversions
; C code
;
; extern int* PTR;
;
; int* baz() {
; // Since bar was not declared yet it assumed to have default
; // prototype this may lead to errors in processing
;  return bar(__builtin_intel_padded(PTR, 8));
; }
;
; static int* bar(int* P) {
;   return __builtin_intel_padded(bar(P), 16);
; }

;CHECK:      ==== INITIAL FUNCTION SET ====
;CHECK:      Function info(bar):
;CHECK-NEXT:   HasUnknownCallSites: 1
;CHECK-NEXT:   Return Padding: -1
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:     i32* %P : 0
;CHECK-NEXT:   Value paddings:
;CHECK:      i32* %P :: 0
;CHECK:      %0 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %call, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @1, i64 0, i64 0), i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i64 0, i64 0), i32 8, i8* null) :: 16
;CHECK:      Function info(baz):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Return Padding: -1
;CHECK-NEXT:   Value paddings:
;CHECK-NEXT:     %1 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %0, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i64 0, i64 0), i32 4, i8* null) :: 8
;CHECK:      ==== END OF INITIAL FUNCTION SET ====

;CHECK:      ==== TRANSFORMED FUNCTION SET ====
;CHECK:      Function info(bar):
;CHECK-NEXT:   HasUnknownCallSites: 1
;CHECK-NEXT:   Return Padding: 16
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:     i32* %P : 0
;CHECK-NEXT:   Value paddings:
;CHECK-NEXT:      i32* %P :: 0
;CHECK-NEXT:      %0 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %call, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @1, i64 0, i64 0), i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i64 0, i64 0), i32 8, i8* null) :: 16
;CHECK:      Function info(baz):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Return Padding: -1
;CHECK-NEXT:   Value paddings:
;CHECK-NEXT:     %1 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %0, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i64 0, i64 0), i32 4, i8* null) :: 8
;CHECK:      ==== END OF TRANSFORMED FUNCTION SET ====


@PTR = external global i32*
@0 = private constant [15 x i8] c"padded 8 bytes\00"
@.str = private constant [14 x i8] c"recursion_2.c\00", section "llvm.metadata"
@1 = private constant [16 x i8] c"padded 16 bytes\00"

define i32* @baz() {
entry:
  %0 = load i32*, i32** @PTR
  %1 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %0, i8* getelementptr ([15 x i8], [15 x i8]* @0, i64 0, i64 0), i8* getelementptr ([14 x i8], [14 x i8]* @.str, i64 0, i64 0), i32 4, i8* null)
  %call = tail call i32 (i32*, ...) bitcast (i32* (i32*)* @bar to i32 (i32*, ...)*)(i32* %1)
  %conv = sext i32 %call to i64
  %2 = inttoptr i64 %conv to i32*
  ret i32* %2
}

define i32* @bar(i32* %P) {
entry:
  %call = tail call i32* @bar(i32* %P)
  %0 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %call, i8* getelementptr ([16 x i8], [16 x i8]* @1, i64 0, i64 0), i8* getelementptr ([14 x i8], [14 x i8]* @.str, i64 0, i64 0), i32 8, i8* null)
  ret i32* %0
}

declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)




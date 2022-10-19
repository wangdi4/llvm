; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -internalize -disable-output -padded-pointer-prop -padded-pointer-info < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="internalize,padded-pointer-prop" < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Checks merging of padding for PHINode

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
;CHECK-NEXT:   Return Padding: 2
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:     i32* %p : -1
;CHECK-NEXT:   Value paddings:
;CHECK:          %x.0 = phi i32* [ %5, %sw.default ], [ %4, %sw.bb3 ], [ %3, %sw.bb2 ], [ %2, %sw.bb1 ], [ %1, %sw.bb ] :: 2
;CHECK:      ==== END OF TRANSFORMED FUNCTION SET ====

@0 = private unnamed_addr constant [15 x i8] c"padded 6 bytes\00"
@1 = private unnamed_addr constant [15 x i8] c"padded 5 bytes\00"
@2 = private unnamed_addr constant [15 x i8] c"padded 3 bytes\00"
@3 = private unnamed_addr constant [15 x i8] c"padded 4 bytes\00"
@4 = private unnamed_addr constant [15 x i8] c"padded 2 bytes\00"
@.str = private unnamed_addr constant [6 x i8] c"phi.c\00"

define i32* @foo(i32* %p)  {
entry:
  %0 = load i32, i32* %p
  switch i32 %0, label %sw.default [
    i32 2, label %sw.bb
    i32 3, label %sw.bb1
    i32 4, label %sw.bb2
    i32 5, label %sw.bb3
  ]

sw.bb:
  %1 = tail call i32* @llvm.ptr.annotation.p0i32(i32* nonnull %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), i32 5, i8* null)
  br label %sw.epilog

sw.bb1:
  %2 = tail call i32* @llvm.ptr.annotation.p0i32(i32* nonnull %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @1, i64 0, i64 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), i32 8, i8* null)
  br label %sw.epilog

sw.bb2:
  %3 = tail call i32* @llvm.ptr.annotation.p0i32(i32* nonnull %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @4, i64 0, i64 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), i32 11, i8* null)
  br label %sw.epilog

sw.bb3:
  %4 = tail call i32* @llvm.ptr.annotation.p0i32(i32* nonnull %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @2, i64 0, i64 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), i32 14, i8* null)
  br label %sw.epilog

sw.default:
  %5 = tail call i32* @llvm.ptr.annotation.p0i32(i32* nonnull %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @3, i64 0, i64 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), i32 17, i8* null)
  br label %sw.epilog

sw.epilog:
  %x.0 = phi i32* [ %5, %sw.default ], [ %4, %sw.bb3 ], [ %3, %sw.bb2 ], [ %2, %sw.bb1 ], [ %1, %sw.bb ]
  %x.010 = call i32* @llvm.ptr.annotation.p0i32(i32* %x.0, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @4, i64 0, i64 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), i32 0, i8* null)
  ret i32* %x.010
}

declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)

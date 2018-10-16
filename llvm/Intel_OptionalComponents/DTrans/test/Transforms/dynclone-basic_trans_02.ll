; This test verifies that basic transformations are done correctly for
; Sub/MemFunc instructions when DynClone transformation is triggered.

;  RUN: opt < %s -S -whole-program-assume -dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -S -whole-program-assume -passes=dtrans-dynclone 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i32.
; Size of Original %struct.test.01: 48
; Size after transformation: 30
;
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; CHECK-LABEL:   define internal void @proc1()

; This routine has basic instructions to transform. proc1 will be
; cloned but none of the instructions in cloned routine is transformed.
define void @proc1() {

; Fixed alloc size
  %call1 = tail call noalias i8* @calloc(i64 8, i64 48)
; CHECK: %call1 = tail call noalias i8* @calloc(i64 8, i64 30)

  %tp1 = bitcast i8* %call1 to %struct.test.01*

; Fixed alloc size
  %call2 = tail call noalias i8* @malloc(i64 480)
; CHECK: %call2 = tail call noalias i8* @malloc(i64 300)

  %tp2 = bitcast i8* %call2 to %struct.test.01*

; Fixed alloc size
  %call3 = tail call noalias i8* @realloc(i8* %call2, i64 48)
; CHECK: %call3 = tail call noalias i8* @realloc(i8* %call2, i64 30)

  %tp3 = bitcast i8* %call3 to %struct.test.01*

; Fixed memset size
  call void @llvm.memset.p0i8.i64(i8* %call3, i8 0, i64 48, i1 false)
; CHECK: call void @llvm.memset.p0i8.i64(i8* %call3, i8 0, i64 30, i1 false)

; Fixed memcpy size
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 48, i1 false)
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 30, i1 false)

; Fixed memmove size
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 48, i1 false)
; CHECK: call void @llvm.memmove.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 30, i1 false)

  %p1 = ptrtoint %struct.test.01* %tp1 to i64
  %p2 = ptrtoint %struct.test.01* %tp2 to i64
  %diff1 = sub i64  %p1, %p2

; Fixed size
  %num1 = sdiv i64 %diff1, 48
; CHECK: %num1 = sdiv i64 %diff1, 30

; Fixed size
  %num2 = udiv i64 %diff1, 48
; CHECK: %num2 = udiv i64 %diff1, 30

  ret void
}

; Make sure none of instructions is transformed in cloned
; routine.
; CHECK-LABEL:   define internal void @proc1{{.*}}
; CHECK: %call1 = tail call noalias i8* @calloc(i64 8, i64 48)
; CHECK: %call2 = tail call noalias i8* @malloc(i64 480)
; CHECK: %call3 = tail call noalias i8* @realloc(i8* %call2, i64 48)
; CHECK: call void @llvm.memset.p0i8.i64(i8* %call3, i8 0, i64 48, i1 false)
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 48, i1 false)
; CHECK: call void @llvm.memmove.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 48, i1 false)
; CHECK: %num1 = sdiv i64 %diff1, 48
; CHECK: %num2 = udiv i64 %diff1, 48

; This routine is selected as InitRoutine.
define void @init() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  call void @init();
;  call void @proc1();
  ret i32 0
}
; Function Attrs: nounwind
declare noalias i8* @calloc(i64, i64)
declare noalias i8* @malloc(i64)
declare noalias i8* @realloc(i8*, i64)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memmove.p0i8.p0i8.i64(i8* , i8*, i64, i1)

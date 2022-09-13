; This test verifies that basic transformations are done correctly for
; Sub/MemFunc instructions when DynClone transformation is triggered.

;  RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dyncloneop 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i16.
; Size of Original %struct.test.01: 48
; Size after transformation: 26
;
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; CHECK-LABEL:   define internal void @proc1()

; This routine has basic instructions to transform. proc1 will be
; cloned but none of the instructions in cloned routine is transformed.
define void @proc1() {

; Fixed alloc size
  %call1 = tail call i8* @calloc(i64 8, i64 48)
; CHECK: %call1 = tail call i8* @calloc(i64 8, i64 26)

  %tp1 = bitcast i8* %call1 to %struct.test.01*

; Assignment for candidate fields dependancy.
  %I1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %L = load i64, i64* %I1
  %I6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  store i64 %L, i64* %I6

; Fixed alloc size
  %call2 = tail call i8* @malloc(i64 480)
; CHECK: %call2 = tail call i8* @malloc(i64 260)

  %tp2 = bitcast i8* %call2 to %struct.test.01*
  %J1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  %K = load i64, i64* %J1
  %J6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 %K, i64* %J6

; Fixed alloc size
  %call3 = tail call i8* @realloc(i8* %call2, i64 48)
; CHECK: %call3 = tail call i8* @realloc(i8* %call2, i64 26)

  %tp3 = bitcast i8* %call3 to %struct.test.01*
  %M1 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 1
  %N = load i64, i64* %M1
  %M6 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 6
  store i64 %N, i64* %M6

; Fixed memset size
  call void @llvm.memset.p0i8.i64(i8* %call3, i8 0, i64 48, i1 false)
; CHECK: call void @llvm.memset.p0i8.i64(i8* %call3, i8 0, i64 26, i1 false)

; Fixed memcpy size
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 48, i1 false)
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 26, i1 false)

; Fixed memmove size
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 48, i1 false)
; CHECK: call void @llvm.memmove.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 26, i1 false)

  %p1 = ptrtoint %struct.test.01* %tp1 to i64
  %p2 = ptrtoint %struct.test.01* %tp2 to i64
  %diff1 = sub i64  %p1, %p2

; Fixed size
  %num1 = sdiv i64 %diff1, 48
; CHECK: %num1 = sdiv i64 %diff1, 26

; Fixed size
  %num2 = udiv i64 %diff1, 48
; CHECK: %num2 = udiv i64 %diff1, 26

  ret void
}

; Make sure none of instructions is transformed in cloned
; routine.
; CHECK-LABEL:   define internal void @proc1{{.*}}
; CHECK: %call1 = tail call i8* @calloc(i64 8, i64 48)
; CHECK: %call2 = tail call i8* @malloc(i64 480)
; CHECK: %call3 = tail call i8* @realloc(i8* %call2, i64 48)
; CHECK: call void @llvm.memset.p0i8.i64(i8* %call3, i8 0, i64 48, i1 false)
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 48, i1 false)
; CHECK: call void @llvm.memmove.p0i8.p0i8.i64(i8* %call2, i8* %call3, i64 48, i1 false)
; CHECK: %num1 = sdiv i64 %diff1, 48
; CHECK: %num2 = udiv i64 %diff1, 48

; This routine is selected as InitRoutine.
define void @init() {
  %call1 = tail call i8* @calloc(i64 10, i64 48)
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
declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)
declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @malloc(i64)
declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" i8* @realloc(i8* "intel_dtrans_func_index"="2", i64)
declare !intel.dtrans.func.type !9 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)
declare !intel.dtrans.func.type !10 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)
declare !intel.dtrans.func.type !11 void @llvm.memmove.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1" , i8* "intel_dtrans_func_index"="2", i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = distinct !{!5, !5}
!9 = distinct !{!5}
!10 = distinct !{!5, !5}
!11 = distinct !{!5, !5}
!12 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2} ; { i32, i64, i32, i32, i16, i64*, i64 }

!intel.dtrans.types = !{!12}

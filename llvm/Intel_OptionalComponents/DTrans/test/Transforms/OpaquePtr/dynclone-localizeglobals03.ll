; This test checks that the GlobalVariables @opt is not split during dynamic
; cloning because it appears in a non-cloned function that does not meet the
; special extra clone function criterion. Also that @myglobal does not get
; split because it does not appear in a cloned function.

;  RUN: opt < %s -dtransop-allow-typed-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -dtrans-dyncloneop 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i32 }

; CHECK: @opt = dso_local local_unnamed_addr global i64 0, align 8
; CHECK-NOT: @opt.[[S1:[0-9]+]] = global i64 0
; CHECK: @myglobal = dso_local local_unnamed_addr global i64 0, align 8
; CHECK-NOT: @myglobal.[[S1:[0-9]+]] = global i64 0

; CHECK: define void @init()
; CHECK: define i32 @main()
; CHECK: define void @proc1()
; CHECK: define void @proc2()
; CHECK: define void @proc3()
; CHECK: define void @proc1.[[S2:[0-9]+]]()
; CHECK: define void @proc2.[[S3:[0-9]+]]()
; CHECK-NOT: define void @proc3.[[S4:[0-9]+]]()
; CHECK-NOT: define void @proc4.[[S4:[0-9]+]]()

; This routine is selected as InitRoutine. Runtime checks are generated
; in this routine.
define void @init() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
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
  call void @proc1();
  ret i32 0
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  %L = load i64, i64* %F6
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  store i64 %L, i64* %F1
  call void @proc2();
  %t0 = load i64, i64* @opt, align 8
  %add = add nsw i64 %t0, 54
  store i64 %add, i64* @opt, align 8
  call void @proc2();
  ret void
}

define void @proc2() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  %L = load i64, i64* %F6
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  store i64 %L, i64* %F1
  %t0 = load i64, i64* @opt, align 8
  %add = add nsw i64 %t0, 54
  store i64 %add, i64* @opt, align 8
  call void @proc3();
  ret void
}

define void @proc3() {
  %t0 = load i64, i64* @myglobal, align 8
  %add = add nsw i64 %t0, 54
  store i64 %add, i64* @myglobal, align 8
  call void @proc4();
  ret void
}

define void @proc4() {
  %t0 = load i64, i64* @opt, align 8
  %add = add nsw i64 %t0, 54
  store i64 %add, i64* @opt, align 8
  ret void
}

@opt = dso_local local_unnamed_addr global i64 0, align 8
@myglobal = dso_local local_unnamed_addr global i64 0, align 8

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !1} ; { i32, i64, i32, i32, i16, i64*, i64, i32 }

!intel.dtrans.types = !{!7}

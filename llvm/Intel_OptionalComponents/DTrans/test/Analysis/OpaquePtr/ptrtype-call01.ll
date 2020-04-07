; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery for "call" instructions.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test call to metadata annotated routine for getting result type.
%struct.test01a = type { i32, i32 }
%struct.test01 = type { %struct.test01a* }
define internal %struct.test01a* @test01get(%struct.test01* %in) !dtrans_type !4 {
  %f = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %v = load %struct.test01a*, %struct.test01a** %f
  ret %struct.test01a* %v
}

define internal void @test01() {
  %struct1 = alloca %struct.test01
  %struct2 = call %struct.test01a* @test01get(%struct.test01* %struct1)
  ret void
}
; CHECK-LABEL: void @test01()
; CHECK-CUR:   %struct2 = call %struct.test01a* @test01get(%struct.test01* %struct1)
; CHECK-FUT:   %struct2 = call p0 @test01get(p0 %struct1)
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test01a*
; CHECK-NEXT: No element pointees.


; Test call to intrinsic involving pointers
%struct.test02 = type { i64, i64 }
define internal void @test02() {
  %struct = alloca [2 x %struct.test02]
  %mem = bitcast [2 x %struct.test02]* %struct to i8*
  call void @llvm.memset.p0i8.i64(i8* %mem, i8 0, i64 32, i1 false)
  ret void
}
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
; In this case, the memset call does not return pointer information, but the
; call should trigger the result of the bitcast instruction to be seen as
; an i8*.

; CHECK-LABEL: void @test02()
; CHECK-CUR:  %mem = bitcast [2 x %struct.test02]* %struct to i8*
; CHECK-FUT:  %mem = bitcast p0 %struct to p0
; CHECK:      Aliased types:
; CHECK-NEXT:   [2 x %struct.test02]*
; CHECK-NEXT:   i8*
; CHECK-NEXT: No element pointees.


; Test call to library function which has special case handling.
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
@test03name = private constant [8 x i8] c"foo.txt\00"
@test03mode = private constant [2 x i8] c"r\00"
define internal void @test03() {
  %name = getelementptr [8 x i8], [8 x i8]* @test03name, i64 0, i32 0
  %mode = getelementptr [2 x i8], [2 x i8]* @test03mode, i64 0, i64 0
  %handle = call %struct._IO_FILE* @fopen(i8* %name, i8* %mode)
  ret void
}
declare %struct._IO_FILE* @fopen(i8*, i8*)
; CHECK-LABEL: void @test03()
; CHECK-CUR:  %handle = call %struct._IO_FILE* @fopen(i8* %name, i8* %mode)
; CHECK-FUT:  %handle = call p0 @fopen(p0 %name, p0 %mode)
; CHECK:      Aliased types:
; CHECK-NEXT:    %struct._IO_FILE*
; CHECK-NEXT:  No element pointees.


; Test with indirect call involving metadata
define internal void @test04(i8* (i8*)* %f) !dtrans_type !17 {
%c = alloca i8
%r = call i8* %f(i8* %c), !dtrans_type !19
ret void
}
; CHECK-CUR: %r = call i8* %f(i8* %c)
; CHECK-FUT: %r = call p0 %f(p0 %c)
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i8*
; CHECK-NEXT: No element pointees.


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!3, i32 1}  ; %struct.test01a*
!3 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!4 = !{!"F", i1 false, i32 1, !2, !5}  ; %struct.test01a* (%struct.test01*)
!5 = !{!6, i32 1}  ; %struct.test01*
!6 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!7 = !{i64 0, i32 0}  ; i64
!8 = !{i8 0, i32 1}  ; i8*
!9 = !{i8 0, i32 0}  ; i8
!10 = !{!11, i32 1}  ; %struct._IO_marker*
!11 = !{!"R", %struct._IO_marker zeroinitializer, i32 0}  ; %struct._IO_marker
!12 = !{!13, i32 1}  ; %struct._IO_FILE*
!13 = !{!"R", %struct._IO_FILE zeroinitializer, i32 0}  ; %struct._IO_FILE
!14 = !{i16 0, i32 0}  ; i16
!15 = !{!"A", i32 1, !9}  ; [1 x i8]
!16 = !{!"A", i32 20, !9}  ; [20 x i8]
!17 = !{!"F", i1 false, i32 1, !18, !8}  ; void (i8*)
!18 = !{!"void", i32 0}  ; void
!19 = !{!"F", i1 false, i32 1, !8, !8}  ; i8 (i8*)
!20 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!21 = !{!"S", %struct.test01 zeroinitializer, i32 1, !2} ; { %struct.test01a* }
!22 = !{!"S", %struct.test02 zeroinitializer, i32 2, !7, !7} ; { i64, i64 }
!23 = !{!"S", %struct._IO_FILE zeroinitializer, i32 29, !1, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !8, !10, !12, !1, !1, !7, !14, !9, !15, !8, !7, !8, !8, !8, !8, !7, !1, !16} ; { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
!24 = !{!"S", %struct._IO_marker zeroinitializer, i32 3, !10, !12, !1} ; { %struct._IO_marker*, %struct._IO_FILE*, i32 }

!dtrans_types = !{!20, !21, !22, !23, !24}

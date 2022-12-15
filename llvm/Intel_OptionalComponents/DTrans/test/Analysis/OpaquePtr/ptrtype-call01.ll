; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery for "call" instructions.


; Test call to metadata annotated routine for getting result type.
%struct.test01a = type { i32, i32 }
%struct.test01 = type { ptr }
define internal "intel_dtrans_func_index"="1" ptr @test01get(ptr "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type !4 {
  %f = getelementptr %struct.test01, ptr %in, i64 0, i32 0
  %v = load ptr, ptr %f
  ret ptr %v
}

define internal void @test01() {
  %struct1 = alloca %struct.test01
  %struct2 = call ptr @test01get(ptr %struct1)
  ret void
}
; CHECK-LABEL: void @test01()
; CHECK:   %struct2 = call ptr @test01get(ptr %struct1)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01a*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test call to intrinsic involving pointers
%struct.test02 = type { i64, i64 }
define internal void @test02() {
  %struct = alloca [2 x %struct.test02]
  %mem = bitcast ptr %struct to ptr
  call void @llvm.memset.p0i8.i64(ptr %mem, i8 0, i64 32, i1 false)
  ret void
}
declare !intel.dtrans.func.type !7 void @llvm.memset.p0i8.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)
; In this case, the memset call does not return pointer information, but the
; call should trigger the result of the bitcast instruction to be seen as
; an i8*.

; CHECK-LABEL: void @test02()
; CHECK:  %mem = bitcast ptr %struct to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   [2 x %struct.test02]*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test call to library function which has special case handling.
%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type { ptr, ptr, i32 }
@test03name = private constant [8 x i8] c"foo.txt\00"
@test03mode = private constant [2 x i8] c"r\00"
define internal void @test03() {
  %name = getelementptr [8 x i8], ptr @test03name, i64 0, i32 0
  %mode = getelementptr [2 x i8], ptr @test03mode, i64 0, i64 0
  %handle = call ptr @fopen(ptr %name, ptr %mode)
  ret void
}
declare !intel.dtrans.func.type !14 "intel_dtrans_func_index"="1" ptr @fopen(ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3")
; CHECK-LABEL: void @test03()
; CHECK:  %handle = call ptr @fopen(ptr %name, ptr %mode)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:    %struct._IO_FILE*{{ *$}}
; CHECK-NEXT:  No element pointees.


; Test with indirect call involving metadata
define internal void @test04(ptr "intel_dtrans_func_index"="1" %f) !intel.dtrans.func.type !17 {
%c = alloca i8
%r = call ptr %f(ptr %c), !intel_dtrans_type !15
ret void
}

; CHECK: ; intel.dtrans.func.type = %struct._IO_FILE* (i8*, i8*)
; CHECK: define internal void @test04
; CHECK: !intel_dtrans_type = i8* (i8*)
; CHECK: %r = call ptr %f(ptr %c)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!2, !3}
!5 = !{i64 0, i32 0}  ; i64
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{%struct._IO_marker zeroinitializer, i32 1}  ; %struct._IO_marker*
!9 = !{%struct._IO_FILE zeroinitializer, i32 1}  ; %struct._IO_FILE*
!10 = !{i16 0, i32 0}  ; i16
!11 = !{i8 0, i32 0}  ; i8
!12 = !{!"A", i32 1, !11}  ; [1 x i8]
!13 = !{!"A", i32 20, !11}  ; [20 x i8]
!14 = distinct !{!9, !6, !6}
!15 = !{!"F", i1 false, i32 1, !6, !6}  ; i8* (i8*)
!16 = !{!15, i32 1}  ; i8* (i8*)*
!17 = distinct !{!16}
!18 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!19 = !{!"S", %struct.test01 zeroinitializer, i32 1, !2} ; { %struct.test01a* }
!20 = !{!"S", %struct.test02 zeroinitializer, i32 2, !5, !5} ; { i64, i64 }
!21 = !{!"S", %struct._IO_FILE zeroinitializer, i32 29, !1, !6, !6, !6, !6, !6, !6, !6, !6, !6, !6, !6, !8, !9, !1, !1, !5, !10, !11, !12, !6, !5, !6, !6, !6, !6, !5, !1, !13} ; { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
!22 = !{!"S", %struct._IO_marker zeroinitializer, i32 3, !8, !9, !1} ; { %struct._IO_marker*, %struct._IO_FILE*, i32 }

!intel.dtrans.types = !{!18, !19, !20, !21, !22}

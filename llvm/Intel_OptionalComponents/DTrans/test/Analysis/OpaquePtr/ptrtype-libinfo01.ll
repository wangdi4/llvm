; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Check that the DTrans PtrTypeAnalyzer does not set UNHANDLED on pointers used
; for library functions that are declared without DTrans metadata attached to
; them because an internal lookup table can be used to resolve their types.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS8_IO_FILE._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._ZTS10_IO_marker._IO_marker*, %struct._ZTS8_IO_FILE._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._ZTS11_IO_codecvt._IO_codecvt*, %struct._ZTS13_IO_wide_data._IO_wide_data*, %struct._ZTS8_IO_FILE._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._ZTS10_IO_marker._IO_marker = type opaque
%struct._ZTS11_IO_codecvt._IO_codecvt = type opaque
%struct._ZTS13_IO_wide_data._IO_wide_data = type opaque

%struct.ident_t = type { i32, i32, i32, i32, i8* }

@.source.0.0.694 = private constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private global %struct.ident_t  { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.694, i32 0, i32 0) }
; CHECK: @.kmpc_loc.0.0 = private global %struct.ident_t
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

@.gomp_critical_user.var = internal global [8 x i32] zeroinitializer
; CHECK: @.gomp_critical_user.var = internal global [8 x i32]
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

declare !intel.dtrans.func.type !12 void @fn(i8* "intel_dtrans_func_index"="1")

define void @test_libfunc() {
  %p8.1 = alloca i8
; CHECK: %p8.1 = alloca i8
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %p8.2 = alloca i8
; CHECK: %p8.2 = alloca i8
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %p8.3 = alloca i8
; CHECK: %p8.3 = alloca i8
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %p32.1 = alloca i32
; CHECK: %p32.1 = alloca i32
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %p64.1 = alloca i64
; CHECK: %p64.1 = alloca i64
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %p64.2 = alloca i64
; CHECK: %p64.2 = alloca i64
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %p64.3 = alloca i64
; CHECK: %p64.3 = alloca i64
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %pFile = alloca %struct._ZTS8_IO_FILE._IO_FILE
; CHECK: %pFile = alloca %struct._ZTS8_IO_FILE._IO_FILE
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %bcmp = call i32 @bcmp(i8* %p8.1, i8* %p8.2, i64 0)
  %calloc = call i8* @calloc(i64 10, i64 10)
  call void @__clang_call_terminate(i8* %p8.1)
  %cxa_atexit = call i32 @__cxa_atexit(void(i8*)* @fn, i8* %p8.1, i8* %p8.2)
  %cxa_allocate_exception = call i8* @__cxa_allocate_exception(i64 0)
  %cxa_begin_catch = call i8* @__cxa_begin_catch(i8* %p8.1)
; CHECK: %cxa_begin_catch = call
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  call void @__cxa_free_exception(i8* %p8.1)
  call void @__cxa_throw(i8* %p8.1, i8* %p8.2, i8* %p8.3)
  %frwite = call i64 @fwrite(i8* %p8.1, i64 0, i64 0, %struct._ZTS8_IO_FILE._IO_FILE* %pFile)
  %fputc = call i32 @fputc(i32 0, %struct._ZTS8_IO_FILE._IO_FILE* %pFile)
  %memchr = call i8* @memchr(i8* %p8.1, i32 0, i64 0)
; CHECK: %memchr = call
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %puts = call i32 @puts(i8* %p8.1)
  %stpcpy = call i8* @stpcpy(i8* %p8.1, i8* %p8.2)
; CHECK: %stpcpy = call
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  call void @__kmpc_barrier(%struct.ident_t* @.kmpc_loc.0.0, i32 0)
  call void @__kmpc_critical(%struct.ident_t* @.kmpc_loc.0.0, i32 0, [8 x i32]* @.gomp_critical_user.var)
  call void @__kmpc_end_critical(%struct.ident_t* @.kmpc_loc.0.0, i32 0, [8 x i32]* @.gomp_critical_user.var)
  call void @__kmpc_end_masked(%struct.ident_t* @.kmpc_loc.0.0, i32 0)
  call void @__kmpc_end_single(%struct.ident_t* @.kmpc_loc.0.0, i32 0)
  call void @__kmpc_for_static_init_8(%struct.ident_t* @.kmpc_loc.0.0, i32 0, i32 0, i32* %p32.1, i64* %p64.1, i64* %p64.2, i64* %p64.3, i64 0, i64 0)
  call void @__kmpc_for_static_init_8u(%struct.ident_t* @.kmpc_loc.0.0, i32 0, i32 0, i32* %p32.1, i64* %p64.1, i64* %p64.2, i64* %p64.3, i64 0, i64 0)
  call void @__kmpc_for_static_fini(%struct.ident_t* @.kmpc_loc.0.0, i32 0)
  %kmpc_global_thread_num = call i32 @__kmpc_global_thread_num(%struct.ident_t* @.kmpc_loc.0.0)
  %kmpc_masked = call i32 @__kmpc_masked(%struct.ident_t* @.kmpc_loc.0.0, i32 0, i32 0)
  call void @__kmpc_push_num_threads(%struct.ident_t* @.kmpc_loc.0.0, i32 0, i32 0)
  %kmpc_single = call i32 @__kmpc_single(%struct.ident_t* @.kmpc_loc.0.0, i32 0)

  ret void
}

; Library functions declared without DTrans metadata.
declare i32 @bcmp(i8*, i8*, i64)
declare i8* @calloc(i64, i64)
declare void @__clang_call_terminate(i8*)
declare i32 @__cxa_atexit(void(i8*)*, i8*, i8*)
declare i8* @__cxa_allocate_exception(i64)
declare i8* @__cxa_begin_catch(i8*)
declare void @__cxa_free_exception(i8*)
declare void @__cxa_throw(i8*, i8*, i8*)
declare i64 @fwrite(i8*, i64, i64, %struct._ZTS8_IO_FILE._IO_FILE*)
declare i32 @fputc(i32, %struct._ZTS8_IO_FILE._IO_FILE*)
declare i8* @memchr(i8*, i32, i64)
declare i32 @puts(i8*)
declare i8* @stpcpy(i8*, i8*)

declare void @__kmpc_barrier(%struct.ident_t*, i32)
declare void @__kmpc_critical(%struct.ident_t*, i32, [8 x i32]*)
declare void @__kmpc_end_critical(%struct.ident_t*, i32, [8 x i32]*)
declare void @__kmpc_end_masked(%struct.ident_t*, i32)
declare void @__kmpc_end_single(%struct.ident_t*, i32)
declare void @__kmpc_for_static_init_8(%struct.ident_t*, i32, i32, i32*, i64*, i64*, i64*, i64, i64)
declare void @__kmpc_for_static_init_8u(%struct.ident_t*, i32, i32, i32*, i64*, i64*, i64*, i64, i64)
declare void @__kmpc_for_static_fini(%struct.ident_t*, i32)
declare i32 @__kmpc_global_thread_num(%struct.ident_t*)
declare i32 @__kmpc_masked(%struct.ident_t*, i32, i32)
declare void @__kmpc_push_num_threads(%struct.ident_t*, i32, i32)
declare i32 @__kmpc_single(%struct.ident_t*, i32)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{%struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 1}  ; %struct._ZTS10_IO_marker._IO_marker*
!4 = !{%struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 1}  ; %struct._ZTS8_IO_FILE._IO_FILE*
!5 = !{i64 0, i32 0}  ; i64
!6 = !{i16 0, i32 0}  ; i16
!7 = !{i8 0, i32 0}  ; i8
!8 = !{!"A", i32 1, !7}  ; [1 x i8]
!9 = !{%struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 1}  ; %struct._ZTS11_IO_codecvt._IO_codecvt*
!10 = !{%struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 1}  ; %struct._ZTS13_IO_wide_data._IO_wide_data*
!11 = !{!"A", i32 20, !7}  ; [20 x i8]
!12 = distinct !{!2}
!13 = !{!"S", %struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 29, !1, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !3, !4, !1, !1, !5, !6, !7, !8, !2, !5, !9, !10, !4, !2, !5, !1, !11} ; { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._ZTS10_IO_marker._IO_marker*, %struct._ZTS8_IO_FILE._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._ZTS11_IO_codecvt._IO_codecvt*, %struct._ZTS13_IO_wide_data._IO_wide_data*, %struct._ZTS8_IO_FILE._IO_FILE*, i8*, i64, i32, [20 x i8] }
!14 = !{!"S", %struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 0} ; opaque
!15 = !{!"S", %struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 0} ; opaque
!16 = !{!"S", %struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 0} ; opaque
!17 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !1, !1, !1, !1, !2} ; { i32, i32, i32, i32, i8* }

!intel.dtrans.types = !{!13, !14, !15, !16, !17}

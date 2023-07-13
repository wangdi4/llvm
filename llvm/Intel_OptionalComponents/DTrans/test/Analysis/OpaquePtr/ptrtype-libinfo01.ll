; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Check that the DTrans PtrTypeAnalyzer does not set UNHANDLED on pointers used
; for library functions that are declared without DTrans metadata attached to
; them because an internal lookup table can be used to resolve their types.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS8_IO_FILE._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._ZTS10_IO_marker._IO_marker = type opaque
%struct._ZTS11_IO_codecvt._IO_codecvt = type opaque
%struct._ZTS13_IO_wide_data._IO_wide_data = type opaque

%struct.ident_t = type { i32, i32, i32, i32, ptr }

@.source.0.0.694 = private constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private global %struct.ident_t  { i32 0, i32 838860802, i32 0, i32 0, ptr getelementptr inbounds ([22 x i8], ptr @.source.0.0.694, i32 0, i32 0) }
; CHECK: @.kmpc_loc.0.0 = private global %struct.ident_t
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

@.gomp_critical_user.var = internal global [8 x i32] zeroinitializer
; CHECK: @.gomp_critical_user.var = internal global [8 x i32]
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

declare !intel.dtrans.func.type !12 void @fn(ptr "intel_dtrans_func_index"="1")

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

  %bcmp = call i32 @bcmp(ptr %p8.1, ptr %p8.2, i64 0)
  %calloc = call ptr @calloc(i64 10, i64 10)
  call void @__clang_call_terminate(ptr %p8.1)
  %cxa_atexit = call i32 @__cxa_atexit(ptr @fn, ptr %p8.1, ptr %p8.2)
  %cxa_allocate_exception = call ptr @__cxa_allocate_exception(i64 0)
  %cxa_begin_catch = call ptr @__cxa_begin_catch(ptr %p8.1)
; CHECK: %cxa_begin_catch = call
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  call void @__cxa_free_exception(ptr %p8.1)
  call void @__cxa_throw(ptr %p8.1, ptr %p8.2, ptr %p8.3)
  %frwite = call i64 @fwrite(ptr %p8.1, i64 0, i64 0, ptr %pFile)
  %fputc = call i32 @fputc(i32 0, ptr %pFile)
  %memchr = call ptr @memchr(ptr %p8.1, i32 0, i64 0)
; CHECK: %memchr = call
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %puts = call i32 @puts(ptr %p8.1)
  %stpcpy = call ptr @stpcpy(ptr %p8.1, ptr %p8.2)
; CHECK: %stpcpy = call
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  call void @__kmpc_barrier(ptr @.kmpc_loc.0.0, i32 0)
  call void @__kmpc_critical(ptr @.kmpc_loc.0.0, i32 0, ptr @.gomp_critical_user.var)
  call void @__kmpc_end_critical(ptr @.kmpc_loc.0.0, i32 0, ptr @.gomp_critical_user.var)
  call void @__kmpc_end_masked(ptr @.kmpc_loc.0.0, i32 0)
  call void @__kmpc_end_single(ptr @.kmpc_loc.0.0, i32 0)
  call void @__kmpc_for_static_init_8(ptr @.kmpc_loc.0.0, i32 0, i32 0, ptr %p32.1, ptr %p64.1, ptr %p64.2, ptr %p64.3, i64 0, i64 0)
  call void @__kmpc_for_static_init_8u(ptr @.kmpc_loc.0.0, i32 0, i32 0, ptr %p32.1, ptr %p64.1, ptr %p64.2, ptr %p64.3, i64 0, i64 0)
  call void @__kmpc_for_static_fini(ptr @.kmpc_loc.0.0, i32 0)
  %kmpc_global_thread_num = call i32 @__kmpc_global_thread_num(ptr @.kmpc_loc.0.0)
  %kmpc_masked = call i32 @__kmpc_masked(ptr @.kmpc_loc.0.0, i32 0, i32 0)
  call void @__kmpc_push_num_threads(ptr @.kmpc_loc.0.0, i32 0, i32 0)
  %kmpc_single = call i32 @__kmpc_single(ptr @.kmpc_loc.0.0, i32 0)

  ret void
}

; Library functions declared without DTrans metadata.
declare i32 @bcmp(ptr, ptr, i64)
declare ptr @calloc(i64, i64)
declare void @__clang_call_terminate(ptr)
declare i32 @__cxa_atexit(ptr, ptr, ptr)
declare ptr @__cxa_allocate_exception(i64)
declare ptr @__cxa_begin_catch(ptr)
declare void @__cxa_free_exception(ptr)
declare void @__cxa_throw(ptr, ptr, ptr)
declare i64 @fwrite(ptr, i64, i64, ptr)
declare i32 @fputc(i32, ptr)
declare ptr @memchr(ptr, i32, i64)
declare i32 @puts(ptr)
declare ptr @stpcpy(ptr, ptr)

declare void @__kmpc_barrier(ptr, i32)
declare void @__kmpc_critical(ptr, i32, ptr)
declare void @__kmpc_end_critical(ptr, i32, ptr)
declare void @__kmpc_end_masked(ptr, i32)
declare void @__kmpc_end_single(ptr, i32)
declare void @__kmpc_for_static_init_8(ptr, i32, i32, ptr, ptr, ptr, ptr, i64, i64)
declare void @__kmpc_for_static_init_8u(ptr, i32, i32, ptr, ptr, ptr, ptr, i64, i64)
declare void @__kmpc_for_static_fini(ptr, i32)
declare i32 @__kmpc_global_thread_num(ptr)
declare i32 @__kmpc_masked(ptr, i32, i32)
declare void @__kmpc_push_num_threads(ptr, i32, i32)
declare i32 @__kmpc_single(ptr, i32)

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

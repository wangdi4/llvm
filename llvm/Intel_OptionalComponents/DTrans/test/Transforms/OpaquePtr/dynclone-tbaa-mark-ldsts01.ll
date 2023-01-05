; This test verifies that !tbaa tags on load(s) and store(s) from a user routine
; will be carried into its corresponding dyn-clone transformed user routine.

;  RUN: opt < %s -opaque-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64 }
%struct.ns = type { ptr, ptr, ptr, ptr }

; Memory allocation pointers, which are returned by calloc in "init" routine,
; are stored in 2nd and 3rd array fields of @n.
@n = internal global %struct.ns zeroinitializer, align 8

; AOSTOSOA allocation call is marked with this annotation.
@__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"

; AOSTOSOA allocation call is marked with this annotation.
@__intel_dtrans_aostosoa_index = private constant [41 x i8] c"{dtrans} AOS-to-SOA peeling index {id:0}\00"

; This routine has accesses to 2nd and 3rd fields of %struct.test.01, which
; are marked as aostosoa index fields.
; CHECK: define internal void @proc1()

define void @proc1() {
  %call1 = tail call ptr @calloc(i64 10, i64 48)
  %F2 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 2
  %A2 = call ptr @llvm.ptr.annotation.p0i32(ptr %F2, ptr getelementptr inbounds ([41 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr null, i32 0, ptr null)

  %L1 = load i32, ptr %F2, !tbaa !0
; CHECK: [[LD1:%[0-9]+]] = load i16, ptr %F2, align 2, !tbaa ![[TBAA_LD:[0-9]+]]
;                                                      ^^^^^ tbaa is carried on load
; CHECK: %L1 = zext i16 [[LD1]] to i32

  %F3 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 3
  %A3 = call ptr @llvm.ptr.annotation.p0i32(ptr %F3, ptr getelementptr inbounds ([41 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr null, i32 0, ptr null)

  store i32 0, ptr %F3, !tbaa !4
; CHECK: [[TRUNC1:%[0-9]+]] = trunc i32 0 to i16
; CHECK: store i16 [[TRUNC1]], ptr %F3, align 2, !tbaa ![[TBAA_ST:[0-9]+]]
;                                                ^^^^^ tbaa is carried on store

  %I6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
  ret void
}

; This routine is selected as InitRoutine.
define "intel_dtrans_func_index"="1" ptr @init() !intel.dtrans.func.type !13 {
  %call0 = call ptr @calloc(i64 1000, i64 32)
  %call.ptr = call ptr @llvm.ptr.annotation.p0i8(ptr %call0, ptr getelementptr inbounds ([38 x i8], ptr @__intel_dtrans_aostosoa_alloc, i32 0, i32 0), ptr null, i32 0, ptr null)
  %C01 = getelementptr i8, ptr %call0, i64 0
  store ptr %C01, ptr getelementptr (%struct.ns, ptr @n, i64 0, i32 0)
  %call1 = tail call ptr @calloc(i64 10, i64 48)
  %tp2 = getelementptr %struct.test.01, ptr %call1, i64 2

; Below instructions are needed to select this routine as InitRoutine and
; increase field frequency of struct.test.01.
  %F1 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, ptr %F6, align 8

  ret ptr null
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  %tp2 = call ptr @init();
  call void @proc1();
  ret i32 0
}

declare !intel.dtrans.func.type !15 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0
declare ptr @llvm.ptr.annotation.p0i8(ptr, ptr, ptr, i32, ptr)
declare ptr @llvm.ptr.annotation.p0i32(ptr, ptr, ptr, i32, ptr)

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{!5, !6, i64 0}
!5 = !{!"struct@ns", !6, i64 0, !6, i64 8}
!6 = !{!"pointer@_ZTSP10struct.test.01", !2, i64 0}

!7 = !{i32 0, i32 0}  ; i32
!8 = !{i64 0, i32 0}  ; i64
!9 = !{i16 0, i32 0}  ; i16
!10 = !{i64 0, i32 1}  ; i64*
!11 = !{%struct.test.01 zeroinitializer, i32 2}  ; %struct.test.01**
!12 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!13 = distinct !{!12}
!14 = !{i8 0, i32 1}  ; i8*
!15 = distinct !{!14}
!16 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !7, !8, !7, !7, !9, !10, !8} ; { i32, i64, i32, i32, i16, i64*, i64 }
!17 = !{!"S", %struct.ns zeroinitializer, i32 4, !10, !11, !11, !10} ; { i64*, %struct.test.01**, %struct.test.01**, i64* }

!intel.dtrans.types = !{!16, !17}

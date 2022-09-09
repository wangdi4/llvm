; This test verifies that !tbaa tags on load(s) and store(s) from a user routine
; will be carried into its corresponding dyn-clone transformed user routine.

; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dynclone 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dynclone' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }
%struct.ns = type { i64*, %struct.test.01**, %struct.test.01**, i64* }

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
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F2 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 2
  %A2 = call i32* @llvm.ptr.annotation.p0i32(i32* %F2, i8* getelementptr inbounds ([41 x i8], [41 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* null, i32 0, i8* null)

  %L1 = load i32, i32* %F2, !tbaa !0
; CHECK: [[BC1:%[0-9]+]] = bitcast i32* %F2 to i16*
; CHECK: [[LD1:%[0-9]+]] = load i16, i16* [[BC1]], align 2, !tbaa !0
;                                                           ^^^^^ tbaa is carried on load
; CHECK: %L1 = zext i16 [[LD1]] to i32

  %F3 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 3
  %A3 = call i32* @llvm.ptr.annotation.p0i32(i32* %F3, i8* getelementptr inbounds ([41 x i8], [41 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* null, i32 0, i8* null)

  store i32 0, i32* %F3, !tbaa !4
; CHECK: [[TRUNC1:%[0-9]+]] = trunc i32 0 to i16
; CHECK: [[BC2:%[0-9]+]] = bitcast i32* %F3 to i16*
; CHECK: store i16 [[TRUNC1]], i16* [[BC2]], align 2, !tbaa !4
;                                                     ^^^^^ tbaa is carried on store

  %I6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  ret void
}

; This routine is selected as InitRoutine.
define %struct.test.01* @init() {
  %call0 = call noalias i8* @calloc(i64 1000, i64 32)
  %call.ptr = call i8* @llvm.ptr.annotation.p0i8(i8* %call0, i8* getelementptr inbounds ([38 x i8], [38 x i8]* @__intel_dtrans_aostosoa_alloc, i32 0, i32 0), i8* null, i32 0, i8* null)
  %C01 = getelementptr i8, i8* %call0, i64 0
  %C02 = bitcast i8* %C01 to i64*
  store i64* %C02, i64** getelementptr (%struct.ns, %struct.ns* @n, i64 0, i32 0)
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %tp2 = getelementptr %struct.test.01, %struct.test.01* %tp1, i64 2

; Below instructions are needed to select this routine as InitRoutine and
; increase field frequency of struct.test.01.
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8

  ret %struct.test.01* null
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  %tp2 = call %struct.test.01* @init();
  call void @proc1();
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare dso_local noalias i8* @llvm.ptr.annotation.p0i8(i8*, i8*, i8*, i32, i8*)
declare dso_local noalias i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{!5, !6, i64 0}
!5 = !{!"struct@ns", !6, i64 0, !6, i64 8}
!6 = !{!"pointer@_ZTSP10struct.test.01", !2, i64 0}

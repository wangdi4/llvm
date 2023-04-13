; REQUIRES: asserts
; RUN: opt -opaque-pointers=0 < %s -disable-output -debug-only=dopevectorconstprop -passes=dopevectorconstprop -dope-vector-local-const-prop  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; This test case checks that dope vector constant propagation did not
; happen for assumed shape arrays that are using inside local functions.
; The reason is that the allocation site wasn't found. This is the same
; test case as local_dvcp04.ll, but it checks the IR. It was created from
; the following example, but the calls to @for_alloc_allocatable_handle
; were removed.

; subroutine assumedshape_arrs()
;   implicit none
;
;   integer, parameter :: NX = 10, NY = 64, NZ = 128
;   real, allocatable :: array_source(:,:,:), array_target(:,:,:), array_add(:,:,:)
;   integer :: i, j, k
;
;   allocate(array_source(NX,NY,NZ), array_target(NX,NY,NZ), array_add(NX,NY,NZ))
;
;   do k=1,NZ
;     do j=1,NY
;       do i=1,NX
;         array_target(i,j,k) = array_source(i,j,k) + array_add(i,j,k)
;       end do
;     end do
;   end do
;
;   deallocate(array_source, array_target, array_add)
;
; end subroutine assumedshape_arrs

; ifx -c -O3 -fiopenmp -xCORE-AVX512 -fpp -traceback -flto -align array64byte -what -V simple.F90 -mllmv -dope-vector-local-const-prop

; CHECK: LOCAL DV FOUND:   %"assumedshape_arrs_$ARRAY_ADD" = alloca %"QNCA_a0$float*$rank3$", align 8
; CHECK:   RANK: 3
; CHECK:   TYPE: float
; CHECK:   ANALYSIS RESULT: NOT VALID
; CHECK: LOCAL DV FOUND:   %"assumedshape_arrs_$ARRAY_TARGET" = alloca %"QNCA_a0$float*$rank3$", align 8
; CHECK:   RANK: 3
; CHECK:   TYPE: float
; CHECK:   ANALYSIS RESULT: NOT VALID
; CHECK: LOCAL DV FOUND:   %"assumedshape_arrs_$ARRAY_SOURCE" = alloca %"QNCA_a0$float*$rank3$", align 8
; CHECK:   RANK: 3
; CHECK:   TYPE: float
; CHECK:   ANALYSIS RESULT: NOT VALID

; ModuleID = '/tmp/ifxDtVxlK.i90'
source_filename = "/tmp/ifxDtVxlK.i90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank3$" = type { float*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

; Function Attrs: nofree nounwind uwtable
define void @assumedshape_arrs_() local_unnamed_addr #0 {
alloca_0:
  %"assumedshape_arrs_$ARRAY_ADD" = alloca %"QNCA_a0$float*$rank3$"
  %"assumedshape_arrs_$ARRAY_TARGET" = alloca %"QNCA_a0$float*$rank3$"
  %"assumedshape_arrs_$ARRAY_SOURCE" = alloca %"QNCA_a0$float*$rank3$"
  %fetch.1.fca.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 0
  %fetch.1.fca.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 1
  %fetch.1.fca.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 2
  %fetch.1.fca.3.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 3
  %0 = bitcast %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_ADD" to i64*
  store i64 0, i64* %0
  %fetch.1.fca.4.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 4
  %fetch.1.fca.5.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 5
  %fetch.1.fca.6.0.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 0
  %fetch.1.fca.6.0.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 1
  %fetch.1.fca.6.0.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 2
  %fetch.2.fca.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 0
  %1 = getelementptr inbounds i64, i64* %fetch.1.fca.5.gep, i64 1
  %2 = bitcast i64* %1 to i8*
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(72) %2, i8 0, i64 72, i1 false)
  %fetch.2.fca.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 1
  %fetch.2.fca.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 2
  %fetch.2.fca.3.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 3
  %3 = bitcast %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_TARGET" to i64*
  store i64 0, i64* %3
  %fetch.2.fca.4.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 4
  %fetch.2.fca.5.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 5
  %fetch.2.fca.6.0.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 0
  %fetch.2.fca.6.0.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 1
  %fetch.2.fca.6.0.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 2
  %fetch.3.fca.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 0
  %4 = getelementptr inbounds i64, i64* %fetch.2.fca.5.gep, i64 1
  %5 = bitcast i64* %4 to i8*
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(72) %5, i8 0, i64 72, i1 false)
  %fetch.3.fca.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 1
  %fetch.3.fca.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 2
  %fetch.3.fca.3.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 3
  %6 = bitcast %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_SOURCE" to i64*
  store i64 0, i64* %6
  %fetch.3.fca.4.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 4
  %fetch.3.fca.5.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 5
  %fetch.3.fca.6.0.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 0
  %fetch.3.fca.6.0.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 1
  %fetch.3.fca.6.0.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 2
  %7 = getelementptr inbounds i64, i64* %fetch.3.fca.5.gep, i64 1
  %8 = bitcast i64* %7 to i8*
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(72) %8, i8 0, i64 72, i1 false)
  store i64 0, i64* %fetch.3.fca.5.gep
  store i64 4, i64* %fetch.3.fca.1.gep
  store i64 3, i64* %fetch.3.fca.4.gep
  store i64 0, i64* %fetch.3.fca.2.gep
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]153" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.3.fca.6.0.2.gep, i32 0)
  store i64 1, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]153"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.3.fca.6.0.0.gep, i32 0)
  store i64 10, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]157" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.3.fca.6.0.2.gep, i32 1)
  store i64 1, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]157"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]160" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.3.fca.6.0.0.gep, i32 1)
  store i64 64, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]160"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]163" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.3.fca.6.0.2.gep, i32 2)
  store i64 1, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]163"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]166" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.3.fca.6.0.0.gep, i32 2)
  store i64 128, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]166"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]169" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.3.fca.6.0.1.gep, i32 0)
  store i64 4, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]169"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]172" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.3.fca.6.0.1.gep, i32 1)
  store i64 40, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]172"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]175" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.3.fca.6.0.1.gep, i32 2)
  store i64 2560, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]175"
  store i64 1610612869, i64* %fetch.3.fca.3.gep
  %"(i8**)assumedshape_arrs_$ARRAY_SOURCE.addr_a0$$" = bitcast %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_SOURCE" to i8**

  store i64 0, i64* %fetch.2.fca.5.gep
  store i64 4, i64* %fetch.2.fca.1.gep
  store i64 3, i64* %fetch.2.fca.4.gep
  store i64 0, i64* %fetch.2.fca.2.gep
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.2.fca.6.0.2.gep, i32 0)
  store i64 1, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.2.fca.6.0.0.gep, i32 0)
  store i64 10, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$22[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.2.fca.6.0.2.gep, i32 1)
  store i64 1, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$22[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$26[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.2.fca.6.0.0.gep, i32 1)
  store i64 64, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$26[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$30[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.2.fca.6.0.2.gep, i32 2)
  store i64 1, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$30[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$34[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.2.fca.6.0.0.gep, i32 2)
  store i64 128, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$34[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.2.fca.6.0.1.gep, i32 0)
  store i64 4, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$40[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.2.fca.6.0.1.gep, i32 1)
  store i64 40, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$40[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$44[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.2.fca.6.0.1.gep, i32 2)
  store i64 2560, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$44[]"
  store i64 1610612869, i64* %fetch.2.fca.3.gep
  %"(i8**)assumedshape_arrs_$ARRAY_TARGET.addr_a0$$" = bitcast %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_TARGET" to i8**

  store i64 0, i64* %fetch.1.fca.5.gep
  store i64 4, i64* %fetch.1.fca.1.gep
  store i64 3, i64* %fetch.1.fca.4.gep
  store i64 0, i64* %fetch.1.fca.2.gep
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.1.fca.6.0.2.gep, i32 0)
  store i64 1, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.1.fca.6.0.0.gep, i32 0)
  store i64 10, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$88[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.1.fca.6.0.2.gep, i32 1)
  store i64 1, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$88[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$92[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.1.fca.6.0.0.gep, i32 1)
  store i64 64, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$92[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$96[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.1.fca.6.0.2.gep, i32 2)
  store i64 1, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$96[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$100[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.1.fca.6.0.0.gep, i32 2)
  store i64 128, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$100[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.1.fca.6.0.1.gep, i32 0)
  store i64 4, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$106[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.1.fca.6.0.1.gep, i32 1)
  store i64 40, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$106[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$110[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %fetch.1.fca.6.0.1.gep, i32 2)
  store i64 2560, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$110[]"
  store i64 1610612869, i64* %fetch.1.fca.3.gep
  %"(i8**)assumedshape_arrs_$ARRAY_ADD.addr_a0$$" = bitcast %"QNCA_a0$float*$rank3$"* %"assumedshape_arrs_$ARRAY_ADD" to i8**

  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31" = load float*, float** %fetch.3.fca.0.gep
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.32" = load i64, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]153"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]_fetch.34" = load i64, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]172"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.35" = load i64, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]157"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]_fetch.37" = load i64, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]175"
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.38" = load i64, i64* %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]163"
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46" = load float*, float** %fetch.1.fca.0.gep
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.47" = load i64, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]_fetch.49" = load i64, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$106[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.50" = load i64, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$88[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]_fetch.52" = load i64, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$110[]"
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.53" = load i64, i64* %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$96[]"
  %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61" = load float*, float** %fetch.2.fca.0.gep
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.62" = load i64, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]_fetch.64" = load i64, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$40[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.65" = load i64, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$22[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]_fetch.67" = load i64, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$44[]"
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.68" = load i64, i64* %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$30[]"
  %9 = bitcast float* %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31" to i8*
  %10 = bitcast float* %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61" to i8*
  %11 = bitcast float* %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46" to i8*
  br label %bb1

bb1:                                              ; preds = %bb8, %alloca_0
  %indvars.iv518 = phi i64 [ %indvars.iv.next519, %bb8 ], [ 1, %alloca_0 ]
  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.38", i64 %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]_fetch.37", float* elementtype(float) %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31", i64 %indvars.iv518)
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.53", i64 %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]_fetch.52", float* elementtype(float) %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46", i64 %indvars.iv518)
  %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.68", i64 %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]_fetch.67", float* elementtype(float) %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61", i64 %indvars.iv518)
  br label %bb5

bb5:                                              ; preds = %bb12, %bb1
  %indvars.iv515 = phi i64 [ %indvars.iv.next516, %bb12 ], [ 1, %bb1 ]
  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.35", i64 %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]_fetch.34", float* elementtype(float) %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[]", i64 %indvars.iv515)
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.50", i64 %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]_fetch.49", float* elementtype(float) %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[]", i64 %indvars.iv515)
  %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.65", i64 %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]_fetch.64", float* elementtype(float) %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[]", i64 %indvars.iv515)
  br label %bb9

bb9:                                              ; preds = %bb9, %bb5
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb9 ], [ 1, %bb5 ]
  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.32", i64 4, float* elementtype(float) %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][]", i64 %indvars.iv)
  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][][]_fetch.45" = load float, float* %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][][]", align 4
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.47", i64 4, float* elementtype(float) %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][]", i64 %indvars.iv)
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][][]_fetch.60" = load float, float* %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][][]", align 4
  %add.11 = fadd reassoc ninf nsz arcp contract afn float %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][][]_fetch.45", %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][][]_fetch.60"
  %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.62", i64 4, float* elementtype(float) %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[][]", i64 %indvars.iv)
  store float %add.11, float* %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[][][]", align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1

  %exitcond.not = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond.not, label %bb12, label %bb9

bb12:                                             ; preds = %bb9
  %indvars.iv.next516 = add nuw nsw i64 %indvars.iv515, 1

  %exitcond517.not = icmp eq i64 %indvars.iv.next516, 65
  br i1 %exitcond517.not, label %bb8, label %bb5

bb8:                                              ; preds = %bb12
  %indvars.iv.next519 = add nuw nsw i64 %indvars.iv518, 1

  %exitcond520.not = icmp eq i64 %indvars.iv.next519, 129
  br i1 %exitcond520.not, label %bb4, label %bb1

bb4:                                              ; preds = %bb8
  %"assumedshape_arrs_$ARRAY_SOURCE.flags$289_fetch.83" = load i64, i64* %fetch.3.fca.3.gep
  %"assumedshape_arrs_$ARRAY_SOURCE.flags$289_fetch.83.tr" = trunc i64 %"assumedshape_arrs_$ARRAY_SOURCE.flags$289_fetch.83" to i32
  %12 = shl i32 %"assumedshape_arrs_$ARRAY_SOURCE.flags$289_fetch.83.tr", 1
  %or.29 = and i32 %12, 6
  %13 = lshr i32 %"assumedshape_arrs_$ARRAY_SOURCE.flags$289_fetch.83.tr", 3
  %int_zext295 = and i32 %13, 256
  %14 = lshr i64 %"assumedshape_arrs_$ARRAY_SOURCE.flags$289_fetch.83", 15
  %15 = trunc i64 %14 to i32
  %int_zext299 = and i32 %15, 31457280
  %int_zext301 = and i32 %15, 33554432
  %or.30 = or i32 %int_zext295, %or.29
  %or.32 = or i32 %or.30, %int_zext299
  %or.33 = or i32 %or.32, %int_zext301
  %or.34 = or i32 %or.33, 393216
  %"assumedshape_arrs_$ARRAY_SOURCE.reserved$303_fetch.84" = load i64, i64* %fetch.3.fca.5.gep
  %"(i8*)assumedshape_arrs_$ARRAY_SOURCE.reserved$303_fetch.84$" = inttoptr i64 %"assumedshape_arrs_$ARRAY_SOURCE.reserved$303_fetch.84" to i8*
  %func_result305 = tail call i32 @for_dealloc_allocatable_handle(i8* nonnull %9, i32 %or.34, i8* %"(i8*)assumedshape_arrs_$ARRAY_SOURCE.reserved$303_fetch.84$") #6
  %rel.4 = icmp eq i32 %func_result305, 0
  br i1 %rel.4, label %bb_new14_then, label %bb15_endif

bb_new14_then:                                    ; preds = %bb4
  store float* null, float** %fetch.3.fca.0.gep
  %and.64 = and i64 %"assumedshape_arrs_$ARRAY_SOURCE.flags$289_fetch.83", -1030792153090
  store i64 %and.64, i64* %fetch.3.fca.3.gep
  br label %bb15_endif

bb15_endif:                                       ; preds = %bb4, %bb_new14_then
  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$380_fetch.106509" = phi i8* [ %9, %bb4 ], [ null, %bb_new14_then ]
  %"assumedshape_arrs_$ARRAY_SOURCE.flags$378_fetch.105" = phi i64 [ %"assumedshape_arrs_$ARRAY_SOURCE.flags$289_fetch.83", %bb4 ], [ %and.64, %bb_new14_then ]
  %"assumedshape_arrs_$ARRAY_TARGET.flags$320_fetch.91" = load i64, i64* %fetch.2.fca.3.gep
  %"assumedshape_arrs_$ARRAY_TARGET.flags$320_fetch.91.tr" = trunc i64 %"assumedshape_arrs_$ARRAY_TARGET.flags$320_fetch.91" to i32
  %16 = shl i32 %"assumedshape_arrs_$ARRAY_TARGET.flags$320_fetch.91.tr", 1
  %or.37 = and i32 %16, 6
  %17 = lshr i32 %"assumedshape_arrs_$ARRAY_TARGET.flags$320_fetch.91.tr", 3
  %int_zext326 = and i32 %17, 256
  %18 = lshr i64 %"assumedshape_arrs_$ARRAY_TARGET.flags$320_fetch.91", 15
  %19 = trunc i64 %18 to i32
  %int_zext330 = and i32 %19, 31457280
  %int_zext332 = and i32 %19, 33554432
  %or.38 = or i32 %int_zext326, %or.37
  %or.40 = or i32 %or.38, %int_zext330
  %or.41 = or i32 %or.40, %int_zext332
  %or.42 = or i32 %or.41, 393216
  %"assumedshape_arrs_$ARRAY_TARGET.reserved$334_fetch.92" = load i64, i64* %fetch.2.fca.5.gep
  %"(i8*)assumedshape_arrs_$ARRAY_TARGET.reserved$334_fetch.92$" = inttoptr i64 %"assumedshape_arrs_$ARRAY_TARGET.reserved$334_fetch.92" to i8*
  %func_result336 = tail call i32 @for_dealloc_allocatable_handle(i8* nonnull %10, i32 %or.42, i8* %"(i8*)assumedshape_arrs_$ARRAY_TARGET.reserved$334_fetch.92$") #6
  %rel.5 = icmp eq i32 %func_result336, 0
  br i1 %rel.5, label %bb_new17_then, label %bb17_endif

bb_new17_then:                                    ; preds = %bb15_endif
  store float* null, float** %fetch.2.fca.0.gep
  %and.80 = and i64 %"assumedshape_arrs_$ARRAY_TARGET.flags$320_fetch.91", -1030792153090
  store i64 %and.80, i64* %fetch.2.fca.3.gep
  br label %bb17_endif

bb17_endif:                                       ; preds = %bb15_endif, %bb_new17_then
  %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$409_fetch.113511" = phi i8* [ %10, %bb15_endif ], [ null, %bb_new17_then ]
  %"assumedshape_arrs_$ARRAY_TARGET.flags$407_fetch.112" = phi i64 [ %"assumedshape_arrs_$ARRAY_TARGET.flags$320_fetch.91", %bb15_endif ], [ %and.80, %bb_new17_then ]
  %"assumedshape_arrs_$ARRAY_ADD.flags$351_fetch.99" = load i64, i64* %fetch.1.fca.3.gep
  %"assumedshape_arrs_$ARRAY_ADD.flags$351_fetch.99.tr" = trunc i64 %"assumedshape_arrs_$ARRAY_ADD.flags$351_fetch.99" to i32
  %20 = shl i32 %"assumedshape_arrs_$ARRAY_ADD.flags$351_fetch.99.tr", 1
  %or.45 = and i32 %20, 6
  %21 = lshr i32 %"assumedshape_arrs_$ARRAY_ADD.flags$351_fetch.99.tr", 3
  %int_zext357 = and i32 %21, 256
  %22 = lshr i64 %"assumedshape_arrs_$ARRAY_ADD.flags$351_fetch.99", 15
  %23 = trunc i64 %22 to i32
  %int_zext361 = and i32 %23, 31457280
  %int_zext363 = and i32 %23, 33554432
  %or.46 = or i32 %int_zext357, %or.45
  %or.48 = or i32 %or.46, %int_zext361
  %or.49 = or i32 %or.48, %int_zext363
  %or.50 = or i32 %or.49, 393216
  %"assumedshape_arrs_$ARRAY_ADD.reserved$365_fetch.100" = load i64, i64* %fetch.1.fca.5.gep
  %"(i8*)assumedshape_arrs_$ARRAY_ADD.reserved$365_fetch.100$" = inttoptr i64 %"assumedshape_arrs_$ARRAY_ADD.reserved$365_fetch.100" to i8*
  %func_result367 = tail call i32 @for_dealloc_allocatable_handle(i8* nonnull %11, i32 %or.50, i8* %"(i8*)assumedshape_arrs_$ARRAY_ADD.reserved$365_fetch.100$") #6
  %rel.6 = icmp eq i32 %func_result367, 0
  br i1 %rel.6, label %bb_new20_then, label %bb19_endif

bb_new20_then:                                    ; preds = %bb17_endif
  store float* null, float** %fetch.1.fca.0.gep
  %and.96 = and i64 %"assumedshape_arrs_$ARRAY_ADD.flags$351_fetch.99", -1030792153090
  store i64 %and.96, i64* %fetch.1.fca.3.gep
  br label %bb19_endif

bb19_endif:                                       ; preds = %bb17_endif, %bb_new20_then
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$438_fetch.120513" = phi i8* [ %11, %bb17_endif ], [ null, %bb_new20_then ]
  %"assumedshape_arrs_$ARRAY_ADD.flags$436_fetch.119" = phi i64 [ %"assumedshape_arrs_$ARRAY_ADD.flags$351_fetch.99", %bb17_endif ], [ %and.96, %bb_new20_then ]
  %and.97 = and i64 %"assumedshape_arrs_$ARRAY_SOURCE.flags$378_fetch.105", 1
  %rel.7 = icmp eq i64 %and.97, 0
  br i1 %rel.7, label %dealloc.list.end22, label %dealloc.list.then21

dealloc.list.then21:                              ; preds = %bb19_endif
  %"assumedshape_arrs_$ARRAY_SOURCE.flags$378_fetch.105.tr" = trunc i64 %"assumedshape_arrs_$ARRAY_SOURCE.flags$378_fetch.105" to i32
  %24 = shl i32 %"assumedshape_arrs_$ARRAY_SOURCE.flags$378_fetch.105.tr", 1
  %int_zext384 = and i32 %24, 4
  %25 = lshr i32 %"assumedshape_arrs_$ARRAY_SOURCE.flags$378_fetch.105.tr", 3
  %int_zext388 = and i32 %25, 256
  %26 = lshr i64 %"assumedshape_arrs_$ARRAY_SOURCE.flags$378_fetch.105", 15
  %27 = trunc i64 %26 to i32
  %int_zext392 = and i32 %27, 31457280
  %int_zext394 = and i32 %27, 33554432
  %or.53 = or i32 %int_zext388, %int_zext384
  %or.55 = or i32 %or.53, %int_zext392
  %or.56 = or i32 %or.55, %int_zext394
  %or.58 = or i32 %or.56, 393218
  %func_result398 = tail call i32 @for_dealloc_allocatable_handle(i8* %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$380_fetch.106509", i32 %or.58, i8* %"(i8*)assumedshape_arrs_$ARRAY_SOURCE.reserved$303_fetch.84$") #6
  %rel.8 = icmp eq i32 %func_result398, 0
  br i1 %rel.8, label %bb_new25_then, label %dealloc.list.end22

bb_new25_then:                                    ; preds = %dealloc.list.then21
  store float* null, float** %fetch.3.fca.0.gep
  %and.112 = and i64 %"assumedshape_arrs_$ARRAY_SOURCE.flags$378_fetch.105", -2050
  store i64 %and.112, i64* %fetch.3.fca.3.gep
  br label %dealloc.list.end22

dealloc.list.end22:                               ; preds = %bb_new25_then, %dealloc.list.then21, %bb19_endif
  %and.113 = and i64 %"assumedshape_arrs_$ARRAY_TARGET.flags$407_fetch.112", 1
  %rel.9 = icmp eq i64 %and.113, 0
  br i1 %rel.9, label %dealloc.list.end27, label %dealloc.list.then26

dealloc.list.then26:                              ; preds = %dealloc.list.end22
  %"assumedshape_arrs_$ARRAY_TARGET.flags$407_fetch.112.tr" = trunc i64 %"assumedshape_arrs_$ARRAY_TARGET.flags$407_fetch.112" to i32
  %28 = shl i32 %"assumedshape_arrs_$ARRAY_TARGET.flags$407_fetch.112.tr", 1
  %int_zext413 = and i32 %28, 4
  %29 = lshr i32 %"assumedshape_arrs_$ARRAY_TARGET.flags$407_fetch.112.tr", 3
  %int_zext417 = and i32 %29, 256
  %30 = lshr i64 %"assumedshape_arrs_$ARRAY_TARGET.flags$407_fetch.112", 15
  %31 = trunc i64 %30 to i32
  %int_zext421 = and i32 %31, 31457280
  %int_zext423 = and i32 %31, 33554432
  %or.60 = or i32 %int_zext417, %int_zext413
  %or.62 = or i32 %or.60, %int_zext421
  %or.63 = or i32 %or.62, %int_zext423
  %or.65 = or i32 %or.63, 393218
  %func_result427 = tail call i32 @for_dealloc_allocatable_handle(i8* %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$409_fetch.113511", i32 %or.65, i8* %"(i8*)assumedshape_arrs_$ARRAY_TARGET.reserved$334_fetch.92$") #6
  %rel.10 = icmp eq i32 %func_result427, 0
  br i1 %rel.10, label %bb_new30_then, label %dealloc.list.end27

bb_new30_then:                                    ; preds = %dealloc.list.then26
  store float* null, float** %fetch.2.fca.0.gep
  %and.128 = and i64 %"assumedshape_arrs_$ARRAY_TARGET.flags$407_fetch.112", -2050
  store i64 %and.128, i64* %fetch.2.fca.3.gep
  br label %dealloc.list.end27

dealloc.list.end27:                               ; preds = %bb_new30_then, %dealloc.list.then26, %dealloc.list.end22
  %and.129 = and i64 %"assumedshape_arrs_$ARRAY_ADD.flags$436_fetch.119", 1
  %rel.11 = icmp eq i64 %and.129, 0
  br i1 %rel.11, label %dealloc.list.end32, label %dealloc.list.then31

dealloc.list.then31:                              ; preds = %dealloc.list.end27
  %"assumedshape_arrs_$ARRAY_ADD.flags$436_fetch.119.tr" = trunc i64 %"assumedshape_arrs_$ARRAY_ADD.flags$436_fetch.119" to i32
  %32 = shl i32 %"assumedshape_arrs_$ARRAY_ADD.flags$436_fetch.119.tr", 1
  %int_zext442 = and i32 %32, 4
  %33 = lshr i32 %"assumedshape_arrs_$ARRAY_ADD.flags$436_fetch.119.tr", 3
  %int_zext446 = and i32 %33, 256
  %34 = lshr i64 %"assumedshape_arrs_$ARRAY_ADD.flags$436_fetch.119", 15
  %35 = trunc i64 %34 to i32
  %int_zext450 = and i32 %35, 31457280
  %int_zext452 = and i32 %35, 33554432
  %or.67 = or i32 %int_zext446, %int_zext442
  %or.69 = or i32 %or.67, %int_zext450
  %or.70 = or i32 %or.69, %int_zext452
  %or.72 = or i32 %or.70, 393218
  %func_result456 = tail call i32 @for_dealloc_allocatable_handle(i8* %"assumedshape_arrs_$ARRAY_ADD.addr_a0$438_fetch.120513", i32 %or.72, i8* %"(i8*)assumedshape_arrs_$ARRAY_ADD.reserved$365_fetch.100$") #6
  %rel.12 = icmp eq i32 %func_result456, 0
  br i1 %rel.12, label %bb_new35_then, label %dealloc.list.end32

bb_new35_then:                                    ; preds = %dealloc.list.then31
  store float* null, float** %fetch.1.fca.0.gep
  %and.144 = and i64 %"assumedshape_arrs_$ARRAY_ADD.flags$436_fetch.119", -2050
  store i64 %and.144, i64* %fetch.1.fca.3.gep
  br label %dealloc.list.end32

dealloc.list.end32:                               ; preds = %bb_new35_then, %dealloc.list.then31, %dealloc.list.end27
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #2

; Function Attrs: nofree
declare i32 @for_alloc_allocatable_handle(i64, i8** nocapture, i32, i8*) local_unnamed_addr #3

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #2

; Function Attrs: nofree
declare i32 @for_dealloc_allocatable_handle(i8* nocapture readonly, i32, i8*) local_unnamed_addr #3

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #4

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #5

attributes #0 = { nofree nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="non-leaf" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nofree nosync nounwind readnone speculatable }
attributes #3 = { nofree "intel-lang"="fortran" }
attributes #4 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #5 = { argmemonly nofree nounwind willreturn writeonly }
attributes #6 = { nounwind }
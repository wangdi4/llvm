; REQUIRES: asserts
; RUN: opt < %s  -disable-output -debug-only=dopevectorconstprop -passes=dopevectorconstprop -dope-vector-local-const-prop  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; This test case checks that dope vector constant propagation did not
; happen for assumed shape arrays that are using inside local functions.
; The reason is that the dope vectors are passed to another function. It
; was created from the following example, but a call to @external_function
; was added.

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

; CHECK: FUNCTION assumedshape_arrs_ DOES NOT HAVE LOCAL LINKAGE
; CHECK:   LOCAL DV FOUND:   %"assumedshape_arrs_$ARRAY_ADD" = alloca %"QNCA_a0$float*$rank3$", align 8
; CHECK:     RANK: 3
; CHECK:     TYPE:  <UNKNOWN ELEMENT TYPE>
; CHECK:     ANALYSIS RESULT: NOT VALID
; CHECK:   LOCAL DV FOUND:   %"assumedshape_arrs_$ARRAY_TARGET" = alloca %"QNCA_a0$float*$rank3$", align 8
; CHECK:     RANK: 3
; CHECK:     TYPE:  <UNKNOWN ELEMENT TYPE>
; CHECK:     ANALYSIS RESULT: NOT VALID
; CHECK:   LOCAL DV FOUND:   %"assumedshape_arrs_$ARRAY_SOURCE" = alloca %"QNCA_a0$float*$rank3$", align 8
; CHECK:     RANK: 3
; CHECK:     TYPE:  <UNKNOWN ELEMENT TYPE>
; CHECK:     ANALYSIS RESULT: NOT VALID

; ifx -c -O3 -fiopenmp -xCORE-AVX512 -fpp -traceback -flto -align array64byte -what -V simple.F90 -mllmv -dope-vector-local-const-prop

source_filename = "/tmp/ifxDtVxlK.i90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

; Function Attrs: nofree nounwind uwtable
define void @assumedshape_arrs_() local_unnamed_addr #0 {
alloca_0:
  %"assumedshape_arrs_$ARRAY_ADD" = alloca %"QNCA_a0$float*$rank3$", align 8
  %"assumedshape_arrs_$ARRAY_TARGET" = alloca %"QNCA_a0$float*$rank3$", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE" = alloca %"QNCA_a0$float*$rank3$", align 8
  %fetch.1.fca.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 0
  %fetch.1.fca.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 1
  %fetch.1.fca.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 2
  %fetch.1.fca.3.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 3
  %0 = bitcast ptr %"assumedshape_arrs_$ARRAY_ADD" to ptr
  store i64 0, ptr %0, align 8
  %fetch.1.fca.4.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 4
  %fetch.1.fca.5.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 5
  %fetch.1.fca.6.0.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 0
  %fetch.1.fca.6.0.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 1
  %fetch.1.fca.6.0.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 2
  %fetch.2.fca.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 0
  %1 = getelementptr inbounds i64, ptr %fetch.1.fca.5.gep, i64 1
  %2 = bitcast ptr %1 to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(72) %2, i8 0, i64 72, i1 false)
  %fetch.2.fca.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 1
  %fetch.2.fca.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 2
  %fetch.2.fca.3.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 3
  %3 = bitcast ptr %"assumedshape_arrs_$ARRAY_TARGET" to ptr
  store i64 0, ptr %3, align 8
  %fetch.2.fca.4.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 4
  %fetch.2.fca.5.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 5
  %fetch.2.fca.6.0.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 0
  %fetch.2.fca.6.0.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 1
  %fetch.2.fca.6.0.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 2
  %fetch.3.fca.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 0
  %4 = getelementptr inbounds i64, ptr %fetch.2.fca.5.gep, i64 1
  %5 = bitcast ptr %4 to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(72) %5, i8 0, i64 72, i1 false)
  %fetch.3.fca.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 1
  %fetch.3.fca.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 2
  %fetch.3.fca.3.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 3
  %6 = bitcast ptr %"assumedshape_arrs_$ARRAY_SOURCE" to ptr
  store i64 0, ptr %6, align 8
  %fetch.3.fca.4.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 4
  %fetch.3.fca.5.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 5
  %fetch.3.fca.6.0.0.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 0
  %fetch.3.fca.6.0.1.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 1
  %fetch.3.fca.6.0.2.gep = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %"assumedshape_arrs_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 2
  %7 = getelementptr inbounds i64, ptr %fetch.3.fca.5.gep, i64 1
  %8 = bitcast ptr %7 to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(72) %8, i8 0, i64 72, i1 false)
  store i64 0, ptr %fetch.3.fca.5.gep, align 8
  store i64 4, ptr %fetch.3.fca.1.gep, align 8
  store i64 3, ptr %fetch.3.fca.4.gep, align 8
  store i64 0, ptr %fetch.3.fca.2.gep, align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]153" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.3.fca.6.0.2.gep, i32 0)
  store i64 1, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]153", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.3.fca.6.0.0.gep, i32 0)
  store i64 10, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]157" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.3.fca.6.0.2.gep, i32 1)
  store i64 1, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]157", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]160" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.3.fca.6.0.0.gep, i32 1)
  store i64 64, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]160", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]163" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.3.fca.6.0.2.gep, i32 2)
  store i64 1, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]163", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]166" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.3.fca.6.0.0.gep, i32 2)
  store i64 128, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.extent$[]166", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]169" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.3.fca.6.0.1.gep, i32 0)
  store i64 4, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]169", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]172" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.3.fca.6.0.1.gep, i32 1)
  store i64 40, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]172", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]175" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.3.fca.6.0.1.gep, i32 2)
  store i64 2560, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]175", align 8
  store i64 1610612869, ptr %fetch.3.fca.3.gep, align 8
  %"(i8**)assumedshape_arrs_$ARRAY_SOURCE.addr_a0$$" = bitcast ptr %"assumedshape_arrs_$ARRAY_SOURCE" to ptr
  %func_result = call i32 @for_alloc_allocatable_handle(i64 327680, ptr nonnull %"(i8**)assumedshape_arrs_$ARRAY_SOURCE.addr_a0$$", i32 393218, ptr null) #5
  store i64 0, ptr %fetch.2.fca.5.gep, align 8
  store i64 4, ptr %fetch.2.fca.1.gep, align 8
  store i64 3, ptr %fetch.2.fca.4.gep, align 8
  store i64 0, ptr %fetch.2.fca.2.gep, align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.2.fca.6.0.2.gep, i32 0)
  store i64 1, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.2.fca.6.0.0.gep, i32 0)
  store i64 10, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$22[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.2.fca.6.0.2.gep, i32 1)
  store i64 1, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$22[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$26[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.2.fca.6.0.0.gep, i32 1)
  store i64 64, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$26[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$30[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.2.fca.6.0.2.gep, i32 2)
  store i64 1, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$30[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$34[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.2.fca.6.0.0.gep, i32 2)
  store i64 128, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.extent$34[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.2.fca.6.0.1.gep, i32 0)
  store i64 4, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$40[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.2.fca.6.0.1.gep, i32 1)
  store i64 40, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$40[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$44[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.2.fca.6.0.1.gep, i32 2)
  store i64 2560, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$44[]", align 8
  store i64 1610612869, ptr %fetch.2.fca.3.gep, align 8
  %"(i8**)assumedshape_arrs_$ARRAY_TARGET.addr_a0$$" = bitcast ptr %"assumedshape_arrs_$ARRAY_TARGET" to ptr
  %func_result60 = call i32 @for_alloc_allocatable_handle(i64 327680, ptr nonnull %"(i8**)assumedshape_arrs_$ARRAY_TARGET.addr_a0$$", i32 393218, ptr null) #5
  store i64 0, ptr %fetch.1.fca.5.gep, align 8
  store i64 4, ptr %fetch.1.fca.1.gep, align 8
  store i64 3, ptr %fetch.1.fca.4.gep, align 8
  store i64 0, ptr %fetch.1.fca.2.gep, align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.1.fca.6.0.2.gep, i32 0)
  store i64 1, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.1.fca.6.0.0.gep, i32 0)
  store i64 10, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$88[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.1.fca.6.0.2.gep, i32 1)
  store i64 1, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$88[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$92[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.1.fca.6.0.0.gep, i32 1)
  store i64 64, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$92[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$96[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.1.fca.6.0.2.gep, i32 2)
  store i64 1, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$96[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$100[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.1.fca.6.0.0.gep, i32 2)
  store i64 128, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.extent$100[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.1.fca.6.0.1.gep, i32 0)
  store i64 4, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$106[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.1.fca.6.0.1.gep, i32 1)
  store i64 40, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$106[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$110[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %fetch.1.fca.6.0.1.gep, i32 2)
  store i64 2560, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$110[]", align 8
  store i64 1610612869, ptr %fetch.1.fca.3.gep, align 8
  %"(i8**)assumedshape_arrs_$ARRAY_ADD.addr_a0$$" = bitcast ptr %"assumedshape_arrs_$ARRAY_ADD" to ptr
  %func_result128 = call i32 @for_alloc_allocatable_handle(i64 327680, ptr nonnull %"(i8**)assumedshape_arrs_$ARRAY_ADD.addr_a0$$", i32 393218, ptr null) #5
  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31" = load ptr, ptr %fetch.3.fca.0.gep, align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.32" = load i64, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]153", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]_fetch.34" = load i64, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]172", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.35" = load i64, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]157", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]_fetch.37" = load i64, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]175", align 8
  %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.38" = load i64, ptr %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]163", align 8
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46" = load ptr, ptr %fetch.1.fca.0.gep, align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.47" = load i64, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]_fetch.49" = load i64, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$106[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.50" = load i64, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$88[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]_fetch.52" = load i64, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$110[]", align 8
  %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.53" = load i64, ptr %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$96[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61" = load ptr, ptr %fetch.2.fca.0.gep, align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.62" = load i64, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]_fetch.64" = load i64, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$40[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.65" = load i64, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$22[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]_fetch.67" = load i64, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$44[]", align 8
  %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.68" = load i64, ptr %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$30[]", align 8
  %9 = bitcast ptr %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31" to ptr
  %10 = bitcast ptr %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61" to ptr
  %11 = bitcast ptr %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46" to ptr
  br label %bb1

bb1:                                              ; preds = %bb8, %alloca_0
  %indvars.iv518 = phi i64 [ %indvars.iv.next519, %bb8 ], [ 1, %alloca_0 ]
  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.38", i64 %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]_fetch.37", ptr elementtype(float) %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31", i64 %indvars.iv518)
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.53", i64 %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]_fetch.52", ptr elementtype(float) %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46", i64 %indvars.iv518)
  %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.68", i64 %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]_fetch.67", ptr elementtype(float) %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61", i64 %indvars.iv518)
  br label %bb5

bb5:                                              ; preds = %bb12, %bb1
  %indvars.iv515 = phi i64 [ %indvars.iv.next516, %bb12 ], [ 1, %bb1 ]
  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.35", i64 %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.spacing$[]_fetch.34", ptr elementtype(float) %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[]", i64 %indvars.iv515)
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.50", i64 %"assumedshape_arrs_$ARRAY_ADD.dim_info$.spacing$[]_fetch.49", ptr elementtype(float) %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[]", i64 %indvars.iv515)
  %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.65", i64 %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.spacing$[]_fetch.64", ptr elementtype(float) %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[]", i64 %indvars.iv515)
  br label %bb9

bb9:                                              ; preds = %bb9, %bb5
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb9 ], [ 1, %bb5 ]
  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"assumedshape_arrs_$ARRAY_SOURCE.dim_info$.lower_bound$[]_fetch.32", i64 4, ptr elementtype(float) %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][]", i64 %indvars.iv)
  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][][]_fetch.45" = load float, ptr %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][][]", align 4
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"assumedshape_arrs_$ARRAY_ADD.dim_info$.lower_bound$[]_fetch.47", i64 4, ptr elementtype(float) %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][]", i64 %indvars.iv)
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][][]_fetch.60" = load float, ptr %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][][]", align 4
  %add.11 = fadd reassoc ninf nsz arcp contract afn float %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$_fetch.31[][][]_fetch.45", %"assumedshape_arrs_$ARRAY_ADD.addr_a0$_fetch.46[][][]_fetch.60"
  %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"assumedshape_arrs_$ARRAY_TARGET.dim_info$.lower_bound$[]_fetch.62", i64 4, ptr elementtype(float) %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[][]", i64 %indvars.iv)
  store float %add.11, ptr %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$_fetch.61[][][]", align 4
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
  tail call void (...) @extern_func_(ptr %"assumedshape_arrs_$ARRAY_ADD", ptr %"assumedshape_arrs_$ARRAY_TARGET", ptr %"assumedshape_arrs_$ARRAY_SOURCE")
  %"assumedshape_arrs_$ARRAY_SOURCE.flags$289_fetch.83" = load i64, ptr %fetch.3.fca.3.gep, align 8
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
  %"assumedshape_arrs_$ARRAY_SOURCE.reserved$303_fetch.84" = load i64, ptr %fetch.3.fca.5.gep, align 8
  %"(i8*)assumedshape_arrs_$ARRAY_SOURCE.reserved$303_fetch.84$" = inttoptr i64 %"assumedshape_arrs_$ARRAY_SOURCE.reserved$303_fetch.84" to ptr
  %func_result305 = tail call i32 @for_dealloc_allocatable_handle(ptr nonnull %9, i32 %or.34, ptr %"(i8*)assumedshape_arrs_$ARRAY_SOURCE.reserved$303_fetch.84$") #5
  %rel.4 = icmp eq i32 %func_result305, 0
  br i1 %rel.4, label %bb_new14_then, label %bb15_endif

bb_new14_then:                                    ; preds = %bb4
  store ptr null, ptr %fetch.3.fca.0.gep, align 8
  %and.64 = and i64 %"assumedshape_arrs_$ARRAY_SOURCE.flags$289_fetch.83", -1030792153090
  store i64 %and.64, ptr %fetch.3.fca.3.gep, align 8
  br label %bb15_endif

bb15_endif:                                       ; preds = %bb_new14_then, %bb4
  %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$380_fetch.106509" = phi ptr [ %9, %bb4 ], [ null, %bb_new14_then ]
  %"assumedshape_arrs_$ARRAY_SOURCE.flags$378_fetch.105" = phi i64 [ %"assumedshape_arrs_$ARRAY_SOURCE.flags$289_fetch.83", %bb4 ], [ %and.64, %bb_new14_then ]
  %"assumedshape_arrs_$ARRAY_TARGET.flags$320_fetch.91" = load i64, ptr %fetch.2.fca.3.gep, align 8
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
  %"assumedshape_arrs_$ARRAY_TARGET.reserved$334_fetch.92" = load i64, ptr %fetch.2.fca.5.gep, align 8
  %"(i8*)assumedshape_arrs_$ARRAY_TARGET.reserved$334_fetch.92$" = inttoptr i64 %"assumedshape_arrs_$ARRAY_TARGET.reserved$334_fetch.92" to ptr
  %func_result336 = tail call i32 @for_dealloc_allocatable_handle(ptr nonnull %10, i32 %or.42, ptr %"(i8*)assumedshape_arrs_$ARRAY_TARGET.reserved$334_fetch.92$") #5
  %rel.5 = icmp eq i32 %func_result336, 0
  br i1 %rel.5, label %bb_new17_then, label %bb17_endif

bb_new17_then:                                    ; preds = %bb15_endif
  store ptr null, ptr %fetch.2.fca.0.gep, align 8
  %and.80 = and i64 %"assumedshape_arrs_$ARRAY_TARGET.flags$320_fetch.91", -1030792153090
  store i64 %and.80, ptr %fetch.2.fca.3.gep, align 8
  br label %bb17_endif

bb17_endif:                                       ; preds = %bb_new17_then, %bb15_endif
  %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$409_fetch.113511" = phi ptr [ %10, %bb15_endif ], [ null, %bb_new17_then ]
  %"assumedshape_arrs_$ARRAY_TARGET.flags$407_fetch.112" = phi i64 [ %"assumedshape_arrs_$ARRAY_TARGET.flags$320_fetch.91", %bb15_endif ], [ %and.80, %bb_new17_then ]
  %"assumedshape_arrs_$ARRAY_ADD.flags$351_fetch.99" = load i64, ptr %fetch.1.fca.3.gep, align 8
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
  %"assumedshape_arrs_$ARRAY_ADD.reserved$365_fetch.100" = load i64, ptr %fetch.1.fca.5.gep, align 8
  %"(i8*)assumedshape_arrs_$ARRAY_ADD.reserved$365_fetch.100$" = inttoptr i64 %"assumedshape_arrs_$ARRAY_ADD.reserved$365_fetch.100" to ptr
  %func_result367 = tail call i32 @for_dealloc_allocatable_handle(ptr nonnull %11, i32 %or.50, ptr %"(i8*)assumedshape_arrs_$ARRAY_ADD.reserved$365_fetch.100$") #5
  %rel.6 = icmp eq i32 %func_result367, 0
  br i1 %rel.6, label %bb_new20_then, label %bb19_endif

bb_new20_then:                                    ; preds = %bb17_endif
  store ptr null, ptr %fetch.1.fca.0.gep, align 8
  %and.96 = and i64 %"assumedshape_arrs_$ARRAY_ADD.flags$351_fetch.99", -1030792153090
  store i64 %and.96, ptr %fetch.1.fca.3.gep, align 8
  br label %bb19_endif

bb19_endif:                                       ; preds = %bb_new20_then, %bb17_endif
  %"assumedshape_arrs_$ARRAY_ADD.addr_a0$438_fetch.120513" = phi ptr [ %11, %bb17_endif ], [ null, %bb_new20_then ]
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
  %func_result398 = tail call i32 @for_dealloc_allocatable_handle(ptr %"assumedshape_arrs_$ARRAY_SOURCE.addr_a0$380_fetch.106509", i32 %or.58, ptr %"(i8*)assumedshape_arrs_$ARRAY_SOURCE.reserved$303_fetch.84$") #5
  %rel.8 = icmp eq i32 %func_result398, 0
  br i1 %rel.8, label %bb_new25_then, label %dealloc.list.end22

bb_new25_then:                                    ; preds = %dealloc.list.then21
  store ptr null, ptr %fetch.3.fca.0.gep, align 8
  %and.112 = and i64 %"assumedshape_arrs_$ARRAY_SOURCE.flags$378_fetch.105", -2050
  store i64 %and.112, ptr %fetch.3.fca.3.gep, align 8
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
  %func_result427 = tail call i32 @for_dealloc_allocatable_handle(ptr %"assumedshape_arrs_$ARRAY_TARGET.addr_a0$409_fetch.113511", i32 %or.65, ptr %"(i8*)assumedshape_arrs_$ARRAY_TARGET.reserved$334_fetch.92$") #5
  %rel.10 = icmp eq i32 %func_result427, 0
  br i1 %rel.10, label %bb_new30_then, label %dealloc.list.end27

bb_new30_then:                                    ; preds = %dealloc.list.then26
  store ptr null, ptr %fetch.2.fca.0.gep, align 8
  %and.128 = and i64 %"assumedshape_arrs_$ARRAY_TARGET.flags$407_fetch.112", -2050
  store i64 %and.128, ptr %fetch.2.fca.3.gep, align 8
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
  %func_result456 = tail call i32 @for_dealloc_allocatable_handle(ptr %"assumedshape_arrs_$ARRAY_ADD.addr_a0$438_fetch.120513", i32 %or.72, ptr %"(i8*)assumedshape_arrs_$ARRAY_ADD.reserved$365_fetch.100$") #5
  %rel.12 = icmp eq i32 %func_result456, 0
  br i1 %rel.12, label %bb_new35_then, label %dealloc.list.end32

bb_new35_then:                                    ; preds = %dealloc.list.then31
  store ptr null, ptr %fetch.1.fca.0.gep, align 8
  %and.144 = and i64 %"assumedshape_arrs_$ARRAY_ADD.flags$436_fetch.119", -2050
  store i64 %and.144, ptr %fetch.1.fca.3.gep, align 8
  br label %dealloc.list.end32

dealloc.list.end32:                               ; preds = %bb_new35_then, %dealloc.list.then31, %dealloc.list.end27
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nofree
declare i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #2

; Function Attrs: nofree
declare i32 @for_dealloc_allocatable_handle(ptr nocapture readonly, i32, ptr) local_unnamed_addr #2

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

declare void @extern_func_(...) local_unnamed_addr

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #4

attributes #0 = { nofree nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="non-leaf" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nounwind readnone speculatable }
attributes #4 = { argmemonly nofree nounwind willreturn writeonly }
attributes #5 = { nounwind }

!ifx.types.dv = !{!0}
!0 = !{%"QNCA_a0$float*$rank3$" zeroinitializer, float 0.000000e+00}

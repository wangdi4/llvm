; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test that for F90 DVs, a check is emitted for the size of the data
; being non-zero, before allocating space for the private copy.

; Test src:

; program main
;   integer, allocatable :: x(:)
;
;   !$omp parallel firstprivate(x) num_threads(1)
;   allocate(x(10))
;   x(1)=123
;   print *, x(1)
;   deallocate(x)
;   !$omp end parallel
; end program main

; CHECK: [[DV_PRIV:%[^ ]+]] = alloca %"QNCA_a0$i32*$rank1$", align 16
; CHECK: [[DV_PRIV_CAST:%[^ ]+]] = bitcast %"QNCA_a0$i32*$rank1$"* [[DV_PRIV]] to i8*
; CHECK: [[DV_SIZE:%[^ ]+]] = call i64 @_f90_dope_vector_init(i8* [[DV_PRIV_CAST]], i8* bitcast (%"QNCA_a0$i32*$rank1$"* @"main_$X" to i8*))
; CHECK: [[NUM_ELEMENTS:%[^ ]+]] = udiv i64 [[DV_SIZE]], 4
; CHECK: [[IS_ALLOCATED:%[^ ]+]] = icmp ne i64 [[DV_SIZE]], 0
; CHECK: br i1 [[IS_ALLOCATED]], label %[[IF_THEN:[^ ]+]], label %[[IF_CONTINUE:[^, ]+]]

; CHECK: [[IF_THEN]]:
; CHECK: [[ADDR0:%[^ ]+]] = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* [[DV_PRIV]], i32 0, i32 0
; CHECK: [[DATA_ALLOC:%[^ ]+]] = alloca i32, i64 [[NUM_ELEMENTS]]
; CHECK: store i32* [[DATA_ALLOC]], i32** [[ADDR0]], align 8
; CHECK: br label %[[IF_CONTINUE]]

; CHECK: [[IF_CONTINUE]]:
; CHECK: [[DV_PRIV_CAST1:%[^ ]+]] = bitcast %"QNCA_a0$i32*$rank1$"* [[DV_PRIV]] to i8*
; CHECK: call void @_f90_firstprivate_copy(i8* [[DV_PRIV_CAST1]], i8* bitcast (%"QNCA_a0$i32*$rank1$"* @"main_$X" to i8*))

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i32*$rank1$" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@"main_$X" = internal global %"QNCA_a0$i32*$rank1$" { i32* null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@0 = internal unnamed_addr constant i32 2

; Function Attrs: nounwind uwtable
define void @MAIN__() #0 {
alloca_0:
  %"var$2" = alloca [8 x i64], align 8
  %"var$3" = alloca i64, align 8
  %"var$4" = alloca i32, align 4
  %addressof = alloca [4 x i8], align 1
  %argblock = alloca { i32 }, align 8
  %"var$5" = alloca i64, align 8
  %func_result = call i32 @for_set_reentrancy(i32* nonnull @0) #1
  br label %bb3

bb3:                                              ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i64 1), "QUAL.OMP.FIRSTPRIVATE:F90_DV"(%"QNCA_a0$i32*$rank1$"* @"main_$X"), "QUAL.OMP.PRIVATE"(i64* %"var$5"), "QUAL.OMP.PRIVATE"(i32* %"var$4"), "QUAL.OMP.PRIVATE"(i64* %"var$3"), "QUAL.OMP.FIRSTPRIVATE"([8 x i64]* %"var$2") ]

  %_fetch153 = load i64, i64* getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 3), align 8
  %or157 = and i64 %_fetch153, 1030792151296
  %or162 = or i64 %or157, 133
  store i64 %or162, i64* getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 3), align 8
  store i64 4, i64* getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 4), align 16
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 2), align 16
  %"[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %"[]", align 1
  %"[]5" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, i64* %"[]5", align 1
  %"[]8" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 4, i64* %"[]8", align 1
  %_fetch = load i64, i64* getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 3), align 8
  %and = and i64 %_fetch, -68451041281
  %or = or i64 %and, 1073741824
  store i64 %or, i64* getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 3), align 8
  %_fetch.tr = trunc i64 %_fetch to i32
  %1 = shl i32 %_fetch.tr, 1
  %int_zext = and i32 %1, 2
  %2 = lshr i64 %_fetch, 15
  %3 = trunc i64 %2 to i32
  %int_zext36 = and i32 %3, 31457280
  %or38 = or i32 %int_zext, %int_zext36
  %or42 = or i32 %or38, 262144
  %func_result44 = call i32 @for_alloc_allocatable(i64 40, i8** bitcast (%"QNCA_a0$i32*$rank1$"* @"main_$X" to i8**), i32 %or42) #1
  %_fetch58 = load i32*, i32** getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 0), align 16
  %"[]50" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 6, i64 0, i32 2), i32 0)
  %"[]50_fetch" = load i64, i64* %"[]50", align 1
  %"_fetch[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %"[]50_fetch", i64 4, i32* elementtype(i32) %_fetch58, i64 1)
  store i32 123, i32* %"_fetch[]", align 1
  %_fetch72 = load i32*, i32** getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 0), align 16
  %"[]61" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 6, i64 0, i32 2), i32 0)
  %"[]61_fetch" = load i64, i64* %"[]61", align 1
  %"_fetch[]69" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %"[]61_fetch", i64 4, i32* elementtype(i32) %_fetch72, i64 1)
  %"_fetch[]69_fetch" = load i32, i32* %"_fetch[]69", align 1
  %addressof.repack = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 0
  store i8 9, i8* %addressof.repack, align 1
  %addressof.repack2 = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 1
  store i8 1, i8* %addressof.repack2, align 1
  %addressof.repack3 = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 2
  store i8 1, i8* %addressof.repack3, align 1
  %addressof.repack4 = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 3
  store i8 0, i8* %addressof.repack4, align 1
  %BLKFIELD_ = getelementptr inbounds { i32 }, { i32 }* %argblock, i64 0, i32 0
  store i32 %"_fetch[]69_fetch", i32* %BLKFIELD_, align 8
  %"(i8*)var$2" = bitcast [8 x i64]* %"var$2" to i8*
  %"(i8*)addressof" = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 0
  %"(i8*)argblock" = bitcast { i32 }* %argblock to i8*
  %func_result71 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %"(i8*)var$2", i32 -1, i64 1239157112576, i8* nonnull %"(i8*)addressof", i8* nonnull %"(i8*)argblock") #1
  %_fetch765 = load i8*, i8** bitcast (%"QNCA_a0$i32*$rank1$"* @"main_$X" to i8**), align 16
  %_fetch78 = load i64, i64* getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 3), align 8
  %_fetch78.tr = trunc i64 %_fetch78 to i32
  %4 = shl i32 %_fetch78.tr, 1
  %int_zext86 = and i32 %4, 4
  %_fetch78.tr6 = trunc i64 %_fetch78 to i32
  %5 = shl i32 %_fetch78.tr6, 1
  %int_zext96 = and i32 %5, 2
  %or98 = or i32 %int_zext86, %int_zext96
  %6 = lshr i64 %_fetch78, 3
  %7 = trunc i64 %6 to i32
  %int_zext108 = and i32 %7, 256
  %or110 = or i32 %or98, %int_zext108
  %8 = lshr i64 %_fetch78, 15
  %9 = trunc i64 %8 to i32
  %int_zext132 = and i32 %9, 31457280
  %or134 = or i32 %or110, %int_zext132
  %or138 = or i32 %or134, 262144
  %func_result140 = call i32 @for_dealloc_allocatable(i8* %_fetch765, i32 %or138) #1
  store i32* null, i32** getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 0), align 16
  %_fetch142 = load i64, i64* getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 3), align 8
  %and152 = and i64 %_fetch142, -1030792153090
  store i64 %and152, i64* getelementptr inbounds (%"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* @"main_$X", i64 0, i32 3), align 8
  br label %bb33

bb33:                                             ; preds = %bb3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void

}

declare i32 @for_set_reentrancy(i32*)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #2

declare i32 @for_alloc_allocatable(i64, i8**, i32)

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #2

declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...)

declare i32 @for_dealloc_allocatable(i8*, i32)

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }

!omp_offload.info = !{}
; end INTEL_CUSTOMIZATION

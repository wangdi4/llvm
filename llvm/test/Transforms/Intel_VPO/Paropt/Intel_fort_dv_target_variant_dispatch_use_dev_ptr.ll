; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-prepare -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; Test src:

; module submodule
; contains
;     subroutine work_gpu(y,handle)   !! variant function
;       use iso_c_binding
;       integer :: y(:)
;       type(c_ptr) :: handle
; !#ifdef DEBUG
; !      print *, "Enter work_gpu (variant function). y(1) = ", y(2), "."
; !#endif
; !      !$omp target is_device_ptr(y)
;       y(2) = 42
; !      !$omp end target
;     end subroutine work_gpu
; !
;     subroutine work(y)              !! base function
;       integer :: y(:)
;       !$omp  declare variant( work_gpu) &
;       !$omp  match(construct={target variant dispatch}, device={arch(gen)})
; !#ifdef DEBUG
; !      print *, "Enter work (base function). y(2) = ", y(2), "."
; !#endif
;       y(2) = 21
;     end subroutine
; end module submodule
;
; program main
;     use submodule
;     integer, allocatable :: y(:)
;     allocate(y(10))
;     y(2) = 2
; !    !$omp target data map(y)
;
;     !$omp target variant dispatch device(0) use_device_ptr(y)
;     call work(y)
;     !$omp end target variant dispatch
;
; !    !$omp end target data
; !    if (y(2) .eq. 42) then
; !    print *, "PASSED"
; !    else
; !    print *, "FAILED. work_gpu was not called. y(2) is", y(2), "."
; !    end if
;
; end program

; Check that the map-type used for the target data calls for the variant region is
; TGT_RETURN_PARAM (64):
; CHECK: @.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 64]

; Check that the return-param map is created for the addr0 pointer of the dope vector.
; CHECK: [[ADDR0_GEP:%[^ ]+]] = getelementptr inbounds %"QNCA_a0$i32*$rank1$.1", ptr @"main_$Y", i32 0, i32 0
; CHECK: [[ADDR0:%[^ ]+]] = load ptr, ptr [[ADDR0_GEP]], align 8
; CHECK: [[MAP_BASEPTR_STRUCT_GEP:%[^ ]+]] = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs, i32 0, i32 0
; CHECK: store ptr [[ADDR0]], ptr [[MAP_BASEPTR_STRUCT_GEP]], align 8
; CHECK: call void @__tgt_target_data_begin({{.*}})

; Check that a new host dope vector is initialized with the device pointer returned from
; the __tgt_target_data_begin call, and used in the variant call.
; CHECK: [[DEV_PTR:%[^ ]+]] = load ptr, ptr [[MAP_BASEPTR_STRUCT_GEP]], align 8
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 8 [[NEW_DV:%[^ ]+]], ptr align 8 @"main_$Y", i64 72, i1 false)
; CHECK: [[NEW_DV_ADDR0_GEP:%[^ ]+]] = getelementptr inbounds %"QNCA_a0$i32*$rank1$.1", ptr [[NEW_DV]], i32 0, i32 0
; CHECK: store ptr [[DEV_PTR]], ptr [[NEW_DV_ADDR0_GEP]], align 8
; CHECK: call void @[[VARIANT_WRAPPER:[^ ]*work_gpu_.wrapper[^ (]*]](ptr %"main_$Y.new")
; CHECK: call void @__tgt_target_data_end({{.*}})

; Check that variant function is called in the variant wrapper.
; CHECK: define internal void @[[VARIANT_WRAPPER]](ptr [[Y:%[^ ,(]+]])
; CHECK: call void @submodule_mp_work_gpu_(ptr [[Y]], ptr %interop.obj.sync)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%"QNCA_a0$i32*$rank1$.1" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@"main_$Y" = internal global %"QNCA_a0$i32*$rank1$.1" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@0 = internal unnamed_addr constant i32 2

; Function Attrs: nounwind uwtable
declare void @submodule._() #0

; Function Attrs: nounwind uwtable
declare void @submodule_mp_work_gpu_(ptr dereferenceable(72) "assumed_shape" "ptrnoalias", ptr) #0

; Function Attrs: nounwind uwtable
declare void @submodule_mp_work_(ptr dereferenceable(72) "assumed_shape" "ptrnoalias") #1

; Function Attrs: nounwind uwtable
define void @MAIN__() #0 {
alloca_3:
  %func_result = call i32 @for_set_reentrancy(ptr @0)
  %val_fetch60 = load i64, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank1$.1", ptr @"main_$Y", i32 0, i32 3), align 1
  %and61 = and i64 %val_fetch60, 256
  %lshr62 = lshr i64 %and61, 8
  %shl63 = shl i64 %lshr62, 8
  %or64 = or i64 133, %shl63
  %and65 = and i64 %val_fetch60, 1030792151040
  %lshr66 = lshr i64 %and65, 36
  %and67 = and i64 %or64, -1030792151041
  %shl68 = shl i64 %lshr66, 36
  %or69 = or i64 %and67, %shl68
  store i64 %or69, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank1$.1", ptr @"main_$Y", i32 0, i32 3), align 1
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank1$.1", ptr @"main_$Y", i32 0, i32 1), align 1
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank1$.1", ptr @"main_$Y", i32 0, i32 4), align 1

  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank1$.1", ptr @"main_$Y", i32 0, i32 2), align 1
  %"val$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank1$.1", ptr @"main_$Y", i32 0, i32 6, i32 0, i32 2), i32 0)
  store i64 1, ptr %"val$[]", align 1
  %"val$[]5" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank1$.1", ptr @"main_$Y", i32 0, i32 6, i32 0, i32 0), i32 0)
  store i64 10, ptr %"val$[]5", align 1
  %"val$[]8" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank1$.1", ptr @"main_$Y", i32 0, i32 6, i32 0, i32 1), i32 0)
  store i64 4, ptr %"val$[]8", align 1
  %val_fetch = load i64, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank1$.1", ptr @"main_$Y", i32 0, i32 3), align 1
  %and = and i64 %val_fetch, -68451041281
  %or = or i64 %and, 1073741824
  store i64 %or, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank1$.1", ptr @"main_$Y", i32 0, i32 3), align 1
  %and12 = and i64 %val_fetch, 1
  %shl = shl i64 %and12, 1
  %int_zext = trunc i64 %shl to i32
  %and16 = and i32 %int_zext, -17
  %and18 = and i64 %val_fetch, 256
  %lshr = lshr i64 %and18, 8
  %and20 = and i32 %and16, -2097153
  %shl22 = shl i64 %lshr, 21
  %int_zext24 = trunc i64 %shl22 to i32
  %or26 = or i32 %and20, %int_zext24
  %and28 = and i64 %val_fetch, 1030792151040
  %lshr30 = lshr i64 %and28, 36
  %and32 = and i32 %or26, -31457281
  %shl34 = shl i64 %lshr30, 21
  %int_zext36 = trunc i64 %shl34 to i32
  %or38 = or i32 %and32, %int_zext36
  %and40 = and i64 %val_fetch, 1099511627776
  %lshr42 = lshr i64 %and40, 40
  %and44 = and i32 %or38, -33554433
  %shl46 = shl i64 %lshr42, 25
  %int_zext48 = trunc i64 %shl46 to i32
  %or50 = or i32 %and44, %int_zext48
  %and52 = and i32 %or50, -2031617
  %or54 = or i32 %and52, 262144
  %func_result56 = call i32 @for_alloc_allocatable(i64 40, ptr @"main_$Y", i32 %or54)
  %val_fetch80 = load ptr, ptr @"main_$Y", align 1
  %"val$[]72_fetch" = load i64, ptr %"val$[]", align 1
  %"val_fetch[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"val$[]72_fetch", i64 4, ptr elementtype(i32) %val_fetch80, i64 2)
  store i32 2, ptr %"val_fetch[]", align 1
  br label %region.entry

region.entry:                                     ; preds = %alloca_3
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0),
    "QUAL.OMP.USE_DEVICE_PTR:F90_DV.TYPED"(ptr @"main_$Y", %"QNCA_a0$i32*$rank1$.1" zeroinitializer, i32 0) ]
  br label %region.body

region.body:                                      ; preds = %region.entry
  call void @submodule_mp_work_(ptr @"main_$Y")
  br label %region.exit

region.exit:                                      ; preds = %region.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  ret void
}

declare i32 @for_set_reentrancy(ptr)

declare i32 @for_alloc_allocatable(i64, ptr, i32)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

attributes #0 = { nounwind uwtable "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind uwtable "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "openmp-variant"="name:submodule_mp_work_gpu_;construct:target_variant_dispatch;arch:gen" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone speculatable }

!omp_offload.info = !{}
; end INTEL_CUSTOMIZATION

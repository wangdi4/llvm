; INTEL_CUSTOMIZATION
; RUN: opt -opaque-pointers=0 -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

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
;       !$omp  declare variant(work_gpu) &
;       !$omp  match(construct={dispatch}, device={arch(gen)}) adjust_args(need_device_ptr:y) append_args(interop(target))
; !#ifdef DEBUG
; !      print *, "Enter work (base function). y(2) = ", y(2), "."
; !#endif
;       y(2) = 21
;     end subroutine
; end module submodule
;
; program main
;     use submodule
; !    integer, allocatable :: y(:)
; !    allocate(y(10))
; !    y(2) = 2
; !    !$omp target data map(y)
; !    call foo(z)
; !    !$omp end target data
;
; !    if (y(2) .eq. 42) then
; !    print *, "PASSED"
; !    else
; !    print *, "FAILED. work_gpu was not called. y(2) is", y(2), "."
; !    end if
;
; contains
;   subroutine foo(z)
;     integer :: z(:)
;
;     !$omp dispatch device(0)
;     call work(z)
;   end subroutine
;
; end program

; CHECK: [[DV_NEW:%.+]] = alloca %"QNCA_a0$i32*$rank1$", align 8

; CHECK: [[INTEROP_OBJ:%interop.obj.sync]] = call i8* @__tgt_create_interop_obj(i64 0, i8 0, i8* null)
; CHECK: [[ADDR0_GEP:%.+]] = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %"(%QNCA_a0$i32*$rank1$*)submodule_$z_.10$", i32 0, i32 0
; CHECK: [[ADDR0:%.+]] = load i32*, i32** [[ADDR0_GEP]], align 8

; CHECK: [[ADDR0_CAST:%.+]] = bitcast i32* [[ADDR0]] to i8*
; CHECK: [[MAP_GEP:%.+]] = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
; CHECK: store i8* [[ADDR0_CAST]], i8** [[MAP_GEP]], align 8

; CHECK: call void @__tgt_target_data_begin_mapper

; CHECK: [[MAP_GEP_CAST:%.+]] = bitcast i8** [[MAP_GEP]] to i32**
; CHECK: [[ADDR0_UPDATED:%.+]] = load i32*, i32** [[MAP_GEP_CAST]], align 8

; CHECK: [[DV_NEW_CAST:%.+]] = bitcast %"QNCA_a0$i32*$rank1$"* [[DV_NEW]] to i8*
; CHECK: [[DV_ORIG_CAST:%.+]] = bitcast %"QNCA_a0$i32*$rank1$"* %"(%QNCA_a0$i32*$rank1$*)submodule_$z_.10$" to i8*
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 [[DV_NEW_CAST]], i8* align 8 [[DV_ORIG_CAST]], i64 72, i1 false)

; CHECK: [[DV_NEW_ADDR0_GEP:%.+]] = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* [[DV_NEW]], i32 0, i32 0
; CHECK: store i32* [[ADDR0_UPDATED]], i32** [[DV_NEW_ADDR0_GEP]], align 8

; CHECK: call void bitcast (void (%"QNCA_a0$i32*$rank1$.0"*, %"ISO_C_BINDING$.btC_PTR"*)* @submodule_mp_work_gpu_ to void (%"QNCA_a0$i32*$rank1$"*, i8*)*)(%"QNCA_a0$i32*$rank1$"* [[DV_NEW]], i8* [[INTEROP_OBJ]])

; CHECK: call void @__tgt_target_data_end_mapper

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%"QNCA_a0$i32*$rank1$.0" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ISO_C_BINDING$.btC_PTR" = type <{ i64 }>
%"QNCA_a0$i32*$rank1$" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$i32*$rank1$.1" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: noinline nounwind optnone uwtable
define void @main_IP_foo_(%"QNCA_a0$i32*$rank1$.1"* noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"submodule_$z_$argptr") #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0) ]

  %"(%QNCA_a0$i32*$rank1$*)submodule_$z_.10$" = bitcast %"QNCA_a0$i32*$rank1$.1"* %"submodule_$z_$argptr" to %"QNCA_a0$i32*$rank1$"*
  call void @submodule_mp_work_(%"QNCA_a0$i32*$rank1$"* %"(%QNCA_a0$i32*$rank1$*)submodule_$z_.10$") [ "QUAL.OMP.DISPATCH.CALL"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]

  ret void
}

define void @submodule_mp_work_gpu_(%"QNCA_a0$i32*$rank1$.0"* noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"Y$argptr", %"ISO_C_BINDING$.btC_PTR"* noalias dereferenceable(8) %"HANDLE$argptr") #0 {
  ret void
}

define void @submodule_mp_work_(%"QNCA_a0$i32*$rank1$"* noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"Y$argptr") #1 {
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "openmp-variant"="name:submodule_mp_work_gpu_;construct:dispatch;arch:gen;need_device_ptr:F90_DV;interop:target" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!llvm.module.flags = !{!0}
!0 = !{i32 7, !"openmp", i32 50}
; end INTEL_CUSTOMIZATION

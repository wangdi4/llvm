; INTEL_CUSTOMIZATION
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL

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

; ALL: [[DV_NEW:%.+]] = alloca %"QNCA_a0$i32*$rank1$", align 8

; OCG: [[INTEROP_OBJ:%interop.obj.sync]] = call ptr @__tgt_create_interop_obj(i64 0, i8 0, ptr null)
; NCG: [[INTEROP_OBJ:%interop.obj]] = call ptr @__tgt_get_interop_obj(ptr @{{.*}}, i32 0, i32 0, ptr null, i64 0, i32 %my.tid, ptr %current.task)
; ALL: [[ADDR0_GEP:%.+]] = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"submodule_$z_$argptr", i32 0, i32 0
; ALL: [[ADDR0:%.+]] = load ptr, ptr [[ADDR0_GEP]], align 8

; ALL: [[MAP_GEP:%.+]] = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs, i32 0, i32 0
; ALL: store ptr [[ADDR0]], ptr [[MAP_GEP]], align 8

; ALL: call void @__tgt_target_data_begin_mapper

; ALL: [[ADDR0_UPDATED:%.+]] = load ptr, ptr [[MAP_GEP]], align 8

; ALL: call void @llvm.memcpy.p0.p0.i64(ptr align 8 [[DV_NEW]], ptr align 8 %"submodule_$z_$argptr", i64 72, i1 false)

; ALL: [[DV_NEW_ADDR0_GEP:%.+]] = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr [[DV_NEW]], i32 0, i32 0
; ALL: store ptr [[ADDR0_UPDATED]], ptr [[DV_NEW_ADDR0_GEP]], align 8

; ALL: call void @submodule_mp_work_gpu_(ptr [[DV_NEW]], ptr [[INTEROP_OBJ]])

; ALL: call void @__tgt_target_data_end_mapper

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

define void @main_IP_foo_(ptr noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"submodule_$z_$argptr") #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0) ]

  call void @submodule_mp_work_(ptr %"submodule_$z_$argptr") [ "QUAL.OMP.DISPATCH.CALL"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]

  ret void
}

define void @submodule_mp_work_gpu_(ptr noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"Y$argptr", ptr noalias dereferenceable(8) %"HANDLE$argptr") #0 {
  ret void
}

define void @submodule_mp_work_(ptr noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"Y$argptr") #1 {
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "openmp-variant"="name:submodule_mp_work_gpu_;construct:dispatch;arch:gen;need_device_ptr:F90_DV.QNCA_a0$i32*$rank1$;interop:target" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!llvm.module.flags = !{!0}
!0 = !{i32 7, !"openmp", i32 50}
; end INTEL_CUSTOMIZATION

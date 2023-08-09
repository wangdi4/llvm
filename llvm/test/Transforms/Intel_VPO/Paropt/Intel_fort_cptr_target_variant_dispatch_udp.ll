; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
; module submodule
; contains
;     subroutine work_gpu(c_yp, handle)   !! variant function
;       use iso_c_binding
; !      type(c_ptr):: c_yp, c_yp1
;       integer, pointer :: yp
;       type(c_ptr) :: handle
; !#ifdef DEBUG
; !      call c_f_pointer(c_yp, yp)
; !      print "(a,z)", "Enter work_gpu. &y = ", loc(yp)
; !#endif
; !      !$omp target is_device_ptr(c_yp) map(from:c_yp1)
; !      c_yp1 = c_yp
; !      !$omp end target
; !
; !      call c_f_pointer(c_yp1, yp)
; !      print "(a, z)", "Exit work_gpu. &y = ", loc(yp)
; !      print *, "Passed."
;     end subroutine work_gpu
;
;     subroutine work(c_yp)              !! base function
;       use iso_c_binding
;       type(c_ptr):: c_yp
;       integer, pointer :: yp
;
;       !$omp  declare variant(work_gpu) &
;       !$omp  match(construct={target variant dispatch}, device={arch(gen)})
; !      call c_f_pointer(c_yp, yp)
; !      print "(a,z)", "Enter work. &y = ", loc(yp), ", y = ", yp, "."
; !      print *, "Failed."
;     end subroutine
; end module submodule
;
; program main
;     use iso_c_binding
;     use submodule
;     integer           :: y, nb
;     type(c_ptr):: c_yp
;     y = 2
;
; !#ifdef DEBUG
; !      print "(a,z)", "main. &y = ", loc(y)
; !#endif
; !    !$omp target data map(y)
; !    c_yp = c_loc(y)
;     !$omp target variant dispatch device(0) use_device_ptr(c_yp)
;     call work(c_yp)
;     !$omp end target variant dispatch
;
; !    !$omp end target data
;
; end program

; Check that the map-type of the use_device_ptr operand is 64 (TGT_RETURN_PARAM):
; CHECK: @.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 64]

; Check for the copy of c_yp created to call the variant function.
; CHECK: [[NEWV:%[^ ]+C_YP[^ ]+]] = alloca ptr, align 8

; Check for the map created to obtain the device pointer for c_yp
; CHECK: [[ORIG_VAL:%[^ ]+]] = load ptr, ptr %"main_$C_YP", align 8
; CHECK: [[MAP_GEP:%[^ ]+]] = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs{{[^ ,]*}}, i32 0, i32 0
; CHECK: store ptr [[ORIG_VAL]], ptr [[MAP_GEP]], align 8

; Check that the outlined function for variant region is called with an updated value for the c_yp
; CHECK: [[UPDATED_VAL:%[^ ]+]] = load ptr, ptr [[MAP_GEP]], align 8
; CHECK: store ptr [[UPDATED_VAL:%[^ ]+]], ptr [[NEWV]], align 8
; CHECK: call void @[[VARIANT_WRAPPER:[^ ]*work_gpu_.wrapper[^ (]*]](ptr [[NEWV]])


; Check that variant function is called in the variant wrapper.
; CHECK: define internal void @[[VARIANT_WRAPPER]](ptr [[NEWV_PASSED:%[^ ,]+]])
; CHECK: [[INTEROP:%[^ ]+]] = call ptr @__tgt_create_interop_obj(i64 0, i8 0, ptr null)
; CHECK: call void @submodule_mp_work_gpu_(ptr [[NEWV_PASSED]], ptr [[INTEROP]])
; CHECK: %{{.+}} = call i32 @__tgt_release_interop_obj(ptr [[INTEROP]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%"ISO_C_BINDING$.btC_PTR" = type { i64 }

@0 = internal unnamed_addr constant i32 2

; Function Attrs: noinline nounwind uwtable
define void @MAIN__() #0 {
alloca_3:
  %"var$3" = alloca [8 x i64], align 8
  %"main_$C_YP" = alloca %"ISO_C_BINDING$.btC_PTR", align 8
  %"main_$NB" = alloca i32, align 8
  %"main_$Y" = alloca i32, align 8
  %func_result = call i32 @for_set_reentrancy(ptr @0)
  store i32 2, ptr %"main_$Y", align 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0),
    "QUAL.OMP.USE_DEVICE_PTR:CPTR"(ptr %"main_$C_YP") ]

  call void @submodule_mp_work_(ptr %"main_$C_YP")

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  ret void
}

; Function Attrs: noinline nounwind uwtable
declare void @submodule_mp_work_gpu_(ptr dereferenceable(4), ptr dereferenceable(8)) #0

; Function Attrs: noinline nounwind uwtable
declare void @submodule_mp_work_(ptr dereferenceable(8)) #1

declare i32 @for_set_reentrancy(ptr)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind uwtable "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { noinline nounwind uwtable "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "openmp-variant"="name:submodule_mp_work_gpu_;construct:target_variant_dispatch;arch:gen" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #2 = { nounwind }

!omp_offload.info = !{}
; end INTEL_CUSTOMIZATION

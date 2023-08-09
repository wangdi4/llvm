; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test for CPTR operands on use_device_ptr on target data constructs.

; Test src:
;   program main
;       use iso_c_binding
;       type(c_ptr) :: a_cptr
;       integer, pointer :: a_fptr
;       integer :: a
;       a_cptr = c_loc(a)
;       a = 10
; !      print "(z)", loc(a) ! Should print host address
;
;       !$omp target data map(a) use_device_ptr(a_cptr)
;       call foo(a_cptr)
;       !$omp end target data
;
; !   contains
; !       subroutine foo(a_cptr1)
; !           type(c_ptr) ::a_cptr1, a_cptr2
; !           integer, pointer :: a_fptr
; !           call c_f_pointer(a_cptr1, a_fptr)
; !           print "(z)", loc(a_fptr) ! Should print device address
; !
; !           !$omp target is_device_ptr(a_cptr1) map(from:a_cptr2)
; !           a_cptr2 = a_cptr1
; !           !$omp end target
; !
; !           call c_f_pointer(a_cptr2, a_fptr)
; !           print "(z)", loc(a_fptr) ! Should print device address
; !       end subroutine
;   end program

; Check that the map-type the use_device_ptr operand is 64 (TGT_RETURN_PARAM):
; CHECK: @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 3, i64 64]

; Check for the local copy of %"main_$A_CPTR". Since the input clause is untyped,
; the type used is "ptr" for the CPTR clause operand.
; CHECK: [[NEWV:[^ ]+.new"]] = alloca ptr, align 8

; Check that the map we emit is for "load i8*, (bitcast %a_cptr to i8**)"
; CHECK: [[VAL:%[^ ]+]] = load ptr, ptr %"main_$A_CPTR", align 8
; CHECK: [[MAP_GEP:%[^ ]+]] = getelementptr inbounds [2 x ptr], ptr %.offload_baseptrs{{[^ ,]*}}, i32 0, i32 1
; CHECK: store ptr [[VAL]], ptr [[MAP_GEP]], align 8

; Check that the updated value is store to NEWV and then passed into the target data region.
; CHECK: [[UPDATED_VAL:%[^ ]+]] = load ptr, ptr [[MAP_GEP]], align 8
; CHECK: store ptr [[UPDATED_VAL]], ptr [[NEWV]], align 8
; CHECK: call void @MAIN__.DIR.OMP.TARGET.DATA{{[^ (]+}}(ptr [[NEWV]], ptr %"main_$A")

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%"ISO_C_BINDING$.btC_PTR" = type { i64 }

@0 = internal unnamed_addr constant i32 2
; Function Attrs: noinline nounwind uwtable
define void @MAIN__() #0 {
alloca_0:
  %"var$1" = alloca [8 x i64], align 8
  %"main_$A" = alloca i32, align 8
  %"main_$A_FPTR" = alloca ptr, align 8
  %"main_$A_CPTR" = alloca %"ISO_C_BINDING$.btC_PTR", align 8
  %"var$2" = alloca %"ISO_C_BINDING$.btC_PTR", align 8
  %func_result = call i32 @for_set_reentrancy(ptr @0)
  call void @iso_c_binding_mp_c_loc_private_(ptr %"var$2", ptr %"main_$A")
  %"var$2_fetch" = load %"ISO_C_BINDING$.btC_PTR", ptr %"var$2", align 1
  call void @llvm.memcpy.p0.p0.i64(ptr %"main_$A_CPTR", ptr %"var$2", i64 8, i1 false)
  store i32 10, ptr %"main_$A", align 1
  br label %bb12

bb12:                                             ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.USE_DEVICE_PTR:CPTR"(ptr %"main_$A_CPTR"),
    "QUAL.OMP.MAP.TOFROM"(ptr %"main_$A", ptr %"main_$A", i64 4, i64 3, ptr null, ptr null) ]

  call void @foo_.t0p(ptr %"main_$A_CPTR")

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

declare void @iso_c_binding_mp_c_loc_private_(ptr dereferenceable(8), ptr nocapture readonly dereferenceable(4))

declare i32 @for_set_reentrancy(ptr)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: noinline nounwind uwtable
define internal void @foo_.t0p(ptr %arg0) #2 {
wrap_start:
  call void (...) @foo_(ptr %arg0)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare void @foo_(...)

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

attributes #0 = { noinline nounwind uwtable "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #3 = { argmemonly nofree nounwind willreturn }

!omp_offload.info = !{}
; end INTEL_CUSTOMIZATION

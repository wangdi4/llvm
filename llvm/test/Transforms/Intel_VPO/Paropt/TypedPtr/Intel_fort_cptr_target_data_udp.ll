; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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

; Check for the local copy of %"main_$A_CPTR"
; CHECK: [[NEWV:[^ ]+.new"]] = alloca %"ISO_C_BINDING$.btC_PTR", align 8

; Check that the map we emit is for "load i8*, (bitcast %a_cptr to i8**)"
; CHECK: [[ORIG_CAST:%[^ ]+]] = bitcast %"ISO_C_BINDING$.btC_PTR"* %"main_$A_CPTR" to i8**
; CHECK: [[VAL:%[^ ]+]] = load i8*, i8** [[ORIG_CAST]], align 8
; CHECK: [[MAP_GEP:%[^ ]+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs{{[^ ,]*}}, i32 0, i32 1
; CHECK: store i8* [[VAL]], i8** [[MAP_GEP]], align 8

; Check that the updated value is store to NEWV and then passed into the target data region.
; CHECK: [[UPDATED_VAL:%[^ ]+]] = load i8*, i8** [[MAP_GEP]], align 8
; CHECK: [[NEWV_CAST:%[^ ]+]] = bitcast %"ISO_C_BINDING$.btC_PTR"* [[NEWV]] to i8**
; CHECK: store i8* [[UPDATED_VAL]], i8** [[NEWV_CAST]], align 8
; CHECK: call void @MAIN__.DIR.OMP.TARGET.DATA{{[^ (]+}}(%"ISO_C_BINDING$.btC_PTR"* [[NEWV]], i32* %"main_$A")

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
  %"main_$A_FPTR" = alloca i32*, align 8
  %"main_$A_CPTR" = alloca %"ISO_C_BINDING$.btC_PTR", align 8
  %"var$2" = alloca %"ISO_C_BINDING$.btC_PTR", align 8
  %func_result = call i32 @for_set_reentrancy(i32* @0)
  call void @iso_c_binding_mp_c_loc_private_(%"ISO_C_BINDING$.btC_PTR"* %"var$2", i32* %"main_$A")
  %"var$2_fetch" = load %"ISO_C_BINDING$.btC_PTR", %"ISO_C_BINDING$.btC_PTR"* %"var$2", align 1
  call void @"llvm.memcpy.p0s_ISO_C_BINDING$.btC_PTRs.p0s_ISO_C_BINDING$.btC_PTRs.i64"(%"ISO_C_BINDING$.btC_PTR"* %"main_$A_CPTR", %"ISO_C_BINDING$.btC_PTR"* %"var$2", i64 8, i1 false)
  store i32 10, i32* %"main_$A", align 1
  br label %bb12

bb12:                                             ; preds = %alloca_0
%0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.USE_DEVICE_PTR:CPTR"(%"ISO_C_BINDING$.btC_PTR"* %"main_$A_CPTR"), "QUAL.OMP.MAP.TOFROM"(i32* %"main_$A", i32* %"main_$A", i64 4, i64 3, i8* null, i8* null) ]

  call void @foo_.t0p(%"ISO_C_BINDING$.btC_PTR"* %"main_$A_CPTR")

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

declare void @iso_c_binding_mp_c_loc_private_(%"ISO_C_BINDING$.btC_PTR"* dereferenceable(8) %0, i32* nocapture readonly dereferenceable(4) %1)

declare i32 @for_set_reentrancy(i32* %0)

; Function Attrs: argmemonly nounwind willreturn
declare void @"llvm.memcpy.p0s_ISO_C_BINDING$.btC_PTRs.p0s_ISO_C_BINDING$.btC_PTRs.i64"(%"ISO_C_BINDING$.btC_PTR"* noalias nocapture writeonly %0, %"ISO_C_BINDING$.btC_PTR"* noalias nocapture readonly %1, i64 %2, i1 immarg %3) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: noinline nounwind uwtable
define internal void @foo_.t0p(%"ISO_C_BINDING$.btC_PTR"* %arg0) #3 {
wrap_start:
  call void (...) @foo_(%"ISO_C_BINDING$.btC_PTR"* %arg0)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #2

declare void @foo_(...)

attributes #0 = { noinline nounwind uwtable "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { noinline nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
; end INTEL_CUSTOMIZATION

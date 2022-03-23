; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test for CPTR operands on is_device_ptr on target constructs.

;   program main
;       use iso_c_binding
; !      type(c_ptr) :: a_cptr
; !      integer, pointer :: a_fptr
; !      integer :: a
; !      a_cptr = c_loc(a)
; !      a = 10
; !      print "(z)", loc(a) ! Should print host address
; !
; !      !$omp target data map(a) use_device_ptr(a_cptr)
; !      call foo(a_cptr)
; !      !$omp end target data
; !
;   contains
;       subroutine foo(a_cptr1)
;           type(c_ptr) ::a_cptr1, a_cptr2
;           integer, pointer :: a_fptr
; !          call c_f_pointer(a_cptr1, a_fptr)
; !          print "(z)", loc(a_fptr) ! Should print device address
;
;           !$omp target is_device_ptr(a_cptr1) map(from:a_cptr2)
;           a_cptr2 = a_cptr1
;           !$omp end target
;
; !          call c_f_pointer(a_cptr2, a_fptr)
; !          print "(z)", loc(a_fptr) ! Should print device address
;       end subroutine
;   end program

; Check that the map-type of the is_device_ptr operand is 288 (TGT_PARAM | TGT_LITERAL):
; CHECK: @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 34, i64 288]

; Check that the map we emit is for "load i8*, (bitcast %a_cptr1 to i8**)"
; CHECK: [[ORIG_CAST:%[^ ]+]] = bitcast %"ISO_C_BINDING$.btC_PTR"* %A_CPTR1 to i8**
; CHECK: [[VAL:%[^ ]+]] = load i8*, i8** [[ORIG_CAST]], align 8
; CHECK: [[MAP_GEP:%[^ ]+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs{{[^ ,]*}}, i32 0, i32 1
; CHECK: store i8* [[VAL]], i8** [[MAP_GEP]], align 8

; Check that the loaded value VAL is passed into the target region, and a
; new cptr is initialized using it.
; CHECK: call void @__omp_offloading{{[^ (]+}}(%"ISO_C_BINDING$.btC_PTR"* %"foo$A_CPTR2$_3", i8* [[VAL]])
; CHECK-LABEL: define internal void @__omp_offloading{{[^ (]+}}
; CHECK-SAME: (%"ISO_C_BINDING$.btC_PTR"* %{{[^ ,]+}}, i8* [[VAL_PASSED:%[^ ,]+]])
; CHECK: [[NEWV:%A_CPTR1[^ ]+]] = alloca %"ISO_C_BINDING$.btC_PTR", align 1
; CHECK: [[NEWV_CAST:%[^ ]+]] = bitcast %"ISO_C_BINDING$.btC_PTR"* [[NEWV]] to i8**
; CHECK: store i8* [[VAL_PASSED]], i8** [[NEWV_CAST]], align 8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%"ISO_C_BINDING$.btC_PTR" = type { i64 }

@0 = internal unnamed_addr constant i32 2

; Function Attrs: noinline nounwind uwtable
define void @main_IP_foo_(%"ISO_C_BINDING$.btC_PTR"* dereferenceable(8) %A_CPTR1) #0 {
alloca_1:
  %"var$2" = alloca [8 x i64], align 8
  %"foo$A_FPTR$_3" = alloca i32*, align 8
  %"foo$A_CPTR2$_3" = alloca %"ISO_C_BINDING$.btC_PTR", align 8

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.FROM"(%"ISO_C_BINDING$.btC_PTR"* %"foo$A_CPTR2$_3", %"ISO_C_BINDING$.btC_PTR"* %"foo$A_CPTR2$_3", i64 8, i64 34, i8* null, i8* null), "QUAL.OMP.IS_DEVICE_PTR:CPTR"(%"ISO_C_BINDING$.btC_PTR"* %A_CPTR1) ]

  %A_CPTR1_fetch = load %"ISO_C_BINDING$.btC_PTR", %"ISO_C_BINDING$.btC_PTR"* %A_CPTR1, align 1
  call void @"llvm.memcpy.p0s_ISO_C_BINDING$.btC_PTRs.p0s_ISO_C_BINDING$.btC_PTRs.i64"(%"ISO_C_BINDING$.btC_PTR"* %"foo$A_CPTR2$_3", %"ISO_C_BINDING$.btC_PTR"* %A_CPTR1, i64 8, i1 false)

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare i32 @for_set_reentrancy(i32* %0)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: argmemonly nounwind willreturn
declare void @"llvm.memcpy.p0s_ISO_C_BINDING$.btC_PTRs.p0s_ISO_C_BINDING$.btC_PTRs.i64"(%"ISO_C_BINDING$.btC_PTR"* noalias nocapture writeonly %0, %"ISO_C_BINDING$.btC_PTR"* noalias nocapture readonly %1, i64 %2, i1 immarg %3) #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

attributes #0 = { noinline nounwind uwtable "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind willreturn }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2055, i32 150321767, !"main_IP_foo_", i32 21, i32 0, i32 0}
; end INTEL_CUSTOMIZATION

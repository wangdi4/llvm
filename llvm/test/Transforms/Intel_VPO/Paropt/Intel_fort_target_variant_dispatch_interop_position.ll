; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s
;
; In Fortran, when the variant function has string parameters, the compiler adds
; special hidden parameters at the end of the function call for the string lengths.
; In this case, we cannot append the interop_obj parameter at the end of the
; function call. The FFE emits an interop position number in the declare variant
; attribute, and the compiler emits the interop_obj in that position.
;
; Example:
;
; module submodule
; contains
;   SUBROUTINE foo_gpu( string, num, interop )
;     USE iso_c_binding,   ONLY : c_ptr
;     CHARACTER(LEN=*), INTENT(IN) :: string
;     INTEGER, INTENT(IN)          :: num
;     TYPE(c_ptr)                  :: interop
;     print *, "Enter Variant function:", string, num
;   END SUBROUTINE foo_gpu
;   SUBROUTINE foo( string, num )
;     CHARACTER(LEN=*), INTENT(IN) :: string
;     INTEGER, INTENT(IN)          :: num
;     !$omp declare variant (foo_gpu) match( construct={target variant dispatch}, device={arch(gen9)} )
;     print *, "Enter Base function:", string, num
;   END SUBROUTINE foo
; end module submodule
;
; program main
; use submodule
;   !$omp target variant dispatch device(0)
;     call foo("hello", 123)
;   !$omp end target variant dispatch
; end program
;
;
; The "openmp-variant" attribute of the variant function foo_gpu is:
;   "openmp-variant"="name:submodule_mp_foo_gpu_;construct:target_variant_dispatch;arch:gen9;interop_position:3"
;
; so Paropt invokes foo_gpu as:
;   call foo_gpu(string, num, interop_obj, hidden_parameter_for_string_length)
;   (arg position:  1     2     **3**          4 )
;
; CHECK: [[INTEROPOBJ:%[^ ]+]] = call ptr @__tgt_create_interop_obj(i64 0, i8 0, ptr null)
; CHECK-NEXT: call{{.*}}@submodule_mp_foo_gpu_{{.*}}(ptr @strlit.2, ptr @0, ptr [[INTEROPOBJ]], i64 5)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%"ISO_C_BINDING$.btC_PTR" = type { i64 }

@strlit.2 = internal unnamed_addr constant [5 x i8] c"hello"
@0 = internal unnamed_addr constant i32 123

define void @MAIN__() {
alloca_3:
  %"$io_ctx" = alloca [8 x i64], align 16
  %strlit.2_fetch.13 = load [5 x i8], ptr @strlit.2, align 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0) ]
  call void @submodule_mp_foo_(ptr @strlit.2, ptr @0, i64 5)
  br label %bb5

bb5:                                              ; preds = %alloca_3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  ret void
}

declare void @submodule_mp_foo_gpu_(ptr readonly %STRING, ptr readonly dereferenceable(4) %NUM, ptr dereferenceable(8) %INTEROP, i64 %"var$1$val")
declare void @submodule_mp_foo_(ptr readonly %STRING, ptr readonly dereferenceable(4) %NUM, i64 %"var$2$val") #0
declare i32 @for_write_seq_lis(ptr %0, i32 %1, i64 %2, ptr %3, ptr %4, ...)
declare i32 @for_write_seq_lis_xmit(ptr nocapture readonly %0, ptr nocapture readonly %1, ptr %2)
declare i32 @for_set_reentrancy(ptr nocapture readonly %0)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)

attributes #0 = { "openmp-variant"="name:submodule_mp_foo_gpu_;construct:target_variant_dispatch;arch:gen9;interop_position:3" }
; end INTEL_CUSTOMIZATION

; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
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
; CHECK: [[INTEROPOBJ:%[^ ]+]] = call i8* @__tgt_create_interop_obj(i64 0, i8 0, i8* null)
; CHECK-NEXT: call{{.*}}@submodule_mp_foo_gpu_{{.*}}(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @strlit.2, i32 0, i32 0), i32* @1, i8* [[INTEROPOBJ]], i64 5)

source_filename = "lit_position.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%"ISO_C_BINDING$.btC_PTR" = type { i64 }
%"QNCA_a0$i8*$rank0$" = type { i8*, i64, i64, i64, i64, i64 }
%"QNCA_a0$i8*$rank0$.0" = type { i8*, i64, i64, i64, i64, i64 }

@strlit = internal unnamed_addr constant [20 x i8] c"Enter Base function:"
@strlit.1 = internal unnamed_addr constant [23 x i8] c"Enter Variant function:"
@strlit.2 = internal unnamed_addr constant [5 x i8] c"hello"
@0 = internal unnamed_addr constant i32 2
@1 = internal unnamed_addr constant i32 123

; Function Attrs: nounwind uwtable
define void @submodule._() #0 {
alloca_0:
  ret void
}

; Function Attrs: nounwind uwtable
define void @submodule_mp_foo_gpu_(i8* readonly %STRING, i32* readonly dereferenceable(4) %NUM, %"ISO_C_BINDING$.btC_PTR"* dereferenceable(8) %INTEROP, i64 %"var$1$val") #0 {
alloca_1:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"var$3" = alloca %"QNCA_a0$i8*$rank0$", align 8
  %"var$1" = alloca i64, align 8
  %"var$4" = alloca i32, align 4
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca { i64, i8* }, align 8
  %"(&)val$5" = alloca [4 x i8], align 1
  %argblock6 = alloca { i64, i8* }, align 8
  %"(&)val$11" = alloca [4 x i8], align 1
  %argblock12 = alloca { i32 }, align 8
  store i64 %"var$1$val", i64* %"var$1", align 1
  %strlit.1_fetch.3 = load [23 x i8], [23 x i8]* @strlit.1, align 1
  %"var$3.flags$" = getelementptr inbounds %"QNCA_a0$i8*$rank0$", %"QNCA_a0$i8*$rank0$"* %"var$3", i32 0, i32 3
  store i64 1, i64* %"var$3.flags$", align 1
  %"var$1_fetch.1" = load i64, i64* %"var$1", align 1
  %"var$3.addr_length$" = getelementptr inbounds %"QNCA_a0$i8*$rank0$", %"QNCA_a0$i8*$rank0$"* %"var$3", i32 0, i32 1
  store i64 %"var$1_fetch.1", i64* %"var$3.addr_length$", align 1
  %"var$3.dim$" = getelementptr inbounds %"QNCA_a0$i8*$rank0$", %"QNCA_a0$i8*$rank0$"* %"var$3", i32 0, i32 4
  store i64 0, i64* %"var$3.dim$", align 1
  %"var$3.codim$" = getelementptr inbounds %"QNCA_a0$i8*$rank0$", %"QNCA_a0$i8*$rank0$"* %"var$3", i32 0, i32 2
  store i64 0, i64* %"var$3.codim$", align 1
  %"var$3.addr_a0$" = getelementptr inbounds %"QNCA_a0$i8*$rank0$", %"QNCA_a0$i8*$rank0$"* %"var$3", i32 0, i32 0
  store i8* %STRING, i8** %"var$3.addr_a0$", align 1
  %"var$3.flags$1" = getelementptr inbounds %"QNCA_a0$i8*$rank0$", %"QNCA_a0$i8*$rank0$"* %"var$3", i32 0, i32 3
  %"var$3.flags$_fetch.2" = load i64, i64* %"var$3.flags$1", align 1
  %or.1 = or i64 %"var$3.flags$_fetch.2", 1
  %"var$3.flags$2" = getelementptr inbounds %"QNCA_a0$i8*$rank0$", %"QNCA_a0$i8*$rank0$"* %"var$3", i32 0, i32 3
  store i64 %or.1, i64* %"var$3.flags$2", align 1
  store [4 x i8] c"8\04\02\00", [4 x i8]* %"(&)val$", align 1
  %"argblock.field_0$" = getelementptr inbounds { i64, i8* }, { i64, i8* }* %argblock, i32 0, i32 0
  store i64 23, i64* %"argblock.field_0$", align 1
  %"argblock.field_1$" = getelementptr inbounds { i64, i8* }, { i64, i8* }* %argblock, i32 0, i32 1
  store i8* getelementptr inbounds ([23 x i8], [23 x i8]* @strlit.1, i32 0, i32 0), i8** %"argblock.field_1$", align 1
  %"(i8*)$io_ctx$" = bitcast [8 x i64]* %"$io_ctx" to i8*
  %"(i8*)(&)val$$" = bitcast [4 x i8]* %"(&)val$" to i8*
  %"(i8*)argblock$" = bitcast { i64, i8* }* %argblock to i8*
  %func_result = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %"(i8*)$io_ctx$", i32 -1, i64 1239157112576, i8* %"(i8*)(&)val$$", i8* %"(i8*)argblock$")
  %"var$3.addr_a0$4" = getelementptr inbounds %"QNCA_a0$i8*$rank0$", %"QNCA_a0$i8*$rank0$"* %"var$3", i32 0, i32 0
  %"var$3.addr_a0$4_fetch.4" = load i8*, i8** %"var$3.addr_a0$4", align 1
  %"var$1_fetch.5" = load i64, i64* %"var$1", align 1
  store [4 x i8] c"8\04\02\00", [4 x i8]* %"(&)val$5", align 1
  %"argblock6.field_0$" = getelementptr inbounds { i64, i8* }, { i64, i8* }* %argblock6, i32 0, i32 0
  store i64 %"var$1_fetch.5", i64* %"argblock6.field_0$", align 1
  %"argblock6.field_1$" = getelementptr inbounds { i64, i8* }, { i64, i8* }* %argblock6, i32 0, i32 1
  store i8* %"var$3.addr_a0$4_fetch.4", i8** %"argblock6.field_1$", align 1
  %"(i8*)$io_ctx$8" = bitcast [8 x i64]* %"$io_ctx" to i8*
  %"(i8*)(&)val$5$" = bitcast [4 x i8]* %"(&)val$5" to i8*
  %"(i8*)argblock6$" = bitcast { i64, i8* }* %argblock6 to i8*
  %func_result10 = call i32 @for_write_seq_lis_xmit(i8* %"(i8*)$io_ctx$8", i8* %"(i8*)(&)val$5$", i8* %"(i8*)argblock6$")
  %NUM_fetch.6 = load i32, i32* %NUM, align 1
  store [4 x i8] c"\09\01\01\00", [4 x i8]* %"(&)val$11", align 1
  %"argblock12.field_0$" = getelementptr inbounds { i32 }, { i32 }* %argblock12, i32 0, i32 0
  store i32 %NUM_fetch.6, i32* %"argblock12.field_0$", align 1
  %"(i8*)$io_ctx$14" = bitcast [8 x i64]* %"$io_ctx" to i8*
  %"(i8*)(&)val$11$" = bitcast [4 x i8]* %"(&)val$11" to i8*
  %"(i8*)argblock12$" = bitcast { i32 }* %argblock12 to i8*
  %func_result16 = call i32 @for_write_seq_lis_xmit(i8* %"(i8*)$io_ctx$14", i8* %"(i8*)(&)val$11$", i8* %"(i8*)argblock12$")
  ret void
}

; Function Attrs: nounwind uwtable
define void @submodule_mp_foo_(i8* readonly %STRING, i32* readonly dereferenceable(4) %NUM, i64 %"var$2$val") #1 {
alloca_2:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"var$5" = alloca %"QNCA_a0$i8*$rank0$.0", align 8
  %"var$2" = alloca i64, align 8
  %"var$6" = alloca i32, align 4
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca { i64, i8* }, align 8
  %"(&)val$5" = alloca [4 x i8], align 1
  %argblock6 = alloca { i64, i8* }, align 8
  %"(&)val$11" = alloca [4 x i8], align 1
  %argblock12 = alloca { i32 }, align 8
  store i64 %"var$2$val", i64* %"var$2", align 1
  %strlit_fetch.9 = load [20 x i8], [20 x i8]* @strlit, align 1
  %"var$5.flags$" = getelementptr inbounds %"QNCA_a0$i8*$rank0$.0", %"QNCA_a0$i8*$rank0$.0"* %"var$5", i32 0, i32 3
  store i64 1, i64* %"var$5.flags$", align 1
  %"var$2_fetch.7" = load i64, i64* %"var$2", align 1
  %"var$5.addr_length$" = getelementptr inbounds %"QNCA_a0$i8*$rank0$.0", %"QNCA_a0$i8*$rank0$.0"* %"var$5", i32 0, i32 1
  store i64 %"var$2_fetch.7", i64* %"var$5.addr_length$", align 1
  %"var$5.dim$" = getelementptr inbounds %"QNCA_a0$i8*$rank0$.0", %"QNCA_a0$i8*$rank0$.0"* %"var$5", i32 0, i32 4
  store i64 0, i64* %"var$5.dim$", align 1
  %"var$5.codim$" = getelementptr inbounds %"QNCA_a0$i8*$rank0$.0", %"QNCA_a0$i8*$rank0$.0"* %"var$5", i32 0, i32 2
  store i64 0, i64* %"var$5.codim$", align 1
  %"var$5.addr_a0$" = getelementptr inbounds %"QNCA_a0$i8*$rank0$.0", %"QNCA_a0$i8*$rank0$.0"* %"var$5", i32 0, i32 0
  store i8* %STRING, i8** %"var$5.addr_a0$", align 1
  %"var$5.flags$1" = getelementptr inbounds %"QNCA_a0$i8*$rank0$.0", %"QNCA_a0$i8*$rank0$.0"* %"var$5", i32 0, i32 3
  %"var$5.flags$_fetch.8" = load i64, i64* %"var$5.flags$1", align 1
  %or.2 = or i64 %"var$5.flags$_fetch.8", 1
  %"var$5.flags$2" = getelementptr inbounds %"QNCA_a0$i8*$rank0$.0", %"QNCA_a0$i8*$rank0$.0"* %"var$5", i32 0, i32 3
  store i64 %or.2, i64* %"var$5.flags$2", align 1
  store [4 x i8] c"8\04\02\00", [4 x i8]* %"(&)val$", align 1
  %"argblock.field_0$" = getelementptr inbounds { i64, i8* }, { i64, i8* }* %argblock, i32 0, i32 0
  store i64 20, i64* %"argblock.field_0$", align 1
  %"argblock.field_1$" = getelementptr inbounds { i64, i8* }, { i64, i8* }* %argblock, i32 0, i32 1
  store i8* getelementptr inbounds ([20 x i8], [20 x i8]* @strlit, i32 0, i32 0), i8** %"argblock.field_1$", align 1
  %"(i8*)$io_ctx$" = bitcast [8 x i64]* %"$io_ctx" to i8*
  %"(i8*)(&)val$$" = bitcast [4 x i8]* %"(&)val$" to i8*
  %"(i8*)argblock$" = bitcast { i64, i8* }* %argblock to i8*
  %func_result = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %"(i8*)$io_ctx$", i32 -1, i64 1239157112576, i8* %"(i8*)(&)val$$", i8* %"(i8*)argblock$")
  %"var$5.addr_a0$4" = getelementptr inbounds %"QNCA_a0$i8*$rank0$.0", %"QNCA_a0$i8*$rank0$.0"* %"var$5", i32 0, i32 0
  %"var$5.addr_a0$4_fetch.10" = load i8*, i8** %"var$5.addr_a0$4", align 1
  %"var$2_fetch.11" = load i64, i64* %"var$2", align 1
  store [4 x i8] c"8\04\02\00", [4 x i8]* %"(&)val$5", align 1
  %"argblock6.field_0$" = getelementptr inbounds { i64, i8* }, { i64, i8* }* %argblock6, i32 0, i32 0
  store i64 %"var$2_fetch.11", i64* %"argblock6.field_0$", align 1
  %"argblock6.field_1$" = getelementptr inbounds { i64, i8* }, { i64, i8* }* %argblock6, i32 0, i32 1
  store i8* %"var$5.addr_a0$4_fetch.10", i8** %"argblock6.field_1$", align 1
  %"(i8*)$io_ctx$8" = bitcast [8 x i64]* %"$io_ctx" to i8*
  %"(i8*)(&)val$5$" = bitcast [4 x i8]* %"(&)val$5" to i8*
  %"(i8*)argblock6$" = bitcast { i64, i8* }* %argblock6 to i8*
  %func_result10 = call i32 @for_write_seq_lis_xmit(i8* %"(i8*)$io_ctx$8", i8* %"(i8*)(&)val$5$", i8* %"(i8*)argblock6$")
  %NUM_fetch.12 = load i32, i32* %NUM, align 1
  store [4 x i8] c"\09\01\01\00", [4 x i8]* %"(&)val$11", align 1
  %"argblock12.field_0$" = getelementptr inbounds { i32 }, { i32 }* %argblock12, i32 0, i32 0
  store i32 %NUM_fetch.12, i32* %"argblock12.field_0$", align 1
  %"(i8*)$io_ctx$14" = bitcast [8 x i64]* %"$io_ctx" to i8*
  %"(i8*)(&)val$11$" = bitcast [4 x i8]* %"(&)val$11" to i8*
  %"(i8*)argblock12$" = bitcast { i32 }* %argblock12 to i8*
  %func_result16 = call i32 @for_write_seq_lis_xmit(i8* %"(i8*)$io_ctx$14", i8* %"(i8*)(&)val$11$", i8* %"(i8*)argblock12$")
  ret void
}

declare i32 @for_write_seq_lis(i8* %0, i32 %1, i64 %2, i8* %3, i8* %4, ...)

declare i32 @for_write_seq_lis_xmit(i8* nocapture readonly %0, i8* nocapture readonly %1, i8* %2)

; Function Attrs: nounwind uwtable
define void @MAIN__() #0 {
alloca_3:
  %"$io_ctx" = alloca [8 x i64], align 16
  %strlit.2_fetch.13 = load [5 x i8], [5 x i8]* @strlit.2, align 1
  %func_result = call i32 @for_set_reentrancy(i32* @0)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(), "QUAL.OMP.DEVICE"(i32 0) ]
  call void @submodule_mp_foo_(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @strlit.2, i32 0, i32 0), i32* @1, i64 5)
  br label %bb5

bb5:                                              ; preds = %alloca_3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  ret void
}

declare i32 @for_set_reentrancy(i32* nocapture readonly %0)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #2

attributes #0 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "openmp-variant"="name:submodule_mp_foo_gpu_;construct:target_variant_dispatch;arch:gen9;interop_position:3" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #2 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
; end INTEL_CUSTOMIZATION

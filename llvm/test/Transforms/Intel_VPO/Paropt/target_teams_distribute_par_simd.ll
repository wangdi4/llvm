; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-paropt-enable-device-simd-codegen -vpo-paropt-emit-spirv-builtins -vpo-paropt-gpu-execution-scheme=0 -enable-device-simd -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -vpo-paropt-enable-device-simd-codegen -vpo-paropt-emit-spirv-builtins -vpo-paropt-gpu-execution-scheme=0 -enable-device-simd -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; #include <iostream>
; #include <cstdlib>
; #include <cmath>
; #include <complex>
; using std::complex;
;
; #pragma omp declare reduction(+: complex<double>: omp_out += omp_in)
; void test_target_teams__distribute_par_simd() {
;    const int N0 { 262144 };
;    const complex<double> expected_value { N0 };
;    complex<double> counter_N0{};
;    #pragma omp target teams map(tofrom: counter_N0) reduction(+: counter_N0)
;    #pragma omp distribute parallel for simd reduction(+: counter_N0)
;    for (int i0 = 0 ; i0 < N0 ; i0++ ) {
;         counter_N0 = counter_N0 + complex<double> {  1. };
;    }
; }

; CHECK: call spir_func i64 @_Z27__spirv_LocalInvocationId_xv()
; CHECK: call spir_func i64 @_Z27__spirv_LocalInvocationId_yv()
; CHECK: call spir_func i64 @_Z27__spirv_LocalInvocationId_zv()

; ModuleID = 'target_teams_distribute_par_simd.cpp'
source_filename = "target_teams_distribute_par_simd.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%"struct.std::complex" = type { { double, double } }

$_ZNSt7complexIdEC2Edd = comdat any

$_ZNSt7complexIdEpLIdEERS0_RKS_IT_E = comdat any

$_ZStplIdESt7complexIT_ERKS2_S4_ = comdat any

$_ZNKSt7complexIdE4realB5cxx11Ev = comdat any

$_ZNKSt7complexIdE4imagB5cxx11Ev = comdat any

@"@tid.addr" = external global i32

; Function Attrs: convergent noinline nounwind mustprogress
define hidden spir_func void @_Z38test_target_teams__distribute_par_simdv() #0 {
entry:
  %N0 = alloca i32, align 4
  %N0.ascast = addrspacecast ptr %N0 to ptr addrspace(4)
  %expected_value = alloca %"struct.std::complex", align 8
  %expected_value.ascast = addrspacecast ptr %expected_value to ptr addrspace(4)
  %counter_N0 = alloca %"struct.std::complex", align 8
  %counter_N0.ascast = addrspacecast ptr %counter_N0 to ptr addrspace(4)
  %i0 = alloca i32, align 4
  %i0.ascast = addrspacecast ptr %i0 to ptr addrspace(4)
  %ref.tmp = alloca %"struct.std::complex", align 8
  %ref.tmp.ascast = addrspacecast ptr %ref.tmp to ptr addrspace(4)
  %ref.tmp1 = alloca %"struct.std::complex", align 8
  %ref.tmp1.ascast = addrspacecast ptr %ref.tmp1 to ptr addrspace(4)
  store i32 262144, ptr addrspace(4) %N0.ascast, align 4, !tbaa !7
  call spir_func void @_ZNSt7complexIdEC2Edd(ptr addrspace(4) dereferenceable_or_null(16) %expected_value.ascast, double 2.621440e+05, double 0.000000e+00) #7
  call spir_func void @_ZNSt7complexIdEC2Edd(ptr addrspace(4) dereferenceable_or_null(16) %counter_N0.ascast, double 0.000000e+00, double 0.000000e+00) #7
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %counter_N0.ascast, ptr addrspace(4) %counter_N0.ascast, i64 16, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp.ascast, %"struct.std::complex" zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp1.ascast, %"struct.std::complex" zeroinitializer, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr addrspace(4) %counter_N0.ascast, %"struct.std::complex" zeroinitializer, i32 1, ptr @_ZTSSt7complexIdE.omp.def_constr, ptr @_ZTSSt7complexIdE.omp.destr, ptr @.omp_combiner., ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp.ascast, %"struct.std::complex" zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp1.ascast, %"struct.std::complex" zeroinitializer, i32 1) ]

  store i32 0, ptr addrspace(4) %i0.ascast, align 4, !tbaa !7
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, ptr addrspace(4) %i0.ascast, align 4, !tbaa !7
  %cmp = icmp slt i32 %2, 262144
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  call spir_func void @_ZNSt7complexIdEC2Edd(ptr addrspace(4) dereferenceable_or_null(16) %ref.tmp1.ascast, double 1.000000e+00, double 0.000000e+00) #8
  call spir_func void @_ZStplIdESt7complexIT_ERKS2_S4_(ptr addrspace(4) sret(%"struct.std::complex") align 8 %ref.tmp.ascast, ptr addrspace(4) align 8 dereferenceable(16) %counter_N0.ascast, ptr addrspace(4) align 8 dereferenceable(16) %ref.tmp1.ascast) #8
  %3 = bitcast ptr addrspace(4) %counter_N0.ascast to ptr addrspace(4)
  %4 = bitcast ptr addrspace(4) %ref.tmp.ascast to ptr addrspace(4)
  call void @llvm.memcpy.p4i8.p4i8.i64(ptr addrspace(4) align 8 %3, ptr addrspace(4) align 8 %4, i64 16, i1 false), !tbaa.struct !11
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %5 = load i32, ptr addrspace(4) %i0.ascast, align 4, !tbaa !7
  %inc = add nsw i32 %5, 1
  store i32 %inc, ptr addrspace(4) %i0.ascast, align 4, !tbaa !7
  br label %for.cond, !llvm.loop !13

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: convergent nounwind
define linkonce_odr hidden spir_func void @_ZNSt7complexIdEC2Edd(ptr addrspace(4) dereferenceable_or_null(16) %this, double %__r, double %__i) unnamed_addr #1 comdat align 2 {
entry:
  %_M_value = getelementptr inbounds %"struct.std::complex", ptr addrspace(4) %this, i32 0, i32 0, !intel-tbaa !15
  %_M_value.realp = getelementptr inbounds { double, double }, ptr addrspace(4) %_M_value, i32 0, i32 0, !intel-tbaa !18
  store double %__r, ptr addrspace(4) %_M_value.realp, align 8, !tbaa !18
  %_M_value.imagp = getelementptr inbounds { double, double }, ptr addrspace(4) %_M_value, i32 0, i32 1, !intel-tbaa !19
  store double %__i, ptr addrspace(4) %_M_value.imagp, align 8, !tbaa !19
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: alwaysinline convergent nounwind
define internal void @.omp_combiner.(ptr addrspace(4) noalias %0, ptr addrspace(4) noalias %1) #3 {
entry:
  %call = call spir_func align 8 dereferenceable(16) ptr addrspace(4) @_ZNSt7complexIdEpLIdEERS0_RKS_IT_E(ptr addrspace(4) dereferenceable_or_null(16) %0, ptr addrspace(4) align 8 dereferenceable(16) %1) #7
  ret void
}

; Function Attrs: convergent nounwind mustprogress
define linkonce_odr hidden spir_func align 8 dereferenceable(16) ptr addrspace(4) @_ZNSt7complexIdEpLIdEERS0_RKS_IT_E(ptr addrspace(4) dereferenceable_or_null(16) %this, ptr addrspace(4) align 8 dereferenceable(16) %__z) #4 comdat align 2 {
entry:
  %call = call fast spir_func double @_ZNKSt7complexIdE4realB5cxx11Ev(ptr addrspace(4) dereferenceable_or_null(16) %__z) #7
  %_M_value = getelementptr inbounds %"struct.std::complex", ptr addrspace(4) %this, i32 0, i32 0, !intel-tbaa !15
  %_M_value.realp = getelementptr inbounds { double, double }, ptr addrspace(4) %_M_value, i32 0, i32 0
  %0 = load double, ptr addrspace(4) %_M_value.realp, align 8, !tbaa !20
  %add = fadd fast double %0, %call
  store double %add, ptr addrspace(4) %_M_value.realp, align 8, !tbaa !20
  %call2 = call fast spir_func double @_ZNKSt7complexIdE4imagB5cxx11Ev(ptr addrspace(4) dereferenceable_or_null(16) %__z) #7
  %_M_value3.imagp = getelementptr inbounds { double, double }, ptr addrspace(4) %_M_value, i32 0, i32 1
  %1 = load double, ptr addrspace(4) %_M_value3.imagp, align 8, !tbaa !20
  %add4 = fadd fast double %1, %call2
  store double %add4, ptr addrspace(4) %_M_value3.imagp, align 8, !tbaa !20
  ret ptr addrspace(4) %this
}

; Function Attrs: convergent nounwind
define internal spir_func ptr addrspace(4) @_ZTSSt7complexIdE.omp.def_constr(ptr addrspace(4) %0) #1 {
entry:
  call spir_func void @_ZNSt7complexIdEC2Edd(ptr addrspace(4) dereferenceable_or_null(16) %0, double 0.000000e+00, double 0.000000e+00) #7
  ret ptr addrspace(4) %0
}

; Function Attrs: convergent nounwind
define internal spir_func void @_ZTSSt7complexIdE.omp.destr(ptr addrspace(4) %0) #1 {
entry:
  ret void
}

; Function Attrs: convergent inlinehint nounwind mustprogress
define linkonce_odr hidden spir_func void @_ZStplIdESt7complexIT_ERKS2_S4_(ptr addrspace(4) noalias sret(%"struct.std::complex") align 8 %agg.result, ptr addrspace(4) align 8 dereferenceable(16) %__x, ptr addrspace(4) align 8 dereferenceable(16) %__y) #5 comdat {
entry:
  %0 = bitcast ptr addrspace(4) %agg.result to ptr addrspace(4)
  %1 = bitcast ptr addrspace(4) %__x to ptr addrspace(4)
  call void @llvm.memcpy.p4i8.p4i8.i64(ptr addrspace(4) align 8 %0, ptr addrspace(4) align 8 %1, i64 16, i1 false), !tbaa.struct !11
  %call = call spir_func align 8 dereferenceable(16) ptr addrspace(4) @_ZNSt7complexIdEpLIdEERS0_RKS_IT_E(ptr addrspace(4) dereferenceable_or_null(16) %agg.result, ptr addrspace(4) align 8 dereferenceable(16) %__y) #7
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p4i8.p4i8.i64(ptr addrspace(4) noalias nocapture writeonly, ptr addrspace(4) noalias nocapture readonly, i64, i1 immarg) #6

; Function Attrs: convergent nounwind mustprogress
define linkonce_odr hidden spir_func double @_ZNKSt7complexIdE4realB5cxx11Ev(ptr addrspace(4) dereferenceable_or_null(16) %this) #4 comdat align 2 {
entry:
  %_M_value = getelementptr inbounds %"struct.std::complex", ptr addrspace(4) %this, i32 0, i32 0, !intel-tbaa !15
  %_M_value.realp = getelementptr inbounds { double, double }, ptr addrspace(4) %_M_value, i32 0, i32 0
  %0 = load double, ptr addrspace(4) %_M_value.realp, align 8, !tbaa !20
  ret double %0
}

; Function Attrs: convergent nounwind mustprogress
define linkonce_odr hidden spir_func double @_ZNKSt7complexIdE4imagB5cxx11Ev(ptr addrspace(4) dereferenceable_or_null(16) %this) #4 comdat align 2 {
entry:
  %_M_value = getelementptr inbounds %"struct.std::complex", ptr addrspace(4) %this, i32 0, i32 0, !intel-tbaa !15
  %_M_value.imagp = getelementptr inbounds { double, double }, ptr addrspace(4) %_M_value, i32 0, i32 1
  %0 = load double, ptr addrspace(4) %_M_value.imagp, align 8, !tbaa !20
  ret double %0
}

attributes #0 = { convergent noinline nounwind mustprogress "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { convergent nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }
attributes #3 = { alwaysinline convergent nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #4 = { convergent nounwind mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #5 = { convergent inlinehint nounwind mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #6 = { argmemonly nofree nosync nounwind willreturn }
attributes #7 = { convergent }
attributes #8 = { convergent nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!6}

!0 = !{i32 0, i32 2050, i32 56990741, !"_Z38test_target_teams__distribute_par_simdv", i32 12, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{}
!5 = !{!"cl_doubles"}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C++ TBAA"}
!11 = !{i64 0, i64 8, !12, i64 8, i64 8, !12}
!12 = !{!"double", !9, i64 0}
!13 = distinct !{!13, !14}
!14 = !{!"llvm.loop.mustprogress"}
!15 = !{!16, !17, i64 0}
!16 = !{!"struct@_ZTSSt7complexIdE", !17, i64 0}
!17 = !{!"_Complex@_ZTSd", !12, i64 0, !12, i64 8}
!18 = !{!17, !12, i64 0}
!19 = !{!17, !12, i64 8}
!20 = !{!12, !12, i64 0}

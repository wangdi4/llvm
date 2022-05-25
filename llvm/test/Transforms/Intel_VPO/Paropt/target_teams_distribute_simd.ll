; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


; This test is to check the ref.tmp.ascast and ref.tmp1.ascast in private
; clauses with distribute + simd construct are privatized to a global variable
; with local address space.
;
; #include <iostream>
; #include <cstdlib>
; #include <cmath>
; #include <complex>
; using std::complex;
; bool almost_equal(complex<double> x, complex<double> gold, float tol) {
;   return std::abs(gold) * (1-tol) <= std::abs(x) && std::abs(x) <= std::abs(gold) * (1 + tol);
; }
; #pragma omp declare reduction(+: complex<double>: omp_out += omp_in)
; void test_target_teams__distribute_simd() {
;   const int N0 { 262144 };
;   const complex<double> expected_value { N0 };
;   complex<double> counter_N0{};
; #pragma omp target teams map(tofrom: counter_N0) reduction(+: counter_N0)
; #pragma omp distribute simd reduction(+: counter_N0)
;   for (int i0 = 0 ; i0 < N0 ; i0++ )
;   {
;     counter_N0 = counter_N0 + complex<double> {  1. };
;   }
;   if (!almost_equal(counter_N0, expected_value, 0.1)) {
;     std::cerr << "Expected: " << expected_value << " Got: " << counter_N0 << std::endl;
;     std::exit(112);
;   }
; }
; int main()
; {
;   test_target_teams__distribute_simd();
; }

; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: br i1 %is.master.thread, label %[[IF_MASTER_1:[^ ,]+]]
; CHECK: [[IF_MASTER_1]]:
; CHECK:  %{{.*}} = call spir_func %"struct.std::complex" addrspace(4)* @_ZTSSt7complexIdE.omp.def_constr(%"struct.std::complex" addrspace(4)* addrspacecast (%"struct.std::complex" addrspace(3)* @counter_N0.ascast.red.__local to %"struct.std::complex" addrspace(4)*))
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)

; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: br i1 %is.master.thread, label %[[IF_MASTER_2:[^ ,]+]]
; CHECK: [[IF_MASTER_2]]:
; CHECK: store i32 0, i32 addrspace(3)* @.omp.lb.ascast.priv.__local{{.*}}
; CHECK: store i32 262143, i32 addrspace(3)* @.omp.ub.ascast.priv.__local{{.*}}
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)

; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: br i1 %is.master.thread, label %[[IF_MASTER_3:[^ ,]+]]
; CHECK: [[IF_MASTER_3]]:

; CHECK: store i32 %{{.*}}, i32 addrspace(3)* @i0.ascast.priv.__local
; CHECK: call spir_func void @_ZNSt7complexIdEC2Edd(%"struct.std::complex" addrspace(4)* {{.*}}, double 1.000000e+00, double 0.000000e+00)
; CHECK: call spir_func void @_ZStplIdESt7complexIT_ERKS2_S4_(%"struct.std::complex" addrspace(4)* sret(%"struct.std::complex") align 8 {{.*}}, %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16) addrspacecast (%"struct.std::complex" addrspace(3)* @counter_N0.ascast.red.__local to %"struct.std::complex" addrspace(4)*), %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16) {{.*}})
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: %[[COUNTER:[^,]+]] = bitcast %"struct.std::complex" addrspace(3)* @counter_N0.ascast.red.__local to i8 addrspace(3)*
; CHECK: %[[REF:[^,]+]] = bitcast %"struct.std::complex" addrspace(3)* @ref.tmp.ascast.priv.__local to i8 addrspace(3)*
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: br i1 %is.master.thread, label %[[IF_MASTER_4:[^ ,]+]]
; CHECK: [[IF_MASTER_4]]:
; CHECK: call void @llvm.memcpy.p3i8.p3i8.i64(i8 addrspace(3)* align 8 %[[COUNTER]], i8 addrspace(3)* align 8 %[[REF]], i64 16, i1 false)
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)


; ModuleID = 'target_teams__distribute_simd.cpp'
source_filename = "target_teams__distribute_simd.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream" addrspace(4)*, i8, i8, %"class.std::basic_streambuf" addrspace(4)*, %"class.std::ctype" addrspace(4)*, %"class.std::num_put" addrspace(4)*, %"class.std::num_get" addrspace(4)* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct.std::ios_base::_Callback_list" addrspace(4)*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words" addrspace(4)*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list" addrspace(4)*, void (i32, %"class.std::ios_base" addrspace(4)*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8 addrspace(4)*, i64 }
%"class.std::locale" = type { %"class.std::locale::_Impl" addrspace(4)* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet" addrspace(4)* addrspace(4)*, i64, %"class.std::locale::facet" addrspace(4)* addrspace(4)*, i8 addrspace(4)* addrspace(4)* }
%"class.std::locale::facet" = type <{ i32 (...)**, i32, [4 x i8] }>
%"class.std::basic_streambuf" = type { i32 (...)**, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], %struct.__locale_struct addrspace(4)*, i8, [7 x i8], i32 addrspace(4)*, i32 addrspace(4)*, i16 addrspace(4)*, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ i32 (...)**, i32 }>
%struct.__locale_struct = type { [13 x %struct.__locale_data addrspace(4)*], i16 addrspace(4)*, i32 addrspace(4)*, i32 addrspace(4)*, [13 x i8 addrspace(4)*] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::num_get" = type { %"class.std::locale::facet.base", [4 x i8] }
%"struct.std::complex" = type { { double, double } }

$_ZNSt7complexIdEC2Edd = comdat any

$_ZNSt7complexIdEpLIdEERS0_RKS_IT_E = comdat any

$_ZStplIdESt7complexIT_ERKS2_S4_ = comdat any

$_ZNKSt7complexIdE5__repEv = comdat any

@_ZSt4cerr = external addrspace(1) global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr addrspace(1) constant [11 x i8] c"Expected: \00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [7 x i8] c" Got: \00", align 1

; Function Attrs: noinline nounwind optnone
define hidden spir_func void @_Z34test_target_teams__distribute_simdv() #0 {
entry:
  %N0 = alloca i32, align 4
  %N0.ascast = addrspacecast i32* %N0 to i32 addrspace(4)*
  %expected_value = alloca %"struct.std::complex", align 8
  %expected_value.ascast = addrspacecast %"struct.std::complex"* %expected_value to %"struct.std::complex" addrspace(4)*
  %counter_N0 = alloca %"struct.std::complex", align 8
  %counter_N0.ascast = addrspacecast %"struct.std::complex"* %counter_N0 to %"struct.std::complex" addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %i0 = alloca i32, align 4
  %i0.ascast = addrspacecast i32* %i0 to i32 addrspace(4)*
  %ref.tmp = alloca %"struct.std::complex", align 8
  %ref.tmp.ascast = addrspacecast %"struct.std::complex"* %ref.tmp to %"struct.std::complex" addrspace(4)*
  %ref.tmp1 = alloca %"struct.std::complex", align 8
  %ref.tmp1.ascast = addrspacecast %"struct.std::complex"* %ref.tmp1 to %"struct.std::complex" addrspace(4)*
  %agg.tmp = alloca %"struct.std::complex", align 8
  %agg.tmp.ascast = addrspacecast %"struct.std::complex"* %agg.tmp to %"struct.std::complex" addrspace(4)*
  %agg.tmp3 = alloca %"struct.std::complex", align 8
  %agg.tmp3.ascast = addrspacecast %"struct.std::complex"* %agg.tmp3 to %"struct.std::complex" addrspace(4)*
  store i32 262144, i32 addrspace(4)* %N0.ascast, align 4
  call spir_func void @_ZNSt7complexIdEC2Edd(%"struct.std::complex" addrspace(4)* %expected_value.ascast, double 2.621440e+05, double 0.000000e+00)
  call spir_func void @_ZNSt7complexIdEC2Edd(%"struct.std::complex" addrspace(4)* %counter_N0.ascast, double 0.000000e+00, double 0.000000e+00)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(%"struct.std::complex" addrspace(4)* %counter_N0.ascast, %"struct.std::complex" addrspace(4)* %counter_N0.ascast, i64 16, i64 35), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i0.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp1.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.UDR"(%"struct.std::complex" addrspace(4)* %counter_N0.ascast, %"struct.std::complex" addrspace(4)* (%"struct.std::complex" addrspace(4)*)* @_ZTSSt7complexIdE.omp.def_constr, void (%"struct.std::complex" addrspace(4)*)* @_ZTSSt7complexIdE.omp.destr, void (%"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)*)* @.omp_combiner., i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i0.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp1.ascast) ]
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 262143, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i0.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp1.ascast) ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.UDR"(%"struct.std::complex" addrspace(4)* %counter_N0.ascast, %"struct.std::complex" addrspace(4)* (%"struct.std::complex" addrspace(4)*)* @_ZTSSt7complexIdE.omp.def_constr, void (%"struct.std::complex" addrspace(4)*)* @_ZTSSt7complexIdE.omp.destr, void (%"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)*)* @.omp_combiner., i8* null), "QUAL.OMP.LINEAR:IV"(i32 addrspace(4)* %i0.ascast, i32 1), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp1.ascast) ]
  %4 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %4, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %6 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i0.ascast, align 4
  call spir_func void @_ZNSt7complexIdEC2Edd(%"struct.std::complex" addrspace(4)* %ref.tmp1.ascast, double 1.000000e+00, double 0.000000e+00) #2
  call spir_func void @_ZStplIdESt7complexIT_ERKS2_S4_(%"struct.std::complex" addrspace(4)* sret(%"struct.std::complex") align 8 %ref.tmp.ascast, %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16) %counter_N0.ascast, %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16) %ref.tmp1.ascast) #2
  %8 = bitcast %"struct.std::complex" addrspace(4)* %counter_N0.ascast to i8 addrspace(4)*
  %9 = bitcast %"struct.std::complex" addrspace(4)* %ref.tmp.ascast to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* align 8 %8, i8 addrspace(4)* align 8 %9, i64 16, i1 false)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add2 = add nsw i32 %10, 1
  store i32 %add2, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %11 = bitcast %"struct.std::complex" addrspace(4)* %agg.tmp.ascast to i8 addrspace(4)*
  %12 = bitcast %"struct.std::complex" addrspace(4)* %counter_N0.ascast to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* align 8 %11, i8 addrspace(4)* align 8 %12, i64 16, i1 false)
  %13 = bitcast %"struct.std::complex" addrspace(4)* %agg.tmp3.ascast to i8 addrspace(4)*
  %14 = bitcast %"struct.std::complex" addrspace(4)* %expected_value.ascast to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* align 8 %13, i8 addrspace(4)* align 8 %14, i64 16, i1 false)
  %agg.tmp.ascast.ascast = addrspacecast %"struct.std::complex" addrspace(4)* %agg.tmp.ascast to %"struct.std::complex"*
  %agg.tmp3.ascast.ascast = addrspacecast %"struct.std::complex" addrspace(4)* %agg.tmp3.ascast to %"struct.std::complex"*
  %call = call spir_func zeroext i1 @_Z12almost_equalSt7complexIdES0_f(%"struct.std::complex"* byval(%"struct.std::complex") align 8 %agg.tmp.ascast.ascast, %"struct.std::complex"* byval(%"struct.std::complex") align 8 %agg.tmp3.ascast.ascast, float 0x3FB99999A0000000)
  br i1 %call, label %if.end, label %if.then

if.then:                                          ; preds = %omp.loop.exit
  %call4 = call spir_func align 8 dereferenceable(8) %"class.std::basic_ostream" addrspace(4)* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream" addrspace(4)* align 8 dereferenceable(8) addrspacecast (%"class.std::basic_ostream" addrspace(1)* @_ZSt4cerr to %"class.std::basic_ostream" addrspace(4)*), i8 addrspace(4)* getelementptr inbounds ([11 x i8], [11 x i8] addrspace(4)* addrspacecast ([11 x i8] addrspace(1)* @.str to [11 x i8] addrspace(4)*), i64 0, i64 0))
  %call5 = call spir_func align 8 dereferenceable(8) %"class.std::basic_ostream" addrspace(4)* @_ZStlsIdcSt11char_traitsIcEERSt13basic_ostreamIT0_T1_ES6_RKSt7complexIT_E(%"class.std::basic_ostream" addrspace(4)* align 8 dereferenceable(8) %call4, %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16) %expected_value.ascast)
  %call6 = call spir_func align 8 dereferenceable(8) %"class.std::basic_ostream" addrspace(4)* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream" addrspace(4)* align 8 dereferenceable(8) %call5, i8 addrspace(4)* getelementptr inbounds ([7 x i8], [7 x i8] addrspace(4)* addrspacecast ([7 x i8] addrspace(1)* @.str.1 to [7 x i8] addrspace(4)*), i64 0, i64 0))
  %call7 = call spir_func align 8 dereferenceable(8) %"class.std::basic_ostream" addrspace(4)* @_ZStlsIdcSt11char_traitsIcEERSt13basic_ostreamIT0_T1_ES6_RKSt7complexIT_E(%"class.std::basic_ostream" addrspace(4)* align 8 dereferenceable(8) %call6, %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16) %counter_N0.ascast)
  %call8 = call spir_func align 8 dereferenceable(8) %"class.std::basic_ostream" addrspace(4)* @_ZNSolsEPFRSoS_E(%"class.std::basic_ostream" addrspace(4)* %call7, %"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*)* @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_)
  call spir_func void @exit(i32 112) #7
  unreachable

if.end:                                           ; preds = %omp.loop.exit
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr hidden spir_func void @_ZNSt7complexIdEC2Edd(%"struct.std::complex" addrspace(4)* %this, double %__r, double %__i) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %this.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %this.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %__r.addr = alloca double, align 8
  %__r.addr.ascast = addrspacecast double* %__r.addr to double addrspace(4)*
  %__i.addr = alloca double, align 8
  %__i.addr.ascast = addrspacecast double* %__i.addr to double addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %this, %"struct.std::complex" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  store double %__r, double addrspace(4)* %__r.addr.ascast, align 8
  store double %__i, double addrspace(4)* %__i.addr.ascast, align 8
  %this1 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  %_M_value = getelementptr inbounds %"struct.std::complex", %"struct.std::complex" addrspace(4)* %this1, i32 0, i32 0
  %0 = load double, double addrspace(4)* %__r.addr.ascast, align 8
  %1 = load double, double addrspace(4)* %__i.addr.ascast, align 8
  %_M_value.realp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 0
  %_M_value.imagp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 1
  store double %0, double addrspace(4)* %_M_value.realp, align 8
  store double %1, double addrspace(4)* %_M_value.imagp, align 8
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline nounwind
define internal void @.omp_combiner.(%"struct.std::complex" addrspace(4)* noalias %0, %"struct.std::complex" addrspace(4)* noalias %1) #3 {
entry:
  %.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %.addr1 = alloca %"struct.std::complex" addrspace(4)*, align 8
  %.addr1.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %.addr1 to %"struct.std::complex" addrspace(4)* addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %0, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  store %"struct.std::complex" addrspace(4)* %1, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr1.ascast, align 8
  %2 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr1.ascast, align 8
  %3 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  %call = call spir_func align 8 dereferenceable(16) %"struct.std::complex" addrspace(4)* @_ZNSt7complexIdEpLIdEERS0_RKS_IT_E(%"struct.std::complex" addrspace(4)* %3, %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16) %2)
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr hidden spir_func align 8 dereferenceable(16) %"struct.std::complex" addrspace(4)* @_ZNSt7complexIdEpLIdEERS0_RKS_IT_E(%"struct.std::complex" addrspace(4)* %this, %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16) %__z) #1 comdat align 2 {
entry:
  %retval = alloca %"struct.std::complex" addrspace(4)*, align 8
  %retval.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %retval to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %this.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %this.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %this.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %__z.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %__z.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %__z.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %tmp = alloca { double, double }, align 8
  %tmp.ascast = addrspacecast { double, double }* %tmp to { double, double } addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %this, %"struct.std::complex" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  store %"struct.std::complex" addrspace(4)* %__z, %"struct.std::complex" addrspace(4)* addrspace(4)* %__z.addr.ascast, align 8
  %this1 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  %0 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %__z.addr.ascast, align 8
  call spir_func void @_ZNKSt7complexIdE5__repEv({ double, double } addrspace(4)* sret({ double, double }) align 8 %tmp.ascast, %"struct.std::complex" addrspace(4)* %0)
  %tmp.ascast.realp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %tmp.ascast, i32 0, i32 0
  %tmp.ascast.real = load double, double addrspace(4)* %tmp.ascast.realp, align 8
  %tmp.ascast.imagp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %tmp.ascast, i32 0, i32 1
  %tmp.ascast.imag = load double, double addrspace(4)* %tmp.ascast.imagp, align 8
  %_M_value = getelementptr inbounds %"struct.std::complex", %"struct.std::complex" addrspace(4)* %this1, i32 0, i32 0
  %_M_value.realp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 0
  %_M_value.real = load double, double addrspace(4)* %_M_value.realp, align 8
  %_M_value.imagp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 1
  %_M_value.imag = load double, double addrspace(4)* %_M_value.imagp, align 8
  %add.r = fadd double %_M_value.real, %tmp.ascast.real
  %add.i = fadd double %_M_value.imag, %tmp.ascast.imag
  %_M_value.realp2 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 0
  %_M_value.imagp3 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 1
  store double %add.r, double addrspace(4)* %_M_value.realp2, align 8
  store double %add.i, double addrspace(4)* %_M_value.imagp3, align 8
  ret %"struct.std::complex" addrspace(4)* %this1
}

; Function Attrs: noinline nounwind
define internal spir_func %"struct.std::complex" addrspace(4)* @_ZTSSt7complexIdE.omp.def_constr(%"struct.std::complex" addrspace(4)* %0) #3 {
entry:
  %retval = alloca %"struct.std::complex" addrspace(4)*, align 8
  %retval.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %retval to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %0, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  %1 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  call spir_func void @_ZNSt7complexIdEC2Edd(%"struct.std::complex" addrspace(4)* %1, double 0.000000e+00, double 0.000000e+00)
  ret %"struct.std::complex" addrspace(4)* %1
}

; Function Attrs: noinline nounwind
define internal spir_func void @_ZTSSt7complexIdE.omp.destr(%"struct.std::complex" addrspace(4)* %0) #3 {
entry:
  %.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %0, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone
define linkonce_odr hidden spir_func void @_ZStplIdESt7complexIT_ERKS2_S4_(%"struct.std::complex" addrspace(4)* noalias sret(%"struct.std::complex") align 8 %agg.result, %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16) %__x, %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16) %__y) #1 comdat {
entry:
  %__x.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %__x.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %__x.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %__y.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %__y.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %__y.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %__x, %"struct.std::complex" addrspace(4)* addrspace(4)* %__x.addr.ascast, align 8
  store %"struct.std::complex" addrspace(4)* %__y, %"struct.std::complex" addrspace(4)* addrspace(4)* %__y.addr.ascast, align 8
  %0 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %__x.addr.ascast, align 8
  %1 = bitcast %"struct.std::complex" addrspace(4)* %agg.result to i8 addrspace(4)*
  %2 = bitcast %"struct.std::complex" addrspace(4)* %0 to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* align 8 %1, i8 addrspace(4)* align 8 %2, i64 16, i1 false)
  %3 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %__y.addr.ascast, align 8
  %call = call spir_func align 8 dereferenceable(16) %"struct.std::complex" addrspace(4)* @_ZNSt7complexIdEpLIdEERS0_RKS_IT_E(%"struct.std::complex" addrspace(4)* %agg.result, %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16) %3)
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* noalias nocapture writeonly, i8 addrspace(4)* noalias nocapture readonly, i64, i1 immarg) #4

declare spir_func zeroext i1 @_Z12almost_equalSt7complexIdES0_f(%"struct.std::complex"* byval(%"struct.std::complex") align 8, %"struct.std::complex"* byval(%"struct.std::complex") align 8, float) #5

declare spir_func align 8 dereferenceable(8) %"class.std::basic_ostream" addrspace(4)* @_ZStlsIdcSt11char_traitsIcEERSt13basic_ostreamIT0_T1_ES6_RKSt7complexIT_E(%"class.std::basic_ostream" addrspace(4)* align 8 dereferenceable(8), %"struct.std::complex" addrspace(4)* align 8 dereferenceable(16)) #5

declare spir_func align 8 dereferenceable(8) %"class.std::basic_ostream" addrspace(4)* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream" addrspace(4)* align 8 dereferenceable(8), i8 addrspace(4)*) #5

declare spir_func align 8 dereferenceable(8) %"class.std::basic_ostream" addrspace(4)* @_ZNSolsEPFRSoS_E(%"class.std::basic_ostream" addrspace(4)*, %"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*)*) #5

declare spir_func align 8 dereferenceable(8) %"class.std::basic_ostream" addrspace(4)* @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(%"class.std::basic_ostream" addrspace(4)* align 8 dereferenceable(8)) #5

; Function Attrs: noreturn nounwind
declare spir_func void @exit(i32) #6

; Function Attrs: noinline nounwind optnone
define linkonce_odr hidden spir_func void @_ZNKSt7complexIdE5__repEv({ double, double } addrspace(4)* noalias sret({ double, double }) align 8 %agg.result, %"struct.std::complex" addrspace(4)* %this) #1 comdat align 2 {
entry:
  %this.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %this.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %this.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %this, %"struct.std::complex" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  %this1 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  %_M_value = getelementptr inbounds %"struct.std::complex", %"struct.std::complex" addrspace(4)* %this1, i32 0, i32 0
  %_M_value.realp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 0
  %_M_value.real = load double, double addrspace(4)* %_M_value.realp, align 8
  %_M_value.imagp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 1
  %_M_value.imag = load double, double addrspace(4)* %_M_value.imagp, align 8
  %agg.result.realp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %agg.result, i32 0, i32 0
  %agg.result.imagp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %agg.result, i32 0, i32 1
  store double %_M_value.real, double addrspace(4)* %agg.result.realp, align 8
  store double %_M_value.imag, double addrspace(4)* %agg.result.imagp, align 8
  %agg.result.realp2 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %agg.result, i32 0, i32 0
  %agg.result.real = load double, double addrspace(4)* %agg.result.realp2, align 8
  %agg.result.imagp3 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %agg.result, i32 0, i32 1
  %agg.result.imag = load double, double addrspace(4)* %agg.result.imagp3, align 8
  %agg.result.realp4 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %agg.result, i32 0, i32 0
  %agg.result.imagp5 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %agg.result, i32 0, i32 1
  store double %agg.result.real, double addrspace(4)* %agg.result.realp4, align 8
  store double %agg.result.imag, double addrspace(4)* %agg.result.imagp5, align 8
  ret void
}

attributes #0 = { noinline nounwind optnone "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { argmemonly nounwind willreturn }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { noreturn nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!5}

!0 = !{i32 0, i32 2065, i32 1585078, !"_Z34test_target_teams__distribute_simdv", i32 14, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"cl_doubles"}
!5 = !{!"clang version 9.0.0"}

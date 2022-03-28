; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction=true -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-atomic-free-reduction=true -S %s | FileCheck %s

; Check that reduction items which are unsupported by atomic-free approach (UDR in this exmaple) do not pass WG and WI limits to libomptarget via kernel_info
; CHECK: @__omp_offloading_805_b43487__Z3foo_l3_kernel_info = weak target_declare addrspace(1) constant %0 { i32 4, i32 3, [3 x %1] [%1 { i32 0, i32 8 }, %1 { i32 0, i32 8 }, %1 { i32 0, i32 8 }], i64 1, i64 0, i64 0 }


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%"struct.std::complex" = type { { double, double } }

; Function Attrs: convergent mustprogress noinline nounwind
define protected spir_func void @foo() #0 {
entry:
  %N0 = alloca i32, align 4
  %expected_value = alloca %"struct.std::complex", align 8
  %counter_N0 = alloca %"struct.std::complex", align 8
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i0 = alloca i32, align 4
  %ref.tmp = alloca %"struct.std::complex", align 8
  %ref.tmp1 = alloca %"struct.std::complex", align 8
  %agg.tmp = alloca %"struct.std::complex", align 8
  %agg.tmp3 = alloca %"struct.std::complex", align 8
  %N0.ascast = addrspacecast i32* %N0 to i32 addrspace(4)*
  %expected_value.ascast = addrspacecast %"struct.std::complex"* %expected_value to %"struct.std::complex" addrspace(4)*
  %counter_N0.ascast = addrspacecast %"struct.std::complex"* %counter_N0 to %"struct.std::complex" addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i0.ascast = addrspacecast i32* %i0 to i32 addrspace(4)*
  %ref.tmp.ascast = addrspacecast %"struct.std::complex"* %ref.tmp to %"struct.std::complex" addrspace(4)*
  %ref.tmp1.ascast = addrspacecast %"struct.std::complex"* %ref.tmp1 to %"struct.std::complex" addrspace(4)*
  %agg.tmp.ascast = addrspacecast %"struct.std::complex"* %agg.tmp to %"struct.std::complex" addrspace(4)*
  %agg.tmp3.ascast = addrspacecast %"struct.std::complex"* %agg.tmp3 to %"struct.std::complex" addrspace(4)*
  store i32 32768, i32 addrspace(4)* %N0.ascast, align 4
  call spir_func void @_ZNSt7complexIdEC2Edd(%"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable_or_null(16) %expected_value.ascast, double noundef 3.276800e+04, double noundef 0.000000e+00) #9
  call spir_func void @_ZNSt7complexIdEC2Edd(%"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable_or_null(16) %counter_N0.ascast, double noundef 0.000000e+00, double noundef 0.000000e+00) #9
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 32767, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(%"struct.std::complex" addrspace(4)* %counter_N0.ascast, %"struct.std::complex" addrspace(4)* %counter_N0.ascast, i64 16, i64 547, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i0.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp1.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.UDR"(%"struct.std::complex" addrspace(4)* %counter_N0.ascast, %"struct.std::complex" addrspace(4)* (%"struct.std::complex" addrspace(4)*)* @_ZTSSt7complexIdE.omp.def_constr, void (%"struct.std::complex" addrspace(4)*)* @_ZTSSt7complexIdE.omp.destr, void (%"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)*)* @.omp_combiner., i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i0.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp1.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.REDUCTION.UDR"(%"struct.std::complex" addrspace(4)* %counter_N0.ascast, %"struct.std::complex" addrspace(4)* (%"struct.std::complex" addrspace(4)*)* @_ZTSSt7complexIdE.omp.def_constr, void (%"struct.std::complex" addrspace(4)*)* @_ZTSSt7complexIdE.omp.destr, void (%"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)*)* @.omp_combiner., i8* null), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i0.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp.ascast), "QUAL.OMP.PRIVATE"(%"struct.std::complex" addrspace(4)* %ref.tmp1.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i0.ascast, align 4
  call spir_func void @_ZNSt7complexIdEC2Edd(%"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable_or_null(16) %ref.tmp1.ascast, double noundef 1.000000e+00, double noundef 0.000000e+00) #10
  call spir_func void @_ZStplIdESt7complexIT_ERKS2_S4_(%"struct.std::complex" addrspace(4)* sret(%"struct.std::complex") align 8 %ref.tmp.ascast, %"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable(16) %counter_N0.ascast, %"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable(16) %ref.tmp1.ascast) #10
  %7 = bitcast %"struct.std::complex" addrspace(4)* %counter_N0.ascast to i8 addrspace(4)*
  %8 = bitcast %"struct.std::complex" addrspace(4)* %ref.tmp.ascast to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* align 8 %7, i8 addrspace(4)* align 8 %8, i64 16, i1 false)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %10 = bitcast %"struct.std::complex" addrspace(4)* %agg.tmp.ascast to i8 addrspace(4)*
  %11 = bitcast %"struct.std::complex" addrspace(4)* %counter_N0.ascast to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* align 8 %10, i8 addrspace(4)* align 8 %11, i64 16, i1 false)
  %12 = bitcast %"struct.std::complex" addrspace(4)* %agg.tmp3.ascast to i8 addrspace(4)*
  %13 = bitcast %"struct.std::complex" addrspace(4)* %expected_value.ascast to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* align 8 %12, i8 addrspace(4)* align 8 %13, i64 16, i1 false)
  %call = call spir_func noundef zeroext i1 @_Z12almost_equalSt7complexIdES0_dd(%"struct.std::complex" addrspace(4)* noundef byval(%"struct.std::complex") align 8 %agg.tmp.ascast, %"struct.std::complex" addrspace(4)* noundef byval(%"struct.std::complex") align 8 %agg.tmp3.ascast, double noundef 1.000000e-02, double noundef 0.000000e+00) #9
  br i1 %call, label %if.end, label %if.then

if.then:                                          ; preds = %omp.loop.exit
  call spir_func void @exit(i32 noundef 112) #11
  unreachable

if.end:                                           ; preds = %omp.loop.exit
  ret void
}

; Function Attrs: convergent nounwind
define linkonce_odr spir_func void @_ZNSt7complexIdEC2Edd(%"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable_or_null(16) %this, double noundef %__r, double noundef %__i) unnamed_addr #1 {
entry:
  %this.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %__r.addr = alloca double, align 8
  %__i.addr = alloca double, align 8
  %this.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %this.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %__r.addr.ascast = addrspacecast double* %__r.addr to double addrspace(4)*
  %__i.addr.ascast = addrspacecast double* %__i.addr to double addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %this, %"struct.std::complex" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  store double %__r, double addrspace(4)* %__r.addr.ascast, align 8
  store double %__i, double addrspace(4)* %__i.addr.ascast, align 8
  %this1 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  %_M_value = getelementptr inbounds %"struct.std::complex", %"struct.std::complex" addrspace(4)* %this1, i32 0, i32 0
  %0 = load double, double addrspace(4)* %__r.addr.ascast, align 8
  %1 = load double, double addrspace(4)* %__i.addr.ascast, align 8
  %_M_value.realp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 0
  store double %0, double addrspace(4)* %_M_value.realp, align 8
  %_M_value.imagp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 1
  store double %1, double addrspace(4)* %_M_value.imagp, align 8
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: alwaysinline convergent nounwind
define internal void @.omp_combiner.(%"struct.std::complex" addrspace(4)* noalias noundef %0, %"struct.std::complex" addrspace(4)* noalias noundef %1) #3 {
entry:
  %.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %.addr1 = alloca %"struct.std::complex" addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %.addr1.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %.addr1 to %"struct.std::complex" addrspace(4)* addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %0, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  store %"struct.std::complex" addrspace(4)* %1, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr1.ascast, align 8
  %2 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr1.ascast, align 8
  %3 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  %call = call spir_func noundef align 8 dereferenceable(16) %"struct.std::complex" addrspace(4)* @_ZNSt7complexIdEpLIdEERS0_RKS_IT_E(%"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable_or_null(16) %3, %"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable(16) %2) #9
  ret void
}

; Function Attrs: convergent mustprogress nounwind
define linkonce_odr spir_func noundef align 8 dereferenceable(16) %"struct.std::complex" addrspace(4)* @_ZNSt7complexIdEpLIdEERS0_RKS_IT_E(%"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable_or_null(16) %this, %"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable(16) %__z) #4 {
entry:
  %retval = alloca %"struct.std::complex" addrspace(4)*, align 8
  %this.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %__z.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %tmp = alloca { double, double }, align 8
  %retval.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %retval to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %this.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %this.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %__z.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %__z.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %tmp.ascast = addrspacecast { double, double }* %tmp to { double, double } addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %this, %"struct.std::complex" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  store %"struct.std::complex" addrspace(4)* %__z, %"struct.std::complex" addrspace(4)* addrspace(4)* %__z.addr.ascast, align 8
  %this1 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  %0 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %__z.addr.ascast, align 8
  call spir_func void @_ZNKSt7complexIdE5__repEv({ double, double } addrspace(4)* sret({ double, double }) align 8 %tmp.ascast, %"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable_or_null(16) %0) #9
  %tmp.ascast.realp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %tmp.ascast, i32 0, i32 0
  %tmp.ascast.real = load double, double addrspace(4)* %tmp.ascast.realp, align 8
  %tmp.ascast.imagp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %tmp.ascast, i32 0, i32 1
  %tmp.ascast.imag = load double, double addrspace(4)* %tmp.ascast.imagp, align 8
  %_M_value = getelementptr inbounds %"struct.std::complex", %"struct.std::complex" addrspace(4)* %this1, i32 0, i32 0
  %_M_value.realp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 0
  %_M_value.real = load double, double addrspace(4)* %_M_value.realp, align 8
  %_M_value.imagp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 1
  %_M_value.imag = load double, double addrspace(4)* %_M_value.imagp, align 8
  %add.r = fadd fast double %_M_value.real, %tmp.ascast.real
  %add.i = fadd fast double %_M_value.imag, %tmp.ascast.imag
  %_M_value.realp2 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 0
  store double %add.r, double addrspace(4)* %_M_value.realp2, align 8
  %_M_value.imagp3 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %_M_value, i32 0, i32 1
  store double %add.i, double addrspace(4)* %_M_value.imagp3, align 8
  ret %"struct.std::complex" addrspace(4)* %this1
}

; Function Attrs: convergent nounwind
define internal spir_func noundef %"struct.std::complex" addrspace(4)* @_ZTSSt7complexIdE.omp.def_constr(%"struct.std::complex" addrspace(4)* noundef %0) #1 section ".text.startup" {
entry:
  %retval = alloca %"struct.std::complex" addrspace(4)*, align 8
  %.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %retval.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %retval to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %0, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  %1 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  call spir_func void @_ZNSt7complexIdEC2Edd(%"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable_or_null(16) %1, double noundef 0.000000e+00, double noundef 0.000000e+00) #9
  ret %"struct.std::complex" addrspace(4)* %1
}

; Function Attrs: convergent nounwind
define internal spir_func void @_ZTSSt7complexIdE.omp.destr(%"struct.std::complex" addrspace(4)* noundef %0) #1 section ".text.startup" {
entry:
  %.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %0, %"struct.std::complex" addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  ret void
}

; Function Attrs: convergent inlinehint mustprogress nounwind
define linkonce_odr spir_func void @_ZStplIdESt7complexIT_ERKS2_S4_(%"struct.std::complex" addrspace(4)* noalias sret(%"struct.std::complex") align 8 %agg.result, %"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable(16) %__x, %"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable(16) %__y) #5 {
entry:
  %__x.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %__y.addr = alloca %"struct.std::complex" addrspace(4)*, align 8
  %__x.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %__x.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  %__y.addr.ascast = addrspacecast %"struct.std::complex" addrspace(4)** %__y.addr to %"struct.std::complex" addrspace(4)* addrspace(4)*
  store %"struct.std::complex" addrspace(4)* %__x, %"struct.std::complex" addrspace(4)* addrspace(4)* %__x.addr.ascast, align 8
  store %"struct.std::complex" addrspace(4)* %__y, %"struct.std::complex" addrspace(4)* addrspace(4)* %__y.addr.ascast, align 8
  %0 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %__x.addr.ascast, align 8
  %1 = bitcast %"struct.std::complex" addrspace(4)* %agg.result to i8 addrspace(4)*
  %2 = bitcast %"struct.std::complex" addrspace(4)* %0 to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* align 8 %1, i8 addrspace(4)* align 8 %2, i64 16, i1 false)
  %3 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(4)* %__y.addr.ascast, align 8
  %call = call spir_func noundef align 8 dereferenceable(16) %"struct.std::complex" addrspace(4)* @_ZNSt7complexIdEpLIdEERS0_RKS_IT_E(%"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable_or_null(16) %agg.result, %"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable(16) %3) #9
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* noalias nocapture writeonly, i8 addrspace(4)* noalias nocapture readonly, i64, i1 immarg) #6

; Function Attrs: convergent
declare spir_func noundef zeroext i1 @_Z12almost_equalSt7complexIdES0_dd(%"struct.std::complex" addrspace(4)* noundef byval(%"struct.std::complex") align 8, %"struct.std::complex" addrspace(4)* noundef byval(%"struct.std::complex") align 8, double noundef, double noundef) #7

; Function Attrs: convergent noreturn nounwind
declare spir_func void @exit(i32 noundef) #8

; Function Attrs: convergent mustprogress nounwind
define linkonce_odr spir_func void @_ZNKSt7complexIdE5__repEv({ double, double } addrspace(4)* noalias sret({ double, double }) align 8 %agg.result, %"struct.std::complex" addrspace(4)* noundef align 8 dereferenceable_or_null(16) %this) #4 {
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
  store double %_M_value.real, double addrspace(4)* %agg.result.realp, align 8
  %agg.result.imagp = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %agg.result, i32 0, i32 1
  store double %_M_value.imag, double addrspace(4)* %agg.result.imagp, align 8
  %agg.result.realp2 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %agg.result, i32 0, i32 0
  %agg.result.real = load double, double addrspace(4)* %agg.result.realp2, align 8
  %agg.result.imagp3 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %agg.result, i32 0, i32 1
  %agg.result.imag = load double, double addrspace(4)* %agg.result.imagp3, align 8
  %agg.result.realp4 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %agg.result, i32 0, i32 0
  store double %agg.result.real, double addrspace(4)* %agg.result.realp4, align 8
  %agg.result.imagp5 = getelementptr inbounds { double, double }, { double, double } addrspace(4)* %agg.result, i32 0, i32 1
  store double %agg.result.imag, double addrspace(4)* %agg.result.imagp5, align 8
  ret void
}


attributes #0 = { convergent noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 2053, i32 11809927, !"_Z3foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"clang version 13.0.0"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}

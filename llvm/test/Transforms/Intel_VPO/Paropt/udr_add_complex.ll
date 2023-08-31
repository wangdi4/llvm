; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED

; #include <omp.h>
; #include <complex>
;
; using namespace std;
;
; #pragma omp declare reduction(complex_add: complex<float> : omp_out+=omp_in) \
;                         initializer(omp_priv=complex<float>(0.0, 0.0))
;
; int main(void)
; {
;   complex<float> c (0.1, 0.1); // = 0.1 + 0.1i;
;   complex<float> sum (0.123,0.123);// = 0.123 + 0.123i;
;
; #pragma omp parallel for reduction(complex_add:sum)
;   for (int i=0; i<100; i++)
;     sum+= c;
;
;   return 0;
; }


; ModuleID = 'udr_add_complex.cpp'
source_filename = "udr_add_complex.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"struct.std::complex" = type { { float, float } }

$_ZNSt7complexIfEC2Eff = comdat any

$_ZNSt7complexIfEpLIfEERS0_RKS_IT_E = comdat any

$_ZNKSt7complexIfE4realB5cxx11Ev = comdat any

$_ZNKSt7complexIfE4imagB5cxx11Ev = comdat any

; Function Attrs: noinline norecurse optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %c = alloca %"struct.std::complex", align 4
  %sum = alloca %"struct.std::complex", align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @_ZNSt7complexIfEC2Eff(ptr %c, float 0x3FB99999A0000000, float 0x3FB99999A0000000)
  call void @_ZNSt7complexIfEC2Eff(ptr %sum, float 0x3FBF7CEDA0000000, float 0x3FBF7CEDA0000000)
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr %sum, %"struct.std::complex" zeroinitializer, i32 1, ptr null, ptr @_ZTSSt7complexIfE.omp.destr, ptr @.omp_combiner., ptr @.omp_initializer.),
    "QUAL.OMP.SHARED:TYPED"(ptr %c, %"struct.std::complex" zeroinitializer, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]


; CRITICAL-NOT: "QUAL.OMP.REDUCTION.UDR"
; CRITICAL: call void @.omp_initializer.(ptr %sum.red, ptr %sum)
; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: call void @.omp_combiner.(ptr %sum, ptr %sum.red)
; CRITICAL: call void @__kmpc_end_critical({{.*}})
; CRITICAL: call void @_ZTSSt7complexIfE.omp.destr(ptr %sum.red)

; FASTRED-NOT: "QUAL.OMP.REDUCTION.UDR"
; FASTRED-NOT: __kmpc_atomic
; FASTRED: call void @.omp_initializer.(ptr %sum.fast_red, ptr %sum)
; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: call void @.omp_combiner.(ptr %sum, ptr %sum.fast_red)
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})
; FASTRED-DAG: call void @_ZTSSt7complexIfE.omp.destr(ptr %sum.fast_red)

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %call = call dereferenceable(8) ptr @_ZNSt7complexIfEpLIfEERS0_RKS_IT_E(ptr %sum, ptr dereferenceable(8) %c) #2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt7complexIfEC2Eff(ptr %this, float %__r, float %__i) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %__r.addr = alloca float, align 4
  %__i.addr = alloca float, align 4
  store ptr %this, ptr %this.addr, align 8
  store float %__r, ptr %__r.addr, align 4
  store float %__i, ptr %__i.addr, align 4
  %this1 = load ptr, ptr %this.addr, align 8
  %_M_value = getelementptr inbounds %"struct.std::complex", ptr %this1, i32 0, i32 0
  %0 = load float, ptr %__r.addr, align 4
  %1 = load float, ptr %__i.addr, align 4
  %_M_value.realp = getelementptr inbounds { float, float }, { float, float }* %_M_value, i32 0, i32 0
  %_M_value.imagp = getelementptr inbounds { float, float }, { float, float }* %_M_value, i32 0, i32 1
  store float %0, ptr %_M_value.realp, align 4
  store float %1, ptr %_M_value.imagp, align 4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline uwtable
define internal void @.omp_combiner.(ptr noalias %0, ptr noalias %1) #3 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  %call = call dereferenceable(8) ptr @_ZNSt7complexIfEpLIfEERS0_RKS_IT_E(ptr %3, ptr dereferenceable(8) %2)
  ret void
}

; Function Attrs: noinline optnone uwtable
define linkonce_odr dso_local dereferenceable(8) ptr @_ZNSt7complexIfEpLIfEERS0_RKS_IT_E(ptr %this, ptr dereferenceable(8) %__z) #4 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %__z.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  store ptr %__z, ptr %__z.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %__z.addr, align 8
  %call = call float @_ZNKSt7complexIfE4realB5cxx11Ev(ptr %0)
  %_M_value = getelementptr inbounds %"struct.std::complex", ptr %this1, i32 0, i32 0
  %_M_value.realp = getelementptr inbounds { float, float }, { float, float }* %_M_value, i32 0, i32 0
  %1 = load float, ptr %_M_value.realp, align 4
  %add = fadd float %1, %call
  store float %add, ptr %_M_value.realp, align 4
  %2 = load ptr, ptr %__z.addr, align 8
  %call2 = call float @_ZNKSt7complexIfE4imagB5cxx11Ev(ptr %2)
  %_M_value3 = getelementptr inbounds %"struct.std::complex", ptr %this1, i32 0, i32 0
  %_M_value3.imagp = getelementptr inbounds { float, float }, { float, float }* %_M_value3, i32 0, i32 1
  %3 = load float, ptr %_M_value3.imagp, align 4
  %add4 = fadd float %3, %call2
  store float %add4, ptr %_M_value3.imagp, align 4
  ret ptr %this1
}

; Function Attrs: noinline uwtable
define internal void @.omp_initializer.(ptr noalias %0, ptr noalias %1) #3 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  call void @_ZNSt7complexIfEC2Eff(ptr %3, float 0.000000e+00, float 0.000000e+00)
  ret void
}

; Function Attrs: noinline uwtable
define internal void @_ZTSSt7complexIfE.omp.destr(ptr %0) #3 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local float @_ZNKSt7complexIfE4realB5cxx11Ev(ptr %this) #1 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %_M_value = getelementptr inbounds %"struct.std::complex", ptr %this1, i32 0, i32 0
  %_M_value.realp = getelementptr inbounds { float, float }, { float, float }* %_M_value, i32 0, i32 0
  %0 = load float, ptr %_M_value.realp, align 4
  ret float %0
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local float @_ZNKSt7complexIfE4imagB5cxx11Ev(ptr %this) #1 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %_M_value = getelementptr inbounds %"struct.std::complex", ptr %this1, i32 0, i32 0
  %_M_value.imagp = getelementptr inbounds { float, float }, { float, float }* %_M_value, i32 0, i32 1
  %0 = load float, ptr %_M_value.imagp, align 4
  ret float %0
}

attributes #0 = { noinline norecurse optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}

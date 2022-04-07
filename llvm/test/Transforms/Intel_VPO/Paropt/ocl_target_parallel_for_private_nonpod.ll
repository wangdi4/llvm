; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; #include <omp.h>
; #include <stdio.h>
;
; class C {
; public:
;   int x;
;   short y;
;   C() : x(111), y(111){};
;   ~C() = default;
; };
;
; void foo() {
;   C c;
;   c.x = 222;
; #pragma omp target private(c)
;   {
; //  if (c.x != 111) printf("failed check 1. c.x = %d. Expected = %d\n", c.x, 111);
;     c.x = 333;
;
; #pragma omp parallel for private(c)
;     for (int i = 0; i < 10; i++) {
; //    if (omp_get_thread_num() == 0 && c.x != 111) printf("failed check 2. c.x = %d. Expected = %d\n", c.x, 111);
;       c.x = 444;
;     }
; //  if (c.x != 333) printf("failed check 3. c.x = %d. Expected = %d\n", c.x, 333);
;   }
; //  if (c.x != 222) printf("failed check 4. c.x = %d. Expected = %d\n", c.x, 222);
; }
;
;  //int main() { foo(); }

; Check for the private copy of @c for the target construct
; CHECK: [[C_TGT:@c.*__global]] = internal addrspace(1) global %class.C zeroinitializer, align 1
; CHECK-DAG: call spir_func %class.C addrspace(4)* @_ZTS1C.omp.def_constr(%class.C addrspace(4)* addrspacecast (%class.C addrspace(1)* [[C_TGT]] to %class.C addrspace(4)*))
; CHECK-DAG: [[C_TGT_X:%[^ ]+]] = getelementptr inbounds %class.C, %class.C addrspace(1)* [[C_TGT]], i32 0, i32 0
; CHECK-DAG: store i32 333, i32 addrspace(1)* [[C_TGT_X]], align 4
; CHECK-DAG: call spir_func void @_ZTS1C.omp.destr(%class.C addrspace(4)* addrspacecast (%class.C addrspace(1)* [[C_TGT]] to %class.C addrspace(4)*))

; Check for the private copy of @c for the parallel-for construct
; CHECK-DAG: [[C_PAR:%c.*]] = alloca %class.C, align 1
; CHECK-DAG: {{%[^ ]+}} = call spir_func %class.C addrspace(4)* @_ZTS1C.omp.def_constr(%class.C addrspace(4)* [[C_PAR_CAST:%[^, ]+]])
; CHECK-DAG: [[C_PAR_CAST]] = addrspacecast %class.C* [[C_PAR]] to %class.C addrspace(4)*
; CHECK-DAG: [[C_PAR_X:%[^ ]+]] = getelementptr inbounds %class.C, %class.C* [[C_PAR]], i32 0, i32 0
; CHECK-DAG: store i32 444, i32* [[C_PAR_X]], align 4
; CHECK-DAG: call spir_func void @_ZTS1C.omp.destr(%class.C addrspace(4)* [[C_PAR_CAST]])


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.C = type <{ i32, i16, [2 x i8] }>

$_ZN1CC2Ev = comdat any

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @_Z3foov() #0 {
entry:
  %c = alloca %class.C, align 4
  %c.ascast = addrspacecast %class.C* %c to %class.C addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  call spir_func void @_ZN1CC2Ev(%class.C addrspace(4)* %c.ascast)
  %x = getelementptr inbounds %class.C, %class.C addrspace(4)* %c.ascast, i32 0, i32 0
  store i32 222, i32 addrspace(4)* %x, align 4

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE:NONPOD"(%class.C addrspace(4)* %c.ascast, %class.C addrspace(4)* (%class.C addrspace(4)*)* @_ZTS1C.omp.def_constr, void (%class.C addrspace(4)*)* @_ZTS1C.omp.destr), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]

  %x1 = getelementptr inbounds %class.C, %class.C addrspace(4)* %c.ascast, i32 0, i32 0
  store i32 333, i32 addrspace(4)* %x1, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.PRIVATE:NONPOD"(%class.C addrspace(4)* %c.ascast, %class.C addrspace(4)* (%class.C addrspace(4)*)* @_ZTS1C.omp.def_constr, void (%class.C addrspace(4)*)* @_ZTS1C.omp.destr), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]

  %2 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %2, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %4 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %x2 = getelementptr inbounds %class.C, %class.C addrspace(4)* %c.ascast, i32 0, i32 0
  store i32 444, i32 addrspace(4)* %x2, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %6, 1
  store i32 %add3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
declare hidden spir_func void @_ZN1CC2Ev(%class.C addrspace(4)* %this) unnamed_addr #1 align 2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline nounwind uwtable
declare %class.C addrspace(4)* @_ZTS1C.omp.def_constr(%class.C addrspace(4)* %0) #3

; Function Attrs: noinline nounwind uwtable
declare void @_ZTS1C.omp.destr(%class.C addrspace(4)* %0) #3

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 0, i32 2055, i32 150400254, !"_Z3foov", i32 15, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}

; RUN: opt -switch-to-offload -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Test src:
;
; template<typename DataType = double>
; class binomial
; {
; public:
;   void run();
; };
;
; template<typename DataType>
; void binomial<DataType>::run()
; {
;   #pragma omp target
;     int dummy;
; }
;
; template void binomial<double>::run();
;
; int main() {
;   return 0;
; }

; Pass conditon: the declaration of the function _ZN8binomialIdE3runEv()
; should not exist in target IR.
; CHECK-NOT: declare {{.*}} @_ZN8binomialIdE3runEv

; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

%class.binomial = type { i8 }

$_ZN8binomialIdE3runEv = comdat any

@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_ZN8binomialIdE3runEv(%class.binomial addrspace(4)* %this) #0 comdat align 2 {
entry:
  %this.addr = alloca %class.binomial addrspace(4)*, align 8
  %this.addr.ascast = addrspacecast %class.binomial addrspace(4)** %this.addr to %class.binomial addrspace(4)* addrspace(4)*
  %dummy = alloca i32, align 4
  %dummy.ascast = addrspacecast i32* %dummy to i32 addrspace(4)*
  store %class.binomial addrspace(4)* %this, %class.binomial addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  %this1 = load %class.binomial addrspace(4)*, %class.binomial addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  br label %DIR.OMP.TARGET.1.split

DIR.OMP.TARGET.1.split:                           ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %dummy.ascast) ]
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.1

DIR.OMP.END.TARGET.1:                             ; preds = %DIR.OMP.TARGET.2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2051, i32 146624982, !"_ZN8binomialIdE3runEv", i32 11, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}

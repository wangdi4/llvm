; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-paropt -S %s | FileCheck %s
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

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

$_ZN8binomialIdE3runEv = comdat any

@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind uwtable
define dso_local void @_ZN8binomialIdE3runEv(ptr addrspace(4) %this) comdat align 2 {
entry:
  %this.addr = alloca ptr addrspace(4), align 8
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  %dummy = alloca i32, align 4
  %dummy.ascast = addrspacecast ptr %dummy to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  br label %DIR.OMP.TARGET.1.split

DIR.OMP.TARGET.1.split:                           ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %dummy.ascast, i32 0, i32 1) ]

  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  br label %DIR.OMP.END.TARGET.1

DIR.OMP.END.TARGET.1:                             ; preds = %DIR.OMP.TARGET.2
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)


!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2051, i32 146624982, !"_ZN8binomialIdE3runEv", i32 11, i32 0, i32 0}

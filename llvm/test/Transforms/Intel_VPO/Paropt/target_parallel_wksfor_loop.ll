; RUN: opt -switch-to-offload -vpo-paropt -S %s | FileCheck %s -check-prefix=DEFAULT -check-prefix=ALL
; RUN: opt -passes='vpo-paropt' -switch-to-offload -S %s | FileCheck %s -check-prefix=DEFAULT -check-prefix=ALL
; RUN: opt -switch-to-offload -vpo-paropt -vpo-paropt-emit-wks-loops-implicit-barrier-for-target=false -S %s | FileCheck %s -check-prefix=NO_BARRIER -check-prefix=ALL
; RUN: opt -passes='vpo-paropt' -switch-to-offload -vpo-paropt-emit-wks-loops-implicit-barrier-for-target=false -S %s | FileCheck %s -check-prefix=NO_BARRIER -check-prefix=ALL
;
; #include <iostream>
; int main()
; {
;   int counts1 = 0;
;   int counts2 = 0;
;   #pragma omp target teams map(from:counts1)
;   {
;      int counts_team = 0;
;      #pragma omp parallel
;      {
;         #pragma omp for
;         for (int i=0; i<4; i++)
;            #pragma omp atomic
;            counts_team += 1;
;      }
;      counts1 = counts_team;
;  }
;
;  #pragma omp target teams map(from:counts2)
;  {
;     int counts_team = 0;
;     #pragma omp parallel
;     {
;        #pragma omp for reduction(+:counts_team)
;        for (int i=0; i<4; i++)
;           counts_team += 1;
;     }
;     counts2 = counts_team;
; }
;
;  if (counts1 != 4)
;    std::cout << " wrong counts1 = " << counts1 << " should be 4!" << std::endl;
;  else
;    std::cout << " passed " << std::endl;
;
;  if (counts2 != 4)
;    std::cout << " wrong counts2 = " << counts2 << " should be 4!" << std::endl;
;  else
;    std::cout << " passed " << std::endl;
; }
;
; ALL: %{{.*}} = call spir_func i64 @_Z14get_local_sizej(i32 0)
; ALL: %{{.*}} = call spir_func i64 @_Z12get_local_idj(i32 0)
; Check for the emission of implicit barrier for "#pragma omp for".
; DEFAULT: call spir_func void @__kmpc_barrier()
; NO_BARRIER-NOT: call{{.*}}@__kmpc_barrier


;
; ModuleID = 'target_parallel_wksfor_loop.cpp'
source_filename = "target_parallel_wksfor_loop.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream" addrspace(4)*, i8, i8, %"class.std::basic_streambuf" addrspace(4)*, %"class.std::ctype" addrspace(4)*, %"class.std::num_put" addrspace(4)*, %"class.std::num_get" addrspace(4)* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct.std::ios_base::_Callback_list" addrspace(4)*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words" addrspace(4)*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list" addrspace(4)*, void (i32, %"class.std::ios_base" addrspace(4)*, i32) addrspace(4)*, i32, i32 }
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

@_ZSt4cout = external dso_local addrspace(1) global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr addrspace(1) constant [18 x i8] c" wrong counts1 = \00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [14 x i8] c" should be 4!\00", align 1
@.str.2 = private unnamed_addr addrspace(1) constant [9 x i8] c" passed \00", align 1
@.str.3 = private unnamed_addr addrspace(1) constant [18 x i8] c" wrong counts2 = \00", align 1
@"@tid.addr" = external global i32

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %counts1 = alloca i32, align 4
  %counts1.ascast = addrspacecast i32* %counts1 to i32 addrspace(4)*
  %counts2 = alloca i32, align 4
  %counts2.ascast = addrspacecast i32* %counts2 to i32 addrspace(4)*
  %counts_team = alloca i32, align 4
  %counts_team.ascast = addrspacecast i32* %counts_team to i32 addrspace(4)*
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
  %atomic-temp = alloca i32, align 4
  %atomic-temp.ascast = addrspacecast i32* %atomic-temp to i32 addrspace(4)*
  %atomic-temp1 = alloca i32, align 4
  %atomic-temp1.ascast = addrspacecast i32* %atomic-temp1 to i32 addrspace(4)*
  %counts_team4 = alloca i32, align 4
  %counts_team4.ascast = addrspacecast i32* %counts_team4 to i32 addrspace(4)*
  %tmp5 = alloca i32, align 4
  %tmp5.ascast = addrspacecast i32* %tmp5 to i32 addrspace(4)*
  %.omp.iv6 = alloca i32, align 4
  %.omp.iv6.ascast = addrspacecast i32* %.omp.iv6 to i32 addrspace(4)*
  %.omp.lb7 = alloca i32, align 4
  %.omp.lb7.ascast = addrspacecast i32* %.omp.lb7 to i32 addrspace(4)*
  %.omp.ub8 = alloca i32, align 4
  %.omp.ub8.ascast = addrspacecast i32* %.omp.ub8 to i32 addrspace(4)*
  %i12 = alloca i32, align 4
  %i12.ascast = addrspacecast i32* %i12 to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  store i32 0, i32 addrspace(4)* %counts1.ascast, align 4
  store i32 0, i32 addrspace(4)* %counts2.ascast, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.FROM"(i32 addrspace(4)* %counts1.ascast, i32 addrspace(4)* %counts1.ascast, i32 4, i64 34, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %counts_team.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp1.ascast) ]
  br label %DIR.OMP.TARGET.3151

DIR.OMP.TARGET.3151:                              ; preds = %DIR.OMP.TARGET.2
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.3151
  br label %DIR.OMP.TEAMS.4

DIR.OMP.TEAMS.4:                                  ; preds = %DIR.OMP.TARGET.3
  br label %DIR.OMP.TEAMS.5152

DIR.OMP.TEAMS.5152:                               ; preds = %DIR.OMP.TEAMS.4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %counts_team.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %counts1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp1.ascast) ]
  br label %DIR.OMP.TEAMS.6

DIR.OMP.TEAMS.6:                                  ; preds = %DIR.OMP.TEAMS.5152
  br label %DIR.OMP.TEAMS.5

DIR.OMP.TEAMS.5:                                  ; preds = %DIR.OMP.TEAMS.6
  store i32 0, i32 addrspace(4)* %counts_team.ascast, align 4
  br label %DIR.OMP.PARALLEL.7

DIR.OMP.PARALLEL.7:                               ; preds = %DIR.OMP.TEAMS.5
  br label %DIR.OMP.PARALLEL.8153

DIR.OMP.PARALLEL.8153:                            ; preds = %DIR.OMP.PARALLEL.7
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %counts_team.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp1.ascast) ]
  br label %DIR.OMP.PARALLEL.9

DIR.OMP.PARALLEL.9:                               ; preds = %DIR.OMP.PARALLEL.8153
  br label %DIR.OMP.PARALLEL.8

DIR.OMP.PARALLEL.8:                               ; preds = %DIR.OMP.PARALLEL.9
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store volatile i32 3, i32 addrspace(4)* %.omp.ub.ascast, align 4
  br label %DIR.OMP.LOOP.10154

DIR.OMP.LOOP.10154:                               ; preds = %DIR.OMP.PARALLEL.8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp1.ascast) ]
  br label %DIR.OMP.LOOP.11

DIR.OMP.LOOP.11:                                  ; preds = %DIR.OMP.LOOP.10154
  br label %DIR.OMP.LOOP.10

DIR.OMP.LOOP.10:                                  ; preds = %DIR.OMP.LOOP.11
  %4 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store volatile i32 %4, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.LOOP.10
  %5 = load volatile i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %6 = load volatile i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit.split

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load volatile i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %8 = bitcast i32 addrspace(4)* %counts_team.ascast to i8 addrspace(4)*
  %9 = bitcast i32 addrspace(4)* %atomic-temp.ascast to i8 addrspace(4)*
  call void @__kmpc_atomic_load(i64 4, i8 addrspace(4)* %8, i8 addrspace(4)* %9, i32 0)
  br label %atomic_cont

atomic_cont:                                      ; preds = %atomic_cont, %omp.inner.for.body
  %10 = load i32, i32 addrspace(4)* %atomic-temp.ascast, align 4
  %add2 = add nsw i32 %10, 1
  store i32 %add2, i32 addrspace(4)* %atomic-temp1.ascast, align 4
  %11 = bitcast i32 addrspace(4)* %counts_team.ascast to i8 addrspace(4)*
  %12 = bitcast i32 addrspace(4)* %atomic-temp.ascast to i8 addrspace(4)*
  %13 = bitcast i32 addrspace(4)* %atomic-temp1.ascast to i8 addrspace(4)*
  %call = call zeroext i1 @__kmpc_atomic_compare_exchange(i64 4, i8 addrspace(4)* %11, i8 addrspace(4)* %12, i8 addrspace(4)* %13, i32 0, i32 0)
  br i1 %call, label %omp.inner.for.inc, label %atomic_cont

omp.inner.for.inc:                                ; preds = %atomic_cont
  %14 = load volatile i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %14, 1
  store volatile i32 %add3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond
  br label %DIR.OMP.END.LOOP.12

DIR.OMP.END.LOOP.12:                              ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.11.split

DIR.OMP.END.LOOP.11.split:                        ; preds = %DIR.OMP.END.LOOP.12
  br label %DIR.OMP.END.PARALLEL.13

DIR.OMP.END.PARALLEL.13:                          ; preds = %DIR.OMP.END.LOOP.11.split
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.14

DIR.OMP.END.PARALLEL.14:                          ; preds = %DIR.OMP.END.PARALLEL.13
  %15 = load i32, i32 addrspace(4)* %counts_team.ascast, align 4
  store i32 %15, i32 addrspace(4)* %counts1.ascast, align 4
  br label %DIR.OMP.END.TEAMS.13.split

DIR.OMP.END.TEAMS.13.split:                       ; preds = %DIR.OMP.END.PARALLEL.14
  br label %DIR.OMP.END.TEAMS.15

DIR.OMP.END.TEAMS.15:                             ; preds = %DIR.OMP.END.TEAMS.13.split
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TEAMS.14.split

DIR.OMP.END.TEAMS.14.split:                       ; preds = %DIR.OMP.END.TEAMS.15
  br label %DIR.OMP.END.TARGET.16

DIR.OMP.END.TARGET.16:                            ; preds = %DIR.OMP.END.TEAMS.14.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.17

DIR.OMP.END.TARGET.17:                            ; preds = %DIR.OMP.END.TARGET.16
  br label %DIR.OMP.TARGET.18

DIR.OMP.TARGET.18:                                ; preds = %DIR.OMP.END.TARGET.17
  br label %DIR.OMP.TARGET.19

DIR.OMP.TARGET.19:                                ; preds = %DIR.OMP.TARGET.18
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), "QUAL.OMP.MAP.FROM"(i32 addrspace(4)* %counts2.ascast, i32 addrspace(4)* %counts2.ascast, i32 4, i64 34, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub8.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb7.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i12.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv6.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %counts_team4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp5.ascast) ]
  br label %DIR.OMP.TARGET.20

DIR.OMP.TARGET.20:                                ; preds = %DIR.OMP.TARGET.19
  br label %DIR.OMP.TARGET.17

DIR.OMP.TARGET.17:                                ; preds = %DIR.OMP.TARGET.20
  br label %DIR.OMP.TEAMS.21

DIR.OMP.TEAMS.21:                                 ; preds = %DIR.OMP.TARGET.17
  br label %DIR.OMP.TEAMS.22

DIR.OMP.TEAMS.22:                                 ; preds = %DIR.OMP.TEAMS.21
  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %counts_team4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub8.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb7.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i12.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv6.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %counts2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp5.ascast) ]
  br label %DIR.OMP.TEAMS.23

DIR.OMP.TEAMS.23:                                 ; preds = %DIR.OMP.TEAMS.22
  br label %DIR.OMP.TEAMS.19

DIR.OMP.TEAMS.19:                                 ; preds = %DIR.OMP.TEAMS.23
  store i32 0, i32 addrspace(4)* %counts_team4.ascast, align 4
  br label %DIR.OMP.PARALLEL.24

DIR.OMP.PARALLEL.24:                              ; preds = %DIR.OMP.TEAMS.19
  br label %DIR.OMP.PARALLEL.25

DIR.OMP.PARALLEL.25:                              ; preds = %DIR.OMP.PARALLEL.24
  %18 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv6.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb7.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub8.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i12.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %counts_team4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp5.ascast) ]
  br label %DIR.OMP.PARALLEL.26

DIR.OMP.PARALLEL.26:                              ; preds = %DIR.OMP.PARALLEL.25
  br label %DIR.OMP.PARALLEL.22

DIR.OMP.PARALLEL.22:                              ; preds = %DIR.OMP.PARALLEL.26
  store i32 0, i32 addrspace(4)* %.omp.lb7.ascast, align 4
  store volatile i32 3, i32 addrspace(4)* %.omp.ub8.ascast, align 4
  br label %DIR.OMP.LOOP.27

DIR.OMP.LOOP.27:                                  ; preds = %DIR.OMP.PARALLEL.22
  %19 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %counts_team4.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb7.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv6.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub8.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i12.ascast) ]
  br label %DIR.OMP.LOOP.28

DIR.OMP.LOOP.28:                                  ; preds = %DIR.OMP.LOOP.27
  br label %DIR.OMP.LOOP.24

DIR.OMP.LOOP.24:                                  ; preds = %DIR.OMP.LOOP.28
  %20 = load i32, i32 addrspace(4)* %.omp.lb7.ascast, align 4
  store volatile i32 %20, i32 addrspace(4)* %.omp.iv6.ascast, align 4
  br label %omp.inner.for.cond9

omp.inner.for.cond9:                              ; preds = %omp.inner.for.body11, %DIR.OMP.LOOP.24
  %21 = load volatile i32, i32 addrspace(4)* %.omp.iv6.ascast, align 4
  %22 = load volatile i32, i32 addrspace(4)* %.omp.ub8.ascast, align 4
  %cmp10 = icmp sle i32 %21, %22
  br i1 %cmp10, label %omp.inner.for.body11, label %omp.loop.exit20.split

omp.inner.for.body11:                             ; preds = %omp.inner.for.cond9
  %23 = load volatile i32, i32 addrspace(4)* %.omp.iv6.ascast, align 4
  %mul13 = mul nsw i32 %23, 1
  %add14 = add nsw i32 0, %mul13
  store i32 %add14, i32 addrspace(4)* %i12.ascast, align 4
  %24 = load i32, i32 addrspace(4)* %counts_team4.ascast, align 4
  %add15 = add nsw i32 %24, 1
  store i32 %add15, i32 addrspace(4)* %counts_team4.ascast, align 4
  %25 = load volatile i32, i32 addrspace(4)* %.omp.iv6.ascast, align 4
  %add18 = add nsw i32 %25, 1
  store volatile i32 %add18, i32 addrspace(4)* %.omp.iv6.ascast, align 4
  br label %omp.inner.for.cond9

omp.loop.exit20.split:                            ; preds = %omp.inner.for.cond9
  br label %DIR.OMP.END.LOOP.29

DIR.OMP.END.LOOP.29:                              ; preds = %omp.loop.exit20.split
  call void @llvm.directive.region.exit(token %19) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.25.split

DIR.OMP.END.LOOP.25.split:                        ; preds = %DIR.OMP.END.LOOP.29
  br label %DIR.OMP.END.PARALLEL.30

DIR.OMP.END.PARALLEL.30:                          ; preds = %DIR.OMP.END.LOOP.25.split
  call void @llvm.directive.region.exit(token %18) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.31

DIR.OMP.END.PARALLEL.31:                          ; preds = %DIR.OMP.END.PARALLEL.30
  %26 = load i32, i32 addrspace(4)* %counts_team4.ascast, align 4
  store i32 %26, i32 addrspace(4)* %counts2.ascast, align 4
  br label %DIR.OMP.END.TEAMS.27.split

DIR.OMP.END.TEAMS.27.split:                       ; preds = %DIR.OMP.END.PARALLEL.31
  br label %DIR.OMP.END.TEAMS.32

DIR.OMP.END.TEAMS.32:                             ; preds = %DIR.OMP.END.TEAMS.27.split
  call void @llvm.directive.region.exit(token %17) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TEAMS.28.split

DIR.OMP.END.TEAMS.28.split:                       ; preds = %DIR.OMP.END.TEAMS.32
  br label %DIR.OMP.END.TARGET.33

DIR.OMP.END.TARGET.33:                            ; preds = %DIR.OMP.END.TEAMS.28.split
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.34

DIR.OMP.END.TARGET.34:                            ; preds = %DIR.OMP.END.TARGET.33
  %27 = load i32, i32 addrspace(4)* %counts1.ascast, align 4
  %cmp21 = icmp ne i32 %27, 4
  br i1 %cmp21, label %if.then, label %if.else

if.then:                                          ; preds = %DIR.OMP.END.TARGET.34
  %call22 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream" addrspace(4)* dereferenceable(272) addrspacecast (%"class.std::basic_ostream" addrspace(1)* @_ZSt4cout to %"class.std::basic_ostream" addrspace(4)*), i8 addrspace(4)* getelementptr inbounds ([18 x i8], [18 x i8] addrspace(4)* addrspacecast ([18 x i8] addrspace(1)* @.str to [18 x i8] addrspace(4)*), i64 0, i64 0))
  %28 = load i32, i32 addrspace(4)* %counts1.ascast, align 4
  %call23 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZNSolsEi(%"class.std::basic_ostream" addrspace(4)* %call22, i32 %28)
  %call24 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream" addrspace(4)* dereferenceable(272) %call23, i8 addrspace(4)* getelementptr inbounds ([14 x i8], [14 x i8] addrspace(4)* addrspacecast ([14 x i8] addrspace(1)* @.str.1 to [14 x i8] addrspace(4)*), i64 0, i64 0))
  %call25 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZNSolsEPFRSoS_E(%"class.std::basic_ostream" addrspace(4)* %call24, %"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*) addrspace(4)* addrspacecast (%"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*)* @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_ to %"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*) addrspace(4)*))
  br label %if.end

if.else:                                          ; preds = %DIR.OMP.END.TARGET.34
  %call26 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream" addrspace(4)* dereferenceable(272) addrspacecast (%"class.std::basic_ostream" addrspace(1)* @_ZSt4cout to %"class.std::basic_ostream" addrspace(4)*), i8 addrspace(4)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(4)* addrspacecast ([9 x i8] addrspace(1)* @.str.2 to [9 x i8] addrspace(4)*), i64 0, i64 0))
  %call27 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZNSolsEPFRSoS_E(%"class.std::basic_ostream" addrspace(4)* %call26, %"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*) addrspace(4)* addrspacecast (%"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*)* @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_ to %"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*) addrspace(4)*))
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %29 = load i32, i32 addrspace(4)* %counts2.ascast, align 4
  %cmp28 = icmp ne i32 %29, 4
  br i1 %cmp28, label %if.then29, label %if.else34

if.then29:                                        ; preds = %if.end
  %call30 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream" addrspace(4)* dereferenceable(272) addrspacecast (%"class.std::basic_ostream" addrspace(1)* @_ZSt4cout to %"class.std::basic_ostream" addrspace(4)*), i8 addrspace(4)* getelementptr inbounds ([18 x i8], [18 x i8] addrspace(4)* addrspacecast ([18 x i8] addrspace(1)* @.str.3 to [18 x i8] addrspace(4)*), i64 0, i64 0))
  %30 = load i32, i32 addrspace(4)* %counts2.ascast, align 4
  %call31 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZNSolsEi(%"class.std::basic_ostream" addrspace(4)* %call30, i32 %30)
  %call32 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream" addrspace(4)* dereferenceable(272) %call31, i8 addrspace(4)* getelementptr inbounds ([14 x i8], [14 x i8] addrspace(4)* addrspacecast ([14 x i8] addrspace(1)* @.str.1 to [14 x i8] addrspace(4)*), i64 0, i64 0))
  %call33 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZNSolsEPFRSoS_E(%"class.std::basic_ostream" addrspace(4)* %call32, %"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*) addrspace(4)* addrspacecast (%"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*)* @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_ to %"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*) addrspace(4)*))
  br label %if.end37

if.else34:                                        ; preds = %if.end
  %call35 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream" addrspace(4)* dereferenceable(272) addrspacecast (%"class.std::basic_ostream" addrspace(1)* @_ZSt4cout to %"class.std::basic_ostream" addrspace(4)*), i8 addrspace(4)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(4)* addrspacecast ([9 x i8] addrspace(1)* @.str.2 to [9 x i8] addrspace(4)*), i64 0, i64 0))
  %call36 = call spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZNSolsEPFRSoS_E(%"class.std::basic_ostream" addrspace(4)* %call35, %"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*) addrspace(4)* addrspacecast (%"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*)* @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_ to %"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*) addrspace(4)*))
  br label %if.end37

if.end37:                                         ; preds = %if.else34, %if.then29
  %31 = load i32, i32 addrspace(4)* %retval.ascast, align 4
  ret i32 %31
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @__atomic_load(i64, i8 addrspace(4)*, i8 addrspace(4)*, i32)

declare dso_local i1 @__atomic_compare_exchange(i64, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i32, i32)

declare dso_local spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream" addrspace(4)* dereferenceable(272), i8 addrspace(4)*) #2

declare dso_local spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZNSolsEi(%"class.std::basic_ostream" addrspace(4)*, i32) #2

declare dso_local spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZNSolsEPFRSoS_E(%"class.std::basic_ostream" addrspace(4)*, %"class.std::basic_ostream" addrspace(4)* (%"class.std::basic_ostream" addrspace(4)*) addrspace(4)*) #2

declare dso_local spir_func dereferenceable(272) %"class.std::basic_ostream" addrspace(4)* @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(%"class.std::basic_ostream" addrspace(4)* dereferenceable(272)) #2

declare dso_local void @__kmpc_atomic_load(i64, i8 addrspace(4)*, i8 addrspace(4)*, i32)

declare dso_local i1 @__kmpc_atomic_compare_exchange(i64, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { noinline norecurse nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 60, i32 -1942937669, !"main", i32 7, i32 0, i32 0}
!1 = !{i32 0, i32 60, i32 -1942937669, !"main", i32 20, i32 1, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{}
!4 = !{!"clang version 9.0.0"}

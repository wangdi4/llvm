; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


; This test is to check that store instruction with non-zero index pointer
; operand is not treated as having side effect, and is put with other
; instructions being executed by all threads.
;
; #include <stdio.h>
;
; int main(void)
; {
;   const int SIZE=100;
;   int a = 1;
;   int privatized_array[2] = {-1, -1};
; #pragma omp target data map(to: a) map(tofrom: privatized_array[0:2])
;   {
; #pragma omp target teams distribute lastprivate(privatized_array) num_teams(2)
;   thread_limit(2)
;     for (int x = 0; x < SIZE; ++x) {
;       privatized_array[0] = a;
;       privatized_array[1] = a * 9;
;     }
;   }
;   if (privatized_array[0] == a && privatized_array[1] == a * 9) {
;     printf("PASS.\n");
;     return 0;
;   }
;
;   printf("FAILED.\n");
;   return 1;
; }

; CHECK: %[[A_FPRIV1:[^,]+]] = load i32, i32* %a.ascast.fpriv, align 4
; CHECK-NEXT: %[[ARRAY_IDX1:[^,]+]] = getelementptr inbounds [2 x i32], [2 x i32]* %privatized_array.ascast.lpriv, i64 0, i64 0
; CHECK-NEXT: store i32 %[[A_FPRIV1]], i32* %[[ARRAY_IDX1]], align 4
; CHECK-NEXT: %[[A_FPRIV2:[^,]+]] = load i32, i32* %a.ascast.fpriv, align 4
; CHECK-NEXT: %[[MUL_RES:[^,]+]] = mul nsw i32 %[[A_FPRIV2]], 9
; CHECK-NEXT: %[[ARRAY_IDX2:[^,]+]] = getelementptr inbounds [2 x i32], [2 x i32]* %privatized_array.ascast.lpriv, i64 0, i64 1
; CHECK-NEXT: store i32 %[[MUL_RES]], i32* %[[ARRAY_IDX2]], align 4


; ModuleID = 'target_teams_distribute_privatized_array.c'
source_filename = "target_teams_distribute_privatized_array.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@__const.main.privatized_array = private unnamed_addr addrspace(1) constant [2 x i32] [i32 -1, i32 -1], align 4
@.str = private unnamed_addr addrspace(1) constant [7 x i8] c"PASS.\0A\00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [9 x i8] c"FAILED.\0A\00", align 1

; Function Attrs: noinline nounwind optnone
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %SIZE = alloca i32, align 4
  %SIZE.ascast = addrspacecast i32* %SIZE to i32 addrspace(4)*
  %a = alloca i32, align 4
  %a.ascast = addrspacecast i32* %a to i32 addrspace(4)*
  %privatized_array = alloca [2 x i32], align 4
  %privatized_array.ascast = addrspacecast [2 x i32]* %privatized_array to [2 x i32] addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %x = alloca i32, align 4
  %x.ascast = addrspacecast i32* %x to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  store i32 100, i32 addrspace(4)* %SIZE.ascast, align 4
  store i32 1, i32 addrspace(4)* %a.ascast, align 4
  %0 = bitcast [2 x i32] addrspace(4)* %privatized_array.ascast to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p1i8.i64(i8 addrspace(4)* align 4 %0, i8 addrspace(1)* align 4 bitcast ([2 x i32] addrspace(1)* @__const.main.privatized_array to i8 addrspace(1)*), i64 8, i1 false)
  %arrayidx = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %privatized_array.ascast, i64 0, i64 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.MAP.TO"(i32 addrspace(4)* %a.ascast, i32 addrspace(4)* %a.ascast, i64 4, i64 33), "QUAL.OMP.MAP.TOFROM"([2 x i32] addrspace(4)* %privatized_array.ascast, i32 addrspace(4)* %arrayidx, i64 8, i64 35) ]
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([2 x i32] addrspace(4)* %privatized_array.ascast, [2 x i32] addrspace(4)* %privatized_array.ascast, i64 8, i64 547), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %a.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %x.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 2), "QUAL.OMP.THREAD_LIMIT"(i32 2), "QUAL.OMP.SHARED"(i32 addrspace(4)* %a.ascast), "QUAL.OMP.SHARED"([2 x i32] addrspace(4)* %privatized_array.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %x.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.LASTPRIVATE"([2 x i32] addrspace(4)* %privatized_array.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %x.ascast) ]
  %5 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %5, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %7 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %x.ascast, align 4
  %9 = load i32, i32 addrspace(4)* %a.ascast, align 4
  %arrayidx1 = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %privatized_array.ascast, i64 0, i64 0
  store i32 %9, i32 addrspace(4)* %arrayidx1, align 4
  %10 = load i32, i32 addrspace(4)* %a.ascast, align 4
  %mul2 = mul nsw i32 %10, 9
  %arrayidx3 = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %privatized_array.ascast, i64 0, i64 1
  store i32 %mul2, i32 addrspace(4)* %arrayidx3, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add4 = add nsw i32 %11, 1
  store i32 %add4, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.DATA"() ]
  %arrayidx5 = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %privatized_array.ascast, i64 0, i64 0
  %12 = load i32, i32 addrspace(4)* %arrayidx5, align 4
  %13 = load i32, i32 addrspace(4)* %a.ascast, align 4
  %cmp6 = icmp eq i32 %12, %13
  br i1 %cmp6, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %omp.loop.exit
  %arrayidx7 = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %privatized_array.ascast, i64 0, i64 1
  %14 = load i32, i32 addrspace(4)* %arrayidx7, align 4
  %15 = load i32, i32 addrspace(4)* %a.ascast, align 4
  %mul8 = mul nsw i32 %15, 9
  %cmp9 = icmp eq i32 %14, %mul8
  br i1 %cmp9, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([7 x i8], [7 x i8] addrspace(4)* addrspacecast ([7 x i8] addrspace(1)* @.str to [7 x i8] addrspace(4)*), i64 0, i64 0))
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  br label %return

if.end:                                           ; preds = %land.lhs.true, %omp.loop.exit
  %call10 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(4)* addrspacecast ([9 x i8] addrspace(1)* @.str.1 to [9 x i8] addrspace(4)*), i64 0, i64 0))
  store i32 1, i32 addrspace(4)* %retval.ascast, align 4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %16 = load i32, i32 addrspace(4)* %retval.ascast, align 4
  ret i32 %16
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.memcpy.p4i8.p1i8.i64(i8 addrspace(4)* noalias nocapture writeonly, i8 addrspace(1)* noalias nocapture readonly, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare spir_func i32 @printf(i8 addrspace(4)*, ...) #3

attributes #0 = { noinline nounwind optnone "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2065, i32 10493904, !"_Z4main", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"clang version 9.0.0"}

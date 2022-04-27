; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


; This test is to check that store instruction with non-zero index pointer
; operand is not treated as having side effect, and is put with other
; instructions being executed by all threads.
;
; #include <stdio.h>
;
; int foo(int n)
; {
;   int a = 1;
;   int privatized_array[n];
;   for (int i=0; i<n; i++)
;     privatized_array[i] = -1;
; #pragma omp target data map(to: a) map(tofrom: privatized_array[0:n])
;   {
; #pragma omp target teams distribute lastprivate(privatized_array) num_teams(2) thread_limit(2)
;     for (int x = 0; x < n; x+=2) {
;       privatized_array[x] = a;
;       privatized_array[x+1] = a * 9;
;     }
;   }
;
;   for (int i=0; i<n; i+=2) {
;     if (privatized_array[i] != a || privatized_array[i+1] != a * 9) {
;       printf("FAILED.\n");
;       return 1;
;     }
;   }
;
;   printf("PASS.\n");
;   return 1;
; }
;
; int main(void)
; {
;   return foo(2);
; }


; CHECK: %[[A_FPRIV1:[^,]+]] = load i32, i32* %a.ascast.fpriv, align 4
; CHECK-NEXT: %[[X_LOCAL1:[^,]+]] = load i32, i32 addrspace(3)* @x.ascast.priv.__local, align 4
; CHECK-NEXT: %[[INDEX1:[^,]+]] = sext i32 %[[X_LOCAL1]] to i64
; CHECK-NEXT: %[[PTR1:[^,]+]] = getelementptr inbounds i32, i32* %vla.ascast.lpriv, i64 %[[INDEX1]]
; CHECK-NEXT: store i32 %[[A_FPRIV1]], i32* %[[PTR1]], align 4
; CHECK-NEXT: %[[A_FPRIV2:[^,]+]] = load i32, i32* %a.ascast.fpriv, align 4
; CHECK-NEXT: %[[MUL_RES:[^,]+]] = mul nsw i32 %[[A_FPRIV2]], 9
; CHECK-NEXT: %[[X_LOCAL2:[^,]+]] = load i32, i32 addrspace(3)* @x.ascast.priv.__local, align 4
; CHECK-NEXT: %[[ADD_RES:[^,]+]] = add nsw i32 %[[X_LOCAL2]], 1
; CHECK-NEXT: %[[INDEX2:[^,]+]] = sext i32 %[[ADD_RES]] to i64
; CHECK-NEXT: %[[PTR2:[^,]+]] = getelementptr inbounds i32, i32* %vla.ascast.lpriv, i64 %[[INDEX2]]
; CHECK-NEXT: store i32 %[[MUL_RES]], i32* %[[PTR2]], align 4


; ModuleID = 'target_teams_distribute_privatized_varlen_array.c'
source_filename = "target_teams_distribute_privatized_varlen_array.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [9 x i8] c"FAILED.\0A\00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [7 x i8] c"PASS.\0A\00", align 1

; Function Attrs: noinline nounwind optnone
define hidden spir_func i32 @foo(i32 %n) #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %n.addr = alloca i32, align 4
  %n.addr.ascast = addrspacecast i32* %n.addr to i32 addrspace(4)*
  %a = alloca i32, align 4
  %a.ascast = addrspacecast i32* %a to i32 addrspace(4)*
  %saved_stack = alloca i8*, align 8
  %saved_stack.ascast = addrspacecast i8** %saved_stack to i8* addrspace(4)*
  %__vla_expr0 = alloca i64, align 8
  %__vla_expr0.ascast = addrspacecast i64* %__vla_expr0 to i64 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %omp.vla.tmp = alloca i64, align 8
  %omp.vla.tmp.ascast = addrspacecast i64* %omp.vla.tmp to i64 addrspace(4)*
  %omp.vla.tmp1 = alloca i64, align 8
  %omp.vla.tmp1.ascast = addrspacecast i64* %omp.vla.tmp1 to i64 addrspace(4)*
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.0.ascast = addrspacecast i32* %.capture_expr.0 to i32 addrspace(4)*
  %.capture_expr.1 = alloca i32, align 4
  %.capture_expr.1.ascast = addrspacecast i32* %.capture_expr.1 to i32 addrspace(4)*
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
  %i16 = alloca i32, align 4
  %i16.ascast = addrspacecast i32* %i16 to i32 addrspace(4)*
  %cleanup.dest.slot = alloca i32, align 4
  %cleanup.dest.slot.ascast = addrspacecast i32* %cleanup.dest.slot to i32 addrspace(4)*
  store i32 %n, i32 addrspace(4)* %n.addr.ascast, align 4
  store i32 1, i32 addrspace(4)* %a.ascast, align 4
  %0 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8* addrspace(4)* %saved_stack.ascast, align 8
  %vla = alloca i32, i64 %1, align 4
  %vla.ascast = addrspacecast i32* %vla to i32 addrspace(4)*
  store i64 %1, i64 addrspace(4)* %__vla_expr0.ascast, align 8
  store i32 0, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %3 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %4 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  %cmp = icmp slt i32 %3, %4
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %5 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %5 to i64
  %ptridx = getelementptr inbounds i32, i32 addrspace(4)* %vla.ascast, i64 %idxprom
  store i32 -1, i32 addrspace(4)* %ptridx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %6 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i64 %1, i64 addrspace(4)* %omp.vla.tmp.ascast, align 8
  %arrayidx = getelementptr inbounds i32, i32 addrspace(4)* %vla.ascast, i64 0
  %7 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  %conv = sext i32 %7 to i64
  %8 = mul nuw i64 %conv, 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.MAP.TO"(i32 addrspace(4)* %a.ascast, i32 addrspace(4)* %a.ascast, i64 4, i64 33), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %vla.ascast, i32 addrspace(4)* %arrayidx, i64 %8, i64 35) ]
  %10 = load i64, i64 addrspace(4)* %omp.vla.tmp.ascast, align 8
  store i64 %10, i64 addrspace(4)* %omp.vla.tmp1.ascast, align 8
  %11 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  store i32 %11, i32 addrspace(4)* %.capture_expr.0.ascast, align 4
  %12 = load i32, i32 addrspace(4)* %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %12, 0
  %sub2 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub2, 2
  %div = sdiv i32 %add, 2
  %sub3 = sub nsw i32 %div, 1
  store i32 %sub3, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  %13 = load i32, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  store i32 %13, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %14 = mul nuw i64 %10, 4
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %vla.ascast, i32 addrspace(4)* %vla.ascast, i64 %14, i64 547), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %a.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %x.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.capture_expr.0.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %omp.vla.tmp1.ascast) ]
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 2), "QUAL.OMP.THREAD_LIMIT"(i32 2), "QUAL.OMP.SHARED"(i32 addrspace(4)* %a.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %vla.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %x.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.capture_expr.0.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.SHARED"(i64 addrspace(4)* %omp.vla.tmp1.ascast) ]
  %17 = load i32, i32 addrspace(4)* %.capture_expr.0.ascast, align 4
  %cmp4 = icmp slt i32 0, %17
  br i1 %cmp4, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %for.end
  %18 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.LASTPRIVATE"(i32 addrspace(4)* %vla.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %x.ascast) ]
  %19 = load i64, i64 addrspace(4)* %omp.vla.tmp1.ascast, align 8
  %20 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %20, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %21 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %22 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp6 = icmp sle i32 %21, %22
  br i1 %cmp6, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %23 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %23, 2
  %add8 = add nsw i32 0, %mul
  store i32 %add8, i32 addrspace(4)* %x.ascast, align 4
  %24 = load i32, i32 addrspace(4)* %a.ascast, align 4
  %25 = load i32, i32 addrspace(4)* %x.ascast, align 4
  %idxprom9 = sext i32 %25 to i64
  %ptridx10 = getelementptr inbounds i32, i32 addrspace(4)* %vla.ascast, i64 %idxprom9
  store i32 %24, i32 addrspace(4)* %ptridx10, align 4
  %26 = load i32, i32 addrspace(4)* %a.ascast, align 4
  %mul11 = mul nsw i32 %26, 9
  %27 = load i32, i32 addrspace(4)* %x.ascast, align 4
  %add12 = add nsw i32 %27, 1
  %idxprom13 = sext i32 %add12 to i64
  %ptridx14 = getelementptr inbounds i32, i32 addrspace(4)* %vla.ascast, i64 %idxprom13
  store i32 %mul11, i32 addrspace(4)* %ptridx14, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %28 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add15 = add nsw i32 %28, 1
  store i32 %add15, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %18) [ "DIR.OMP.END.DISTRIBUTE"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %for.end
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TARGET.DATA"() ]
  store i32 0, i32 addrspace(4)* %i16.ascast, align 4
  br label %for.cond17

for.cond17:                                       ; preds = %for.inc31, %omp.precond.end
  %29 = load i32, i32 addrspace(4)* %i16.ascast, align 4
  %30 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  %cmp18 = icmp slt i32 %29, %30
  br i1 %cmp18, label %for.body20, label %for.end33

for.body20:                                       ; preds = %for.cond17
  %31 = load i32, i32 addrspace(4)* %i16.ascast, align 4
  %idxprom21 = sext i32 %31 to i64
  %ptridx22 = getelementptr inbounds i32, i32 addrspace(4)* %vla.ascast, i64 %idxprom21
  %32 = load i32, i32 addrspace(4)* %ptridx22, align 4
  %33 = load i32, i32 addrspace(4)* %a.ascast, align 4
  %cmp23 = icmp ne i32 %32, %33
  br i1 %cmp23, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %for.body20
  %34 = load i32, i32 addrspace(4)* %i16.ascast, align 4
  %add25 = add nsw i32 %34, 1
  %idxprom26 = sext i32 %add25 to i64
  %ptridx27 = getelementptr inbounds i32, i32 addrspace(4)* %vla.ascast, i64 %idxprom26
  %35 = load i32, i32 addrspace(4)* %ptridx27, align 4
  %36 = load i32, i32 addrspace(4)* %a.ascast, align 4
  %mul28 = mul nsw i32 %36, 9
  %cmp29 = icmp ne i32 %35, %mul28
  br i1 %cmp29, label %if.then, label %if.end

if.then:                                          ; preds = %lor.lhs.false, %for.body20
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(4)* addrspacecast ([9 x i8] addrspace(1)* @.str to [9 x i8] addrspace(4)*), i64 0, i64 0))
  store i32 1, i32 addrspace(4)* %retval.ascast, align 4
  store i32 1, i32 addrspace(4)* %cleanup.dest.slot.ascast, align 4
  br label %cleanup

if.end:                                           ; preds = %lor.lhs.false
  br label %for.inc31

for.inc31:                                        ; preds = %if.end
  %37 = load i32, i32 addrspace(4)* %i16.ascast, align 4
  %add32 = add nsw i32 %37, 2
  store i32 %add32, i32 addrspace(4)* %i16.ascast, align 4
  br label %for.cond17

for.end33:                                        ; preds = %for.cond17
  %call34 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([7 x i8], [7 x i8] addrspace(4)* addrspacecast ([7 x i8] addrspace(1)* @.str.1 to [7 x i8] addrspace(4)*), i64 0, i64 0))
  store i32 1, i32 addrspace(4)* %retval.ascast, align 4
  store i32 1, i32 addrspace(4)* %cleanup.dest.slot.ascast, align 4
  br label %cleanup

cleanup:                                          ; preds = %for.end33, %if.then
  %38 = load i8*, i8* addrspace(4)* %saved_stack.ascast, align 8
  call void @llvm.stackrestore(i8* %38)
  %39 = load i32, i32 addrspace(4)* %retval.ascast, align 4
  ret i32 %39
}

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare spir_func i32 @printf(i8 addrspace(4)*, ...) #2

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #1

attributes #0 = { noinline nounwind optnone "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2065, i32 10488866, !"_Z3foo", i32 11, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"clang version 9.0.0"}

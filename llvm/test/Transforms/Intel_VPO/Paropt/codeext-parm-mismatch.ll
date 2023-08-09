; REQUIRES: asserts
; RUN: opt -switch-to-offload -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt" -debug-only=code-extractor -S %s 2>&1 | FileCheck %s

; Test src:
;
; void b() {
;   int ix;
; #pragma omp target nowait
;   for (ix = 3; ix < 10; ix++)
;     ;
; }
;
; #define N 1000
; int depends() {
;   int dep_1[N];
;
; #pragma omp target depend(out : dep_1) map(tofrom : dep_1 [0:N])
;   {
;     for (int i = 0; i < N; i++)
;       dep_1[i] = 1;
;   }
;   return dep_1[4];
; }

; The target code is outlined into a temporary stub function that takes an
; addrspace(4) argument. The enclosing task makes a firstprivate value copy
; in addrspace(0) and passes it to the stub function. The verifier aborts on
; the addrspace mismatch between the actual parameter and the declaration.

; CHECK-NOT: verification{{.*}}failed

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.kmp_depend_info = type { i64, i64, i8 }

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @b() #0 {
entry:
  %ix = alloca i32, align 4
  %ix.ascast = addrspacecast ptr %ix to ptr addrspace(4)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.TARGET.TASK"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %ix.ascast, i32 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %ix.ascast, i32 0, i32 1) ]
  store i32 3, ptr addrspace(4) %ix.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, ptr addrspace(4) %ix.ascast, align 4
  %cmp = icmp slt i32 %2, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, ptr addrspace(4) %ix.ascast, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr addrspace(4) %ix.ascast, align 4
  br label %for.cond, !llvm.loop !9

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func i32 @depends() #0 {
entry:
  %retval = alloca i32, align 4
  %dep_1 = alloca [1000 x i32], align 4
  %.dep.arr.addr = alloca [1 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  %i = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %dep_1.ascast = addrspacecast ptr %dep_1 to ptr addrspace(4)
  %.dep.arr.addr.ascast = addrspacecast ptr %.dep.arr.addr to ptr addrspace(4)
  %dep.counter.addr.ascast = addrspacecast ptr %dep.counter.addr to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %0 = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr addrspace(4) %.dep.arr.addr.ascast, i64 0, i64 0
  %1 = ptrtoint ptr addrspace(4) %dep_1.ascast to i64
  %2 = getelementptr %struct.kmp_depend_info, ptr addrspace(4) %0, i64 0
  %3 = getelementptr inbounds %struct.kmp_depend_info, ptr addrspace(4) %2, i32 0, i32 0
  store i64 %1, ptr addrspace(4) %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, ptr addrspace(4) %2, i32 0, i32 1
  store i64 4000, ptr addrspace(4) %4, align 8
  %5 = getelementptr inbounds %struct.kmp_depend_info, ptr addrspace(4) %2, i32 0, i32 2
  store i8 3, ptr addrspace(4) %5, align 8
  store i64 1, ptr addrspace(4) %dep.counter.addr.ascast, align 8
  %6 = addrspacecast ptr addrspace(4) %0 to ptr
  %arrayidx = getelementptr inbounds [1000 x i32], ptr addrspace(4) %dep_1.ascast, i64 0, i64 0
  %array.begin = getelementptr inbounds [1000 x i32], ptr addrspace(4) %dep_1.ascast, i32 0, i32 0
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IF"(i32 0),
    "QUAL.OMP.TARGET.TASK"(),
    "QUAL.OMP.DEPARRAY"(i32 1, ptr %6),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %dep_1.ascast, i32 0, i64 1000),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]
  %arrayidx1 = getelementptr inbounds [1000 x i32], ptr addrspace(4) %dep_1.ascast, i64 0, i64 0
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %dep_1.ascast, ptr addrspace(4) %arrayidx1, i64 4000, i64 35, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]
  store i32 0, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %9 = load i32, ptr addrspace(4) %i.ascast, align 4
  %cmp = icmp slt i32 %9, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %10 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %10 to i64
  %arrayidx2 = getelementptr inbounds [1000 x i32], ptr addrspace(4) %dep_1.ascast, i64 0, i64 %idxprom
  store i32 1, ptr addrspace(4) %arrayidx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %11 = load i32, ptr addrspace(4) %i.ascast, align 4
  %inc = add nsw i32 %11, 1
  store i32 %inc, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond, !llvm.loop !11

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TASK"() ]
  %arrayidx3 = getelementptr inbounds [1000 x i32], ptr addrspace(4) %dep_1.ascast, i64 0, i64 4
  %12 = load i32, ptr addrspace(4) %arrayidx3, align 4
  ret i32 %12
}

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2, !3, !4, !5, !6}

!0 = !{i32 0, i32 53, i32 -1927896426, !"_Z7depends", i32 12, i32 1, i32 0}
!1 = !{i32 0, i32 53, i32 -1927896426, !"_Z1b", i32 3, i32 0, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 7, !"openmp", i32 51}
!4 = !{i32 7, !"openmp-device", i32 51}
!5 = !{i32 7, !"PIC Level", i32 2}
!6 = !{i32 7, !"frame-pointer", i32 2}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}

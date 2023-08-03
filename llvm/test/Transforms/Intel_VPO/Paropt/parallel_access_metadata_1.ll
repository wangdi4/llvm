; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck --check-prefixes=CHECK,CHECKPA %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck --check-prefixes=CHECK,CHECKPA %s
; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck --check-prefixes=CHECK,CHECKIV %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck --check-prefixes=CHECK,CHECKIV %s
; end INTEL_CUSTOMIZATION
;
; This test checks that paropt pass adds parallel access metadata to loops
; with non-monotonic property.
;
; void test1(float *P) {
; #pragma omp for
;   for (int I = 0 ; I < 100; I++)
;     P[I] = I;
; }
;
; CHECK-LABEL: define void @test1
; CHECK: call void @__kmpc_for_static_init_4(
; CHECK: [[FP1:%.+]] = sitofp i32 %{{.+}} to float
; CHECK: store float [[FP1]]{{.*}}, !llvm.access.group ![[AG1:[0-9]+]]
; CHECK: br i1 %{{.+}}, label %{{.+}}, label %{{.+}}, !llvm.loop ![[ID1:[0-9]+]]
;
; void test2(float *P) {
; #pragma omp for schedule(nonmonotonic: static)
;   for (int I = 0 ; I < 100; I++)
;     P[I] = I;
; }
;
; CHECK-LABEL: define void @test2
; CHECK: call void @__kmpc_for_static_init_4(
; CHECK: [[FP2:%.+]] = sitofp i32 %{{.+}} to float
; CHECK: store float [[FP2]]{{.*}}, !llvm.access.group ![[AG2:[0-9]+]]
; CHECK: br i1 %{{.+}}, label %{{.+}}, label %{{.+}}, !llvm.loop ![[ID2:[0-9]+]]
;
; void test3(float *P) {
; #pragma omp for schedule(dynamic)
;   for (int I = 0 ; I < 100; I++)
;     P[I] = I;
; }
;
; CHECK-LABEL: define void @test3
; CHECK: call i32 @__kmpc_dispatch_next_4(
; CHECK: [[FP3:%.+]] = sitofp i32 %{{.+}} to float
; CHECK: store float [[FP3]]{{.*}}, !llvm.access.group ![[AG3:[0-9]+]]
; CHECK: br i1 %{{.+}}, label %{{.+}}, label %{{.+}}, !llvm.loop ![[ID3:[0-9]+]]
;
; void test4(float *P) {
; #pragma omp for schedule(guided)
;   for (int I = 0 ; I < 100; I++)
;     P[I] = I;
; }
;
; CHECK-LABEL: define void @test4
; CHECK: call i32 @__kmpc_dispatch_next_4(
; CHECK: [[FP4:%.+]] = sitofp i32 %{{.+}} to float
; CHECK: store float [[FP4]]{{.*}}, !llvm.access.group ![[AG4:[0-9]+]]
; CHECK: br i1 %{{.+}}, label %{{.+}}, label %{{.+}}, !llvm.loop ![[ID4:[0-9]+]]
;
; void test5(float *P) {
; #pragma omp for schedule(nonmonotonic: auto)
;   for (int I = 0 ; I < 100; I++)
;     P[I] = I;
; }
;
; CHECK-LABEL: define void @test5
; CHECK: call i32 @__kmpc_dispatch_next_4(
; CHECK: [[FP5:%.+]] = sitofp i32 %{{.+}} to float
; CHECK: store float [[FP5]]{{.*}}, !llvm.access.group ![[AG5:[0-9]+]]
; CHECK: br i1 %{{.+}}, label %{{.+}}, label %{{.+}}, !llvm.loop ![[ID5:[0-9]+]]
;
; void test6(float *P) {
; #pragma omp simd
;   for (int I = 0 ; I < 100; I++)
;     P[I] = I;
; }
;
; CHECK-LABEL: define void @test6
; CHECK: [[T6:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
; CHECK: [[FP6:%.+]] = sitofp i32 %{{.+}} to float
; CHECK: store float [[FP6]]{{.*}}, !llvm.access.group ![[AG6:[0-9]+]]
; CHECK: br i1 %{{.+}}, label %{{.+}}, label %{{.+}}, !llvm.loop ![[ID6:[0-9]+]]
; CHECK: call void @llvm.directive.region.exit(token [[T6]]) [ "DIR.OMP.END.SIMD"() ]
;
; INTEL_CUSTOMIZATION
; The section below checks the metadata attached to the loops. The combination
; of CHECK and CHECKPA prefixes check the parallel access metadata, while the
; CHECK and CHECKIV prefixes check for the ivdep loop marking.
; end INTEL_CUSTOMIZATION
;
; CHECK:     ![[AG1]] = distinct !{}
; CHECKPA-DAG: ![[PA1:[0-9]+]] = !{!"llvm.loop.parallel_accesses", ![[AG1]]}
; CHECKPA-DAG: ![[ID1]] = distinct !{![[ID1]]{{.*}}, ![[PA1]]{{[,\}]}}
;
; INTEL_CUSTOMIZATION
; CHECKIV-DAG: ![[IV1:[0-9]+]] = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
; CHECKIV-DAG: ![[ID1]] = distinct !{![[ID1]]{{.*}}, ![[IV1]]{{[,\}]}}
; end INTEL_CUSTOMIZATION
;
; CHECK:     ![[AG2]] = distinct !{}
; CHECKPA-DAG: ![[PA2:[0-9]+]] = !{!"llvm.loop.parallel_accesses", ![[AG2]]}
; CHECKPA-DAG: ![[ID2]] = distinct !{![[ID2]]{{.*}}, ![[PA2]]{{[,\}]}}
;
; INTEL_CUSTOMIZATION
; CHECKIV-DAG: ![[ID2]] = distinct !{![[ID2]]{{.*}}, ![[IV1]]{{[,\}]}}
; end INTEL_CUSTOMIZATION
;
; CHECK:     ![[AG3]] = distinct !{}
; CHECKPA-DAG: ![[PA3:[0-9]+]] = !{!"llvm.loop.parallel_accesses", ![[AG3]]}
; CHECKPA-DAG: ![[ID3]] = distinct !{![[ID3]]{{.*}}, ![[PA3]]{{[,\}]}}
;
; INTEL_CUSTOMIZATION
; CHECKIV-DAG: ![[ID3]] = distinct !{![[ID3]]{{.*}}, ![[IV1]]{{[,\}]}}
; end INTEL_CUSTOMIZATION
;
; CHECK:     ![[AG4]] = distinct !{}
; CHECKPA-DAG: ![[PA4:[0-9]+]] = !{!"llvm.loop.parallel_accesses", ![[AG4]]}
; CHECKPA-DAG: ![[ID4]] = distinct !{![[ID4]]{{.*}}, ![[PA4]]{{[,\}]}}
;
; INTEL_CUSTOMIZATION
; CHECKIV-DAG: ![[ID4]] = distinct !{![[ID4]]{{.*}}, ![[IV1]]{{[,\}]}}
; end INTEL_CUSTOMIZATION
;
; CHECK:     ![[AG5]] = distinct !{}
; CHECKPA-DAG: ![[PA5:[0-9]+]] = !{!"llvm.loop.parallel_accesses", ![[AG5]]}
; CHECKPA-DAG: ![[ID5]] = distinct !{![[ID5]]{{.*}}, ![[PA5]]{{[,\}]}}
;
; INTEL_CUSTOMIZATION
; CHECKIV-DAG: ![[ID5]] = distinct !{![[ID5]]{{.*}}, ![[IV1]]{{[,\}]}}
; end INTEL_CUSTOMIZATION
;
; CHECK:     ![[AG6]] = distinct !{}
; CHECKPA-DAG: ![[PA6:[0-9]+]] = !{!"llvm.loop.parallel_accesses", ![[AG6]]}
; CHECKPA-DAG: ![[ID6]] = distinct !{![[ID6]]{{.*}}, ![[PA6]]{{[,\}]}}
;
; INTEL_CUSTOMIZATION
; CHECKIV-DAG: ![[ID6]] = distinct !{![[ID6]]{{.*}}, ![[IV1]]{{[,\}]}}
; end INTEL_CUSTOMIZATION
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test1(ptr noundef %P) {
entry:
  %P.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

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
  store i32 %add, ptr %I, align 4
  %5 = load i32, ptr %I, align 4
  %conv = sitofp i32 %5 to float
  %6 = load ptr, ptr %P.addr, align 8
  %7 = load i32, ptr %I, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds float, ptr %6, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define void @test2(ptr noundef %P) {
entry:
  %P.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.SCHEDULE.STATIC:NONMONOTONIC"(i32 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

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
  store i32 %add, ptr %I, align 4
  %5 = load i32, ptr %I, align 4
  %conv = sitofp i32 %5 to float
  %6 = load ptr, ptr %P.addr, align 8
  %7 = load i32, ptr %I, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds float, ptr %6, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]

  ret void
}

define void @test3(ptr noundef %P) {
entry:
  %P.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.SCHEDULE.DYNAMIC"(i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

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
  store i32 %add, ptr %I, align 4
  %5 = load i32, ptr %I, align 4
  %conv = sitofp i32 %5 to float
  %6 = load ptr, ptr %P.addr, align 8
  %7 = load i32, ptr %I, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds float, ptr %6, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]

  ret void
}

define void @test4(ptr noundef %P) {
entry:
  %P.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.SCHEDULE.GUIDED"(i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

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
  store i32 %add, ptr %I, align 4
  %5 = load i32, ptr %I, align 4
  %conv = sitofp i32 %5 to float
  %6 = load ptr, ptr %P.addr, align 8
  %7 = load i32, ptr %I, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds float, ptr %6, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]

  ret void
}

define void @test5(ptr noundef %P) {
entry:
  %P.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.SCHEDULE.AUTO:NONMONOTONIC"(i32 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

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
  store i32 %add, ptr %I, align 4
  %5 = load i32, ptr %I, align 4
  %conv = sitofp i32 %5 to float
  %6 = load ptr, ptr %P.addr, align 8
  %7 = load i32, ptr %I, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds float, ptr %6, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]

  ret void
}

define void @test6(ptr noundef %P) {
entry:
  %P.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %I, i32 0, i32 1, i32 1) ]

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %1 = load i32, ptr %.omp.iv, align 4
  %2 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %3 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %3, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %I, align 4
  %4 = load i32, ptr %I, align 4
  %conv = sitofp i32 %4 to float
  %5 = load ptr, ptr %P.addr, align 8
  %6 = load i32, ptr %I, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds float, ptr %5, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond, !llvm.loop !5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  ret void
}

!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.vectorize.enable", i1 true}

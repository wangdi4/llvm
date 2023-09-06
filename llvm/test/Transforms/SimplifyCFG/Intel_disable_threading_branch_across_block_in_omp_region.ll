; INTEL_CUSTOMIZATION
; RUN: opt -passes="simplifycfg" -S %s | FileCheck %s -check-prefix=OUTPUT

; Test Src:
; void enclosed(int *a, const int thread, const int threads, const int j,
;               const int start_v, const int end_v, const int step,
;               omp_lock_t *plck0, omp_lock_t *plck1) {
;   int i, l = 0;
; #pragma omp for schedule(monotonic : dynamic)
;   for (i = start_v; i < end_v; i += step) {
;     if (l == 0) {
;       l = 1;
;       if (thread == j) {
;         omp_unset_lock(plck0);
;         omp_set_lock(plck1);
;         omp_unset_lock(plck1);
;       } else if (thread == (j + 1) % threads) {
;         omp_unset_lock(plck0);
;         omp_unset_lock(plck1);
;       }
;     }
;     a[(i - start_v) / step] += (1 + thread);
;   }
; }

; Check that simplifycfg pass disables threading a branch across the block 
; when an enclosing OpenMP begin directive is found. The test IR is 
; obtained from ompoC/for10ab test after JumpThreadingPass is done on 
; 'enclosed' function. 

; Check that no other branches/blocks get inserted between the region entry call and the for.body

; OUTPUT: call{{.*}}llvm.directive.region.entry
; OUTPUT: [[DIR:DIR.OMP.LOOP.[0-9]+]]:
; OUTPUT: br i1 {{.*}} %[[INNER_FOR_BODY:omp.inner.for.body]]
; OUTPUT: [[INNER_FOR_BODY]]:
; OUTPUT: %cmp6 = phi i1 [ true, %[[DIR]] ], [ false, [[IF_END:[^,]+]] ]
; OUTPUT: br i1 [[CMP:[^,]+]], label [[IF_THEN:%[^,]+]], label [[IF_END]]
; OUTPUT-NOT: [[IF_THEN]].critedge

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @enclosed(ptr noundef %a, i32 noundef %thread, i32 noundef %threads, i32 noundef %j, i32 noundef %start_v, i32 noundef %end_v, i32 noundef %step, ptr noundef %plck0, ptr noundef %plck1) local_unnamed_addr #0 {
entry:
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i)
  %cmp = icmp slt i32 %start_v, %end_v
  %i.addr = alloca ptr, align 8
  %.omp.lb.addr = alloca ptr, align 8
  br i1 %cmp, label %DIR.OMP.LOOP.237, label %omp.precond.end

DIR.OMP.LOOP.237:                                 ; preds = %entry
  %0 = xor i32 %start_v, -1
  %sub1 = add i32 %0, %end_v
  %add = add i32 %sub1, %step
  %div = udiv i32 %add, %step
  %sub2 = add i32 %div, -1
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.iv)
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.lb)
  store i32 0, ptr %.omp.lb, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.ub)
  store volatile i32 %sub2, ptr %.omp.ub, align 4
  store ptr %i, ptr %i.addr, align 8
  store ptr %.omp.lb, ptr %.omp.lb.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.SCHEDULE.DYNAMIC:MONOTONIC"(i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.OPERAND.ADDR"(ptr %i, ptr %i.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb, ptr %.omp.lb.addr),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.LOOP.3, label %DIR.OMP.LOOP.2

DIR.OMP.LOOP.2:                                   ; preds = %DIR.OMP.LOOP.237
  %i21 = load volatile ptr, ptr %i.addr, align 8
  %.omp.lb22 = load volatile ptr, ptr %.omp.lb.addr, align 8
  %2 = load i32, ptr %.omp.lb22, align 4
  store volatile i32 %2, ptr %.omp.iv, align 4
  %3 = load volatile i32, ptr %.omp.iv, align 4
  %4 = load volatile i32, ptr %.omp.ub, align 4
  %add3 = add i32 %4, 1
  %cmp4 = icmp ult i32 %3, %add3
  br i1 %cmp4, label %omp.inner.for.body, label %DIR.OMP.END.LOOP.3

omp.inner.for.body:                               ; preds = %if.end13, %DIR.OMP.LOOP.2
  %cmp6 = phi i1 [ true, %DIR.OMP.LOOP.2 ], [ false, %if.end13 ]
  %5 = load volatile i32, ptr %.omp.iv, align 4
  %mul = mul i32 %5, %step
  %add5 = add i32 %mul, %start_v
  store i32 %add5, ptr %i21, align 4
  br i1 %cmp6, label %if.then, label %if.end13

if.then:                                          ; preds = %omp.inner.for.body
  %cmp7 = icmp eq i32 %thread, %j
  br i1 %cmp7, label %if.then8, label %if.else

if.then8:                                         ; preds = %if.then
  call void @omp_unset_lock(ptr noundef %plck0)
  call void @omp_set_lock(ptr noundef %plck1)
  call void @omp_unset_lock(ptr noundef %plck1)
  br label %if.end13

if.else:                                          ; preds = %if.then
  %add9 = add nsw i32 %j, 1
  %rem = srem i32 %add9, %threads
  %cmp10 = icmp eq i32 %rem, %thread
  br i1 %cmp10, label %if.then11, label %if.end13

if.then11:                                        ; preds = %if.else
  call void @omp_unset_lock(ptr noundef %plck0)
  call void @omp_unset_lock(ptr noundef %plck1)
  br label %if.end13

if.end13:                                         ; preds = %if.then11, %if.else, %if.then8, %omp.inner.for.body
  %add14 = add nsw i32 %thread, 1
  %6 = load i32, ptr %i21, align 4
  %sub15 = sub nsw i32 %6, %start_v
  %div16 = sdiv i32 %sub15, %step
  %idxprom = sext i32 %div16 to i64
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %idxprom
  %7 = load i32, ptr %arrayidx, align 4
  %add17 = add nsw i32 %7, %add14
  store i32 %add17, ptr %arrayidx, align 4
  %8 = load volatile i32, ptr %.omp.iv, align 4
  %add18 = add nuw i32 %8, 1
  store volatile i32 %add18, ptr %.omp.iv, align 4
  %9 = load volatile i32, ptr %.omp.iv, align 4
  %10 = load volatile i32, ptr %.omp.ub, align 4
  %add19 = add i32 %10, 1
  %cmp20 = icmp ult i32 %9, %add19
  br i1 %cmp20, label %omp.inner.for.body, label %DIR.OMP.END.LOOP.3

DIR.OMP.END.LOOP.3:                               ; preds = %if.end13, %DIR.OMP.LOOP.2, %DIR.OMP.LOOP.237
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.LOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.LOOP.3, %entry
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.ub)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.lb)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.iv)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i)
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @omp_set_lock(ptr noundef)
declare dso_local void @omp_unset_lock(ptr noundef)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

attributes #0 = { "may-have-openmp-directive"="true" }

; end INTEL_CUSTOMIZATION

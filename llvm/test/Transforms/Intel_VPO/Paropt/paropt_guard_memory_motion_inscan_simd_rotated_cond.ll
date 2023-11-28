; RUN: opt -passes='function(vpo-paropt-guard-memory-motion)' -vpo-paropt-guard-memory-motion-for-scan %s -S 2>&1 | FileCheck %s
;
; Test to check that memory guarding directives are placed correctly
; in case a rotated loop has a conditional branch in the loop header.
;
; Source test:
;
;int run_vector(int *m_in, int* __restrict m_out, int c_size){
;  int red = 0;
;  #pragma omp simd reduction(inscan, +: red)
;  for (int i = 0; i < c_size; ++i) {
;      if (i & 10) {
;          m_out[i] = red;
;      }
;      #pragma omp scan exclusive(red)
;      if (i & 10) {
;          red += m_in[i];
;      }
;  }
;  return red;
;}
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; CHECK: omp.inner.for.body:
; CHECK-NEXT:  %pre.scan.guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %red) ]
; CHECK: DIR.OMP.SCAN.3:                                   ; preds = %if.end
; CHECK-NEXT:  call void @llvm.directive.region.exit(token %pre.scan.guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
; CHECK: DIR.OMP.END.SCAN.5:                               ; preds = %DIR.OMP.SCAN.4
; CHECK-NEXT:  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.SCAN"() ]
; CHECK-NEXT:  %post.scan.guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %red) ]
; CHECK: omp.body.continue:                                ; preds = %if.end11
; CHECK-NEXT:  call void @llvm.directive.region.exit(token %post.scan.guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z10run_vectorPiS_i(ptr noundef %m_in, ptr noalias noundef %m_out, i32 noundef %c_size) {
entry:
  %m_in.addr = alloca ptr, align 8
  %m_out.addr = alloca ptr, align 8
  %c_size.addr = alloca i32, align 4
  %red = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %m_in, ptr %m_in.addr, align 8
  store ptr %m_out, ptr %m_out.addr, align 8
  store i32 %c_size, ptr %c_size.addr, align 4
  store i32 0, ptr %red, align 4
  %0 = load i32, ptr %c_size.addr, align 4
  store i32 %0, ptr %.capture_expr.0, align 4
  %1 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4
  %2 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %3 = load i32, ptr %.capture_expr.1, align 4
  store i32 %3, ptr %.omp.ub, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.precond.then
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:INSCAN.TYPED"(ptr %red, i32 0, i32 1, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  store i32 0, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.iv, align 4
  %6 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %5, %6
  br i1 %cmp3, label %omp.inner.for.body.lh, label %omp.inner.for.end

omp.inner.for.body.lh:                            ; preds = %DIR.OMP.SIMD.2
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %omp.inner.for.body.lh
  %7 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %i, align 4
  %8 = load i32, ptr %i, align 4
  %and = and i32 %8, 10
  %tobool = icmp ne i32 %and, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %omp.inner.for.body
  %9 = load i32, ptr %red, align 4
  %10 = load ptr, ptr %m_out.addr, align 8
  %11 = load i32, ptr %i, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds i32, ptr %10, i64 %idxprom
  store i32 %9, ptr %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %omp.inner.for.body
  br label %DIR.OMP.SCAN.3

DIR.OMP.SCAN.3:                                   ; preds = %if.end
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.EXCLUSIVE"(ptr %red, i64 1) ]
  br label %DIR.OMP.SCAN.4

DIR.OMP.SCAN.4:                                   ; preds = %DIR.OMP.SCAN.3
  fence acq_rel
  br label %DIR.OMP.END.SCAN.5

DIR.OMP.END.SCAN.5:                               ; preds = %DIR.OMP.SCAN.4
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.6

DIR.OMP.END.SCAN.6:                               ; preds = %DIR.OMP.END.SCAN.5
  %13 = load i32, ptr %i, align 4
  %and5 = and i32 %13, 10
  %tobool6 = icmp ne i32 %and5, 0
  br i1 %tobool6, label %if.then7, label %if.end11

if.then7:                                         ; preds = %DIR.OMP.END.SCAN.6
  %14 = load ptr, ptr %m_in.addr, align 8
  %15 = load i32, ptr %i, align 4
  %idxprom8 = sext i32 %15 to i64
  %arrayidx9 = getelementptr inbounds i32, ptr %14, i64 %idxprom8
  %16 = load i32, ptr %arrayidx9, align 4
  %17 = load i32, ptr %red, align 4
  %add10 = add nsw i32 %17, %16
  store i32 %add10, ptr %red, align 4
  br label %if.end11

if.end11:                                         ; preds = %if.then7, %DIR.OMP.END.SCAN.6
  br label %omp.body.continue

omp.body.continue:                                ; preds = %if.end11
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %18 = load i32, ptr %.omp.iv, align 4
  %add12 = add nsw i32 %18, 1
  store i32 %add12, ptr %.omp.iv, align 4
  %19 = load i32, ptr %.omp.iv, align 4
  %20 = load i32, ptr %.omp.ub, align 4
  %cmp13 = icmp sle i32 %19, %20
  br i1 %cmp13, label %omp.inner.for.body, label %omp.inner.for.end_crit_edge

omp.inner.for.end_crit_edge:                      ; preds = %omp.inner.for.inc
  br label %omp.inner.for.end

omp.inner.for.end:                                ; preds = %omp.inner.for.end_crit_edge, %DIR.OMP.SIMD.2
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %21 = load i32, ptr %red, align 4
  ret i32 %21
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)


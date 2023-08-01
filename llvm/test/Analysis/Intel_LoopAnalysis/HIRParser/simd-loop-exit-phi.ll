; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that we are able to handle deconstruction of loop exit phi %spec.select.lcssa properly.

; CHECK: %t4 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]

; CHECK: (%.omp.iv)[0] = 0;
; CHECK: %index.017 = -1;

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   %1 = (%a)[i1];
; CHECK: |   %index.017 = (%1 == %val) ? i1 : %index.017;
; CHECK: + END LOOP

; CHECK: (%.omp.iv)[0] = %n;

; CHECK: @llvm.directive.region.exit(%t4); [ DIR.OMP.END.SIMD() ]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly uwtable
define dso_local i32 @foo(i32 %n, ptr nocapture readonly %a, i32 %val) local_unnamed_addr {
entry:
  %.omp.iv = alloca i32, align 4
  %0 = bitcast ptr %.omp.iv to ptr
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %t4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  store i32 0, ptr %.omp.iv, align 4
  %wide.trip.count = sext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.precond.then
  %indvars.iv = phi i64 [ 0, %omp.precond.then ], [ %indvars.iv.next, %omp.inner.for.body ]
  %index.017 = phi i32 [ -1, %omp.precond.then ], [ %spec.select, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %cmp7 = icmp eq i32 %1, %val
  %2 = trunc i64 %indvars.iv to i32
  %spec.select = select i1 %cmp7, i32 %2, i32 %index.017
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  %spec.select.lcssa = phi i32 [ %spec.select, %omp.inner.for.body ]
  store i32 %n, ptr %.omp.iv, align 4
  call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %index.2 = phi i32 [ %spec.select.lcssa, %omp.loop.exit ], [ -1, %entry ]
  ret i32 %index.2
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)


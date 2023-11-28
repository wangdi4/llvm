; RUN: opt -passes='function(vpo-paropt-guard-memory-motion,vpo-cfg-restructuring,vpo-rename-operands)' -S %s -o %t1.ll && FileCheck --input-file=%t1.ll %s
; RUN: opt -passes='function(vpo-restore-operands)' -S %t1.ll -o %t2.ll && FileCheck --input-file=%t2.ll %s --check-prefix=RESTORE

; Test to verify functionality of VPOParoptGuardMemoryMotion and VPORenameOperands
; passes on SIMD loop containing array section reduction idiom.

; C/C++ source
;
; int foo(int b[1000][1000])
; {
;   int a[1000];
;   int i,j;
; #pragma omp simd reduction(+:a[42:500]) private(j)
;   for (i=0; i<1000; i++) {
;     for (j=42; j<542; j++) {
;       a[j] += b[i][j];
;     }
;   }
;
;   return a[42];
; }

; CHECK: @foo
; CHECK:   [[GUARD_START:%.*]] = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %a.red.gep.minus.offset), "QUAL.OMP.OPERAND.ADDR"(ptr %a.red.gep.minus.offset, ptr %a.red.gep.minus.offset.addr) ]

; RESTORE: @foo
; RESTORE:   [[GUARD_START:%.*]] = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %a.red.gep.minus.offset) ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(ptr %b) #0 {
entry:
  %a = alloca [1000 x i32], align 16
  %a.red = alloca [500 x i32], align 16
  %a.red.gep.minus.offset = getelementptr [500 x i32], [500 x i32]* %a.red, i64 0, i64 -42
  call void @llvm.memset.p0i8.i64(ptr noundef nonnull align 16 dereferenceable(2000) %a.red, i8 0, i64 2000, i1 false)
  call void @llvm.memset.p0i8.i64(ptr noundef nonnull align 16 dereferenceable(4000) %a, i8 0, i64 4000, i1 false)
  br label %DIR.OMP.SIMD.170

DIR.OMP.SIMD.170:                                 ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %a.red.gep.minus.offset, i64 0, i64 500, i64 42) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.170, %omp.inner.for.inc
  %indvars.iv59 = phi i64 [ 0, %DIR.OMP.SIMD.170 ], [ %indvars.iv.next60, %omp.inner.for.inc ]
  br label %for.body15

for.body15:                                       ; preds = %omp.inner.for.body, %for.body15
  %indvars.iv = phi i64 [ 42, %omp.inner.for.body ], [ %indvars.iv.next, %for.body15 ]
  %arrayidx19 = getelementptr inbounds [1000 x [1000 x i32]], ptr %b, i64 0, i64 %indvars.iv59, i64 %indvars.iv
  %b.ld = load i32, ptr %arrayidx19, align 4
  %arrayidx21 = getelementptr inbounds [1000 x i32], ptr %a.red.gep.minus.offset, i64 0, i64 %indvars.iv
  %a.ld = load i32, ptr %arrayidx21, align 4
  %add22 = add nsw i32 %a.ld, %b.ld
  store i32 %add22, ptr %arrayidx21, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 542
  br i1 %exitcond.not, label %omp.inner.for.inc, label %for.body15

omp.inner.for.inc:                                ; preds = %for.body15
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond61.not = icmp eq i64 %indvars.iv.next60, 100
  br i1 %exitcond61.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  br label %DIR.OMP.END.SIMD.271

DIR.OMP.END.SIMD.271:                             ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.271
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.memset.p0i8.i64(ptr nocapture writeonly, i8, i64, i1 immarg)

; RUN: opt -passes="function(vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt" -S --vpo-utils-add-typed-privates <%s 2>&1 | FileCheck %s --check-prefixes=CHECK,TYPED

; Test src:
;
; void bar(int *);
; static int foo(int i) {
;   int Array[2050];
;   bar(Array);
;   return Array[i];
; }
;
; void test() {
;   int Out[1024];
;   #pragma omp parallel for simd
;   for (int i = 0; i < 1024; ++i) {
;     Out[i] = foo(i);
;   }
;   bar(Out);
; }

; CHECK: define{{.*}}split
; CHECK: %Array.i = alloca [2050 x i32], align 16
; TYPED: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.priv, i32 0, i32 1, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %i.addr.i, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %Array.i, i32 0, i64 2050) ]
; CHECK-NOT: %Array.i = alloca [2050 x i32], align 16

; After inlining, the alloca for Array is placed inside the SIMD loop. It
; needs to be hoisted out of the loop, and marked private in the SIMD
; directive.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"@tid.addr" = external global i32

; Function Attrs: mustprogress uwtable
define dso_local void @_Z4testv() #0 {
entry:
  %Out = alloca [1024 x i32], align 16
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4096, ptr %Out) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  store volatile i32 1023, ptr %.omp.ub, align 4, !tbaa !3
  %array.begin = getelementptr inbounds [1024 x i32], ptr %Out, i32 0, i32 0
  %i.addr = alloca ptr, align 8
  %i.addr5 = alloca ptr, align 8
  %.omp.lb.addr = alloca ptr, align 8
  %Out.addr = alloca ptr, align 8
  store ptr %i, ptr %i.addr5, align 8
  store ptr %.omp.lb, ptr %.omp.lb.addr, align 8
  store ptr %Out, ptr %Out.addr, align 8
  %end.dir.temp10 = alloca i1, align 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %Out, i32 0, i64 1024),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.OPERAND.ADDR"(ptr %i, ptr %i.addr5),
    "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb, ptr %.omp.lb.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr %Out, ptr %Out.addr),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp10) ]
  %temp.load11 = load volatile i1, ptr %end.dir.temp10, align 1
  %cmp12 = icmp ne i1 %temp.load11, false
  br i1 %cmp12, label %DIR.OMP.END.SIMD.5.split, label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %entry
  %i6 = load volatile ptr, ptr %i.addr5, align 8
  %.omp.lb7 = load volatile ptr, ptr %.omp.lb.addr, align 8
  %Out8 = load volatile ptr, ptr %Out.addr, align 8
  store ptr %i6, ptr %i.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i6, i32 0, i32 1, i32 1),
    "QUAL.OMP.OPERAND.ADDR"(ptr %i6, ptr %i.addr),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  %cmp9 = icmp ne i1 %temp.load, false
  br i1 %cmp9, label %omp.loop.exit.split, label %DIR.OMP.SIMD.4

DIR.OMP.SIMD.4:                                   ; preds = %DIR.OMP.PARALLEL.LOOP.3
  %i4 = load volatile ptr, ptr %i.addr, align 8
  %2 = load i32, ptr %.omp.lb7, align 4, !tbaa !3
  store volatile i32 %2, ptr %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.4
  %3 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !3
  %4 = load volatile i32, ptr %.omp.ub, align 4, !tbaa !3
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit.split

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i4) #2
  %5 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !3
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i4, align 4, !tbaa !3
  %6 = load i32, ptr %i4, align 4, !tbaa !3
  %savedstack = call ptr @llvm.stacksave()
  %i.addr.i = alloca i32, align 4
  %Array.i = alloca [2050 x i32], align 16
  store i32 %6, ptr %i.addr.i, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 8200, ptr %Array.i) #2
  %arraydecay.i = getelementptr inbounds [2050 x i32], ptr %Array.i, i64 0, i64 0
  call void @_Z3barPi(ptr noundef %arraydecay.i) #2
  %7 = load i32, ptr %i.addr.i, align 4, !tbaa !3
  %idxprom.i = sext i32 %7 to i64
  %arrayidx.i = getelementptr inbounds [2050 x i32], ptr %Array.i, i64 0, i64 %idxprom.i, !intel-tbaa !7
  %8 = load i32, ptr %arrayidx.i, align 4, !tbaa !7
  call void @llvm.lifetime.end.p0(i64 8200, ptr %Array.i) #2
  call void @llvm.stackrestore(ptr %savedstack)
  %9 = load i32, ptr %i4, align 4, !tbaa !3
  %idxprom = sext i32 %9 to i64
  %arrayidx = getelementptr inbounds [1024 x i32], ptr %Out8, i64 0, i64 %idxprom, !intel-tbaa !9
  store i32 %8, ptr %arrayidx, align 4, !tbaa !9
  call void @llvm.lifetime.end.p0(i64 4, ptr %i4) #2
  %10 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !3
  %add1 = add nsw i32 %10, 1
  store volatile i32 %add1, ptr %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond, !llvm.loop !11

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond, %DIR.OMP.PARALLEL.LOOP.3
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.5.split

DIR.OMP.END.SIMD.5.split:                         ; preds = %omp.loop.exit.split, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
  %arraydecay = getelementptr inbounds [1024 x i32], ptr %Out, i64 0, i64 0
  call void @_Z3barPi(ptr noundef %arraydecay)
  call void @llvm.lifetime.end.p0(i64 4096, ptr %Out) #2
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare dso_local void @_Z3barPi(ptr noundef) #3

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #4

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #4

attributes #0 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { nocallback nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA2050_i", !4, i64 0}
!9 = !{!10, !4, i64 0}
!10 = !{!"array@_ZTSA1024_i", !4, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.vectorize.enable", i1 true}

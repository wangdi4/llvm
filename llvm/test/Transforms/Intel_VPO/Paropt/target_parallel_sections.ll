; RUN: opt -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
;
; It does a verification whether the "omp target parallel sections" construct is supported in the Paropt codegen for offloading.
;
; void test_square (const int n, double *d) {
;   #pragma omp target map(tofrom: d[0:n*n])
;   {
;     #pragma omp parallel sections
;     {
;       #pragma omp section
;       {
;         d[0] = d[0] + 1.0;
;       }
;       #pragma omp section
;       {
;         d[1] = d[1] + 1.0;
;       }
;       #pragma omp section
;       {
;         d[2] = d[2] + 1.0;
;       }
;       #pragma omp section
;       {
;         d[3] = d[3] + 1.0;
;       }
;     }
;   }
; }
;
; CHECK:  call i32 @__tgt_target({{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: nounwind uwtable
define dso_local void @test_square(i32 %n, double* %d) #0 {
entry:
  %n.addr = alloca i32, align 4
  %d.addr = alloca double*, align 8
  %d.map.ptr.tmp = alloca double*, align 8
  store i32 %n, i32* %n.addr, align 4, !tbaa !4
  store double* %d, double** %d.addr, align 8, !tbaa !8
  %0 = load double*, double** %d.addr, align 8, !tbaa !8
  %1 = load double*, double** %d.addr, align 8
  %2 = load double*, double** %d.addr, align 8, !tbaa !8
  %arrayidx = getelementptr inbounds double, double* %2, i64 0
  %3 = load i32, i32* %n.addr, align 4, !tbaa !4
  %4 = load i32, i32* %n.addr, align 4, !tbaa !4
  %mul = mul nsw i32 %3, %4
  %conv = sext i32 %mul to i64
  %5 = mul nuw i64 %conv, 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(double* %1, double* %arrayidx, i64 %5, i64 35, i8* null, i8* null), "QUAL.OMP.PRIVATE"(double** %d.map.ptr.tmp) ]
  store double* %1, double** %d.map.ptr.tmp, align 8
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.SECTIONS"(), "QUAL.OMP.SHARED"(double** %d.map.ptr.tmp) ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %9 = load double*, double** %d.map.ptr.tmp, align 8, !tbaa !8
  %arrayidx1 = getelementptr inbounds double, double* %9, i64 0
  %10 = load double, double* %arrayidx1, align 8, !tbaa !10
  %add = fadd fast double %10, 1.000000e+00
  %11 = load double*, double** %d.map.ptr.tmp, align 8, !tbaa !8
  %arrayidx2 = getelementptr inbounds double, double* %11, i64 0
  store double %add, double* %arrayidx2, align 8, !tbaa !10
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.SECTION"() ]
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %13 = load double*, double** %d.map.ptr.tmp, align 8, !tbaa !8
  %arrayidx3 = getelementptr inbounds double, double* %13, i64 1
  %14 = load double, double* %arrayidx3, align 8, !tbaa !10
  %add4 = fadd fast double %14, 1.000000e+00
  %15 = load double*, double** %d.map.ptr.tmp, align 8, !tbaa !8
  %arrayidx5 = getelementptr inbounds double, double* %15, i64 1
  store double %add4, double* %arrayidx5, align 8, !tbaa !10
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.SECTION"() ]
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %17 = load double*, double** %d.map.ptr.tmp, align 8, !tbaa !8
  %arrayidx6 = getelementptr inbounds double, double* %17, i64 2
  %18 = load double, double* %arrayidx6, align 8, !tbaa !10
  %add7 = fadd fast double %18, 1.000000e+00
  %19 = load double*, double** %d.map.ptr.tmp, align 8, !tbaa !8
  %arrayidx8 = getelementptr inbounds double, double* %19, i64 2
  store double %add7, double* %arrayidx8, align 8, !tbaa !10
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.SECTION"() ]
  %20 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %21 = load double*, double** %d.map.ptr.tmp, align 8, !tbaa !8
  %arrayidx9 = getelementptr inbounds double, double* %21, i64 3
  %22 = load double, double* %arrayidx9, align 8, !tbaa !10
  %add10 = fadd fast double %22, 1.000000e+00
  %23 = load double*, double** %d.map.ptr.tmp, align 8, !tbaa !8
  %arrayidx11 = getelementptr inbounds double, double* %23, i64 3
  store double %add10, double* %arrayidx11, align 8, !tbaa !10
  call void @llvm.directive.region.exit(token %20) [ "DIR.OMP.END.SECTION"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.SECTIONS"() ]
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind uwtable
define internal void @.omp_offloading.requires_reg() #2 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 66313, i32 12590452, !"_Z11test_square", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 12.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSPd", !6, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"double", !6, i64 0}

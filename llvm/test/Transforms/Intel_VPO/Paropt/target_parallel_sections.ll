; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,simplify-cfg,loop(simplify-cfg),sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S | FileCheck %s
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

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]
@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local void @_Z11test_squareiPd(i32 %n, double* %d) #0 {
entry:
  %d.addr = alloca double*, align 8
  store double* %d, double** %d.addr, align 8, !tbaa !3
  %mul = mul nsw i32 %n, %n
  %0 = zext i32 %mul to i64
  %1 = mul nuw i64 %0, 8
  %num.sects = alloca i32, align 4
  store i32 3, i32* %num.sects, align 4
  %d.addr.addr = alloca double**
  %d.addr.addr2 = alloca double**
  %arrayidx.addr = alloca double*
  %num.sects.addr = alloca i32*
  store double** %d.addr, double*** %d.addr.addr2
  store double* %d, double** %arrayidx.addr
  store i32* %num.sects, i32** %num.sects.addr
  %end.dir.temp5 = alloca i1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(double** %d.addr, double** %d.addr, i64 8), "QUAL.OMP.MAP.TOFROM:AGGR"(double** %d.addr, double* %d, i64 %1), "QUAL.OMP.MAP.TO"(i32* %num.sects), "QUAL.OMP.OPERAND.ADDR"(double** %d.addr, double*** %d.addr.addr2), "QUAL.OMP.OPERAND.ADDR"(double* %d, double** %arrayidx.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %num.sects, i32** %num.sects.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp5) ]
  %temp.load6 = load volatile i1, i1* %end.dir.temp5
  %.sloop.iv.1 = alloca i32
  store i32 0, i32* %.sloop.iv.1
  br i1 %temp.load6, label %DIR.OMP.END.PARALLEL.SECTIONS.18.split, label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %entry
  %d.addr3 = load double**, double*** %d.addr.addr2
  %num.sects4 = load i32*, i32** %num.sects.addr
  store double** %d.addr3, double*** %d.addr.addr
  %end.dir.temp = alloca i1
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.SECTIONS"(), "QUAL.OMP.SHARED"(double** %d.addr3), "QUAL.OMP.NORMALIZED.UB"(i32* %num.sects4), "QUAL.OMP.NORMALIZED.IV"(i32* %.sloop.iv.1), "QUAL.OMP.OPERAND.ADDR"(double** %d.addr3, double*** %d.addr.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  %temp.load = load volatile i1, i1* %end.dir.temp
  br i1 %temp.load, label %DIR.OMP.END.SECTION.17.split, label %.sloop.preheader.1

.sloop.preheader.1:                               ; preds = %DIR.OMP.TARGET.3
  %d.addr1 = load double**, double*** %d.addr.addr
  %sloop.ub = load i32, i32* %num.sects4
  br label %.sloop.header.1

.sloop.header.1:                                  ; preds = %_Z11test_squareiPd.sw.epilog.1, %.sloop.preheader.1
  %4 = load volatile i32, i32* %.sloop.iv.1
  switch i32 %4, label %DIR.OMP.SECTION.6 [
    i32 1, label %DIR.OMP.SECTION.9
    i32 2, label %DIR.OMP.SECTION.12
    i32 3, label %DIR.OMP.SECTION.15
  ]

DIR.OMP.SECTION.6:                                ; preds = %.sloop.header.1
  %5 = load double*, double** %d.addr1, align 8, !tbaa !3
  %6 = load double, double* %5, align 8, !tbaa !7
  %add = fadd double %6, 1.000000e+00
  store double %add, double* %5, align 8, !tbaa !7
  br label %_Z11test_squareiPd.sw.epilog.1

DIR.OMP.SECTION.9:                                ; preds = %.sloop.header.1
  %7 = load double*, double** %d.addr1, align 8, !tbaa !3
  %arrayidx3 = getelementptr inbounds double, double* %7, i64 1
  %8 = load double, double* %arrayidx3, align 8, !tbaa !7
  %add4 = fadd double %8, 1.000000e+00
  store double %add4, double* %arrayidx3, align 8, !tbaa !7
  br label %_Z11test_squareiPd.sw.epilog.1

DIR.OMP.SECTION.12:                               ; preds = %.sloop.header.1
  %9 = load double*, double** %d.addr1, align 8, !tbaa !3
  %arrayidx6 = getelementptr inbounds double, double* %9, i64 2
  %10 = load double, double* %arrayidx6, align 8, !tbaa !7
  %add7 = fadd double %10, 1.000000e+00
  store double %add7, double* %arrayidx6, align 8, !tbaa !7
  br label %_Z11test_squareiPd.sw.epilog.1

DIR.OMP.SECTION.15:                               ; preds = %.sloop.header.1
  %11 = load double*, double** %d.addr1, align 8, !tbaa !3
  %arrayidx9 = getelementptr inbounds double, double* %11, i64 3
  %12 = load double, double* %arrayidx9, align 8, !tbaa !7
  %add10 = fadd double %12, 1.000000e+00
  store double %add10, double* %arrayidx9, align 8, !tbaa !7
  br label %_Z11test_squareiPd.sw.epilog.1

DIR.OMP.END.SECTION.17.split:                     ; preds = %_Z11test_squareiPd.sw.epilog.1, %DIR.OMP.TARGET.3
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL.SECTIONS"() ]
  br label %DIR.OMP.END.PARALLEL.SECTIONS.18.split

DIR.OMP.END.PARALLEL.SECTIONS.18.split:           ; preds = %entry, %DIR.OMP.END.SECTION.17.split
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  ret void

_Z11test_squareiPd.sw.epilog.1:                   ; preds = %DIR.OMP.SECTION.15, %DIR.OMP.SECTION.12, %DIR.OMP.SECTION.9, %DIR.OMP.SECTION.6
  %13 = load volatile i32, i32* %.sloop.iv.1
  %.sloop.inc.1 = add nuw nsw i32 %13, 1
  store volatile i32 %.sloop.inc.1, i32* %.sloop.iv.1
  %14 = load volatile i32, i32* %.sloop.iv.1
  %_Z11test_squareiPd.sloop.cond.1 = icmp sle i32 %14, %sloop.ub
  br i1 %_Z11test_squareiPd.sloop.cond.1, label %.sloop.header.1, label %DIR.OMP.END.SECTION.17.split
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: uwtable
define internal void @.omp_offloading.requires_reg() #2 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare dso_local void @__tgt_register_requires(i64)

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 59, i32 -1929423428, !"_Z11test_squareiPd", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"pointer@_ZTSPd", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"double", !5, i64 0}

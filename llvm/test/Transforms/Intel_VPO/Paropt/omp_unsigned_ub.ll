; RUN: opt -vpo-paropt -S 
; RUN: opt -passes='vpo-paropt' -S 

; Clang-emitted IR for a loop with unsigned UB changed after the
; pulldown of 20190213 (ae03dc76).
;
; Compile the test below (reduced from ak_bitonicsort.cpp) with:
;   icpx -c -fiopenmp -fopenmp-targets=spir64
;
; void transpose_kernel(unsigned height) {
;   #pragma omp target
;   #pragma omp parallel for
;     for (int tiley = 0; tiley < height; ++tiley) { }
; }
;
; IR from Clang before pulldown:
;   %11 = load i32, i32* %.omp.iv
;   %12 = load i32, i32* %.omp.ub
;   %cmp4 = icmp ule i32 %11, %12  <-- UB (%12) was a load before
;
; IR from Clang after pulldown:
;   %11 = load i32, i32* %.omp.iv
;   %12 = load i32, i32* %.omp.ub
;   %add4 = add i32 %12, 1
;   %cmp5 = icmp ult i32 %11, %add4 <-- UB (%add4) is now an add
;
; This IR caused 2 compfails:
; 1. The ULT triggered an assert in fixOmpDoWhileLoopImpl().
; 2. The util findChainToLoad() asserted because it did not expect UB
;    to be an add.
;
; These issues are fixed in https://git-amr-2.devtools.intel.com/gerrit/174312

source_filename = "u.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z16transpose_kernelj(i32 %height) #0 {
entry:
  %height.addr = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tiley = alloca i32, align 4
  store i32 %height, i32* %height.addr, align 4
  br label %DIR.OMP.TARGET.1.split

DIR.OMP.TARGET.1.split:                           ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.), "QUAL.OMP.FIRSTPRIVATE"(i32* %height.addr), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %tiley), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %DIR.OMP.TARGET.1.split
  %1 = load i32, i32* %height.addr, align 4
  store i32 %1, i32* %.capture_expr., align 4
  %2 = load i32, i32* %.capture_expr., align 4
  %sub = sub i32 %2, 0
  %sub2 = sub i32 %sub, 1
  %add = add i32 %sub2, 1
  %div = udiv i32 %add, 1
  %sub3 = sub i32 %div, 1
  store i32 %sub3, i32* %.capture_expr.1, align 4
  %3 = load i32, i32* %.capture_expr., align 4
  %cmp = icmp ult i32 0, %3
  br i1 %cmp, label %omp.precond.then, label %DIR.OMP.END.TARGET.5

omp.precond.then:                                 ; preds = %DIR.OMP.TARGET.1
  store i32 0, i32* %.omp.lb, align 4
  %4 = load i32, i32* %.capture_expr.1, align 4
  store volatile i32 %4, i32* %.omp.ub, align 4
  br label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %omp.precond.then
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %tiley) ]
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:                          ; preds = %DIR.OMP.PARALLEL.LOOP.3
  %6 = load i32, i32* %.omp.lb, align 4
  store volatile i32 %6, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.PARALLEL.LOOP.2
  %7 = load volatile i32, i32* %.omp.iv, align 4
  %8 = load volatile i32, i32* %.omp.ub, align 4

                                          ; NEW IR from Clang:
  %add4 = add i32 %8, 1                   ; <-- UB is an add instead of load
  %cmp5 = icmp ult i32 %7, %add4          ; <-- ult instead of ule

  br i1 %cmp5, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load volatile i32, i32* %.omp.iv, align 4
  %mul = mul i32 %9, 1
  %add6 = add i32 0, %mul
  store i32 %add6, i32* %tiley, align 4
  %10 = load volatile i32, i32* %.omp.iv, align 4
  %add7 = add i32 %10, 1
  store volatile i32 %add7, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.TARGET.1, %omp.loop.exit
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.END.TARGET.5
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.END.TARGET.3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 58, i32 -673150349, !"_Z16transpose_kernelj", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}

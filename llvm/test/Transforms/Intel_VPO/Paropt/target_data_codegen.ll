; RUN: opt -vpo-paropt -S %s
; RUN: opt -passes='vpo-paropt' -S %s

; Before the fix for CMPLRLLVM-8213, compiling the test below with
;   icpx -c -O0 -fiopenmp -fopenmp-targets=x86_64-pc-linux-gnu
; compfailed with an assertion during TARGET DATA codegen.
;
; void foo(int N, float* x) {
;     int NN = N;
;     float* xx = x;
;     int dummy = 123;
;     #pragma omp target data map(tofrom:xx[0:NN])
;     {
;        xx[2] = dummy + NN;
;     }
; }

source_filename = "t1a1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z3fooiPf(i32 %N, float* %x) #0 {
entry:
  %N.addr = alloca i32, align 4
  %x.addr = alloca float*, align 8
  %NN = alloca i32, align 4
  %xx = alloca float*, align 8
  %dummy = alloca i32, align 4
  store i32 %N, i32* %N.addr, align 4
  store float* %x, float** %x.addr, align 8
  %0 = load i32, i32* %N.addr, align 4
  store i32 %0, i32* %NN, align 4
  %1 = load float*, float** %x.addr, align 8
  store float* %1, float** %xx, align 8
  store i32 123, i32* %dummy, align 4
  %2 = load float*, float** %xx, align 8
  %arrayidx = getelementptr inbounds float, float* %2, i64 0
  %3 = load i32, i32* %NN, align 4
  %4 = zext i32 %3 to i64
  %5 = mul nuw i64 %4, 4
  %6 = bitcast float* %arrayidx to i8*
  %7 = call i8* @llvm.launder.invariant.group.p0i8(i8* %6)
  %8 = bitcast i8* %7 to float*
  br label %DIR.OMP.TARGET.DATA.1.split

DIR.OMP.TARGET.DATA.1.split:                      ; preds = %entry
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(float** %xx, float** %xx, i64 8), "QUAL.OMP.MAP.TOFROM:AGGR"(float** %xx, float* %8, i64 %5) ]
  br label %DIR.OMP.TARGET.DATA.1

DIR.OMP.TARGET.DATA.1:                            ; preds = %DIR.OMP.TARGET.DATA.1.split
  %10 = load i32, i32* %dummy, align 4
  %11 = load i32, i32* %NN, align 4
  %add = add nsw i32 %10, %11
  %conv = sitofp i32 %add to float
  %12 = load float*, float** %xx, align 8
  %arrayidx1 = getelementptr inbounds float, float* %12, i64 2
  store float %conv, float* %arrayidx1, align 4
  br label %DIR.OMP.END.TARGET.DATA.3

DIR.OMP.END.TARGET.DATA.3:                        ; preds = %DIR.OMP.TARGET.DATA.1
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TARGET.DATA"() ]
  br label %DIR.OMP.END.TARGET.DATA.2

DIR.OMP.END.TARGET.DATA.2:                        ; preds = %DIR.OMP.END.TARGET.DATA.3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: inaccessiblememonly nounwind speculatable
declare i8* @llvm.launder.invariant.group.p0i8(i8*) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { inaccessiblememonly nounwind speculatable }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}

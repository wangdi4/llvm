; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt -S %s
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt' -S %s

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

; CHECK: call void @_Z3fooiPf.DIR.OMP.TARGET.DATA{{.*}}(float** %xx, i32* %dummy, i32* %NN)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

@"@tid.addr" = external global i32

define dso_local void @_Z3fooiPf(i32 %N, float* %x) {
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
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(float** %xx, float** %xx, i64 8),
    "QUAL.OMP.MAP.TOFROM:AGGR"(float** %xx, float* %8, i64 %5) ]
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

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare i8* @llvm.launder.invariant.group.p0i8(i8*)

; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s
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

; CHECK: call void @_Z3fooiPf.DIR.OMP.TARGET.DATA{{.*}}(ptr %xx, ptr %dummy, ptr %NN)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

@"@tid.addr" = external global i32

define dso_local void @_Z3fooiPf(i32 %N, ptr %x) {
entry:
  %N.addr = alloca i32, align 4
  %x.addr = alloca ptr, align 8
  %NN = alloca i32, align 4
  %xx = alloca ptr, align 8
  %dummy = alloca i32, align 4
  store i32 %N, ptr %N.addr, align 4
  store ptr %x, ptr %x.addr, align 8
  %0 = load i32, ptr %N.addr, align 4
  store i32 %0, ptr %NN, align 4
  %1 = load ptr, ptr %x.addr, align 8
  store ptr %1, ptr %xx, align 8
  store i32 123, ptr %dummy, align 4
  %2 = load ptr, ptr %xx, align 8
  %3 = load i32, ptr %NN, align 4
  %4 = zext i32 %3 to i64
  %5 = mul nuw i64 %4, 4
  %6 = call ptr @llvm.launder.invariant.group.p0(ptr %2)
  br label %DIR.OMP.TARGET.DATA.1.split

DIR.OMP.TARGET.DATA.1.split:                      ; preds = %entry
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.MAP.TOFROM"(ptr %xx, ptr %xx, i64 8, i32 32, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %xx, ptr %6, i64 %5, i32 19, ptr null, ptr null) ]
  br label %DIR.OMP.TARGET.DATA.1

DIR.OMP.TARGET.DATA.1:                            ; preds = %DIR.OMP.TARGET.DATA.1.split
  %8 = load i32, ptr %dummy, align 4
  %9 = load i32, ptr %NN, align 4
  %add = add nsw i32 %8, %9
  %conv = sitofp i32 %add to float
  %10 = load ptr, ptr %xx, align 8
  %arrayidx1 = getelementptr inbounds float, ptr %10, i64 2
  store float %conv, ptr %arrayidx1, align 4
  br label %DIR.OMP.END.TARGET.DATA.3

DIR.OMP.END.TARGET.DATA.3:                        ; preds = %DIR.OMP.TARGET.DATA.1
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TARGET.DATA"() ]
  br label %DIR.OMP.END.TARGET.DATA.2

DIR.OMP.END.TARGET.DATA.2:                        ; preds = %DIR.OMP.END.TARGET.DATA.3
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare ptr @llvm.launder.invariant.group.p0(ptr)

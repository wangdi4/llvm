; RUN: not opt -opaque-pointers=0 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S %s 2>&1 | FileCheck %s
; RUN: not opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s 2>&1 | FileCheck %s

; Test src:
;
; void foo_targ(float *A, int n) {}
;
; #pragma omp declare variant(foo_targ)                                          \
;     match(construct = {target variant dispatch}, device = {arch(gen)})
; void foo_base(float *A, int n) {}
;
; void caller(float *x, int n) {
; #pragma omp target data map(tofrom : x [0:n])
;   {
; #pragma omp target variant dispatch use_device_ptr(x)
;     foo_base(x, n);
;   }
; }

; CHECK: Function 'foo_targ' exists, but has an unexpected type.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo_targ(float* %A, i32 %n) #0 {
entry:
  %A.addr = alloca float*, align 8
  %n.addr = alloca i32, align 4
  store float* %A, float** %A.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo_base(float* %A, i32 %n) #1 {
entry:
  %A.addr = alloca float*, align 8
  %n.addr = alloca i32, align 4
  store float* %A, float** %A.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @caller(float* %x, i32 %n) #2 {
entry:
  %x.addr = alloca float*, align 8
  %n.addr = alloca i32, align 4
  store float* %x, float** %x.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  %0 = load float*, float** %x.addr, align 8
  %1 = load float*, float** %x.addr, align 8
  %arrayidx = getelementptr inbounds float, float* %1, i64 0
  %2 = load i32, i32* %n.addr, align 4
  %conv = sext i32 %2 to i64
  %3 = mul nuw i64 %conv, 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.MAP.TOFROM"(float* %0, float* %arrayidx, i64 %3, i64 35) ]
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(), "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(float** %x.addr) ]
  %6 = load float*, float** %x.addr, align 8
  %7 = load i32, i32* %n.addr, align 4
  call void @foo_base(float* %6, i32 %7) #3
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-variant"="name:foo_targ;construct:target_variant_dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!0 = !{i32 1, !"wchar_size", i32 4}

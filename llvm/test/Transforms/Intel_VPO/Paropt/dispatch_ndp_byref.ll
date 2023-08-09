; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:

; void __attribute__((nothrow,noinline))  foo_gpu(int aaa, int* bbb, int*&ccc) {
;   // printf("\n *** VARIANT FUNCTION ***\n");
; }
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)}) \
;                                      adjust_args(need_device_ptr:bbb,ccc)
; void __attribute__((nothrow,noinline))  foo(int aaa, int* bbb, int*&ccc) {
;   // printf("\n *** BASE FUNCTION ***\n");
; }
; int main() {
;   int *ppp;
;   int *qqq;
;   #pragma omp dispatch device(0)
;     foo(0, ppp, qqq);
;   return 0;
; }

; Pointer ppp is passed by value
; Pointer qqq is passed by reference
; The need_device_ptr string is "need_device_ptr:F,T,PTR_TO_PTR"
;
; Code to get device ptr for ppp and qqq and to call foo_gpu looks like this:
;
;     %0 = load ptr, ptr %ppp, align 8
;     %qqq.load = load ptr, ptr %qqq, align 8
;     ...
;     %2 = getelementptr inbounds [2 x ptr], ptr %.offload_baseptrs, i32 0, i32 0
;     store ptr %0, ptr %2, align 8
;     ...
;     %5 = getelementptr inbounds [2 x ptr], ptr %.offload_baseptrs, i32 0, i32 1
;     store ptr %qqq.load, ptr %5, align 8
;     ...
;     call void @__tgt_target_data_begin_mapper(...)
;     %.updated.val = load ptr, ptr %2, align 8
;     %qqq.updated.val = load ptr, ptr %5, align 8
;     store ptr %qqq.updated.val, ptr %qqq.new, align 8
;     call void @_Z7foo_gpuiPiRS_(i32 0, ptr %.updated.val, ptr %qqq.new)

; CHECK:     [[PLOAD:%[^ ]+]] = load ptr, ptr %ppp
; CHECK:     [[QLOAD:%[^ ]+]] = load ptr, ptr %qqq

; CHECK:     [[PGEP:%[^ ]+]] = getelementptr inbounds [2 x ptr], ptr %.offload_baseptrs, i32 0, i32 0
; CHECK:     store ptr [[PLOAD]], ptr [[PGEP]]

; CHECK:     [[QGEP:%[^ ]+]] = getelementptr inbounds [2 x ptr], ptr %.offload_baseptrs, i32 0, i32 1
; CHECK:     store ptr [[QLOAD]], ptr [[QGEP]]

; CHECK:     call void @__tgt_target_data_begin_mapper

; CHECK:     [[PUPDATEVAL:%[^ ]+]] = load ptr, ptr [[PGEP]]

; CHECK:     [[QUPDATEVAL:%[^ ]+]] = load ptr, ptr [[QGEP]]
; CHECK:     store ptr [[QUPDATEVAL]], ptr [[QNEW:%[^ ]+]],

; CHECK:     call void @_Z7foo_gpuiPiRS_(i32 0, ptr [[PUPDATEVAL]], ptr [[QNEW]])
; CHECK:     call void @__tgt_target_data_end_mapper


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %ppp = alloca ptr, align 8
  %qqq = alloca ptr, align 8
  store i32 0, ptr %retval, align 4

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0) ]

  %1 = load ptr, ptr %ppp, align 8
  call void @_Z3fooiPiRS_(i32 noundef 0, ptr noundef %1, ptr noundef nonnull align 8 dereferenceable(8) %qqq) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]
  ret i32 0
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z7foo_gpuiPiRS_(i32 noundef %aaa, ptr noundef %bbb, ptr noundef nonnull align 8 dereferenceable(8) %ccc) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  %ccc.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  store ptr %ccc, ptr %ccc.addr, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fooiPiRS_(i32 noundef %aaa, ptr noundef %bbb, ptr noundef nonnull align 8 dereferenceable(8) %ccc) #1 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  %ccc.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  store ptr %ccc, ptr %ccc.addr, align 8
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z7foo_gpuiPiRS_;construct:dispatch;arch:gen;need_device_ptr:F,T,PTR_TO_PTR" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress noinline norecurse nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

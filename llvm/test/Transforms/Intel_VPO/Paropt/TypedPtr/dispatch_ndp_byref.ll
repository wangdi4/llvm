; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; // C++ source:
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
;
; Pointer ppp is passed by value
; Pointer qqq is passed by reference
; The need_device_ptr string is "need_device_ptr:F,T,PTR_TO_PTR"
;
; Code to get device ptr for ppp and qqq and to call foo_gpu looks like this:
;
;     %0 = load i32*, i32** %ppp
;     %qqq.load = load i32*, i32** %qqq
;     ...
;     %2 = bitcast i32* %0 to i8*
;     %3 = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
;     store i8* %2, i8** %3
;     ...
;     %7 = bitcast i32* %qqq.load to i8*
;     %8 = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 1
;     store i8* %7, i8** %8
;     ...
;     call void @__tgt_target_data_begin_mapper(...)
;     %.cast = bitcast i8** %3 to i32**
;     %.updated.val = load i32*, i32** %.cast
;     %.cast1 = bitcast i8** %8 to i32**
;     %qqq.updated.val = load i32*, i32** %.cast1
;     store i32* %qqq.updated.val, i32** %qqq.new, align 8
;     call void @_Z7foo_gpuiPiRS_(i32 0, i32* %.updated.val, i32** %qqq.new)

; CHECK:     [[PLOAD:%[^ ]+]] = load i32*, i32** %ppp
; CHECK:     [[QLOAD:%[^ ]+]] = load i32*, i32** %qqq

; CHECK:     [[PCAST:%[^ ]+]] = bitcast i32* [[PLOAD]] to i8*
; CHECK:     [[PGEP:%[^ ]+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
; CHECK:     store i8* [[PCAST]], i8** [[PGEP]]

; CHECK:     [[QCAST:%[^ ]+]] = bitcast i32* [[QLOAD]] to i8*
; CHECK:     [[QGEP:%[^ ]+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 1
; CHECK:     store i8* [[QCAST]], i8** [[QGEP]]

; CHECK:     call void @__tgt_target_data_begin_mapper

; CHECK:     [[PUPDATECAST:%[^ ]+]] = bitcast i8** [[PGEP]] to i32**
; CHECK:     [[PUPDATEVAL:%[^ ]+]] = load i32*, i32** [[PUPDATECAST]]

; CHECK:     [[QUPDATECAST:%[^ ]+]] = bitcast i8** [[QGEP]] to i32**
; CHECK:     [[QUPDATEVAL:%[^ ]+]] = load i32*, i32** [[QUPDATECAST]]
; CHECK:     store i32* [[QUPDATEVAL]], i32** [[QNEW:%[^ ]+]],

; CHECK:     call void @_Z7foo_gpuiPiRS_(i32 0, i32* [[PUPDATEVAL]], i32** [[QNEW]])
; CHECK:     call void @__tgt_target_data_end_mapper


; ModuleID = 'dispatch_ndp_byref.cpp'
source_filename = "dispatch_ndp_byref.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %ppp = alloca i32*, align 8
  %qqq = alloca i32*, align 8
  store i32 0, i32* %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(), "QUAL.OMP.DEVICE"(i32 0) ]
  %1 = load i32*, i32** %ppp, align 8
  call void @_Z3fooiPiRS_(i32 0, i32* %1, i32** nonnull align 8 dereferenceable(8) %qqq) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]
  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z7foo_gpuiPiRS_(i32 %aaa, i32* %bbb, i32** nonnull align 8 dereferenceable(8) %ccc) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  %ccc.addr = alloca i32**, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  store i32** %ccc, i32*** %ccc.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z3fooiPiRS_(i32 %aaa, i32* %bbb, i32** nonnull align 8 dereferenceable(8) %ccc) #1 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  %ccc.addr = alloca i32**, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  store i32** %ccc, i32*** %ccc.addr, align 8
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z7foo_gpuiPiRS_;construct:dispatch;arch:gen;need_device_ptr:F,T,PTR_TO_PTR" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { noinline norecurse nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}

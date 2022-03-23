; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; // C++ source
; int __attribute__((nothrow,noinline))  foo_gpu(int aaa, int* bbb) {
;   // printf("\n *** VARIANT FUNCTION ***\n");
;   return 0;
; }
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)})
; int __attribute__((nothrow,noinline))  foo(int aaa, int* bbb) {
;   // printf("\n *** BASE FUNCTION ***\n");
;   return 0;
; }
; int main() {
;   int *ptr;
;   int aaa, rrr;
;   #pragma omp dispatch device(0) novariants(aaa>5)
;     rrr = foo(0,ptr);
;   return rrr;
; }
;
; The code after Prepare Pass should look like this
;
;     %2 = load i32*, i32** %ptr, align 8
;     %3 = call i32 @__tgt_is_device_available(i64 0, i8* inttoptr (i64 7 to i8*))
;     %4 = icmp ne i32 %3, 0
;     %dovariants = icmp eq i1 %tobool, false
;     %available = and i1 %4, %dovariants
;     br i1 %available, label %if.then, label %if.else
;
;   if.then:                                          ; preds = %DIR.OMP.DISPATCH.2
;     %variant = call i32 @_Z7foo_gpuiPi(i32 0, i32* %2)
;     br label %if.end
;
;   if.else:                                          ; preds = %DIR.OMP.DISPATCH.2
;     %call = call i32 @_Z3fooiPi(i32 0, i32* %2) #3
;     br label %if.end
;
;   if.end:                                           ; preds = %if.else, %if.then
;     %callphi = phi i32 [ %variant, %if.then ], [ %call, %if.else ]
;     store i32 %callphi, i32* %rrr, align 4
;     br label %DIR.OMP.END.DISPATCH.3

; CHECK:      call i32 @__tgt_is_device_available
; CHECK:      [[DOVARIANTS:%[a-zA-Z._0-9]+]] = icmp eq i1 %{{.*}}, false
; CHECK-NEXT: [[AVAILABLE:%[a-zA-Z._0-9]+]] = and i1 %{{.*}}, [[DOVARIANTS]]
; CHECK-NEXT: br i1 [[AVAILABLE]], label %[[IFTHEN:[a-zA-Z._0-9]+]], label %[[IFELSE:[a-zA-Z._0-9]+]]

; CHECK-DAG:  [[IFTHEN]]:
; CHECK-NEXT: [[VARIANT:%[a-zA-Z._0-9]+]] = call i32 @_Z7foo_gpuiPi(i32 0, i32* %{{[0-9]+}})
; CHECK-NEXT: br label %[[IFEND:[a-zA-Z._0-9]+]]

; CHECK-DAG:  [[IFELSE]]:
; CHECK-NEXT: [[BASECALL:%[a-zA-Z._0-9]+]] = call i32 @_Z3fooiPi(i32 0, i32* %{{[0-9]+}})
; CHECK-NEXT: br label %[[IFEND]]

; CHECK:      [[IFEND]]:
; CHECK-NEXT: phi i32 [ [[VARIANT]], %[[IFTHEN]] ], [ [[BASECALL]], %[[IFELSE]] ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %ptr = alloca i32*, align 8
  %aaa = alloca i32, align 4
  %rrr = alloca i32, align 4
  %.capture_expr.0 = alloca i8, align 1
  store i32 0, i32* %retval, align 4
  %0 = load i32, i32* %aaa, align 4
  %cmp = icmp sgt i32 %0, 5
  %frombool = zext i1 %cmp to i8
  store i8 %frombool, i8* %.capture_expr.0, align 1
  %1 = load i8, i8* %.capture_expr.0, align 1
  %tobool = trunc i8 %1 to i1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(), "QUAL.OMP.DEVICE"(i32 0), "QUAL.OMP.NOVARIANTS"(i1 %tobool) ]
  %3 = load i32*, i32** %ptr, align 8
  %call = call i32 @_Z3fooiPi(i32 0, i32* %3) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]
  store i32 %call, i32* %rrr, align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISPATCH"() ]
  %4 = load i32, i32* %rrr, align 4
  ret i32 %4
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local i32 @_Z7foo_gpuiPi(i32 %aaa, i32* %bbb) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local i32 @_Z3fooiPi(i32 %aaa, i32* %bbb) #1 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z7foo_gpuiPi;construct:dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { noinline norecurse nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}

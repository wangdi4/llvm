; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s 2>&1 | FileCheck %s

; // C++ source
; // #include <stdio.h>
; void __attribute__((nothrow,noinline))  foo_XeHP(int aaa, int* bbb) {
;   // printf("\n *** VARIANT FUNCTION for XeHP ***\n");
; }
; void __attribute__((nothrow,noinline))  foo_XeLP(int aaa, int* bbb) {
;   // printf("\n *** VARIANT FUNCTION for XeLP ***\n");
; }
; #pragma omp declare variant(foo_XeHP) match(construct={dispatch}, device={arch(XeLP)})
; #pragma omp declare variant(foo_XeLP) match(construct={dispatch}, device={arch(XeHP)})
; void __attribute__((nothrow,noinline))  foo(int aaa, int* bbb) {
;   // printf("\n *** BASE FUNCTION ***\n");
; }
; int main() {
;   int *ptr;
;   #pragma omp dispatch device(0)
;     foo(0,ptr);
;   return 0;
; }

; CHECK: warning:{{.*}} Found multiple variants for dispatch. Only one will be used in the current implementation. The variant to be used is '_Z8foo_XeLPiPi'

; CHECK-LABEL: if.then:
; CHECK:         call void @_Z8foo_XeLPiPi(i32 0

; CHECK-LABEL: if.else:
; CHECK:         call void @_Z3fooiPi(i32 0


source_filename = "dispatch_multiple_variants_warning.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %ptr = alloca i32*, align 8
  store i32 0, i32* %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(), "QUAL.OMP.DEVICE"(i32 0) ]
  %1 = load i32*, i32** %ptr, align 8
  call void @_Z3fooiPi(i32 0, i32* %1) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]
  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z8foo_XeHPiPi(i32 %aaa, i32* %bbb) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z8foo_XeLPiPi(i32 %aaa, i32* %bbb) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z3fooiPi(i32 %aaa, i32* %bbb) #1 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z8foo_XeLPiPi;construct:dispatch;arch:XeHP;;name:_Z8foo_XeHPiPi;construct:dispatch;arch:XeLP" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { noinline norecurse nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}

; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; // Test the interop_position attribute used for cases where the
; // interop obj is not the last parameter of the variant function.
; // Currently Clang does not emit the interop_position attribute so we
; // hand-modified the IR as follows to create this LIT test:
; //  - swap order of foo_gpu's 2nd and 3rd args (interop and bbb)
; //  - add the "interop_position:2" substring to the openmp-variant string attribute.
;
; void __attribute__((nothrow,noinline))  foo_gpu(int aaa, int *bbb, void* interop){}
; // foo_gpu's IR is hand-modified into:  foo_gpu(int aaa, void* interop, int *bbb){}
;
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)}) \
;                                      append_args(interop(targetsync))
; void __attribute__((nothrow,noinline))  foo(int aaa, int* bbb){}
;
; int main() {
;   int *ptr;
;   #pragma omp dispatch device(0)
;     foo(0,ptr);
;   return 0;
; }

; Check that we call foo_gpu with interop_obj as the second argument instead of the last.
;
; CHECK: [[INTEROPOBJ:%[^ ]+]] = call i8* @__tgt_create_interop_obj(i64 0, i8 0, i8* null)
; CHECK: call void @_Z7foo_gpuiPiPv(i32 0, i8* [[INTEROPOBJ]], i32* %0)

; ModuleID = 'dispatch_interop_position.cpp'
source_filename = "dispatch_interop_position.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z7foo_gpuiPiPv(i32 %aaa, i8* %interop, i32* %bbb) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  %interop.addr = alloca i8*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  store i8* %interop, i8** %interop.addr, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fooiPi(i32 %aaa, i32* %bbb) #1 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  ret void
}

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
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

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { mustprogress noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

;
; The ";interop_position:2" substring below was added by hand
;
attributes #1 = { mustprogress noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z7foo_gpuiPiPv;construct:dispatch;arch:gen;interop:targetsync;interop_position:2" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

attributes #2 = { mustprogress noinline norecurse nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}

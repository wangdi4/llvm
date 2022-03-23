; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; // C++ source:
; void __attribute__((nothrow,noinline))  foo_gpu(int aaa, int *bbb, void* interop) {
;   // printf("\n *** VARIANT FUNCTION (NOWAIT) ***\n");
; }
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)}) \
;                                      append_args(interop(targetsync))
; void __attribute__((nothrow,noinline))  foo(int aaa, int* bbb) {
;   // printf("\n *** BASE FUNCTION ***\n");
; }
; int main() {
;   int *ptr;
;   #pragma omp dispatch device(0) nowait
;     foo(0,ptr);
;   return 0;
; }
;
; This test is similar to the asynchronous case of target variant dispatch nowait.
; The code after Prepare Pass should look like this
;
;   %asyncobj = call i8* @__kmpc_omp_task_alloc(%struct.ident_t* @.kmpc_loc.0.0, i32 0, i32 16, i64 24, i64 0, i8* null)
;   %asyncobj.ptr = bitcast i8* %asyncobj to %__struct.AsyncObj*
;   %task.entry.gep = getelementptr inbounds %__struct.AsyncObj, %__struct.AsyncObj* %asyncobj.ptr, i32 0, i32 1
;   store i8* null, i8** %task.entry.gep, align 8
;   %part.id.gep = getelementptr inbounds %__struct.AsyncObj, %__struct.AsyncObj* %asyncobj.ptr, i32 0, i32 2
;   store i32 0, i32* %part.id.gep, align 4
;   %interop.obj.async = call i8* @__tgt_create_interop_obj(i64 0, i8 1, i8* %asyncobj)
;   call void @_Z7foo_gpuiPiPv(i32 0, i32* %0, i8* %interop.obj.async)

; CHECK: [[ASYNCOBJ:%[a-zA-Z._0-9]+]] = call i8* @__kmpc_omp_task_alloc(%struct.ident_t* {{.*}}, i32 0, i32 16, i64 24, i64 0, i8* null)
; CHECK: [[INTEROPASYNC:%[a-zA-Z._0-9]+]] = call i8* @__tgt_create_interop_obj(i64 0, i8 1, i8* [[ASYNCOBJ]])
; CHECK: call void @_Z7foo_gpuiPiPv(i32 0, i32* {{.*}}, i8* [[INTEROPASYNC]])

; ModuleID = 'dispatch_nowait_interop.cpp'
source_filename = "dispatch_nowait_interop.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %ptr = alloca i32*, align 8
  store i32 0, i32* %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.IMPLICIT"(), "QUAL.OMP.SHARED"(i32** %ptr) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(), "QUAL.OMP.DEVICE"(i32 0), "QUAL.OMP.NOWAIT"() ]
  %2 = load i32*, i32** %ptr, align 8
  call void @_Z3fooiPi(i32 0, i32* %2) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.DISPATCH"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z7foo_gpuiPiPv(i32 %aaa, i32* %bbb, i8* %interop) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  %interop.addr = alloca i8*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  store i8* %interop, i8** %interop.addr, align 8
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

attributes #0 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z7foo_gpuiPiPv;construct:dispatch;arch:gen;interop:targetsync" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { noinline norecurse nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}

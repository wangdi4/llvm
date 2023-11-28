; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-paropt-prepare)' -S <%s | FileCheck %s
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s

; // C source
; #include <omp.h>
; void bar_variant(float *&AAA, float *&BBB, omp_interop_t interop, ...);
;
; #pragma omp declare variant(bar_variant) match(construct={dispatch}, device={arch(gen)}) \
;                                          adjust_args(need_device_ptr:AAA) \
;                                          append_args(interop(target))
; void bar(float *&AAA, float *&BBB, ...);
;
; void foo(float *aaa, float *bbb, float* ccc) {
;   #pragma omp dispatch need_device_ptr(2, 3) // bbb,ccc
;     bar(aaa, bbb, ccc, 4);
; }
;
; Test support for need_device_ptr clause in dispatch construct.
; Function bar() is variadic and its variant function call to bar_variant()
; has an interop obj injected between its 'bbb' and 'ccc' arguments.
;
; Argument aaa is translated due to adjust_args(need_device_ptr:AAA)
; while arguments bbb and ccc are translated due to need_device_ptr(2, 3)

source_filename = "dispatch_need_device_ptr_clause.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@"@tid.addr" = external global i32

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fooPfS_S_(ptr noundef %aaa, ptr noundef %bbb, ptr noundef %ccc) #0 {
entry:
;
; CHECK:  {{.*}}offload_baseptrs = alloca [3 x ptr], align 8
; Check that we allocate an array of 3 pointers for aaa, bbb, and ccc.
;
  %aaa.addr = alloca ptr, align 8
  %bbb.addr = alloca ptr, align 8
  %ccc.addr = alloca ptr, align 8
  store ptr %aaa, ptr %aaa.addr, align 8
  store ptr %bbb, ptr %bbb.addr, align 8
  store ptr %ccc, ptr %ccc.addr, align 8
  br label %DIR.OMP.DISPATCH.1

DIR.OMP.DISPATCH.1:                               ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(), "QUAL.OMP.NEED_DEVICE_PTR:PTR_TO_PTR"(i32 2), "QUAL.OMP.NEED_DEVICE_PTR"(i32 3) ]
  br label %DIR.OMP.DISPATCH.2

DIR.OMP.DISPATCH.2:                               ; preds = %DIR.OMP.DISPATCH.1
  %1 = load ptr, ptr %ccc.addr, align 8
  call void (ptr, ptr, ...) @_Z3barRPfS0_z(ptr noundef nonnull align 8 dereferenceable(8) %aaa.addr, ptr noundef nonnull align 8 dereferenceable(8) %bbb.addr, ptr noundef %1, i32 noundef 4) #1 [ "QUAL.OMP.DISPATCH.CALL"() ]

; CHECK:  call void (ptr, ptr, ptr, ...) @_Z11bar_variantRPfS0_Pvz(ptr %aaa.addr.new, ptr %bbb.addr.new, ptr %interop.obj{{.*}}, ptr {{.*}}.updated.val, i32 4)
; aaa and bbb are PTR_TO_PTR (hence the .addr.new suffix);
; the interop obj is the third argument and is not translated (no addr.new or updated.val suffix);
; ccc's updated val is immediately after the interop obj.

  br label %DIR.OMP.END.DISPATCH.3

DIR.OMP.END.DISPATCH.3:                           ; preds = %DIR.OMP.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]
  br label %DIR.OMP.END.DISPATCH.4

DIR.OMP.END.DISPATCH.4:                           ; preds = %DIR.OMP.END.DISPATCH.3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_Z3barRPfS0_z(ptr noundef nonnull align 8 dereferenceable(8), ptr noundef nonnull align 8 dereferenceable(8), ...) #2

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z11bar_variantRPfS0_Pvz;construct:dispatch;arch:gen;need_device_ptr:PTR_TO_PTR,F,F;interop:target" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}

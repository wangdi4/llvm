; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-prepare -S <%s | FileCheck %s --check-prefix=CHECK-BEFORE --check-prefix=CHECK-ALL
; RUN: opt -passes='function(vpo-paropt-prepare)' -S <%s | FileCheck %s --check-prefix=CHECK-BEFORE --check-prefix=CHECK-ALL

; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-prepare -vpo-paropt-put-interop-after-vararg=true -S <%s | FileCheck %s --check-prefix=CHECK-AFTER --check-prefix=CHECK-ALL
; RUN: opt -passes='function(vpo-paropt-prepare)' -vpo-paropt-put-interop-after-vararg=true -S <%s | FileCheck %s --check-prefix=CHECK-AFTER --check-prefix=CHECK-ALL

; // C source
; #include <omp.h>
; void bar_variant(float *&AAA, ...);
;
; #pragma omp declare variant(bar_variant) match(construct={target variant dispatch}, device={arch(gen)})
; void bar(float *&AAA, ...);
;
; void foo(float *aaa, float *bbb) {
;   #pragma omp target variant dispatch need_device_ptr(1, 2) // aaa,bbb
;     bar(aaa, bbb, 4);
; }
;
; Test support for need_device_ptr clause in target variant dispatch.
; Function bar() is variadic and its variant function call to bar_variant()
; has an interop obj injected by the compiler.
;
; By default the interop obj is before the first variadic argument (bbb):
;   bar_variant(aaa, interop_obj, bbb)
;
; If compiled with "-mllvm -vpo-paropt-put-interop-after-vararg=true" the
; compiler puts the interop_obj last:
;   bar_variant(aaa, bbb, interop_obj)
;
; Argument aaa is PTR_TO_PTR, and argument bbb is not.

; CHECK-ALL: define internal void @_Z3fooPfS_._Z11bar_variantRPfz.wrapper(ptr %bbb.addr, i64 %0, ptr %aaa.addr)
; Check that the translation code is emitted inside the wrapper function
;
; CHECK-ALL:  {{.*}}.offload_baseptrs = alloca [2 x ptr], align 8
; Check that we allocate an array of 2 pointers (for aaa, bbb)
;                                                                                                                                                                                                                          
; CHECK-BEFORE:  call void (ptr, ptr, ...) @_Z11bar_variantRPfz(ptr %aaa.addr.new, ptr %interop.obj.sync, ptr {{.*}}.updated.val, i32 4)
; aaa is PTR_TO_PTR (hence the .addr.new suffix);
; the interop obj is the second argument and is not translated (no addr.new or updated.val suffix);
; bbb's updated val is immediately after the interop obj.                                                                                                                                                                  
;                                                                                                                                                                                                                          
; CHECK-AFTER:  call void (ptr, ...) @_Z11bar_variantRPfz(ptr %aaa.addr.new, ptr {{.*}}.updated.val, i32 4, ptr %interop.obj.sync)
; the interop obj is the last argument; translation of aaa,bbb is same as before

source_filename = "target_variant_dispatch_need_device_ptr_clause.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fooPfS_(ptr noundef %aaa, ptr noundef %bbb) #0 {
entry:
  %aaa.addr = alloca ptr, align 8
  %bbb.addr = alloca ptr, align 8
  store ptr %aaa, ptr %aaa.addr, align 8
  store ptr %bbb, ptr %bbb.addr, align 8
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(), "QUAL.OMP.NEED_DEVICE_PTR:PTR_TO_PTR"(i32 1), "QUAL.OMP.NEED_DEVICE_PTR"(i32 2) ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %1 = load ptr, ptr %bbb.addr, align 8
  call void (ptr, ...) @_Z3barRPfz(ptr noundef nonnull align 8 dereferenceable(8) %aaa.addr, ptr noundef %1, i32 noundef 4) #1 [ "QUAL.OMP.DISPATCH.CALL"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_Z3barRPfz(ptr noundef nonnull align 8 dereferenceable(8), ...) #2

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z11bar_variantRPfz;construct:target_variant_dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}

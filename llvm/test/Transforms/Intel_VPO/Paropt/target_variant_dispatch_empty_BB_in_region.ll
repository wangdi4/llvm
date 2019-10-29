; RUN: opt -vpo-paropt-prepare -S < %s | FileCheck %s
; RUN: opt < %s -passes='function(vpo-paropt-prepare)' -S | FileCheck %s
; Test for TARGET VARIANT DISPATCH construct without a DEVICE clause
; and the associated function returns an INT that is used downstream.
;
; This test is hand-modified based on target_variant_dispatch_intfunc.ll
; I manually added 2 empty BasicBlocks (EmptyBB1 and EmptyBB2) into the
; target variant dispatch region to ensure that the codegen logic handles
; them properly instead of aborting.

; C source:
;
; int foo_gpu(int dnum) { return 456; }
; #pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; int foo(int dnum) { return 123; }
; int dnum;
; int main() {
;   int rrr;
;   #pragma omp target variant dispatch
;   {
;      rrr = foo(dnum);
;   }
;   return rrr;
; }
;
; The dispatch code looks like this:
;
;   %call1 = call i32 @__tgt_is_device_available(i64 -1, i8* null)
;   %dispatch = icmp ne i32 %call1, 0
;   br i1 %dispatch, label %variant.call, label %base.call
;
; variant.call:
;   %variant = call i32 @foo_gpu(i32 %0)
;   br label %if.end
;
; base.call:
;   %call = call i32 @foo(i32 %0)
;   br label %if.end
;
; if.end:
;   %callphi = phi i32 [ %variant, %variant.call ], [ %call, %base.call ]
;   store i32 %callphi, i32* %rrr, align 4

; CHECK: [[CALL:%[a-zA-Z._0-9]+]] = call i32 @__tgt_is_device_available(i64 -1
; CHECK-NEXT: [[DISPATCH:%[a-zA-Z._0-9]+]] = icmp ne i32 [[CALL]], 0
; CHECK-NEXT: br i1 [[DISPATCH]], label %[[VARIANTLBL:[a-zA-Z._0-9]+]], label %[[BASELBL:[a-zA-Z._0-9]+]]

; CHECK-DAG: [[VARIANTLBL]]:
; CHECK-NEXT: [[VARIANT:%[a-zA-Z._0-9]+]] = call i32 @foo_gpu

; CHECK-DAG: [[BASELBL]]:
; CHECK-NEXT: [[BASE:%[a-zA-Z._0-9]+]] = call i32 @foo

; CHECK: phi i32 [ [[VARIANT]], %[[VARIANTLBL]] ], [ [[BASE]], %[[BASELBL]] ]


; ModuleID = 'target_variant_dispatch_intfunc.c'
source_filename = "target_variant_dispatch_intfunc.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@dnum = common dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo_gpu(i32 %dnum) #0 {
entry:
  %dnum.addr = alloca i32, align 4
  store i32 %dnum, i32* %dnum.addr, align 4
  ret i32 456
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo(i32 %dnum) #1 {
entry:
  %dnum.addr = alloca i32, align 4
  store i32 %dnum, i32* %dnum.addr, align 4
  ret i32 123
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %rrr = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"() ]
  br label %EmptyBB1

; Manually added these two empty BBs
EmptyBB1:
  br label %EmptyBB2

EmptyBB2:
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %1 = load i32, i32* @dnum, align 4
  %call = call i32 @foo(i32 %1)
  store i32 %call, i32* %rrr, align 4
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  %2 = load i32, i32* %rrr, align 4
  ret i32 %2
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}

; RUN: opt -vpo-paropt-prepare -S -vpo-paropt-use-interop=false %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -S -vpo-paropt-use-interop=false %s | FileCheck %s
; Test for TARGET VARIANT DISPATCH construct without a DEVICE clause
; and the associated function returns an INT that is used downstream.

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
; ...
;   %available = call i32 @__tgt_is_device_available(i64 %{{.*}}, i8* inttoptr (i64 15 to i8*))
;   %dispatch = icmp ne i32 %available, 0
;   br label %dispatch.check
;
; dispatch.check:
;   br i1 %dispatch, label %variant.call, label %base.call
;
; variant.call:
;   call void @main.foo_gpu.wrapper(i32* %rrr)
;   br label %if.end
;
; base.call:
;   %2 = load i32, i32* @dnum
;   %call.clone = call i32 @foo(i32 %2)
;   store i32 %call.clone, i32* %rrr
;   br label %if.end
; ...
;
; define internal void @main.foo_gpu.wrapper(i32* %rrr) {
;   %0 = load i32, i32* @dnum
;   %variant = call i32 @foo_gpu(i32 %0)
;   store i32 %variant, i32* %rrr
; }
;

; CHECK: [[CALL:%[a-zA-Z._0-9]+]] = call i32 @__tgt_is_device_available(i64 %{{.*}}, i8* inttoptr (i64 15 to i8*))
; CHECK-NEXT: [[DISPATCH:%[a-zA-Z._0-9]+]] = icmp ne i32 [[CALL]], 0
; CHECK: br i1 [[DISPATCH]], label %[[VARIANTLBL:[a-zA-Z._0-9]+]], label %[[BASELBL:[a-zA-Z._0-9]+]]

; CHECK-DAG: [[VARIANTLBL]]:
; CHECK-NEXT: call void @[[VARIANT_WRAPPER:[^ ,]*foo_gpu.wrapper[^ ,)]*]](i32* %rrr)

; CHECK-DAG: [[BASELBL]]:
; CHECK: [[BASE_ARG:%[^ ]+]] = load i32, i32* @dnum
; CHECK: [[BASE:%[a-zA-Z._0-9]+]] = call i32 @foo(i32 [[BASE_ARG]])
; CHECK-NEXT: store i32 [[BASE]], i32* %rrr

; CHECK-DAG: define internal void @[[VARIANT_WRAPPER]](i32* %rrr)
; CHECK: [[ARG:%[^ ]+]] = load i32, i32* @dnum
; CHECK: [[VARIANT:%[^ ]+]] = call i32 @foo_gpu(i32 [[ARG]])
; CHECK: store i32 [[VARIANT]], i32* %rrr

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

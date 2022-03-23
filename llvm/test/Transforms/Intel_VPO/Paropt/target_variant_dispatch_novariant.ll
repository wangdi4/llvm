; RUN: opt -vpo-paropt-prepare -pass-remarks-output=%t -S %s
; RUN: FileCheck --input-file %t %s
; FIXME: This fails with new pass manager as genGlobalPrivatizationLaunderIntrin()
; inserts an empty basic block even when there is nothing to be laundered, which
; changes CFE, but it returns "Changed" as "false". This causes new PM's CFG verification
; to fail.
; COM: opt %s -passes='function(vpo-paropt-prepare)' -pass-remarks-output=%t -S
; COM: FileCheck --input-file %t %s
;
; The test does not declare a variant version of foo().
; Check that this does not cause compilation to fail.
;
; void foo() { }
; void bar() {
;   #pragma omp target variant dispatch
;   {
;      foo();
;   }
; }
;
; Check for the debug string
; CHECK: Pass:{{[ ]*}}openmp
; CHECK: Construct:{{[ ]*}}target variant dispatch
; CHECK: String:{{[ ]*}}' Could not find a matching variant function'
;
source_filename = "lit.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  ret void
}

define dso_local void @bar() #1 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  call void @foo() #2
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.2

DIR.OMP.END.TARGET.VARIANT.DISPATCH.2:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}

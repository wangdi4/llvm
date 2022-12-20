; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -S < %s | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring)" -S < %s | FileCheck %s

; ModuleID = 'cfg_restruct_test.c'
source_filename = "cfg_restruct_test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo() #0 {
entry:

;CHECK: %[[TOKEN:[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
;CHECK-NEXT: br label %DIR.OMP.BARRIER.[[LABEL1:[0-9]+]]
;CHECK: DIR.OMP.BARRIER.[[LABEL1]]:{{.*}}
;CHECK-NEXT: call void @llvm.directive.region.exit(token %[[TOKEN]]) [ "DIR.OMP.END.BARRIER"() ]
;CHECK-NEXT: br label %DIR.OMP.END.BARRIER.[[LABEL2:[0-9]+]]
;CHECK: DIR.OMP.END.BARRIER.[[LABEL2]]:{{.*}}

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.BARRIER"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0"}

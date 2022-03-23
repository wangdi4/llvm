; RUN: opt -vpo-paropt-prepare -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -S %s 2>&1 | FileCheck %s

; Test src:
;
; int a();
; void b() {
;   if (a())
;     goto L1;
;   #pragma omp master
;     ;
;   L1:
; }

; CHECK-NOT: DominatorTree update failed after Master codegen
; CHECK: call i32 @__kmpc_masked({{.*}})
; CHECK: call void @__kmpc_end_masked({{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @b() #0 {
entry:
  br label %outer.bb1

outer.bb1:                                    ; preds = %entry
  br i1 false, label %outer.bb2, label %outer.bb3

outer.bb2:                                    ; preds = %outer.bb1
  br label %region.entry

region.entry:                                 ; preds = %outer.bb2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  br label %body

body:                                         ; preds = %region.entry
  fence acquire
  fence release
  br label %region.exit

region.exit:                                  ; preds = %body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.MASTER"() ]
  br label %outer.bb3

outer.bb3:                                    ; preds = %outer.bb1, %region.exit
  br label %exit

exit:                                         ; preds = %outer.bb3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}

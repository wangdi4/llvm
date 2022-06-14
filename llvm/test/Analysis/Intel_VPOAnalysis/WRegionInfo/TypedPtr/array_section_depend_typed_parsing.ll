; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt -enable-new-pm=0 -opaque-pointers -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck --check-prefix=OPQPTR %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck --check-prefix=OPQPTR %s

; Original code:
;int main() {
;  int a[10];
;  int n = 3;
;#pragma omp taskwait depend(in: a[n:n])
;}

; CHECK: DEPEND clause (size=1): ([10 x i32]* %a, TYPED (TYPE: i32, NUM_ELEMENTS: i64 %conv, ARRAY SECTION OFFSET: i64 %conv1))
; OPQPTR: DEPEND clause (size=1): (ptr %a, TYPED (TYPE: i32, NUM_ELEMENTS: i64 %conv, ARRAY SECTION OFFSET: i64 %conv1))

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %a = alloca [10 x i32], align 16
  %n = alloca i32, align 4
  store i32 3, i32* %n, align 4, !tbaa !4
  %0 = load i32, i32* %n, align 4, !tbaa !4
  %conv = sext i32 %0 to i64
  %1 = load i32, i32* %n, align 4, !tbaa !4
  %conv1 = sext i32 %1 to i64
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(), "QUAL.OMP.DEPEND.IN:ARRSECT.TYPED"([10 x i32]* %a, i32 0, i64 %conv, i64 %conv1) ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASKWAIT"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 13.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

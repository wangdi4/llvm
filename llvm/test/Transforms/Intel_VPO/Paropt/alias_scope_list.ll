; RUN: opt -switch-to-offload -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %s 2>&1 | FileCheck %s

; Test src:
;
; int main() {
;   int x;
; #pragma omp target map(x)
;   { x += 1; }
;
;   return 0;
; }

; Check that a single alias.scope specifier is wrapped into a list of MD nodes:

; CHECK: load{{.*}}!alias.scope [[LIST:![0-9]+]]
; CHECK: store{{.*}}!alias.scope [[LIST]]
; CHECK: [[LIST]] = !{[[SCOPE:![0-9]+]]}
; CHECK: [[SCOPE]] = distinct !{[[SCOPE]], [[DOMAIN:![0-9]+]], !"OMPAliasScope"}
; CHECK: [[DOMAIN]] = distinct !{[[DOMAIN]], !"OMPDomain"}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

; Function Attrs: convergent noinline nounwind optnone uwtable
define protected i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %x, ptr %x, i64 4, i64 35, ptr null, ptr null) ]
  %1 = load i32, ptr %x, align 4
  %add = add nsw i32 %1, 1
  store i32 %add, ptr %x, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind optnone uwtable "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5, !6}

!0 = !{i32 0, i32 53, i32 -1916119748, !"_Z4main", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"uwtable", i32 2}
!6 = !{i32 7, !"frame-pointer", i32 2}

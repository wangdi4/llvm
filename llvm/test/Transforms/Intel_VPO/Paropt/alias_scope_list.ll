; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s

; Original code:
; int main() {
;   int x;
; #pragma omp target map(x)
;   {
;     x += 1;
;   }
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

; Function Attrs: convergent noinline nounwind uwtable
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = bitcast i32* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32* %x, i32* %x, i64 4, i64 35, i8* null, i8* null) ]
  %2 = load i32, i32* %x, align 4, !tbaa !4
  %add = add nsw i32 %2, 1
  store i32 %add, i32* %x, align 4, !tbaa !4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  %3 = bitcast i32* %x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %3) #2
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { convergent noinline nounwind uwtable "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2053, i32 12451846, !"_Z4main", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

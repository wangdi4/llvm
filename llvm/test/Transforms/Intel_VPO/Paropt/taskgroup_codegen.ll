; RUN: opt -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This file tests the implementation to support OMP taskgroup construct.
; void foo() {}
;
; int main() {
;   char a;
;
;   #pragma omp taskgroup
;     a = 2;
;   #pragma omp taskgroup
;     foo();
;   return a;
; }


target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @_Z3foov() #0 {
entry:
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() #1 {
entry:
  %retval = alloca i32, align 4
  %a = alloca i8, align 1
  store i32 0, i32* %retval, align 4
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %a) #3
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"() ]
  store i8 2, i8* %a, align 1, !tbaa !2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKGROUP"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"() ]
  call void @_Z3foov()
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKGROUP"() ]
  %2 = load i8, i8* %a, align 1, !tbaa !2
  %conv = sext i8 %2 to i32
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %a) #3
  ret i32 %conv
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}

; CHECK:  call void @__kmpc_taskgroup({{.*}})
; CHECK:  call void @__kmpc_end_taskgroup({{.*}})

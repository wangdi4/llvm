; RUN: opt -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; unsigned int NY = 100;
;
; void lbm_3() {
;   NY = 100;
; #pragma omp parallel firstprivate(NY)
;   {
;     NY = 102;
; #pragma omp parallel shared(NY)
;     {
;       printf("NY= %d \n", NY); // should print 102
;     }
;   }
;   return;
; }
;
; int main() {
;   lbm_3();
;   return 0;
; }

; ModuleID = 'test_2.host.cpp'
source_filename = "test_2.host.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@NY = dso_local global i32 100, align 4
@.str = private unnamed_addr constant [9 x i8] c"NY= %d \0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @_Z5lbm_3v() #0 {
entry:
  store i32 100, i32* @NY, align 4, !tbaa !2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"(i32* @NY) ]
  store i32 102, i32* @NY, align 4, !tbaa !2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* @NY) ]

; parallel shared
; CHECK: define internal void @_Z5lbm_3v.DIR.OMP.PARALLEL.{{.*}}(i32* %tid, i32* %bid, i32* %NY)
; CHECK: {{%[0-9]+}} = load i32, i32* %NY, align 4

; parallel firstprivate
; CHECK: define internal void @_Z5lbm_3v.DIR.OMP.PARALLEL.{{.*}}(i32* %tid, i32* %bid, i64 %NY.val.zext)
; CHECK: %NY.fpriv = alloca i32, align 4

  %2 = load i32, i32* @NY, align 4, !tbaa !2
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i32 %2) #1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() #3 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  call void @_Z5lbm_3v()
  ret i32 0
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}

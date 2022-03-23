; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck --check-prefix=OPQPTR %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck --check-prefix=OPQPTR %s

; Original code:
;int main() {
;  int x = 100;
;#pragma omp teams distribute parallel for num_teams(x) thread_limit(x) num_threads(x)
;  for (int i = 0; i < 100; ++i);
;  return 0;
;}
; NUM_TEAMS and THREAD_LIMIT clauses were hand modified to take
; pointer argument with the type specifier.

; CHECK: [[NUMTEAMS:%[0-9A-Za-z._]+]] = load i32, i32* %x
; CHECK: [[THREADLIMIT:%[0-9A-Za-z._]+]] = load i32, i32* %x
; CHECK: call void @__kmpc_push_num_teams(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 [[NUMTEAMS]], i32 [[THREADLIMIT]])

; OPQPTR: [[NUMTEAMS:%[0-9A-Za-z._]+]] = load i32, ptr %x
; OPQPTR: [[THREADLIMIT:%[0-9A-Za-z._]+]] = load i32, ptr %x
; OPQPTR: call void @__kmpc_push_num_teams(ptr @{{.*}}, i32 %{{.*}}, i32 [[NUMTEAMS]], i32 [[THREADLIMIT]])


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 100, i32* %x, align 4, !tbaa !4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS:TYPED"(i32* %x, i32 0), "QUAL.OMP.THREAD_LIMIT:TYPED"(i32* %x, i32 0) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
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

; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck --check-prefix=OPQPTR %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck --check-prefix=OPQPTR %s

; Original code:
;int main() {
;  int a[10];
;  int n = 3;
;#pragma omp taskwait depend(in: a[n:n])
;}

; CHECK: [[BASE:%[0-9A-Za-z._]+]] = bitcast [10 x i32]* %a to i32*
; CHECK: [[START:%[0-9A-Za-z._]+]] = getelementptr i32, i32* [[BASE]], i64 %conv1
; CHECK: [[SIZE:%[0-9A-Za-z._]+]] = mul i64 %conv, 4
; CHECK: [[DEPBASEPTR:%[0-9A-Za-z._]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* %{{.*}}, i32 0, i32 0
; CHECK: [[STARTCAST:%[0-9A-Za-z._]+]] = ptrtoint i32* [[START]] to i64
; CHECK: store i64 [[STARTCAST]], i64* [[DEPBASEPTR]]
; CHECK: [[DEPNUMBYTES:%[0-9A-Za-z._]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* %{{.*}}, i32 0, i32 1
; CHECK: store i64 [[SIZE]], i64* [[DEPNUMBYTES]]
; CHECK:  call void @__kmpc_omp_wait_deps(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 1, i8* %{{.*}}, i32 0, i8* null)
; CHECK:  call void @__kmpc_omp_taskwait(%struct.ident_t* @{{.*}}, i32 %{{.*}})

; OPQPTR: [[START:%[0-9A-Za-z._]+]] = getelementptr i32, ptr %a, i64 %conv1
; OPQPTR: [[SIZE:%[0-9A-Za-z._]+]] = mul i64 %conv, 4
; OPQPTR: [[DEPBASEPTR:%[0-9A-Za-z._]+]] = getelementptr inbounds %__struct.kmp_depend_info, ptr %{{.*}}, i32 0, i32 0
; OPQPTR: [[STARTCAST:%[0-9A-Za-z._]+]] = ptrtoint ptr [[START]] to i64
; OPQPTR: store i64 [[STARTCAST]], ptr [[DEPBASEPTR]]
; OPQPTR: [[DEPNUMBYTES:%[0-9A-Za-z._]+]] = getelementptr inbounds %__struct.kmp_depend_info, ptr %{{.*}}, i32 0, i32 1
; OPQPTR: store i64 [[SIZE]], ptr [[DEPNUMBYTES]]
; OPQPTR:  call void @__kmpc_omp_wait_deps(ptr @{{.*}}, i32 %{{.*}}, i32 1, ptr %{{.*}}, i32 0, ptr null)
; OPQPTR:  call void @__kmpc_omp_taskwait(ptr @{{.*}}, i32 %{{.*}})

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

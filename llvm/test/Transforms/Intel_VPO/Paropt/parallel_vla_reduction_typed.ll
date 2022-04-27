; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -verify -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,verify' -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -verify -S %s | FileCheck --check-prefix=OPQPTR %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,verify' -S %s | FileCheck --check-prefix=OPQPTR %s

; Original code:
;void foo(int n) {
;  int s[n];
;#pragma omp parallel reduction(+: s)
;  ;
;}

; CHECK: [[VLASIZEVAR:@[A-Za-z0-9_.]+]] = common thread_local global i64 0
; CHECK: define{{.*}}@foo_tree_reduce_2(i8* [[DST:%[A-Za-z0-9_.]+]], i8* [[SRC:%[A-Za-z0-9_.]+]]) {
; CHECK: [[VLASIZE:%[A-Za-z0-9_.]+]] = load i64, i64* [[VLASIZEVAR]]

; OPQPTR: [[VLASIZEVAR:@[A-Za-z0-9_.]+]] = common thread_local global i64 0
; OPQPTR: define{{.*}}@foo_tree_reduce_2(ptr [[DST:%[A-Za-z0-9_.]+]], ptr [[SRC:%[A-Za-z0-9_.]+]]) {
; OPQPTR: [[VLASIZE:%[A-Za-z0-9_.]+]] = load i64, ptr [[VLASIZEVAR]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  store i32 %n, i32* %n.addr, align 4, !tbaa !4
  %0 = load i32, i32* %n.addr, align 4, !tbaa !4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8** %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, i64* %__vla_expr0, align 8
  store i64 %1, i64* %omp.vla.tmp, align 8, !tbaa !8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(i32* %vla, i32 0, i64 %1), "QUAL.OMP.SHARED"(i64* %omp.vla.tmp) ]
  %4 = load i64, i64* %omp.vla.tmp, align 8
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]
  %5 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %5)
  ret void
}

; Function Attrs: nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8*) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

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
!8 = !{!9, !9, i64 0}
!9 = !{!"long", !6, i64 0}

; RUN: opt -passes="early-cse" -S %s | FileCheck %s

; When OpenMP directives are present, do not CSE threadlocal.address
; intrinsics, as their value may change depending on the OpenMP state.

;__thread int x;
;int main () {
;  int p;
;  x = 6; // address of "x" depends on TLS
;#pragma omp parallel
;  {
;    x = 5;  // this "x" will have a different TLS address than above
;  }
;  return 0;
;}

; CHECK-LABEL: @nocse
; CHECK-LABEL: entry
; CHECK: call{{.*}}llvm.threadlocal.address
; CHECK: DIR.OMP.PARALLEL.3
; CHECK: call{{.*}}llvm.threadlocal.address

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = external dso_local thread_local global i32, align 4

; Function Attrs: mustprogress norecurse nounwind uwtable
define dso_local noundef i32 @nocse() #0 {
entry:
  %0 = call ptr @llvm.threadlocal.address.p0(ptr @x)
  store i32 6, ptr %0, align 4, !tbaa !4
  %1 = call ptr @llvm.threadlocal.address.p0(ptr @x)
  %.addr = alloca ptr, align 8
  store ptr %1, ptr %.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
 "QUAL.OMP.SHARED:TYPED"(ptr %1, i32 0, i32 1),
 "QUAL.OMP.OPERAND.ADDR"(ptr %1, ptr %.addr),
 "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  %cmp = icmp ne i1 %temp.load, false
  br i1 %cmp, label %DIR.OMP.END.PARALLEL.4.split, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %entry
  %3 = call ptr @llvm.threadlocal.address.p0(ptr @x)
  store i32 5, ptr %3, align 4, !tbaa !4
  br label %DIR.OMP.END.PARALLEL.4.split

DIR.OMP.END.PARALLEL.4.split:                     ; preds = %DIR.OMP.PARALLEL.3, %entry
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

; This function does not have OpenMP; we expect CSE to work as usual.
; CHECK-LABEL: @cse
; CHECK-LABEL: entry
; CHECK: call{{.*}}llvm.threadlocal
; CHECK-NOT: call{{.*}}llvm.threadlocal
; CHECK-LABEL: successor
; CHECK-NOT: call{{.*}}llvm.threadlocal

define dso_local noundef i32 @cse() {
entry:
  %0 = call ptr @llvm.threadlocal.address.p0(ptr @x)
  store i32 6, ptr %0, align 4, !tbaa !4
  %1 = call ptr @llvm.threadlocal.address.p0(ptr @x)
  %.addr = alloca ptr, align 8
  store volatile ptr %1, ptr %.addr, align 8
  br i1 undef, label %exitbb, label %successor

successor:
  %2 = call ptr @llvm.threadlocal.address.p0(ptr @x)
  store i32 5, ptr %2, align 4, !tbaa !4
  br label %exitbb

exitbb:
  ret i32 0
}


; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare ptr @llvm.threadlocal.address.p0(ptr) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { mustprogress norecurse nounwind uwtable "may-have-openmp-directive"="true" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}

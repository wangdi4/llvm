; RUN: opt -opaque-pointers=1 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; class A {
; public:
;   A();
; };
; void fn1() {
;   A e[100];
; #pragma omp task firstprivate(e)
;   for (int d = 0; d < 100; d++)
;     ;
; }

; This test is used to check emitting loop of calling copy constructor and
; destructor for fixed size array type with task construct + firstprivate
; clause.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.A = type { i8 }

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z3fn1v() #0 {
entry:
  %e = alloca [100 x %class.A], align 16
  %d = alloca i32, align 4
  %array.begin = getelementptr inbounds [100 x %class.A], ptr %e, i32 0, i32 0
  %arrayctor.end = getelementptr inbounds %class.A, ptr %array.begin, i64 100
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %entry
  %arrayctor.cur = phi ptr [ %array.begin, %entry ], [ %arrayctor.next, %arrayctor.loop ]
  call void @_ZN1AC1Ev(ptr noundef nonnull align 1 dereferenceable(1) %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.A, ptr %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %arrayctor.loop
  %array.begin1 = getelementptr inbounds [100 x %class.A], ptr %e, i32 0, i32 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr %e, %class.A zeroinitializer, i64 100, ptr @_ZTS1A.omp.copy_constr, ptr @_ZTS1A.omp.destr),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %d, i32 0, i32 1) ]
  store i32 0, ptr %d, align 4
  br label %for.cond

; Copy Constructor
; CHECK:  %[[TO:[^,]+]] = getelementptr inbounds [100 x %class.A], ptr %e.priv.gep, i32 0
; CHECK-NEXT:  %[[FROM:[^,]+]] = getelementptr inbounds [100 x %class.A], ptr %e, i32 0
; CHECK-NEXT:  %[[END:[^,]+]] = getelementptr %class.A, ptr %[[TO]], i64 100
; CHECK-NEXT:  %priv.cpyctor.isempty = icmp eq ptr %[[TO]], %[[END]]
; CHECK-NEXT:  br i1 %priv.cpyctor.isempty, label %priv.cpyctor.done, label %priv.cpyctor.body
; CHECK-LABEL: priv.cpyctor.body:
; CHECK-NEXT:  %priv.cpy.dest.ptr = phi ptr [ %[[TO]], %{{.*}} ], [ %priv.cpy.dest.inc, %priv.cpyctor.body ]
; CHECK-NEXT:  %priv.cpy.src.ptr = phi ptr [ %[[FROM]], %{{.*}} ], [ %priv.cpy.src.inc, %priv.cpyctor.body ]
; CHECK-NEXT:  call void @_ZTS1A.omp.copy_constr(ptr %priv.cpy.dest.ptr, ptr %priv.cpy.src.ptr)
; CHECK-NEXT:  %priv.cpy.dest.inc = getelementptr %class.A, ptr %priv.cpy.dest.ptr, i32 1
; CHECK-NEXT:  %priv.cpy.src.inc = getelementptr %class.A, ptr %priv.cpy.src.ptr, i32 1
; CHECK-NEXT:  %priv.cpy.done = icmp eq ptr %priv.cpy.dest.inc, %[[END]]
; CHECK-NEXT:  br i1 %priv.cpy.done, label %priv.cpyctor.done, label %priv.cpyctor.body
; CHECK-LABEL: priv.cpyctor.done:
; CHECK-NEXT:  br label %{{.*}}

; Destructor
; CHECK:  %[[DESTR_BEGIN:[^,]+]] = getelementptr inbounds [100 x %class.A], ptr %{{.*}}, i32 0
; CHECK-NEXT:  %[[DESTR_END:[^,]+]] = getelementptr %class.A, ptr %[[DESTR_BEGIN]], i64 100
; CHECK-NEXT:  %priv.destr.isempty = icmp eq ptr %[[DESTR_BEGIN]], %[[DESTR_END]]
; CHECK-NEXT:  br i1 %priv.destr.isempty, label %priv.destr.done, label %priv.destr.body
; CHECK-LABEL: priv.destr.body:
; CHECK-NEXT:  %priv.cpy.dest.ptr{{.*}} = phi ptr [ %[[DESTR_BEGIN]], %{{.*}} ], [ %priv.cpy.dest.inc{{.*}}, %priv.destr.body ]
; CHECK:  call void @_ZTS1A.omp.destr(ptr %priv.cpy.dest.ptr{{.*}})
; CHECK:  %priv.cpy.dest.inc{{.*}} = getelementptr %class.A, ptr %priv.cpy.dest.ptr{{.*}}, i32 1
; CHECK:  %priv.cpy.done{{.*}} = icmp eq ptr %priv.cpy.dest.inc{{.*}}, %[[DESTR_END]]
; CHECK:  br i1 %priv.cpy.done{{.*}}, label %priv.destr.done, label %priv.destr.body
; CHECK-LABEL: priv.destr.done:
; CHECK-NEXT:  ret i32 0

for.cond:                                         ; preds = %for.inc, %arrayctor.cont
  %1 = load i32, ptr %d, align 4
  %cmp = icmp slt i32 %1, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %2 = load i32, ptr %d, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, ptr %d, align 4
  br label %for.cond, !llvm.loop !5

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret void
}

declare dso_local void @_ZN1AC1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline uwtable
define internal void @_ZTS1A.omp.copy_constr(ptr noundef %0, ptr noundef %1) #3 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr, align 8
  %3 = load ptr, ptr %.addr1, align 8
  ret void
}

; Function Attrs: noinline uwtable
define internal void @_ZTS1A.omp.destr(ptr noundef %0) #3 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  ret void
}

attributes #0 = { mustprogress noinline optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}

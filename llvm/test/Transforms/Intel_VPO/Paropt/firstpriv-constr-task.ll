; RUN: opt -passes="vpo-paropt" -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; class foo {
; public:
;   static int kount;
;   foo() throw() { i = kount++; }
;   ~foo() { i = -1; }
;   foo (const foo &f) {
;     i = 1000 + f.i;
;   }
;   int i;
; };
;
; int foo::kount = 1;
;
; int __attribute__ ((noinline)) Compute(void)
; {
;   foo x; foo y;
;   y.i = 888;
; #pragma omp parallel
; {
; #pragma omp single
; {
; #pragma omp task firstprivate(x) firstprivate(y)
;   for (int i = 0; i < 1; i++) {
;     x.i += i;
;     y.i += i;
;   }
; }
; }
;   return x.i;
; }

; OMP task with 2 firstprivate objects. Verify copy constructor and destructor.

; clang creates wrappers for the object copy constructor and destructor.

; CHECK: define{{.*}}copy_constr
; CHECK: call{{.*}}fooC
; CHECK: ret void
; CHECK: define{{.*}}foo.omp.destr
; CHECK: call{{.*}}fooD
; CHECK: ret void

; Destructor thunk "dtor_thunk" is created which indexes the given kmp_task_t
; pointer to find the objects that need to be destructed, and calls the
; destructor wrapper on the objects.

; CHECK: define{{.*}}dtor_thunk
; CHECK-DAG: [[XADDR:%[0-9]+]] = getelementptr{{.*}}kmp_task_t_with_privates{{.*}} i32 0, i32 1, i32 0
; CHECK-DAG: [[YADDR:%[0-9]+]] = getelementptr{{.*}}kmp_task_t_with_privates{{.*}} i32 0, i32 1, i32 1
; CHECK-DAG: call{{.*}}foo.omp.destr{{.*}}[[XADDR]]
; CHECK-DAG: call{{.*}}foo.omp.destr{{.*}}[[YADDR]]
; CHECK: ret i32

; The kmp_task_t thunk is allocated with the "0x9" flags which indicates that
; the destructor thunk should be called.

; CHECK: omp_task_alloc{{.*}}i32 9

; The destructor thunk pointer is inserted to the right place in the task
; thunk.

; CHECK: [[DTORPTR:%[^ ]+]] = getelementptr{{.*}}struct.kmp_task_t{{.*}}i32 0, i32 3
; CHECK: store{{.*}}dtor_thunk{{.*}}[[DTORPTR]]

; The firstprivate objects are copy-constructed into the local copy in the
; task setup code.

; CHECK: [[PRIVXX:%[^ ]+]] = getelementptr{{.*}}struct.kmp_privates.t
; CHECK-NEXT: call{{.*}}copy_constr{{.*}}[[PRIVXX]]{{.*}}%x
; CHECK: [[PRIVXY:%[^ ]+]] = getelementptr{{.*}}struct.kmp_privates.t
; CHECK-NEXT: call{{.*}}copy_constr{{.*}}[[PRIVXY]]{{.*}}%y

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.foo = type { i32 }

$_ZN3fooC2Ev = comdat any

$_ZN3fooC2ERKS_ = comdat any

$_ZN3fooD2Ev = comdat any

@_ZN3foo5kountE = dso_local global i32 1, align 4

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z7Computev() #0 {
entry:
  %x = alloca %class.foo, align 4
  %y = alloca %class.foo, align 4
  %i1 = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %x) #3
  call void @_ZN3fooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %x) #3
  call void @llvm.lifetime.start.p0(i64 4, ptr %y) #3
  call void @_ZN3fooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %y) #3
  %i = getelementptr inbounds %class.foo, ptr %y, i32 0, i32 0, !intel-tbaa !3
  store i32 888, ptr %i, align 4, !tbaa !3
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %x, %class.foo zeroinitializer, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %y, %class.foo zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i1, i32 0, i32 1) ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  br label %DIR.OMP.SINGLE.4

DIR.OMP.SINGLE.4:                                 ; preds = %DIR.OMP.PARALLEL.3
  fence acquire
  br label %DIR.OMP.TASK.5

DIR.OMP.TASK.5:                                   ; preds = %DIR.OMP.SINGLE.4
  br label %DIR.OMP.TASK.6

DIR.OMP.TASK.6:                                   ; preds = %DIR.OMP.TASK.5
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr %x, %class.foo zeroinitializer, i32 1, ptr @_ZTS3foo.omp.copy_constr, ptr @_ZTS3foo.omp.destr),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr %y, %class.foo zeroinitializer, i32 1, ptr @_ZTS3foo.omp.copy_constr, ptr @_ZTS3foo.omp.destr),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i1, i32 0, i32 1) ]
  br label %DIR.OMP.TASK.7

DIR.OMP.TASK.7:                                   ; preds = %DIR.OMP.TASK.6
  call void @llvm.lifetime.start.p0(i64 4, ptr %i1) #3
  store i32 0, ptr %i1, align 4, !tbaa !8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %DIR.OMP.TASK.7
  %3 = load i32, ptr %i1, align 4, !tbaa !8
  %cmp = icmp slt i32 %3, 1
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0(i64 4, ptr %i1) #3
  br label %for.end

for.body:                                         ; preds = %for.cond
  %4 = load i32, ptr %i1, align 4, !tbaa !8
  %i2 = getelementptr inbounds %class.foo, ptr %x, i32 0, i32 0, !intel-tbaa !3
  %5 = load i32, ptr %i2, align 4, !tbaa !3
  %add = add nsw i32 %5, %4
  store i32 %add, ptr %i2, align 4, !tbaa !3
  %6 = load i32, ptr %i1, align 4, !tbaa !8
  %i3 = getelementptr inbounds %class.foo, ptr %y, i32 0, i32 0, !intel-tbaa !3
  %7 = load i32, ptr %i3, align 4, !tbaa !3
  %add4 = add nsw i32 %7, %6
  store i32 %add4, ptr %i3, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %8 = load i32, ptr %i1, align 4, !tbaa !8
  %inc = add nsw i32 %8, 1
  store i32 %inc, ptr %i1, align 4, !tbaa !8
  br label %for.cond, !llvm.loop !9

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.8

DIR.OMP.END.TASK.8:                               ; preds = %for.end
  fence release
  br label %DIR.OMP.END.SINGLE.9

DIR.OMP.END.SINGLE.9:                             ; preds = %DIR.OMP.END.TASK.8
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SINGLE"() ]
  br label %DIR.OMP.END.SINGLE.10

DIR.OMP.END.SINGLE.10:                            ; preds = %DIR.OMP.END.SINGLE.9
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.11

DIR.OMP.END.PARALLEL.11:                          ; preds = %DIR.OMP.END.SINGLE.10
  %i5 = getelementptr inbounds %class.foo, ptr %x, i32 0, i32 0, !intel-tbaa !3
  %9 = load i32, ptr %i5, align 4, !tbaa !3
  call void @_ZN3fooD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %y) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %y) #3
  call void @_ZN3fooD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %x) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %x) #3
  ret i32 %9
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3fooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8, !tbaa !11
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load i32, ptr @_ZN3foo5kountE, align 4, !tbaa !8
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr @_ZN3foo5kountE, align 4, !tbaa !8
  %i = getelementptr inbounds %class.foo, ptr %this1, i32 0, i32 0, !intel-tbaa !3
  store i32 %0, ptr %i, align 4, !tbaa !3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: uwtable
define internal void @_ZTS3foo.omp.copy_constr(ptr noundef %0, ptr noundef %1) #4 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8, !tbaa !11
  store ptr %1, ptr %.addr1, align 8, !tbaa !11
  %2 = load ptr, ptr %.addr, align 8
  %3 = load ptr, ptr %.addr1, align 8, !tbaa !11
  call void @_ZN3fooC2ERKS_(ptr noundef nonnull align 4 dereferenceable(4) %2, ptr noundef nonnull align 4 dereferenceable(4) %3)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3fooC2ERKS_(ptr noundef nonnull align 4 dereferenceable(4) %this, ptr noundef nonnull align 4 dereferenceable(4) %f) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %f.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8, !tbaa !11
  store ptr %f, ptr %f.addr, align 8, !tbaa !13
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %f.addr, align 8, !tbaa !13
  %i = getelementptr inbounds %class.foo, ptr %0, i32 0, i32 0, !intel-tbaa !3
  %1 = load i32, ptr %i, align 4, !tbaa !3
  %add = add nsw i32 1000, %1
  %i2 = getelementptr inbounds %class.foo, ptr %this1, i32 0, i32 0, !intel-tbaa !3
  store i32 %add, ptr %i2, align 4, !tbaa !3
  ret void
}

; Function Attrs: uwtable
define internal void @_ZTS3foo.omp.destr(ptr noundef %0) #4 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8, !tbaa !11
  %1 = load ptr, ptr %.addr, align 8
  call void @_ZN3fooD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %1) #3
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3fooD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8, !tbaa !11
  %this1 = load ptr, ptr %this.addr, align 8
  %i = getelementptr inbounds %class.foo, ptr %this1, i32 0, i32 0, !intel-tbaa !3
  store i32 -1, ptr %i, align 4, !tbaa !3
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { mustprogress noinline nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }
attributes #4 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!4, !5, i64 0}
!4 = !{!"struct@_ZTS3foo", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!5, !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = !{!12, !12, i64 0}
!12 = !{!"pointer@_ZTSP3foo", !6, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"any pointer", !6, i64 0}

; OMP taskloop with 2 firstprivate objects. Verify copy constructor and
; destructor.

; RUN: opt -passes="vpo-paropt" -S %s | FileCheck %s

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

; Copy-thunk "task_dup" is created to copy-construct the 2 objects into the
; spawned taskloop tasks.

; CHECK: define{{.*}}task_dup
; CHECK-DAG: [[XDST:%[0-9]+]] = getelementptr{{.*}}kmp_task_t_with_privates{{.*}} %0, i32 0, i32 1, i32 0
; CHECK-DAG: [[XSRC:%[0-9]+]] = getelementptr{{.*}}kmp_task_t_with_privates{{.*}} %1, i32 0, i32 1, i32 0
; CHECK: call{{.*}}copy_constr{{.*}}[[XDST]]{{.*}}[[XSRC]]
; CHECK-DAG: [[YDST:%[0-9]+]] = getelementptr{{.*}}kmp_task_t_with_privates{{.*}} %0, i32 0, i32 1, i32 1
; CHECK-DAG: [[YSRC:%[0-9]+]] = getelementptr{{.*}}kmp_task_t_with_privates{{.*}} %1, i32 0, i32 1, i32 1
; CHECK: call{{.*}}copy_constr{{.*}}[[YDST]]{{.*}}[[YSRC]]
; CHECK: ret void


; The kmp_task_t thunk is allocated with the "0x9" flag which indicates that
; the destructor thunk should be called.

; CHECK: omp_task_alloc{{.*}}i32 9

; The destructor thunk pointer is inserted to the right place in the task
; thunk.

; CHECK: [[DTORPTR:%[^ ]+]] = getelementptr{{.*}} i32 0, i32 3
; CHECK: store{{.*}}dtor_thunk{{.*}}[[DTORPTR]]

; The firstprivate objects are copy-constructed into the local copy in the
; task setup code.

; CHECK: [[PRIVXX:%[^ ]+]] = getelementptr{{.*}}struct.kmp_privates.t
; CHECK-NEXT: call{{.*}}copy_constr{{.*}}[[PRIVXX]]{{.*}}%x
; CHECK: [[PRIVXY:%[^ ]+]] = getelementptr{{.*}}struct.kmp_privates.t
; CHECK-NEXT: call{{.*}}copy_constr{{.*}}[[PRIVXY]]{{.*}}%y

; The copy-thunk is passed to kmpc_taskloop.

; CHECK: call{{.*}}kmpc_taskloop_5{{.*}}task_dup

; Source code:
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
; #pragma omp taskloop firstprivate(x) firstprivate(y) num_tasks(2)
;   for (int i = 0; i < 1; i++) {
;     x.i += i;
;     y.i += i;
;   }
; }
; }
;
; return x.i;
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.foo = type { i32 }

$_ZN3fooC2Ev = comdat any

$_ZN3fooC2ERKS_ = comdat any

$_ZN3fooD2Ev = comdat any

$__clang_call_terminate = comdat any

@_ZN3foo5kountE = dso_local global i32 1, align 4

; Function Attrs: noinline uwtable
define dso_local i32 @_Z7Computev() #0 personality ptr @__gxx_personality_v0 {
entry:
  %x = alloca %class.foo, align 4
  %y = alloca %class.foo, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i2 = alloca i32, align 4
  %exn.slot = alloca ptr
  %ehselector.slot = alloca i32
  call void @llvm.lifetime.start.p0(i64 4, ptr %x) #3
  call void @_ZN3fooC2Ev(ptr %x) #3
  call void @llvm.lifetime.start.p0(i64 4, ptr %y) #3
  call void @_ZN3fooC2Ev(ptr %y) #3
  store i32 888, ptr %y, align 4, !tbaa !2
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i64 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i2, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i64 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %x, %class.foo zeroinitializer, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %y, %class.foo zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  br label %DIR.OMP.SINGLE.3

DIR.OMP.SINGLE.3:                                 ; preds = %DIR.OMP.PARALLEL.2
  fence acquire
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #3
  call void @llvm.lifetime.start.p0(i64 8, ptr %.omp.lb) #3
  store i64 0, ptr %.omp.lb, align 8, !tbaa !7
  call void @llvm.lifetime.start.p0(i64 8, ptr %.omp.ub) #3
  store i64 0, ptr %.omp.ub, align 8, !tbaa !7
  br label %DIR.OMP.TASKLOOP.4

DIR.OMP.TASKLOOP.4:                               ; preds = %DIR.OMP.SINGLE.3
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr %x, %class.foo zeroinitializer, i32 1, ptr @_ZTS3foo.omp.copy_constr, ptr @_ZTS3foo.omp.destr),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr %y, %class.foo zeroinitializer, i32 1, ptr @_ZTS3foo.omp.copy_constr, ptr @_ZTS3foo.omp.destr),
    "QUAL.OMP.NUM_TASKS"(i32 2),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i2, i32 0, i32 1) ]
  br label %DIR.OMP.TASKLOOP.5

DIR.OMP.TASKLOOP.5:                               ; preds = %DIR.OMP.TASKLOOP.4
  %3 = load i64, ptr %.omp.lb, align 8, !tbaa !7
  %conv = trunc i64 %3 to i32
  store i32 %conv, ptr %.omp.iv, align 4, !tbaa !9
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.TASKLOOP.5
  %4 = load i32, ptr %.omp.iv, align 4, !tbaa !9
  %conv1 = sext i32 %4 to i64
  %5 = load i64, ptr %.omp.ub, align 8, !tbaa !7
  %cmp = icmp ule i64 %conv1, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i2) #3
  %6 = load i32, ptr %.omp.iv, align 4, !tbaa !9
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i2, align 4, !tbaa !9
  %7 = load i32, ptr %i2, align 4, !tbaa !9
  %8 = load i32, ptr %x, align 4, !tbaa !2
  %add4 = add nsw i32 %8, %7
  store i32 %add4, ptr %x, align 4, !tbaa !2
  %9 = load i32, ptr %i2, align 4, !tbaa !9
  %10 = load i32, ptr %y, align 4, !tbaa !2
  %add6 = add nsw i32 %10, %9
  store i32 %add6, ptr %y, align 4, !tbaa !2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %i2) #3
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, ptr %.omp.iv, align 4, !tbaa !9
  %add7 = add nsw i32 %11, 1
  store i32 %add7, ptr %.omp.iv, align 4, !tbaa !9
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASKLOOP"() ]
  br label %DIR.OMP.END.TASKLOOP.6

DIR.OMP.END.TASKLOOP.6:                           ; preds = %omp.loop.exit
  call void @llvm.lifetime.end.p0(i64 8, ptr %.omp.ub) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr %.omp.lb) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #3
  fence release
  br label %DIR.OMP.END.SINGLE.7

DIR.OMP.END.SINGLE.7:                             ; preds = %DIR.OMP.END.TASKLOOP.6
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SINGLE"() ]
  br label %DIR.OMP.END.SINGLE.8

DIR.OMP.END.SINGLE.8:                             ; preds = %DIR.OMP.END.SINGLE.7
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.END.SINGLE.8
  %12 = load i32, ptr %x, align 4, !tbaa !2
  invoke void @_ZN3fooD2Ev(ptr %y)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %DIR.OMP.END.PARALLEL.9
  call void @llvm.lifetime.end.p0(i64 4, ptr %y) #3
  call void @_ZN3fooD2Ev(ptr %x)
  call void @llvm.lifetime.end.p0(i64 4, ptr %x) #3
  ret i32 %12

lpad:                                             ; preds = %DIR.OMP.END.PARALLEL.9
  %13 = landingpad { ptr, i32 }
          cleanup
  %14 = extractvalue { ptr, i32 } %13, 0
  store ptr %14, ptr %exn.slot, align 8
  %15 = extractvalue { ptr, i32 } %13, 1
  store i32 %15, ptr %ehselector.slot, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr %y) #3
  invoke void @_ZN3fooD2Ev(ptr %x)
          to label %invoke.cont9 unwind label %terminate.lpad

invoke.cont9:                                     ; preds = %lpad
  call void @llvm.lifetime.end.p0(i64 4, ptr %x) #3
  br label %eh.resume

eh.resume:                                        ; preds = %invoke.cont9
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val10 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val10

terminate.lpad:                                   ; preds = %lpad
  %16 = landingpad { ptr, i32 }
          catch ptr null
  %17 = extractvalue { ptr, i32 } %16, 0
  call void @__clang_call_terminate(ptr %17) #6
  unreachable
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3fooC2Ev(ptr %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8, !tbaa !10
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load i32, ptr @_ZN3foo5kountE, align 4, !tbaa !9
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr @_ZN3foo5kountE, align 4, !tbaa !9
  store i32 %0, ptr %this1, align 4, !tbaa !2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: uwtable
define internal void @_ZTS3foo.omp.copy_constr(ptr, ptr) #4 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8, !tbaa !10
  store ptr %1, ptr %.addr1, align 8, !tbaa !10
  %2 = load ptr, ptr %.addr, align 8
  %3 = load ptr, ptr %.addr1, align 8, !tbaa !10
  call void @_ZN3fooC2ERKS_(ptr %2, ptr dereferenceable(4) %3)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3fooC2ERKS_(ptr %this, ptr dereferenceable(4) %f) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %f.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8, !tbaa !10
  store ptr %f, ptr %f.addr, align 8, !tbaa !12
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %f.addr, align 8, !tbaa !12
  %1 = load i32, ptr %0, align 4, !tbaa !2
  %add = add nsw i32 1000, %1
  store i32 %add, ptr %this1, align 4, !tbaa !2
  ret void
}

; Function Attrs: uwtable
define internal void @_ZTS3foo.omp.destr(ptr) #4 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8, !tbaa !10
  %1 = load ptr, ptr %.addr, align 8
  call void @_ZN3fooD2Ev(ptr %1)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3fooD2Ev(ptr %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8, !tbaa !10
  %this1 = load ptr, ptr %this.addr, align 8
  store i32 -1, ptr %this1, align 4, !tbaa !2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(ptr) #5 comdat {
  %2 = call ptr @__cxa_begin_catch(ptr %0) #3
  call void @_ZSt9terminatev() #6
  unreachable
}

declare dso_local ptr @__cxa_begin_catch(ptr)

declare dso_local void @_ZSt9terminatev()

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { noinline noreturn nounwind }
attributes #6 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@_ZTS3foo", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"long", !5, i64 0}
!9 = !{!4, !4, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"pointer@_ZTSP3foo", !5, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"any pointer", !5, i64 0}

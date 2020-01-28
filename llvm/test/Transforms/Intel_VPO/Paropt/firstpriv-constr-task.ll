; OMP task with 2 firstprivate objects. Verify copy constructor and
; destructor.
; RUN: opt < %s  -vpo-paropt  -S | FileCheck %s

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

; The kmp_task_t thunk is allocated with the "9" flag which indicates that
; the destructor thunk should be called.

; CHECK: omp_task_alloc{{.*}}i32 9

; The destructor thunk pointer is inserted to the right place in the task
; thunk.

; CHECK: [[DTORPTR:%[^ ]+]] = getelementptr{{.*}}struct.kmp_task_t{{.*}}i32 0, i32 3, i32 0
; CHECK: store{{.*}}dtor_thunk{{.*}}[[DTORPTR]]

; The firstprivate objects are copy-constructed into the local copy in the
; task setup code.

; CHECK: [[PRIVXX:%[^ ]+]] = getelementptr{{.*}}struct.kmp_privates.t
; CHECK-NEXT: call{{.*}}copy_constr{{.*}}[[PRIVXX]]{{.*}}%x
; CHECK: [[PRIVXY:%[^ ]+]] = getelementptr{{.*}}struct.kmp_privates.t
; CHECK-NEXT: call{{.*}}copy_constr{{.*}}[[PRIVXY]]{{.*}}%y

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
; #pragma omp task firstprivate(x) firstprivate(y)
;   for (int i = 0; i < 1; i++) {
;     x.i += i;
;     y.i += i;
;   }
; }
; }
;   return x.i;
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
define dso_local i32 @_Z7Computev() #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %x = alloca %class.foo, align 4
  %y = alloca %class.foo, align 4
  %i1 = alloca i32, align 4
  %exn.slot = alloca i8*
  %ehselector.slot = alloca i32
  %0 = bitcast %class.foo* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  call void @_ZN3fooC2Ev(%class.foo* %x) #3
  %1 = bitcast %class.foo* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  call void @_ZN3fooC2Ev(%class.foo* %y) #3
  %i = getelementptr inbounds %class.foo, %class.foo* %y, i32 0, i32 0
  store i32 888, i32* %i, align 4, !tbaa !2
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i32* %i1), "QUAL.OMP.SHARED"(%class.foo* %x), "QUAL.OMP.SHARED"(%class.foo* %y) ]
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  br label %DIR.OMP.SINGLE.3

DIR.OMP.SINGLE.3:                                 ; preds = %DIR.OMP.PARALLEL.2
  fence acquire
  br label %DIR.OMP.TASK.4

DIR.OMP.TASK.4:                                   ; preds = %DIR.OMP.SINGLE.3
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%class.foo* %x, void (%class.foo*, %class.foo*)* @_ZTS3foo.omp.copy_constr, void (%class.foo*)* @_ZTS3foo.omp.destr), "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%class.foo* %y, void (%class.foo*, %class.foo*)* @_ZTS3foo.omp.copy_constr, void (%class.foo*)* @_ZTS3foo.omp.destr), "QUAL.OMP.PRIVATE"(i32* %i1) ]
  br label %DIR.OMP.TASK.5

DIR.OMP.TASK.5:                                   ; preds = %DIR.OMP.TASK.4
  %5 = bitcast i32* %i1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #3
  store i32 0, i32* %i1, align 4, !tbaa !7
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %DIR.OMP.TASK.5
  %6 = load i32, i32* %i1, align 4, !tbaa !7
  %cmp = icmp slt i32 %6, 1
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %7 = bitcast i32* %i1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %7) #3
  br label %for.end

for.body:                                         ; preds = %for.cond
  %8 = load i32, i32* %i1, align 4, !tbaa !7
  %i2 = getelementptr inbounds %class.foo, %class.foo* %x, i32 0, i32 0
  %9 = load i32, i32* %i2, align 4, !tbaa !2
  %add = add nsw i32 %9, %8
  store i32 %add, i32* %i2, align 4, !tbaa !2
  %10 = load i32, i32* %i1, align 4, !tbaa !7
  %i3 = getelementptr inbounds %class.foo, %class.foo* %y, i32 0, i32 0
  %11 = load i32, i32* %i3, align 4, !tbaa !2
  %add4 = add nsw i32 %11, %10
  store i32 %add4, i32* %i3, align 4, !tbaa !2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %12 = load i32, i32* %i1, align 4, !tbaa !7
  %inc = add nsw i32 %12, 1
  store i32 %inc, i32* %i1, align 4, !tbaa !7
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.6

DIR.OMP.END.TASK.6:                               ; preds = %for.end
  fence release
  br label %DIR.OMP.END.SINGLE.7

DIR.OMP.END.SINGLE.7:                             ; preds = %DIR.OMP.END.TASK.6
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SINGLE"() ]
  br label %DIR.OMP.END.SINGLE.8

DIR.OMP.END.SINGLE.8:                             ; preds = %DIR.OMP.END.SINGLE.7
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.END.SINGLE.8
  %i5 = getelementptr inbounds %class.foo, %class.foo* %x, i32 0, i32 0
  %13 = load i32, i32* %i5, align 4, !tbaa !2
  invoke void @_ZN3fooD2Ev(%class.foo* %y)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %DIR.OMP.END.PARALLEL.9
  %14 = bitcast %class.foo* %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #3
  call void @_ZN3fooD2Ev(%class.foo* %x)
  %15 = bitcast %class.foo* %x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #3
  ret i32 %13

lpad:                                             ; preds = %DIR.OMP.END.PARALLEL.9
  %16 = landingpad { i8*, i32 }
          cleanup
  %17 = extractvalue { i8*, i32 } %16, 0
  store i8* %17, i8** %exn.slot, align 8
  %18 = extractvalue { i8*, i32 } %16, 1
  store i32 %18, i32* %ehselector.slot, align 4
  %19 = bitcast %class.foo* %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #3
  invoke void @_ZN3fooD2Ev(%class.foo* %x)
          to label %invoke.cont6 unwind label %terminate.lpad

invoke.cont6:                                     ; preds = %lpad
  %20 = bitcast %class.foo* %x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #3
  br label %eh.resume

eh.resume:                                        ; preds = %invoke.cont6
  %exn = load i8*, i8** %exn.slot, align 8
  %sel = load i32, i32* %ehselector.slot, align 4
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn, 0
  %lpad.val7 = insertvalue { i8*, i32 } %lpad.val, i32 %sel, 1
  resume { i8*, i32 } %lpad.val7

terminate.lpad:                                   ; preds = %lpad
  %21 = landingpad { i8*, i32 }
          catch i8* null
  %22 = extractvalue { i8*, i32 } %21, 0
  call void @__clang_call_terminate(i8* %22) #7
  unreachable
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3fooC2Ev(%class.foo* %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %class.foo*, align 8
  store %class.foo* %this, %class.foo** %this.addr, align 8, !tbaa !8
  %this1 = load %class.foo*, %class.foo** %this.addr, align 8
  %0 = load i32, i32* @_ZN3foo5kountE, align 4, !tbaa !7
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* @_ZN3foo5kountE, align 4, !tbaa !7
  %i = getelementptr inbounds %class.foo, %class.foo* %this1, i32 0, i32 0
  store i32 %0, i32* %i, align 4, !tbaa !2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: uwtable
define internal void @_ZTS3foo.omp.copy_constr(%class.foo*, %class.foo*) #4 {
entry:
  %.addr = alloca %class.foo*, align 8
  %.addr1 = alloca %class.foo*, align 8
  store %class.foo* %0, %class.foo** %.addr, align 8, !tbaa !8
  store %class.foo* %1, %class.foo** %.addr1, align 8, !tbaa !8
  %2 = load %class.foo*, %class.foo** %.addr, align 8
  %3 = load %class.foo*, %class.foo** %.addr1, align 8, !tbaa !8
  call void @_ZN3fooC2ERKS_(%class.foo* %2, %class.foo* dereferenceable(4) %3)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3fooC2ERKS_(%class.foo* %this, %class.foo* dereferenceable(4) %f) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %class.foo*, align 8
  %f.addr = alloca %class.foo*, align 8
  store %class.foo* %this, %class.foo** %this.addr, align 8, !tbaa !8
  store %class.foo* %f, %class.foo** %f.addr, align 8, !tbaa !10
  %this1 = load %class.foo*, %class.foo** %this.addr, align 8
  %0 = load %class.foo*, %class.foo** %f.addr, align 8, !tbaa !10
  %i = getelementptr inbounds %class.foo, %class.foo* %0, i32 0, i32 0
  %1 = load i32, i32* %i, align 4, !tbaa !2
  %add = add nsw i32 1000, %1
  %i2 = getelementptr inbounds %class.foo, %class.foo* %this1, i32 0, i32 0
  store i32 %add, i32* %i2, align 4, !tbaa !2
  ret void
}

; Function Attrs: uwtable
define internal void @_ZTS3foo.omp.destr(%class.foo*) #4 section ".text.startup" {
entry:
  %.addr = alloca %class.foo*, align 8
  store %class.foo* %0, %class.foo** %.addr, align 8, !tbaa !8
  %1 = load %class.foo*, %class.foo** %.addr, align 8
  call void @_ZN3fooD2Ev(%class.foo* %1)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3fooD2Ev(%class.foo* %this) unnamed_addr #2 comdat align 2 {
entry:
  %this.addr = alloca %class.foo*, align 8
  store %class.foo* %this, %class.foo** %this.addr, align 8, !tbaa !8
  %this1 = load %class.foo*, %class.foo** %this.addr, align 8
  %i = getelementptr inbounds %class.foo, %class.foo* %this1, i32 0, i32 0
  store i32 -1, i32* %i, align 4, !tbaa !2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8*) #5 comdat {
  %2 = call i8* @__cxa_begin_catch(i8* %0) #3
  call void @_ZSt9terminatev() #7
  unreachable
}

declare dso_local i8* @__cxa_begin_catch(i8*)

declare dso_local void @_ZSt9terminatev()

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { noinline noreturn nounwind }
attributes #6 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@_ZTS3foo", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!4, !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSP3foo", !5, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"any pointer", !5, i64 0}

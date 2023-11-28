; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; class A {
; public:
;   A();
; };
; void fn1(int n) {
;   A e[n];
; #pragma omp parallel for lastprivate(e)
;   for (int d = 0; d < n; d++)
;     ;
; }

; This test is used to check emitting loop of calling constructor, copy
; assignment and destructor for variable length array type with lastprivate
; clause.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.A = type { i8 }

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z3fn1i(i32 noundef %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %omp.vla.tmp = alloca i64, align 8
  %d = alloca i32, align 4
  store i32 %n, ptr %n.addr, align 4
  %0 = load i32, ptr %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr %saved_stack, align 8
  %vla = alloca %class.A, i64 %1, align 16
  store i64 %1, ptr %__vla_expr0, align 8
  %isempty = icmp eq i64 %1, 0
  br i1 %isempty, label %arrayctor.cont, label %new.ctorloop

new.ctorloop:                                     ; preds = %entry
  %arrayctor.end = getelementptr inbounds %class.A, ptr %vla, i64 %1
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %new.ctorloop
  %arrayctor.cur = phi ptr [ %vla, %new.ctorloop ], [ %arrayctor.next, %arrayctor.loop ]
  call void @_ZN1AC1Ev(ptr noundef nonnull align 1 dereferenceable(1) %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.A, ptr %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %entry, %arrayctor.loop
  %3 = load i32, ptr %n.addr, align 4
  store i32 %3, ptr %.capture_expr.0, align 4
  %4 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub nsw i32 %4, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4
  %5 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %5
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %arrayctor.cont
  store i32 0, ptr %.omp.lb, align 4
  %6 = load i32, ptr %.capture_expr.1, align 4
  store i32 %6, ptr %.omp.ub, align 4
  store i64 %1, ptr %omp.vla.tmp, align 8
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:NONPOD.TYPED"(ptr %vla, %class.A zeroinitializer, i64 %1, ptr @_ZTS1A.omp.def_constr, ptr @_ZTS1A.omp.copy_assign, ptr @_ZTS1A.omp.destr),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %d, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.vla.tmp, i64 0, i32 1) ]
  %8 = load i64, ptr %omp.vla.tmp, align 8
  %9 = load i32, ptr %.omp.lb, align 4
  store i32 %9, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

; Constructor
; CHECK:  %[[CONSTR_BEGIN:[^,]+]] = getelementptr inbounds %class.A, ptr %vla.lpriv, i32 0
; CHECK-NEXT:  %[[CONSTR_END:[^,]+]] = getelementptr %class.A, ptr %[[CONSTR_BEGIN]], i64 %{{.*}}
; CHECK-NEXT:  %priv.constr.isempty = icmp eq ptr %[[CONSTR_BEGIN]], %[[CONSTR_END]]
; CHECK-NEXT:  br i1 %priv.constr.isempty, label %priv.constr.done, label %priv.constr.body
; CHECK-LABEL: priv.constr.body:
; CHECK-NEXT:  %priv.cpy.dest.ptr = phi ptr [ %[[CONSTR_BEGIN]], %{{.*}} ], [ %priv.cpy.dest.inc, %priv.constr.body ]
; CHECK-NEXT:  call ptr @_ZTS1A.omp.def_constr(ptr %priv.cpy.dest.ptr)
; CHECK-NEXT:  %priv.cpy.dest.inc = getelementptr %class.A, ptr %priv.cpy.dest.ptr, i32 1
; CHECK-NEXT:  %priv.cpy.done = icmp eq ptr %priv.cpy.dest.inc, %[[CONSTR_END]]
; CHECK-NEXT:  br i1 %priv.cpy.done, label %priv.constr.done, label %priv.constr.body
; CHECK-LABEL: priv.constr.done:
; CHECK-NEXT:  br label %{{.*}}

; Copy Assignment
; CHECK:  %[[TO:[^,]+]] = getelementptr inbounds %class.A, ptr %vla, i32 0
; CHECK-NEXT:  %[[FROM:[^,]+]] = getelementptr inbounds %class.A, ptr %vla.lpriv, i32 0
; CHECK-NEXT:  %[[END:[^,]+]] = getelementptr %class.A, ptr %[[TO]], i64 %{{.*}}
; CHECK-NEXT:  %priv.cpyassn.isempty = icmp eq ptr %[[TO]], %[[END]]
; CHECK-NEXT:  br i1 %priv.cpyassn.isempty, label %priv.cpyassn.done, label %priv.cpyassn.body
; CHECK-LABEL: priv.cpyassn.body:
; CHECK-NEXT:  %priv.cpy.dest.ptr{{.*}} = phi ptr [ %[[TO]], %{{.*}} ], [ %priv.cpy.dest.inc{{.*}}, %priv.cpyassn.body ]
; CHECK-NEXT:  %priv.cpy.src.ptr = phi ptr [ %[[FROM]], %{{.*}} ], [ %priv.cpy.src.inc, %priv.cpyassn.body ]
; CHECK-NEXT:  call void @_ZTS1A.omp.copy_assign(ptr %priv.cpy.dest.ptr{{.*}}, ptr %priv.cpy.src.ptr)
; CHECK-NEXT:  %priv.cpy.dest.inc{{.*}} = getelementptr %class.A, ptr %priv.cpy.dest.ptr{{.*}}, i32 1
; CHECK-NEXT:  %priv.cpy.src.inc = getelementptr %class.A, ptr %priv.cpy.src.ptr, i32 1
; CHECK-NEXT:  %priv.cpy.done{{.*}} = icmp eq ptr %priv.cpy.dest.inc{{.*}}, %[[END]]
; CHECK-NEXT:  br i1 %priv.cpy.done{{.*}}, label %priv.cpyassn.done, label %priv.cpyassn.body
; CHECK-LABEL: priv.cpyassn.done:
; CHECK-NEXT:  br label %{{.*}}

; Destructor
; CHECK:  %[[DESTR_BEGIN:[^,]+]] = getelementptr inbounds %class.A, ptr %vla.lpriv, i32 0
; CHECK-NEXT:  %[[DESTR_END:[^,]+]] = getelementptr %class.A, ptr %[[DESTR_BEGIN]], i64 %{{.*}}
; CHECK-NEXT:  %priv.destr.isempty = icmp eq ptr %[[DESTR_BEGIN]], %[[DESTR_END]]
; CHECK-NEXT:  br i1 %priv.destr.isempty, label %priv.destr.done, label %priv.destr.body
; CHECK-LABEL: priv.destr.body:
; CHECK-NEXT:  %priv.cpy.dest.ptr{{.*}} = phi ptr [ %[[DESTR_BEGIN]], %{{.*}} ], [ %priv.cpy.dest.inc{{.*}}, %priv.destr.body ]
; CHECK:  call void @_ZTS1A.omp.destr(ptr %priv.cpy.dest.ptr{{.*}})
; CHECK:  %priv.cpy.dest.inc{{.*}} = getelementptr %class.A, ptr %priv.cpy.dest.ptr{{.*}}, i32 1
; CHECK:  %priv.cpy.done{{.*}} = icmp eq ptr %priv.cpy.dest.inc{{.*}}, %[[DESTR_END]]
; CHECK:  br i1 %priv.cpy.done{{.*}}, label %priv.destr.done, label %priv.destr.body
; CHECK-LABEL: priv.destr.done:
; CHECK-NEXT:  br label %{{.*}}

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %10 = load i32, ptr %.omp.iv, align 4
  %11 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %10, %11
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %12, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %d, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %13, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %arrayctor.cont
  %14 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %14)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #1

declare dso_local void @_ZN1AC1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: noinline uwtable
define internal noundef ptr @_ZTS1A.omp.def_constr(ptr noundef %0) #4 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  %1 = load ptr, ptr %.addr, align 8
  call void @_ZN1AC1Ev(ptr noundef nonnull align 1 dereferenceable(1) %1)
  ret ptr %1
}

; Function Attrs: noinline uwtable
define internal void @_ZTS1A.omp.copy_assign(ptr noundef %0, ptr noundef %1) #4 {
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
define internal void @_ZTS1A.omp.destr(ptr noundef %0) #4 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #1

attributes #0 = { mustprogress noinline optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }
attributes #4 = { noinline uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}

; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


; This test is used to check emitting loop of calling constructor and destructor
; for fixed size array with private clause.
; class A
; {
; public:
;   A();
; };
; void fn1() {
;   A e[100];
; #pragma omp parallel for private(e)
;   for(int d=0; d<100; d++);
; }


; ModuleID = 'private_fixed_size_array.cpp'
source_filename = "private_fixed_size_array.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.A = type { i8 }

; Function Attrs: noinline optnone uwtable
define dso_local void @_Z3fn1v() #0 {
entry:
  %e = alloca [100 x %class.A], align 16
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %d = alloca i32, align 4
  %array.begin = getelementptr inbounds [100 x %class.A], [100 x %class.A]* %e, i32 0, i32 0
  %arrayctor.end = getelementptr inbounds %class.A, %class.A* %array.begin, i64 100
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %entry
  %arrayctor.cur = phi %class.A* [ %array.begin, %entry ], [ %arrayctor.next, %arrayctor.loop ]
  call void @_ZN1AC1Ev(%class.A* %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.A, %class.A* %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq %class.A* %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %arrayctor.loop
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.PRIVATE:NONPOD"([100 x %class.A]* %e, %class.A* (%class.A*)* @_ZTS1A.omp.def_constr, void (%class.A*)* @_ZTS1A.omp.destr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %d) ]
  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond


; Constructor
; CHECK:  %[[CONSTR_BEGIN:[^,]+]] = getelementptr inbounds [100 x %class.A], [100 x %class.A]* %e.priv, i32 0, i32 0
; CHECK-NEXT:  %[[CONSTR_END:[^,]+]] = getelementptr %class.A, %class.A* %[[CONSTR_BEGIN]], i32 100
; CHECK-NEXT:  %priv.constr.isempty = icmp eq %class.A* %[[CONSTR_BEGIN]], %[[CONSTR_END]]
; CHECK-NEXT:  br i1 %priv.constr.isempty, label %priv.constr.done, label %priv.constr.body
; CHECK-LABEL: priv.constr.body:
; CHECK-NEXT:  %priv.cpy.dest.ptr = phi %class.A* [ %[[CONSTR_BEGIN]], %{{.*}} ], [ %priv.cpy.dest.inc, %priv.constr.body ]
; CHECK-NEXT:  call %class.A* @_ZTS1A.omp.def_constr(%class.A* %priv.cpy.dest.ptr)
; CHECK-NEXT:  %priv.cpy.dest.inc = getelementptr %class.A, %class.A* %priv.cpy.dest.ptr, i32 1
; CHECK-NEXT:  %priv.cpy.done = icmp eq %class.A* %priv.cpy.dest.inc, %[[CONSTR_END]]
; CHECK-NEXT:  br i1 %priv.cpy.done, label %priv.constr.done, label %priv.constr.body
; CHECK-LABEL: priv.constr.done:
; CHECK-NEXT:  br label %{{.*}}

; Destructor
; CHECK:  %[[DESTR_BEGIN:[^,]+]] = getelementptr inbounds [100 x %class.A], [100 x %class.A]* %e.priv, i32 0, i32 0
; CHECK-NEXT:  %[[DESTR_END:[^,]+]] = getelementptr %class.A, %class.A* %[[DESTR_BEGIN]], i32 100
; CHECK-NEXT:  %priv.destr.isempty = icmp eq %class.A* %[[DESTR_BEGIN]], %[[DESTR_END]]
; CHECK-NEXT:  br i1 %priv.destr.isempty, label %priv.destr.done, label %priv.destr.body
; CHECK-LABEL: priv.destr.body:
; CHECK-NEXT:  %priv.cpy.dest.ptr{{.*}} = phi %class.A* [ %[[DESTR_BEGIN]], %{{.*}} ], [ %priv.cpy.dest.inc{{.*}}, %priv.destr.body ]
; CHECK:  call void @_ZTS1A.omp.destr(%class.A* %priv.cpy.dest.ptr{{.*}})
; CHECK:  %priv.cpy.dest.inc{{.*}} = getelementptr %class.A, %class.A* %priv.cpy.dest.ptr{{.*}}, i32 1
; CHECK:  %priv.cpy.done{{.*}} = icmp eq %class.A* %priv.cpy.dest.inc{{.*}}, %[[DESTR_END]]
; CHECK:  br i1 %priv.cpy.done{{.*}}, label %priv.destr.done, label %priv.destr.body
; CHECK-LABEL: priv.destr.done:
; CHECK-NEXT:  br label %{{.*}}


omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %arrayctor.cont
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %d, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

declare dso_local void @_ZN1AC1Ev(%class.A*) unnamed_addr #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline uwtable
define internal %class.A* @_ZTS1A.omp.def_constr(%class.A* %0) #3 section ".text.startup" {
entry:
  %.addr = alloca %class.A*, align 8
  store %class.A* %0, %class.A** %.addr, align 8
  %1 = load %class.A*, %class.A** %.addr, align 8
  call void @_ZN1AC1Ev(%class.A* %1)
  ret %class.A* %1
}

; Function Attrs: noinline uwtable
define internal void @_ZTS1A.omp.destr(%class.A* %0) #3 section ".text.startup" {
entry:
  %.addr = alloca %class.A*, align 8
  store %class.A* %0, %class.A** %.addr, align 8
  ret void
}

attributes #0 = { noinline optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}

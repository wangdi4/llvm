; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


; This test is used to check emitting loop of calling constructor and destructor
; for fixed size array with task construct + private clause.
; class A
; {
; public:
;   A();
; };
; void fn1() {
;   A e[100];
; #pragma omp task private(e)
;   for(int d=0; d<100; d++);
; }


; ModuleID = 'private_task_fixed_size_array.cpp'
source_filename = "private_task_fixed_size_array.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.A = type { i8 }

; Function Attrs: noinline optnone uwtable
define dso_local void @_Z3fn1v() #0 {
entry:
  %e = alloca [100 x %class.A], align 16
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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.PRIVATE:NONPOD"([100 x %class.A]* %e, %class.A* (%class.A*)* @_ZTS1A.omp.def_constr, void (%class.A*)* @_ZTS1A.omp.destr), "QUAL.OMP.PRIVATE"(i32* %d) ]
  store i32 0, i32* %d, align 4
  br label %for.cond


; Constructor
; CHECK:  %[[CONSTR_BEGIN:[^,]+]] = getelementptr inbounds [100 x %class.A], [100 x %class.A]* %e.gep, i32 0, i32 0
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
; CHECK:  %[[DESTR_BEGIN:[^,]+]] = getelementptr inbounds [100 x %class.A], [100 x %class.A]* %e.gep, i32 0, i32 0
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


for.cond:                                         ; preds = %for.inc, %arrayctor.cont
  %1 = load i32, i32* %d, align 4
  %cmp = icmp slt i32 %1, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %2 = load i32, i32* %d, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32* %d, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
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

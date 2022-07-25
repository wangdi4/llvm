; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This test is used to check emitting loop of calling copy constructor and
; destructor for variable length array type with firstprivate clause.
; class A
; {
; public:
;   A();
; };
; void fn1(int n) {
;   A e[n];
; #pragma omp parallel for firstprivate(e)
;   for(int d=0; d<n; d++);
; }


; ModuleID = 'firstpriv_variable_length_array.cpp'
source_filename = "firstpriv_variable_length_array.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.A = type { i8 }

; Function Attrs: noinline optnone uwtable
define dso_local void @_Z3fn1i(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %omp.vla.tmp = alloca i64, align 8
  %d = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8** %saved_stack, align 8
  %vla = alloca %class.A, i64 %1, align 16
  store i64 %1, i64* %__vla_expr0, align 8
  %isempty = icmp eq i64 %1, 0
  br i1 %isempty, label %arrayctor.cont, label %new.ctorloop

new.ctorloop:                                     ; preds = %entry
  %arrayctor.end = getelementptr inbounds %class.A, %class.A* %vla, i64 %1
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %new.ctorloop
  %arrayctor.cur = phi %class.A* [ %vla, %new.ctorloop ], [ %arrayctor.next, %arrayctor.loop ]
  call void @_ZN1AC1Ev(%class.A* %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.A, %class.A* %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq %class.A* %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %entry, %arrayctor.loop
  %3 = load i32, i32* %n.addr, align 4
  store i32 %3, i32* %.capture_expr.0, align 4
  %4 = load i32, i32* %.capture_expr.0, align 4
  %sub = sub nsw i32 %4, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4
  %5 = load i32, i32* %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %5
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %arrayctor.cont
  store i32 0, i32* %.omp.lb, align 4
  %6 = load i32, i32* %.capture_expr.1, align 4
  store i32 %6, i32* %.omp.ub, align 4
  store i64 %1, i64* %omp.vla.tmp, align 8
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%class.A* %vla, void (%class.A*, %class.A*)* @_ZTS1A.omp.copy_constr, void (%class.A*)* @_ZTS1A.omp.destr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %d), "QUAL.OMP.SHARED"(i64* %omp.vla.tmp) ]
  %8 = load i64, i64* %omp.vla.tmp, align 8
  %9 = load i32, i32* %.omp.lb, align 4
  store i32 %9, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond


; Copy constructor
; CHECK:  %[[TO:[^,]+]] = getelementptr inbounds %class.A, %class.A* %vla.fpriv, i32 0
; CHECK-NEXT:  %[[FROM:[^,]+]] = getelementptr inbounds %class.A, %class.A* %vla, i32 0
; CHECK-NEXT:  %[[END:[^,]+]] = getelementptr %class.A, %class.A* %[[TO]], i64 %[[VLA_LEN:[^,]+]]
; CHECK-NEXT:  %priv.cpyctor.isempty = icmp eq %class.A* %[[TO]], %[[END]]
; CHECK-NEXT:  br i1 %priv.cpyctor.isempty, label %priv.cpyctor.done, label %priv.cpyctor.body
; CHECK-LABEL: priv.cpyctor.body:
; CHECK-NEXT:  %priv.cpy.dest.ptr = phi %class.A* [ %[[TO]], %{{.*}} ], [ %priv.cpy.dest.inc, %priv.cpyctor.body ]
; CHECK-NEXT:  %priv.cpy.src.ptr = phi %class.A* [ %[[FROM]], %{{.*}} ], [ %priv.cpy.src.inc, %priv.cpyctor.body ]
; CHECK-NEXT:  call void @_ZTS1A.omp.copy_constr(%class.A* %priv.cpy.dest.ptr, %class.A* %priv.cpy.src.ptr)
; CHECK-NEXT:  %priv.cpy.dest.inc = getelementptr %class.A, %class.A* %priv.cpy.dest.ptr, i32 1
; CHECK-NEXT:  %priv.cpy.src.inc = getelementptr %class.A, %class.A* %priv.cpy.src.ptr, i32 1
; CHECK-NEXT:  %priv.cpy.done = icmp eq %class.A* %priv.cpy.dest.inc, %[[END]]
; CHECK-NEXT:  br i1 %priv.cpy.done, label %priv.cpyctor.done, label %priv.cpyctor.body
; CHECK-LABEL: priv.cpyctor.done:
; CHECK-NEXT:  br label %{{.*}}

; Destructor
; CHECK:  %[[DESTR_BEGIN:[^,]+]] = getelementptr inbounds %class.A, %class.A* %vla.fpriv, i32 0
; CHECK-NEXT:  %[[DESTR_END:[^,]+]] = getelementptr %class.A, %class.A* %[[DESTR_BEGIN]], i64 %[[VLA_LEN]]
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


omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %10 = load i32, i32* %.omp.iv, align 4
  %11 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %10, %11
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %12, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32* %d, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, i32* %.omp.iv, align 4
  %add5 = add nsw i32 %13, 1
  store i32 %add5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %arrayctor.cont
  %14 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %14)
  ret void
}

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #1

declare dso_local void @_ZN1AC1Ev(%class.A*) unnamed_addr #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline uwtable
define internal void @_ZTS1A.omp.copy_constr(%class.A* %0, %class.A* %1) #3 {
entry:
  %.addr = alloca %class.A*, align 8
  %.addr1 = alloca %class.A*, align 8
  store %class.A* %0, %class.A** %.addr, align 8
  store %class.A* %1, %class.A** %.addr1, align 8
  %2 = load %class.A*, %class.A** %.addr, align 8
  %3 = load %class.A*, %class.A** %.addr1, align 8
  ret void
}

; Function Attrs: noinline uwtable
define internal void @_ZTS1A.omp.destr(%class.A* %0) #3 section ".text.startup" {
entry:
  %.addr = alloca %class.A*, align 8
  store %class.A* %0, %class.A** %.addr, align 8
  ret void
}

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #1

attributes #0 = { noinline optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}

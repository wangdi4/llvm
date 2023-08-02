; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


; This test is used to check emitting loop of calling constructor and destructor
; for fixed size array with task construct + private clause.

; Test src:
;
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

; Constructor
; CHECK:  %[[CONSTR_BEGIN:[^,]+]] = getelementptr inbounds [100 x %class.A], ptr %e.gep, i32 0
; CHECK-NEXT:  %[[CONSTR_END:[^,]+]] = getelementptr %class.A, ptr %[[CONSTR_BEGIN]], i64 100
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

; Destructor
; CHECK:  %[[DESTR_BEGIN:[^,]+]] = getelementptr inbounds [100 x %class.A], ptr %e.gep, i32 0
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
; CHECK-NEXT:  br label %{{.*}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.A = type { i8 }

define dso_local void @_Z3fn1v() {
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
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr %e, %class.A zeroinitializer, i64 100, ptr @_ZTS1A.omp.def_constr, ptr @_ZTS1A.omp.destr),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %d, i32 0, i32 1) ]

  store i32 0, ptr %d, align 4
  br label %for.cond

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
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]

  ret void
}

declare dso_local void @_ZN1AC1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare noundef ptr @_ZTS1A.omp.def_constr(ptr noundef %0)

declare void @_ZTS1A.omp.destr(ptr noundef %0)

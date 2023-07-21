; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="function(loop-simplify,vpo-cfg-restructuring),vpo-paropt" -print-after-all -S %s | FileCheck %s

; This file checks whether the ssa update can generate correct code if
; there is only one live-in value.
;
; typedef double MMD_float;
; typedef int MMD_int;
;
; MMD_float Thermo::temperature(Atom &atom)
; {
;   MMD_int i;
;   MMD_float vx, vy, vz;
;
;   MMD_float t = 0.0;
;   t_act = 0;
; #pragma omp barrier
;
;  MMD_float* v = &atom.v[0][0];
; #pragma omp for schedule(static,666)
;
;  for(i = 0; i < atom.nlocal; i++) {
;     vx = v[i * 3 + 0];
;     vy = v[i * 3 + 1];
;     vz = v[i * 3 + 2];
;     t += (vx * vx + vy * vy + vz * vz) * atom.mass;
;   }
;
; #pragma omp atomic
;  t_act += t;
;
; #pragma omp barrier
;
;  MMD_float t1;
; #pragma omp master
;  {
;     if(sizeof(MMD_float) == 4)
;       t1 = t_act;
;
;     else
;       t1 = t_act;
;
;
;   }
;   return t1 * t_scale;
; }

; CHECK-LABEL: dispatch.header
; CHECK: %add27{{.*}} = phi double [ %add27, %dispatch.inc ], [ 0.000000e+00, %omp.inner.for.body.preheader ]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, ptr }
%class.Thermo = type { i32, i32, i32, ptr, ptr, ptr, ptr, double, double, double, double, double, double, double, double, ptr, double }
%class.ThreadData = type { i32, i32, i32, i32 }
%class.Atom = type { i32, i32, i32, i32, ptr, ptr, ptr, ptr, ptr, double, double, i32, i32, i32, %struct.Box }
%struct.Box = type { double, double, double, double, double, double, double, double, double }

@"@tid.addr" = external global i32
@.kmpc_loc.0.0.6 = external hidden unnamed_addr constant %struct.ident_t
@.kmpc_loc.0.0.8 = external hidden unnamed_addr constant %struct.ident_t
@.kmpc_loc.0.0.10 = external hidden unnamed_addr constant %struct.ident_t
@.kmpc_loc.0.0.12 = external hidden unnamed_addr constant %struct.ident_t
@.kmpc_loc.0.0.14 = external hidden unnamed_addr constant %struct.ident_t

declare void @llvm.lifetime.start.p0(i64, ptr nocapture)
declare void @llvm.lifetime.end.p0(i64, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local double @_ZN6Thermo11temperatureER4Atom(ptr %this, ptr dereferenceable(160) %atom) align 2 {
entry:
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %i)
  %t_act = getelementptr inbounds %class.Thermo, ptr %this, i32 0, i32 7
  store double 0.000000e+00, ptr %t_act, align 8
  %my.tid37 = load i32, ptr @"@tid.addr", align 4
  call void @__kmpc_barrier(ptr @.kmpc_loc.0.0.14, i32 %my.tid37)
  %v2 = getelementptr inbounds %class.Atom, ptr %atom, i32 0, i32 5
  %0 = load ptr, ptr %v2, align 8
  %1 = load ptr, ptr %0, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv)
  %nlocal = getelementptr inbounds %class.Atom, ptr %atom, i32 0, i32 1
  %2 = load i32, ptr %nlocal, align 4
  %sub6 = sub nsw i32 %2, 1
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb)
  store i32 0, ptr %.omp.lb, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub)
  store i32 %sub6, ptr %.omp.ub, align 4
  br label %DIR.OMP.LOOP.4

DIR.OMP.LOOP.4:                                   ; preds = %omp.precond.then
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 666),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %4 = load i32, ptr %.omp.lb, align 4
  store volatile i32 %4, ptr %.omp.iv, align 4
  %5 = load volatile i32, ptr %.omp.iv, align 4
  %6 = load i32, ptr %.omp.ub, align 4
  %cmp833 = icmp sle i32 %5, %6
  br i1 %cmp833, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.4
  %t.0 = phi double [ 0.000000e+00, %DIR.OMP.LOOP.4 ], [ %add27, %omp.inner.for.body ]
  %7 = load volatile i32, ptr %.omp.iv, align 4
  store i32 %7, ptr %i, align 4
  %mul10 = mul nsw i32 %7, 3
  %idxprom = sext i32 %mul10 to i64
  %arrayidx12 = getelementptr inbounds double, ptr %1, i64 %idxprom
  %8 = load double, ptr %arrayidx12, align 8
  %add14 = add nsw i32 %mul10, 1
  %idxprom15 = sext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds double, ptr %1, i64 %idxprom15
  %9 = load double, ptr %arrayidx16, align 8
  %add18 = add nsw i32 %mul10, 2
  %idxprom19 = sext i32 %add18 to i64
  %arrayidx20 = getelementptr inbounds double, ptr %1, i64 %idxprom19
  %10 = load double, ptr %arrayidx20, align 8
  %mul21 = fmul double %8, %8
  %mul22 = fmul double %9, %9
  %add23 = fadd double %mul21, %mul22
  %mul24 = fmul double %10, %10
  %add25 = fadd double %add23, %mul24
  %mass = getelementptr inbounds %class.Atom, ptr %atom, i32 0, i32 10
  %11 = load double, ptr %mass, align 8
  %mul26 = fmul double %add25, %11
  %add27 = fadd double %t.0, %mul26
  %12 = load volatile i32, ptr %.omp.iv, align 4
  %add28 = add nsw i32 %12, 1
  store volatile i32 %add28, ptr %.omp.iv, align 4
  %13 = load volatile i32, ptr %.omp.iv, align 4
  %14 = load i32, ptr %.omp.ub, align 4
  %cmp8 = icmp sle i32 %13, %14
  br i1 %cmp8, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.4
  %t.1 = phi double [ %add27, %omp.inner.for.body ], [ 0.000000e+00, %DIR.OMP.LOOP.4 ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %t.2 = phi double [ %t.1, %omp.loop.exit ], [ 0.000000e+00, %entry ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub)
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb)
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv)
  %my.tid36 = load i32, ptr @"@tid.addr", align 4
  call void @__kmpc_atomic_float8_add(ptr @.kmpc_loc.0.0.12, i32 %my.tid36, ptr %t_act, double %t.2)
  %my.tid35 = load i32, ptr @"@tid.addr", align 4
  call void @__kmpc_barrier(ptr @.kmpc_loc.0.0.10, i32 %my.tid35)
  %my.tid = load i32, ptr @"@tid.addr", align 4
  %15 = call i32 @__kmpc_master(ptr @.kmpc_loc.0.0.6, i32 %my.tid)
  %16 = icmp eq i32 %15, 1
  br i1 %16, label %if.then.master.3, label %DIR.OMP.END.MASTER.15

if.then.master.3:                                 ; preds = %omp.precond.end
  %17 = load double, ptr %t_act, align 8
  %my.tid34 = load i32, ptr @"@tid.addr", align 4
  call void @__kmpc_end_master(ptr @.kmpc_loc.0.0.8, i32 %my.tid34)
  br label %DIR.OMP.END.MASTER.15

DIR.OMP.END.MASTER.15:                            ; preds = %if.then.master.3, %omp.precond.end
  %t1.0 = phi double [ %17, %if.then.master.3 ], [ undef, %omp.precond.end ]
  %t_scale = getelementptr inbounds %class.Thermo, ptr %this, i32 0, i32 10
  %18 = load double, ptr %t_scale, align 8
  %mul32 = fmul double %t1.0, %18
  call void @llvm.lifetime.end.p0(i64 4, ptr %i)
  ret double %mul32
}

declare i32 @__kmpc_master(ptr, i32)
declare void @__kmpc_end_master(ptr, i32)
declare void @__kmpc_barrier(ptr, i32)
declare void @__kmpc_atomic_float8_add(ptr, i32, ptr, double)

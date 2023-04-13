; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec" -disable-output -vplan-print-legality < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; Test for HIRLegality handling of SIMD descriptor pointing to a field of a struct.
; The loop has a linear clause which contains pointer to a struct field. In the loop
; body we have a HLInst with a reference to another field of the same struct. We
; mistakenly treated the uses of other fields of the struct as uses of the pointer from
; the linear clause.  This results in the LinearDescriptor getting wrong start value which
; fails later in CFG builder during VPEntities importing.

; Incoming HIR is below. The test does not contain the entire loop, only the taskloop split function
; BEGIN REGION { }
;   %6 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR:IV.TYPED(&((%taskt.withprivates)[0].1.3)011) ]
;   %smax = @llvm.smax.i32(%.lb,  %.ub);
;
;   + DO i1 = 0, zext.i32.i64(((-1 * trunc.i64.i32(%.lb)) + %smax)), 1   <DO_LOOP>  <MAX_TC_EST = 100> <simd> <vectorize>
;   |   (%taskt.withprivates)[0].1.0[i1 + sext.i32.i64(trunc.i64.i32(%.lb))] = i1;
;   + END LOOP
;
;   (%taskt.withprivates)[0].1.3 = %smax + 1;
;   @llvm.directive.region.exit(%6); [ DIR.OMP.END.SIMD() ]
; END REGION

%__struct.kmp_task_t_with_privates = type { %__struct.kmp_task_t, %__struct.kmp_privates.t }
%__struct.kmp_task_t = type { ptr, ptr, i32, ptr, i64, i64, i64, i64, i32 }
%__struct.kmp_privates.t = type { [100 x i32], i32, i32, i32 }

; Function Attrs: nounwind uwtable
define internal void @foo.DIR.OMP.TASKLOOP.4.split27.split(i32 %tid, ptr nocapture %taskt.withprivates) {
; CHECK: HIRLegality LinearList:
; CHECK-NEXT: Ref: &((%taskt.withprivates)[0].1.3)
; CHECK-NOT: InitValue: %taskt.withprivates
DIR.OMP.SIMD.8:
  %.ub.gep = getelementptr inbounds %__struct.kmp_task_t_with_privates, ptr %taskt.withprivates, i64 0, i32 0, i32 6
  %.ub = load i64, ptr %.ub.gep, align 8
  %0 = trunc i64 %.ub to i32
  %i.gep = getelementptr inbounds %__struct.kmp_task_t_with_privates, ptr %taskt.withprivates, i64 0, i32 1, i32 3
  %.omp.lb.gep = getelementptr inbounds %__struct.kmp_task_t_with_privates, ptr %taskt.withprivates, i64 0, i32 1, i32 1
  %.omp.ub.gep = getelementptr inbounds %__struct.kmp_task_t_with_privates, ptr %taskt.withprivates, i64 0, i32 1, i32 2
  %1 = load i32, ptr %.omp.ub.gep, align 4
  %2 = load i32, ptr %.omp.lb.gep, align 4
  %cmp.not25 = icmp sgt i32 %2, %1
  br i1 %cmp.not25, label %DIR.OMP.END.SIMD.8.loopexit, label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.8
  %.lb.gep = getelementptr inbounds %__struct.kmp_task_t_with_privates, ptr %taskt.withprivates, i64 0, i32 0, i32 5
  %.lb = load i64, ptr %.lb.gep, align 8
  %3 = trunc i64 %.lb to i32
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.gep, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.21

DIR.OMP.SIMD.21:                                  ; preds = %DIR.OMP.SIMD.1
  %sext = shl i64 %.lb, 32
  %5 = ashr exact i64 %sext, 32
  %smax = call i32 @llvm.smax.i32(i32 %3, i32 %0)
  %6 = add nsw i32 %smax, 1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.21, %omp.inner.for.body
  %indvars.iv = phi i64 [ %5, %DIR.OMP.SIMD.21 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds %__struct.kmp_task_t_with_privates, ptr %taskt.withprivates, i64 0, i32 1, i32 0, i64 %indvars.iv
  %7 = trunc i64 %indvars.iv to i32
  store i32 %7, ptr %arrayidx, align 4
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %6, %lftr.wideiv
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  %.lcssa = phi i32 [ %7, %omp.inner.for.body ]
  store i32 %.lcssa, ptr %i.gep, align 4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.8.loopexit

DIR.OMP.END.SIMD.8.loopexit:                      ; preds = %DIR.OMP.END.SIMD.3, %DIR.OMP.SIMD.8
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smax.i32(i32, i32)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
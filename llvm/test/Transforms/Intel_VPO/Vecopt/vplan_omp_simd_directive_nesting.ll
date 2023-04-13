; RUN: opt -S < %s -passes='vplan-vec' -vplan-enable-nested-simd 2>&1 | FileCheck %s
;
; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z7nestingPc(ptr nocapture noundef writeonly %a) local_unnamed_addr #0 {
; CHECK: <4 x
DIR.OMP.SIMD.212:
  br label %DIR.OMP.ORDERED.413.lr.ph
DIR.OMP.ORDERED.413.lr.ph:                        ; preds = %DIR.OMP.SIMD.212
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0) ]
  br label %DIR.OMP.ORDERED.413
DIR.OMP.ORDERED.413:                              ; preds = %DIR.OMP.ORDERED.413.lr.ph, %DIR.OMP.END.ORDERED.2
  %indvars.iv = phi i64 [ 0, %DIR.OMP.ORDERED.413.lr.ph ], [ %indvars.iv.next, %DIR.OMP.END.ORDERED.2 ]
  call void @__kmpc_atomic_store(i64 1, ptr null, ptr null, i32 0)
  call void @__kmpc_atomic_load(i64 1, ptr null, ptr null, i32 0)
  %unused = call i1 @__kmpc_atomic_compare_exchange(i64 1, ptr null, ptr null, ptr null, i32 0, i32 0)
  call void @__kmpc_atomic_fixed4_add(ptr null, i32 0, ptr null, i32 0)
  call void @__kmpc_atomic_float8_add(ptr null, i32 0, ptr null, double 0.0)
  %arrayidx = getelementptr inbounds i8, ptr %a, i64 %indvars.iv
  store i8 43, ptr %arrayidx, align 1
  br label %codeRepl
codeRepl:                                         ; preds = %DIR.OMP.ORDERED.413
  %s0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.OMP.ORDERED.3
DIR.OMP.ORDERED.3:                                ; preds = %newFuncRoot
  %c0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(), "QUAL.OMP.ORDERED.SIMD"() ]
  br label %DIR.OMP.ORDERED.1
DIR.OMP.ORDERED.1:                                ; preds = %DIR.OMP.ORDERED.3
  %iv.2 = phi i64 [ 0, %DIR.OMP.ORDERED.3 ], [ %iv.2.next, %DIR.OMP.ORDERED.1 ]
  %a2 = getelementptr inbounds i32, ptr %a, i64 %iv.2
  store i32 707406378, ptr %a2, align 1
  %iv.2.next = add nuw nsw i64 %iv.2, 1
  %e1 = icmp ule i64 %iv.2.next, 4
  br i1 %e1, label %DIR.OMP.ORDERED.1, label %DIR.OMP.END.ORDERED.5
DIR.OMP.END.ORDERED.5:                            ; preds = %DIR.OMP.ORDERED.1
  call void @llvm.directive.region.exit(token %c0) [ "DIR.OMP.END.ORDERED"() ]
  br label %endInnerSimd
endInnerSimd:
  call void @llvm.directive.region.exit(token %s0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.ORDERED.2.exitStub
DIR.OMP.END.ORDERED.2.exitStub:                   ; preds = %DIR.OMP.END.ORDERED.5
  br label %DIR.OMP.END.ORDERED.2
DIR.OMP.END.ORDERED.2:                            ; preds = %codeRepl
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.8.loopexit_crit_edge, label %DIR.OMP.ORDERED.413
omp.inner.for.cond.DIR.OMP.END.SIMD.8.loopexit_crit_edge: ; preds = %DIR.OMP.END.ORDERED.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3
DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.8.loopexit_crit_edge
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2
; Function Attrs: nounwind
declare void @__kmpc_atomic_load(i64, ptr, ptr, i32)
; Function Attrs: nounwind
declare void @__kmpc_atomic_store(i64, ptr, ptr, i32)
; Function Attrs: nounwind
declare i1 @__kmpc_atomic_compare_exchange(i64, ptr, ptr, ptr, i32, i32)
; Function Attrs: nounwind
declare void @__kmpc_atomic_fixed4_add(ptr, i32, ptr, i32)
; Function Attrs: nounwind
declare void @__kmpc_atomic_float8_add(ptr, i32, ptr, double)

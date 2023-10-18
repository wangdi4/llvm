; Verify that HIRParVecAnalysis bails out for function calls that
; don't match expected LibFunc's call signature.

; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>,hir-cg,simplifycfg,intel-ir-optreport-emitter" -disable-output -intel-opt-report=medium < %s 2>&1 | FileCheck %s

; CHECK-NOT: DIR.VPO.AUTO.VEC
; CHECK: remark #15527: Loop was not vectorized: function call to  cannot be vectorized

%complex_128bit = type { double, double }

define internal void @foo(ptr noalias %input, ptr noalias %output) {
entry:
  %priv1 = alloca %complex_128bit, align 16
  %priv2 = alloca %complex_128bit, align 16
  br label %preheader

preheader:                                      ; preds = %entry
  %priv1.imagp = getelementptr inbounds %complex_128bit, ptr %priv1, i64 0, i32 1
  %priv2.imagp = getelementptr inbounds %complex_128bit, ptr %priv2, i64 0, i32 1
  br label %header

header:                                         ; preds = %preheader, %header
  %iv = phi i64 [ 0, %preheader ], [ %iv.next, %header ]
  %input.ptr = getelementptr inbounds %complex_128bit, ptr %input, i64 %iv
  %input.imagp = getelementptr inbounds %complex_128bit, ptr %input.ptr, i64 0, i32 1
  %input.real = load double, ptr %input.ptr, align 16
  %input.imag = load double, ptr %input.imagp, align 16
  store double %input.real, ptr %priv2, align 16
  store double %input.imag, ptr %priv2.imagp, align 16
  call void @cexp(ptr nonnull %priv1, ptr nonnull byval(%complex_128bit) %priv2)
  %priv1.real = load double, ptr %priv1, align 16
  %priv1.imag = load double, ptr %priv1.imagp, align 16
  %output.ptr = getelementptr inbounds %complex_128bit, ptr %output, i64 %iv
  %output.imagp = getelementptr inbounds %complex_128bit, ptr %output.ptr, i64 0, i32 1
  store double %priv1.real, ptr %output.ptr, align 16
  store double %priv1.imag, ptr %output.imagp, align 16
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1024
  br i1 %exitcond, label %exit, label %header, !llvm.loop !0

exit:                                           ; preds = %header
  ret void
}

declare void @cexp(ptr noalias sret(%complex_128bit), ptr byval(%complex_128bit)) #0

attributes #0 = { nounwind }

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}

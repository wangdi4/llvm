;
; RUN: opt -opaque-pointers=0 < %s -passes=vplan-vec -disable-output -print-after=vplan-vec -vplan-force-vf=2 -vplan-enable-peeling 2>&1 | FileCheck %s
;
; LIT test to check that we do not crash in vector code generation when the peeling
; candidate memory address is the return value from a function call that can throw.
; Check for proper peel base address broadcast and that loop is vectorized.
;
; CHECK-LABEL: peel.check{{.*}}:
; CHECK-NEXT:    [[SPLATINSERT:%.*]] = insertelement <2 x i64*> poison, i64* %inv.mem, i64 0
; CHECK-NEXT:    [[SPLAT:%.*]] = shufflevector <2 x i64*> [[SPLATINSERT]], <2 x i64*> poison, <2 x i32> zeroinitializer
; CHECK-NEXT:    [[PTRTOINT:%.*]] = ptrtoint <2 x i64*> [[SPLAT]] to <2 x i64>
;
; CHECK:       vector.body:
;
define dso_local void @_Z3fool(i64 noundef %n1) local_unnamed_addr #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %l1.linear.iv = alloca i64, align 8
  %inv.mem = invoke noundef i64* @_Z6getmeml(i64 noundef %n1)
          to label %try.cont unwind label %try.cont.thread

try.cont.thread:                                  ; preds = %entry
  %0 = landingpad { i8*, i32 }
          catch i8* null
  %1 = extractvalue { i8*, i32 } %0, 0
  %2 = call i8* @__cxa_begin_catch(i8* %1) #3
  call void @__cxa_end_catch()
  br label %exit

try.cont:                                         ; preds = %entry
  %cmp3.not20 = icmp slt i64 %n1, 1
  br i1 %cmp3.not20, label %exit, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %try.cont
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body.lr.ph, %omp.inner.for.body
  %.omp.iv.local.021 = phi i64 [ 0, %omp.inner.for.body.lr.ph ], [ %add5, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %inv.mem, i64 %.omp.iv.local.021
  store i64 %.omp.iv.local.021, i64* %arrayidx, align 8
  %add5 = add nuw nsw i64 %.omp.iv.local.021, 1
  %exitcond.not = icmp eq i64 %add5, %n1
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %omp.inner.for.body

DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret void
}

declare dso_local noundef i64* @_Z6getmeml(i64 noundef) local_unnamed_addr #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nofree
declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local void @__cxa_end_catch() local_unnamed_addr #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

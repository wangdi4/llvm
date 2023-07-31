; RUN: opt -S -passes="vplan-vec" -vplan-print-after-plain-cfg -vplan-entities-dump %s | FileCheck %s

; CHECK-LABEL: Private list
; CHECK-EMPTY:
; CHECK-NEXT:    Exit instr: <2 x i32> [[VP__:%.*]] = select i1 [[VP_CMP3:%.*]] <2 x i32> [[VP0:%.*]] <2 x i32> zeroinitializer
; CHECK-NEXT:    Linked values: <2 x i32> [[VP__]], ptr [[TMP_PRIV0:%.*]],
; CHECK-NEXT:   Memory: ptr [[TMP_PRIV0]]

; CHECK-LABEL: simd_loop
; CHECK: %[[CmpRes:.*]] = icmp sgt <4 x i32> %{{.*}}, zeroinitializer
; CHECK: %[[Replicated:.*]] = shufflevector <4 x i1> %[[CmpRes]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK: select <8 x i1> %[[Replicated]], <8 x i32> 

define void @simd_loop(ptr %A, ptr %B) #0 {
entry:
  %tmp.priv = alloca <2 x i32>, align 8
  br label %DIR.OMP.SIMD.3

DIR.OMP.SIMD.3:                                   ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp.priv, <2 x i32> zeroinitializer, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.3, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.3 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds <2 x i32>, ptr %A, i64 %indvars.iv
  %0 = load <2 x i32>, ptr %arrayidx, align 8
  %1 = extractelement <2 x i32> %0, i64 0
  %rem = srem i32 %1, 2
  %cmp3 = icmp sgt i32 %rem, 0
  %. = select i1 %cmp3, <2 x i32> %0, <2 x i32> zeroinitializer
  %add6 = add <2 x i32> %., <i32 5, i32 5>
  %arrayidx8 = getelementptr inbounds <2 x i32>, ptr %B, i64 %indvars.iv
  store <2 x i32> %add6, ptr %arrayidx8, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  %..lcssa = phi <2 x i32> [ %., %omp.inner.for.body ]
  store <2 x i32> %..lcssa, ptr %tmp.priv, align 8
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

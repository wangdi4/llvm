; RUN: opt -aa-pipeline="tbaa" -passes="aa-eval,gvn" < %s -evaluate-aa-metadata -print-may-aliases 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="tbaa" -passes="aa-eval,loop-mssa(licm)" < %s -evaluate-aa-metadata -print-may-aliases 2>&1 | FileCheck %s


%"struct.std::complex" = type { { float, float } }
@a = global [2 x %"struct.std::complex"] zeroinitializer, align 16
@c = global [2 x float] zeroinitializer, align 16
@ar = local_unnamed_addr global [2 x i32] [i32 0, i32 1], align 4
@b = common local_unnamed_addr global i32 0, align 4
@ra = common global [2 x i32] zeroinitializer, align 4


; Varifies if TBAA generates correct aliasing results for
; the global array type nodes so that GVN does not remove loads that
; are actually clobbered by stores.

define double @intel_testGVN() local_unnamed_addr #0 {
; CHECK: Function
; CHECK: MayAlias:  %0 = load float, ptr @c, align 16, !tbaa !2 <-> store float %call.i.i, ptr     %arrayidx11, align 4, !tbaa !6

entry:
  call void @llvm.memset.p0.i64(ptr bitcast (ptr @c to ptr), i8 0, i64 8, i32 4, i1 false)
  store i32 1073741824, ptr bitcast (ptr @a to ptr), align 8
  store i32 1077936128, ptr bitcast (ptr getelementptr inbounds ([2 x %"struct.std::complex"], ptr @a, i64 0, i64 0, i32 0, i32 1) to ptr), align 4
  store i32 1073741824, ptr bitcast (ptr getelementptr inbounds ([2 x %"struct.std::complex"], ptr @a, i64 0, i64 1) to ptr), align 8
  store i32 1077936128, ptr bitcast (ptr getelementptr inbounds ([2 x %"struct.std::complex"], ptr @a, i64 0, i64 1, i32 0, i32 1) to ptr), align 4
  br label %for.body

for.cond.cleanup:                                ; preds = %for.body
  %0 = load float, ptr getelementptr inbounds ([2 x float], ptr @c, i64 0, i64 0), align 16, !tbaa !2
  %conv.i = fpext float %0 to double
  ret double %conv.i

for.body:                                        ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %_M_value.realp.i.i = getelementptr inbounds [2 x %"struct.std::complex"], ptr @a, i64 0, i64 %indvars.iv, i32 0, i32 0
  %_M_value.real.i.i = load float, ptr %_M_value.realp.i.i, align 8
  %_M_value.imagp.i.i = getelementptr inbounds [2 x %"struct.std::complex"], ptr @a, i64 0, i64 %indvars.iv, i32 0, i32 1
  %_M_value.imag.i.i = load float, ptr %_M_value.imagp.i.i, align 4
  %retval.sroa.0.0.vec.insert.i.i = insertelement <2 x float> undef, float %_M_value.real.i.i, i32 0
  %retval.sroa.0.4.vec.insert.i.i = insertelement <2 x float> %retval.sroa.0.0.vec.insert.i.i, float %_M_value.imag.i.i, i32 1
  %call.i.i = tail call float @cabsf(<2 x float> %retval.sroa.0.4.vec.insert.i.i)
  %arrayidx11 = getelementptr inbounds [2 x float], ptr @c, i64 0, i64 %indvars.iv
  store float %call.i.i, ptr %arrayidx11, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 2
  br i1 %exitcond, label %for.body, label %for.cond.cleanup
}


; Varifies if TBAA generates correct aliasing results for
; the global array type nodes so that LICM does not hoist loads that
; are aliased by stores.

define i32 @intel_testLICM() local_unnamed_addr #3 {
; CHECK: Function
; CHECK: MayAlias: %0 = load i32, ptr @ra, align 4, !tbaa !7 <->   store i32 %j.05, ptr %arrayidx, align 4, !tbaa !2

entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %j.05 = phi i32 [ 1, %entry ], [ %inc, %for.body ]
  %sub = add i32 %j.05, -1
  %idxprom = zext i32 %sub to i64
  %arrayidx = getelementptr inbounds [2 x i32], ptr @ra, i64 0, i64 %idxprom
  store i32 %j.05, ptr %arrayidx, align 4, !tbaa !10
  %0 = load i32, ptr getelementptr inbounds ([2 x i32], ptr @ra, i64 0, i64 0), align 4, !tbaa !8
  %idxprom.i = zext i32 %0 to i64
  %arrayidx1.i = getelementptr inbounds [2 x i32], ptr @ar, i64 0, i64 %idxprom.i
  %1 = load i32, ptr %arrayidx1.i, align 4, !tbaa !10
  store i32 %1, ptr @b, align 4, !tbaa !8
  %inc = add i32 %j.05, 1
  %cmp = icmp ult i32 %inc, 2
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  %2 = load i32, ptr @b, align 4, !tbaa !8
  ret i32 %2
}


declare float @cabsf(<2 x float>) local_unnamed_addr #1
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i32, i1) #2


attributes #0 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (cfe/trunk)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA2_f", !3, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !4, i64 0}
!10 = !{!11, !9, i64 0}
!11 = !{!"array@_ZTSA2_j", !9, i64 0}

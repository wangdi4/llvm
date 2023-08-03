; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cg" -force-hir-cg | lli

; The test checks A[0][11] == 5, it verifies that the strides of dimensions were parsed and generated correctly.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 9, 1   <DO_LOOP>
;       |   |   (%base)[0:i1:40(i32*:0)][0:i2:4(i32*:0)] = 5;
;       |   + END LOOP
;       + END LOOP
; END REGION

@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %base = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %i = phi i64 [ 0, %entry ], [ %i.next, %for.cond.cleanup3 ]
  %pi = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 40, ptr elementtype(i32) %base, i64 %i)
  br label %for.body4

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %j = phi i64 [ 0, %for.cond1.preheader ], [ %j.next, %for.body4 ]
  %pj = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 4, ptr elementtype(i32) %pi, i64 %j)
  store i32 5, ptr %pj, align 4
  %j.next = add nuw nsw i64 %j, 1
  %exitcond = icmp eq i64 %j.next, 10
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %i.next = add nuw nsw i64 %i, 1
  %exitcond20 = icmp eq i64 %i.next, 100
  br i1 %exitcond20, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  %r = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @A, i64 0, i64 11), align 4
  %cmp = icmp ne i32 %r, 5
  %r1 = zext i1 %cmp to i32
  ret i32 %r1
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

attributes #1 = { nounwind readnone speculatable }


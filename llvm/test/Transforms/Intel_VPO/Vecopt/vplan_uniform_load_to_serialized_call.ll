; RUN: opt %s -S -VPlanDriver -vplan-force-vf=2 | FileCheck %s
; RUN: opt %s -S -VPlanDriver -vplan-force-vf=2 -enable-vp-value-codegen | FileCheck %s

; Check that for uniform load followed by serialized call we use that load for
; all the lanes in the serialized calls sequence (used to crash due to not
; finding proper mapping).

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare void @foo(<2 x i64>, i64)

define void @test1(i64 %n, <2 x i64>* %arr) {
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(<2 x i64>* %arr) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  %cond = icmp eq i64 %n, 42
  %gep = getelementptr inbounds <2 x i64>, <2 x i64>* %arr, i64 42
; CHECK:    [[TMP1:%.*]] = getelementptr inbounds <2 x i64>, <2 x i64>* [[ARR:%.*]], i64 42
  %load.uni = load <2 x i64>, <2 x i64>* %gep
; CHECK-NEXT:    [[TMP2:%.*]] = load <2 x i64>, <2 x i64>* [[TMP1]]
  call void @foo(<2 x i64> %load.uni, i64 %indvars.iv)
; CHECK-NEXT:    call void @foo(<2 x i64> [[TMP2]], i64 {{%.*}})
; CHECK-NEXT:    call void @foo(<2 x i64> [[TMP2]], i64 {{%.*}})
  br i1 %cond, label %if.then, label %for.latch

if.then:
  %load.uni.pred = load <2 x i64>, <2 x i64>* %gep
; CHECK:    [[TMP5:%.*]] = load <2 x i64>, <2 x i64>* [[TMP1]]
; CHECK:    [[TMP7:%.*]] = phi <2 x i64> [ undef, [[VECTOR_BODY:.*]] ], [ [[TMP5]], [[PRED_LOAD_IF:.*]] ]
  call void @foo(<2 x i64> %load.uni.pred, i64 %indvars.iv)
; CHECK:    call void @foo(<2 x i64> [[TMP7]], i64 {{%.*}})
; CHECK:    call void @foo(<2 x i64> [[TMP7]], i64 {{%.*}})
  br label %for.latch

for.latch:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %exit

exit:
  ret void
}

; RUN: opt %s -S -VPlanDriver -vplan-force-vf=2 | FileCheck %s --check-prefixes=CHECK,CHECK-LLVM
; RUN: opt %s -S -VPlanDriver -vplan-force-vf=2 -enable-vp-value-codegen | FileCheck %s --check-prefixes=CHECK,CHECK-VPVAL

; Check that for uniform instruction that has vector type before vectorization and
; followed by serialized call we use either that uniform non-widened version or
; a correct broadcast/shuffle sequence that extracts right value of the original
; vector type.

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare void @foo(<2 x i64>, i64)

declare <2 x i64> @bar(<2 x i64>) nounwind readnone

define void @test1(i64 %n, <2 x i64>* %arr) {
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(<2 x i64>* %arr) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  %cond = icmp eq i64 %indvars.iv, 42
  %gep = getelementptr inbounds <2 x i64>, <2 x i64>* %arr, i64 42
; CHECK:    [[TMP1:%.*]] = getelementptr inbounds <2 x i64>, <2 x i64>* [[ARR:%.*]], i64 42
  %load.uni = load <2 x i64>, <2 x i64>* %gep
; CHECK-NEXT:    [[TMP2:%.*]] = load <2 x i64>, <2 x i64>* [[TMP1]]
  call void @foo(<2 x i64> %load.uni, i64 %indvars.iv)
; CHECK-NEXT:    call void @foo(<2 x i64> [[TMP2]], i64 {{%.*}})
; CHECK-NEXT:    call void @foo(<2 x i64> [[TMP2]], i64 {{%.*}})

  %call.uni = call <2 x i64> @bar(<2 x i64> zeroinitializer)
; CHECK-NEXT:    [[UNI_CALL:%.*]] = call <2 x i64> @bar(<2 x i64> zeroinitializer)
  call void @foo(<2 x i64> %call.uni, i64 %indvars.iv)

; VPValue-based codegen uses DA and knows that both lanes contain the same
; value. It's harder to get that information with LLVM-based CG, so just check
; the widen/extract sequence that gives the correct value too, although uses
; more instructions.

; CHECK-LLVM-NEXT:    [[REPLICATED:%.*]] = shufflevector <2 x i64> [[UNI_CALL]], <2 x i64> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>

; Both CGs re-use the scalar call resutls for lane zero.
; CHECK-NEXT:    call void @foo(<2 x i64> [[UNI_CALL]], i64 {{%.*}})

; CHECK-LLVM-NEXT:    [[UNI_CALL_LANE1:%.*]] = shufflevector <4 x i64> [[REPLICATED]], <4 x i64> undef, <2 x i32> <i32 2, i32 3>
; CHECK-LLVM-NEXT:    call void @foo(<2 x i64> [[UNI_CALL_LANE1]], i64 {{%.*}})
; CHECK-VPVAL-NEXT:   call void @foo(<2 x i64> [[UNI_CALL]], i64 {{%.*}})

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

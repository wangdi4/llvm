; RUN: opt -S -passes=vplan-vec -vplan-force-vf=2 < %s | FileCheck %s

;CHECK:       vector.body:                                      ; preds = %vector.body, %VPlannedBB1
;CHECK-NEXT:      [[UNI_PHI0:%.*]] = phi i64 [ 0, [[VPLANNEDBB0:%.*]] ], [ [[TMP28:%.*]], [[VECTOR_BODY:%.*]] ]
;CHECK-NEXT:      [[VEC_PHI:%.*]] = phi <2 x i64> [ <i64 0, i64 1>, [[VPLANNEDBB0:%.*]]], [ %0, [[VECTOR_BODY]] ]
;CHECK-NEXT:      call void @llvm.assume(i1 true) [ "align"(ptr [[A:%.*]], i64 32) ] 
;CHECK-NEXT:      %0 = add nuw nsw <2 x i64> [[VEC_PHI]], <i64 2, i64 2>

define void @foo(ptr noalias %lp1, ptr noalias %lp2, i64 %n1) {
entry:
  br label %for.ph

for.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.ph ], [ %add1, %for.body ]
  call void @llvm.assume(i1 true) [ "align"(ptr %lp2, i64 32) ]
  %add1 = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %add1, 1024
  br i1 %exitcond.not, label %ret, label %for.body

ret:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.assume(i1 noundef)

; RUN: opt -VPlanDriver -disable-vplan-subregions -disable-vplan-predicator -vplan-force-vf=4 -enable-vp-value-codegen=false -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-IRCG
; RUN: opt -VPlanDriver -disable-vplan-subregions -disable-vplan-predicator -vplan-force-vf=4 -enable-vp-value-codegen=true  -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-VPCG

;void foo(int *ip, int n)
;{
;  int i
;
;#pragma omp simd
;  for (i = 0; i < n; i++)
;    ip[i] = i;
;}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; The vectorized loop with scalar remainder

; CHECK-LABEL: var_tripcount
; CHECK: vector.body
; CHECK: %index = phi i64 [ 0,
; CHECK: [[VEC_IND:%.*]] = phi <4 x i64> [
; CHECK: store {{.*}} <4 x i32>
; CHECK-IRCG: add <4 x i64> [[VEC_IND]], <i64 4, i64 4, i64 4, i64 4>
; CHECK-VPCG: add nuw nsw <4 x i64> [[VEC_IND]], <i64 4, i64 4, i64 4, i64 4>
; CHECK: middle.block
; CHECK: scalar.ph
; CHECK-IRCG: %bc.resume.val = phi i64 [ %n.vec, %middle.block ], [ 0, %for.body.preheader ]
; CHECK-VPCG: %bc.resume.val = phi i64 [ 0, %for.body.preheader ], [ %{{.*}}, %middle.block ]
; CHECK: for.body:
; CHECK: %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ %bc.resume.val, %scalar.ph ]

; Function Attrs: nounwind uwtable
define void @var_tripcount(i32* nocapture %ip, i32 %n) local_unnamed_addr #0 {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %cmp5 = icmp sgt i32 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %DIR.QUAL.LIST.END.2
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %ip, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %DIR.QUAL.LIST.END.2
  br label %for.cond.cleanup
  
for.cond.cleanup:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20717)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

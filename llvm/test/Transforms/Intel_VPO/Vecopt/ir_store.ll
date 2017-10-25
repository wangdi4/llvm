; RUN: opt -VPlanDriver -disable-vplan-subregions -disable-vplan-predicator -S < %s  | FileCheck %s


;  for (int i=0; i<n; ++i) {}
;    ip[i] = i;
;}

; CHECK-LABEL: foo
; CHECK: min.iters.checked
; CHECK: vector.body
; CHECK: %index = phi i64 [ 0,
; CHECK: %vec.ind = phi <4 x i64> [
; CHECK: store {{.*}} <4 x i32>
; CHECK: middle.block
; CHECK: scalar.ph
; CHECK: %bc.resume.val = phi i64 [ %n.vec, %middle.block ], [ 0, %for.body.preheader ], [ 0, %min.iters.checked ]
; CHECK: for.body:
; CHECK: %indvars.iv = phi i64 [ %bc.resume.val, %scalar.ph ], [ %indvars.iv.next, %for.body ]

define void @foo(i32* nocapture %ip, i32 %N) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %zext.trip.cnt = zext i32 %N to i64
  br label %for.body.preheader

for.body.preheader:                              ; preds = %DIR.QUAL.LIST.END.2
  br label %for.body

for.body:                                         
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ip, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %zext.trip.cnt
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.cleanup

for.cleanup:                              ; preds = %for.end
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1


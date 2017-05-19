; RUN: opt -S %s -O2 -loopopt=0 -vplan-driver -vplan-build-stress-test | FileCheck %s

;void foo(int *arr1, int *__restrict__ arr2, int *__restrict__ arr3) {
;#pragma omp simd
;  for (int i = 0; i < 100; i++) {
;    int j;
;    if (arr1[i])
;      j = arr2[i] += i;
;    else
;      j = arr3[i] -= i;
;    arr1[i] = j;
;  }
;}

; CHECK-LABEL: foo
; CHECK: vector.body
; CHECK: %wide.load = load <4 x i32>, <4 x i32>* %1, align 4
; CHECK: %[[MASK:.*]] = icmp eq <4 x i32> %wide.load, zeroinitializer
; CHECK: %[[MASK_NOT:.*]] = xor <4 x i1> %[[MASK]], <i1 true, i1 true, i1 true, i1 true>
; CHECK: %[[WIDE_MASK_LOAD1:.*]] = call <4 x i32> @llvm.masked.load.v4i32.p0v4i32{{.*}}, <4 x i1> %[[MASK_NOT]]
; CHECK: %[[WIDE_MASK_LOAD2:.*]] = call <4 x i32> @llvm.masked.load.v4i32.p0v4i32{{.*}}, <4 x i1> %[[MASK]]
; CHECK: %[[PRED_PFI:.*]] = select <4 x i1> %[[MASK]], <4 x i32> 
; CHECK: store <4 x i32> %[[PRED_PFI]], <4 x i32>*

define void @foo(i32* nocapture %arr1, i32* noalias nocapture %arr2, i32* noalias nocapture %arr3) {
entry:
  br label %for.body

for.body:                                         ; preds = %if.end, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds i32, i32* %arr1, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %tobool = icmp eq i32 %0, 0
  br i1 %tobool, label %if.else, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx2 = getelementptr inbounds i32, i32* %arr2, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %2 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %1, %2
  store i32 %add, i32* %arrayidx2, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %arrayidx4 = getelementptr inbounds i32, i32* %arr3, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx4, align 4
  %4 = trunc i64 %indvars.iv to i32
  %sub = sub nsw i32 %3, %4
  store i32 %sub, i32* %arrayidx4, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %j.0 = phi i32 [ %add, %if.then ], [ %sub, %if.else ]
  store i32 %j.0, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %if.end
  ret void
}

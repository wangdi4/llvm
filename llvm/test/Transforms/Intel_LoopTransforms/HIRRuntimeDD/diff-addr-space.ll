; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; The test checks that RTDD is not triggering if there are references from different
; address spaces: %in and @bubble_local.lin.

; BEGIN REGION { }
; + DO i1 = 0, 1023, 1   <DO_LOOP>
; |   %1 = (%in)[i1];
; |   (@bubble_local.lin)[0][i1] = %1;
; + END LOOP
; END REGION

; CHECK: Function
; CHECK: BEGIN REGION
; CHECK-NOT: if
; CHECK: END REGION

;Module Before HIR; ModuleID = '/export/iusers/pgprokof/tests/opencl/bubble.pre.bc'
source_filename = "/data/mmendell/uplift3/p4/regtest/hld/opencl/features/task/bubble_sort/bubble.cl"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

@bubble_local.lin = external hidden unnamed_addr addrspace(3) global [1024 x i16], align 2

; Function Attrs: norecurse nounwind
define spir_kernel void @bubble_local(ptr addrspace(1) nocapture %in, i32 %len) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.024 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %0 = zext i32 %i.024 to i64
  %arrayidx = getelementptr inbounds i16, ptr addrspace(1) %in, i64 %0
  %1 = load i16, ptr addrspace(1) %arrayidx, align 2
  %arrayidx2 = getelementptr inbounds [1024 x i16], ptr addrspace(3) @bubble_local.lin, i64 0, i64 %0
  store i16 %1, ptr addrspace(3) %arrayidx2, align 2
  %inc = add nuw nsw i32 %i.024, 1
  %cmp = icmp ult i32 %inc, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  br label %do.body

do.body:                                          ; preds = %for.end12, %for.end
  %N.0 = phi i32 [ 1024, %for.end ], [ %sub13, %for.end12 ]
  %cmp422 = icmp ugt i32 %N.0, 1
  br i1 %cmp422, label %for.body5.lr.ph, label %do.end

for.body5.lr.ph:                                  ; preds = %do.body
  br label %for.body5

for.body5:                                        ; preds = %swap_local.exit, %for.body5.lr.ph
  %i.123 = phi i32 [ 1, %for.body5.lr.ph ], [ %inc11, %swap_local.exit ]
  %sub = add nsw i32 %i.123, -1
  %idxprom6 = sext i32 %sub to i64
  %arrayidx7 = getelementptr inbounds [1024 x i16], ptr addrspace(3) @bubble_local.lin, i64 0, i64 %idxprom6
  %2 = zext i32 %i.123 to i64
  %arrayidx9 = getelementptr inbounds [1024 x i16], ptr addrspace(3) @bubble_local.lin, i64 0, i64 %2
  %3 = load i16, ptr addrspace(3) %arrayidx7, align 2
  %4 = load i16, ptr addrspace(3) %arrayidx9, align 2
  %cmp.i = icmp sgt i16 %3, %4
  br i1 %cmp.i, label %if.then.i, label %swap_local.exit

if.then.i:                                        ; preds = %for.body5
  store i16 %4, ptr addrspace(3) %arrayidx7, align 2
  store i16 %3, ptr addrspace(3) %arrayidx9, align 2
  br label %swap_local.exit

swap_local.exit:                                  ; preds = %if.then.i, %for.body5
  %inc11 = add nuw nsw i32 %i.123, 1
  %cmp4 = icmp ult i32 %inc11, %N.0
  br i1 %cmp4, label %for.body5, label %for.end12

for.end12:                                        ; preds = %swap_local.exit
  %sub13 = add nsw i32 %N.0, -1
  br i1 %cmp422, label %do.body, label %do.end

do.end:                                           ; preds = %for.end12, %do.body
  br label %for.body17

for.body17:                                       ; preds = %for.body17, %do.end
  %i.221 = phi i32 [ 0, %do.end ], [ %inc23, %for.body17 ]
  %5 = zext i32 %i.221 to i64
  %arrayidx19 = getelementptr inbounds [1024 x i16], ptr addrspace(3) @bubble_local.lin, i64 0, i64 %5
  %6 = load i16, ptr addrspace(3) %arrayidx19, align 2
  %arrayidx21 = getelementptr inbounds i16, ptr addrspace(1) %in, i64 %5
  store i16 %6, ptr addrspace(1) %arrayidx21, align 2
  %inc23 = add nuw nsw i32 %i.221, 1
  %cmp16 = icmp ult i32 %inc23, 1024
  br i1 %cmp16, label %for.body17, label %for.end24

for.end24:                                        ; preds = %for.body17
  ret void
}

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-fra                                        me-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-m                                        ath"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

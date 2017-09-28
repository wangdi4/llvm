; RUN: %oclopt -presucf=true -predicate -verify %s -S -o %t0.ll

; The test checks that preidcator works correctly(does not hang on)
; in the following situaltion:


;      |---------|
;      |  entry  |
;      |---------|
;          ||
;          \/
;      |---------|<=========|---------|
;      | entryBB |          |    E    |
;      |---------|=========>|---------|
;          ||                   /\
;          \/                   ||
;      |---------|              ||
;      | exitBB  |              ||
;      |---------|              ||
;          ||                   ||
;          \/                   ||
;      |---------|<======||     ||
;      |    A    |       ||     ||
;      |---------|===||  ||     ||
;          ||        ||  ||     ||
;          \/        ||  ||     ||
;      |---------|   ||  ||     ||
;      |    B    |   ||  ||     ||
;      |---------|   ||  ||     ||
;          ||        ||  ||     ||
;          \/        ||  ||     ||
;      |---------|<==||  ||     ||
;      |    C    |       ||     ||
;      |---------|=======||     ||
;          ||                   ||
;          \/                   ||
;      |---------|              ||
;      |    D    |================
;      |---------|
;          ||
;          \/
;      |---------|
;      |   exit  |
;      |---------|

%opencl.image2d_t.0 = type opaque

; Function Attrs: nounwind readnone
define void @__Vectorized_.KrnlFilterMain(%opencl.image2d_t.0 addrspace(1)* nocapture %imgIn, %opencl.image2d_t.0 addrspace(1)* nocapture %imgOut, i32 %iImgWidth, i32 %iImgHeight) #0 {
entry:
  %imgLocal1 = alloca [25 x float], align 16
  %imgLocal2 = alloca [25 x float], align 16
  %imgLocal3 = alloca [25 x float], align 16
  %arrayidx34 = getelementptr [25 x float], [25 x float]* %imgLocal1, i64 0, i64 0
  %arrayidx35 = getelementptr [25 x float], [25 x float]* %imgLocal2, i64 0, i64 0
  %arrayidx36 = getelementptr [25 x float], [25 x float]* %imgLocal3, i64 0, i64 0
  br label %while.body

while.cond.loopexit:                              ; preds = %for.inc
  %cmp = icmp slt i32 %mul, 2
  %0 = and i8 %bSwapped.2, 1
  %tobool = icmp eq i8 %0, 0
  %or.cond = and i1 %cmp, %tobool
  br i1 %or.cond, label %while.end, label %phi-split-bb

phi-split-bb:                                     ; preds = %while.cond.loopexit, %while.body
  %new_phi = phi i8 [ %bSwapped.05, %while.body ], [ %bSwapped.2, %while.cond.loopexit ]
  br label %while.body

while.body:                                       ; preds = %phi-split-bb, %entry
  %bSwapped.05 = phi i8 [ 0, %entry ], [ %new_phi, %phi-split-bb ]
  %iGap.04 = phi i32 [ 2, %entry ], [ %mul, %phi-split-bb ]
  %mul = mul nsw i32 %iGap.04, 10
  %cmp11 = icmp slt i32 %mul, 25
  br i1 %cmp11, label %for.body.lr.ph, label %phi-split-bb

for.body.lr.ph:                                   ; preds = %while.body
  %1 = mul i32 %iGap.04, -10
  %2 = add i32 %1, 24
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %bSwapped.12 = phi i8 [ %bSwapped.05, %for.body.lr.ph ], [ %bSwapped.2, %for.inc ]
  %arrayidx7 = getelementptr [25 x float], [25 x float]* %imgLocal1, i64 0, i64 %indvars.iv
  %3 = load float, float* %arrayidx7
  %conv = fptosi float %3 to i32
  %tobool2 = icmp eq i32 %conv, 0
  br i1 %tobool2, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %4 = load float, float* %arrayidx35
  %5 = load float, float* %arrayidx36
  store float 0x3FB99999A0000000, float* %arrayidx34
  store float %4, float* %arrayidx35
  store float %5, float* %arrayidx36
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %bSwapped.2 = phi i8 [ 1, %if.then ], [ %bSwapped.12, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %2
  br i1 %exitcond, label %while.cond.loopexit, label %for.body

while.end:                                        ; preds = %while.cond.loopexit
  ret void
}

; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'amd_dct.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [8 x i8] c"2222000\00"		; <[8 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: getIdx
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.end92.loopexit, %for.end92.loopexit18
; CHECK: phi-split-bb1:                                    ; preds = %for.end, %phi-split-bb
; CHECK: ret

define i32 @getIdx(i32 %blockIdx, i32 %blockIdy, i32 %localIdx, i32 %localIdy, i32 %blockWidth, i32 %globalWidth) nounwind readnone {
entry:
  %mul = mul i32 %blockWidth, %blockIdx
  %mul6 = mul i32 %blockWidth, %blockIdy
  %add8 = add i32 %mul6, %localIdy
  %mul11 = mul i32 %add8, %globalWidth
  %add = add i32 %mul, %localIdx
  %add13 = add i32 %add, %mul11
  ret i32 %add13
}

define void @DCT(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(1)* %dct, float addrspace(1)* %inter, i32 %width, i32 %blockWidth, i32 %inverse, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %call1 = call i32 @get_global_id(i32 1) nounwind
  %call2 = call i32 @get_group_id(i32 0) nounwind
  %call3 = call i32 @get_group_id(i32 1) nounwind
  %call4 = call i32 @get_local_id(i32 0) nounwind
  %call5 = call i32 @get_local_id(i32 1) nounwind
  %mul = mul i32 %call1, %width
  %add = add i32 %mul, %call
  %cmp13 = icmp eq i32 %blockWidth, 0
  br i1 %cmp13, label %for.end.thread, label %for.body.lr.ph

for.end.thread:                                   ; preds = %entry
  %arrayidx4417 = getelementptr float addrspace(1)* %inter, i32 %add
  store float 0.000000e+00, float addrspace(1)* %arrayidx4417, align 4
  call void @barrier(i32 1) nounwind
  br label %for.end92

for.body.lr.ph:                                   ; preds = %entry
  %tobool = icmp eq i32 %inverse, 0
  %mul.i = mul i32 %call2, %blockWidth
  %mul6.i = mul i32 %call3, %blockWidth
  %add.i = add i32 %call5, %mul.i
  %mul15 = mul i32 %call4, %blockWidth
  br i1 %tobool, label %cond.end.us.preheader, label %cond.end.preheader

cond.end.us.preheader:                            ; preds = %for.body.lr.ph
  br label %cond.end.us

cond.end.preheader:                               ; preds = %for.body.lr.ph
  br label %cond.end

cond.end.us:                                      ; preds = %cond.end.us.preheader, %cond.end.us
  %storemerge115.us = phi i32 [ %inc.us, %cond.end.us ], [ 0, %cond.end.us.preheader ]
  %tmp96914.us = phi float [ %add40.us, %cond.end.us ], [ 0.000000e+00, %cond.end.us.preheader ]
  %mul20.us = mul i32 %storemerge115.us, %blockWidth
  %add22.us = add i32 %mul20.us, %call4
  %add8.i.us = add i32 %storemerge115.us, %mul6.i
  %mul11.i.us = mul i32 %add8.i.us, %width
  %add13.i.us = add i32 %add.i, %mul11.i.us
  %arrayidx.us = getelementptr float addrspace(1)* %dct, i32 %add22.us
  %tmp34.us = load float addrspace(1)* %arrayidx.us, align 4
  %arrayidx37.us = getelementptr float addrspace(1)* %input, i32 %add13.i.us
  %tmp38.us = load float addrspace(1)* %arrayidx37.us, align 4
  %mul39.us = fmul float %tmp34.us, %tmp38.us
  %add40.us = fadd float %tmp96914.us, %mul39.us
  %inc.us = add i32 %storemerge115.us, 1
  %cmp.us = icmp ult i32 %inc.us, %blockWidth
  br i1 %cmp.us, label %cond.end.us, label %for.end.loopexit19

cond.end:                                         ; preds = %cond.end.preheader, %cond.end
  %storemerge115 = phi i32 [ %inc, %cond.end ], [ 0, %cond.end.preheader ]
  %tmp96914 = phi float [ %add40, %cond.end ], [ 0.000000e+00, %cond.end.preheader ]
  %add17 = add i32 %storemerge115, %mul15
  %add8.i = add i32 %storemerge115, %mul6.i
  %mul11.i = mul i32 %add8.i, %width
  %add13.i = add i32 %add.i, %mul11.i
  %arrayidx = getelementptr float addrspace(1)* %dct, i32 %add17
  %tmp34 = load float addrspace(1)* %arrayidx, align 4
  %arrayidx37 = getelementptr float addrspace(1)* %input, i32 %add13.i
  %tmp38 = load float addrspace(1)* %arrayidx37, align 4
  %mul39 = fmul float %tmp34, %tmp38
  %add40 = fadd float %tmp96914, %mul39
  %inc = add i32 %storemerge115, 1
  %cmp = icmp ult i32 %inc, %blockWidth
  br i1 %cmp, label %cond.end, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %cond.end
  br label %for.end

for.end.loopexit19:                               ; preds = %cond.end.us
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit19, %for.end.loopexit
  %tmp969.lcssa = phi float [ %add40, %for.end.loopexit ], [ %add40.us, %for.end.loopexit19 ]
  %arrayidx44 = getelementptr float addrspace(1)* %inter, i32 %add
  store float %tmp969.lcssa, float addrspace(1)* %arrayidx44, align 4
  call void @barrier(i32 1) nounwind
  br i1 %cmp13, label %for.end92, label %for.body50.lr.ph

for.body50.lr.ph:                                 ; preds = %for.end
  %mul.i2 = mul i32 %call2, %blockWidth
  %mul6.i3 = mul i32 %call3, %blockWidth
  %add8.i4 = add i32 %mul6.i3, %call4
  %mul11.i5 = mul i32 %add8.i4, %width
  %add.i6 = add i32 %mul11.i5, %mul.i2
  %tobool63 = icmp eq i32 %inverse, 0
  %mul67 = mul i32 %call5, %blockWidth
  br i1 %tobool63, label %cond.end76.us.preheader, label %cond.end76.preheader

cond.end76.us.preheader:                          ; preds = %for.body50.lr.ph
  br label %cond.end76.us

cond.end76.preheader:                             ; preds = %for.body50.lr.ph
  br label %cond.end76

cond.end76.us:                                    ; preds = %cond.end76.us.preheader, %cond.end76.us
  %storemerge12.us = phi i32 [ %inc91.us, %cond.end76.us ], [ 0, %cond.end76.us.preheader ]
  %tmp96811.us = phi float [ %add88.us, %cond.end76.us ], [ 0.000000e+00, %cond.end76.us.preheader ]
  %add13.i7.us = add i32 %add.i6, %storemerge12.us
  %mul73.us = mul i32 %storemerge12.us, %blockWidth
  %add75.us = add i32 %mul73.us, %call5
  %arrayidx81.us = getelementptr float addrspace(1)* %inter, i32 %add13.i7.us
  %tmp82.us = load float addrspace(1)* %arrayidx81.us, align 4
  %arrayidx85.us = getelementptr float addrspace(1)* %dct, i32 %add75.us
  %tmp86.us = load float addrspace(1)* %arrayidx85.us, align 4
  %mul87.us = fmul float %tmp82.us, %tmp86.us
  %add88.us = fadd float %tmp96811.us, %mul87.us
  %inc91.us = add i32 %storemerge12.us, 1
  %cmp49.us = icmp ult i32 %inc91.us, %blockWidth
  br i1 %cmp49.us, label %cond.end76.us, label %for.end92.loopexit18

cond.end76:                                       ; preds = %cond.end76.preheader, %cond.end76
  %storemerge12 = phi i32 [ %inc91, %cond.end76 ], [ 0, %cond.end76.preheader ]
  %tmp96811 = phi float [ %add88, %cond.end76 ], [ 0.000000e+00, %cond.end76.preheader ]
  %add13.i7 = add i32 %add.i6, %storemerge12
  %add69 = add i32 %storemerge12, %mul67
  %arrayidx81 = getelementptr float addrspace(1)* %inter, i32 %add13.i7
  %tmp82 = load float addrspace(1)* %arrayidx81, align 4
  %arrayidx85 = getelementptr float addrspace(1)* %dct, i32 %add69
  %tmp86 = load float addrspace(1)* %arrayidx85, align 4
  %mul87 = fmul float %tmp82, %tmp86
  %add88 = fadd float %tmp96811, %mul87
  %inc91 = add i32 %storemerge12, 1
  %cmp49 = icmp ult i32 %inc91, %blockWidth
  br i1 %cmp49, label %cond.end76, label %for.end92.loopexit

for.end92.loopexit:                               ; preds = %cond.end76
  br label %for.end92

for.end92.loopexit18:                             ; preds = %cond.end76.us
  br label %for.end92

for.end92:                                        ; preds = %for.end92.loopexit18, %for.end92.loopexit, %for.end.thread, %for.end
  %tmp968.lcssa = phi float [ 0.000000e+00, %for.end ], [ 0.000000e+00, %for.end.thread ], [ %add88, %for.end92.loopexit ], [ %add88.us, %for.end92.loopexit18 ]
  %arrayidx95 = getelementptr float addrspace(1)* %output, i32 %add
  store float %tmp968.lcssa, float addrspace(1)* %arrayidx95, align 4
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_local_id(i32)

declare void @barrier(i32)

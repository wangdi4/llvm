; RUN: opt -passes=sycl-kernel-phi-canonicalization %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY-ALL
; RUN: opt -passes=sycl-kernel-phi-canonicalization %s -S -o - | FileCheck %s -check-prefix=SKIP

; RUN: opt -passes=sycl-kernel-phi-canonicalization -sycl-skip-non-barrier-function=false %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefixes=DEBUGIFY-NOSKIP,DEBUGIFY-ALL
; RUN: opt -passes=sycl-kernel-phi-canonicalization -sycl-skip-non-barrier-function=false %s -S -o - | FileCheck %s -check-prefix=NOSKIP

; ModuleID = 'PhiCanonCase8.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; This module was already processed by -O3 -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify passes
; SKIP-NOT: phi-split-bb

; NOSKIP: @PhiCanonCase8
; NOSKIP-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; NOSKIP: phi-split-bb:                                     ; preds = %for.cond65.preheader, %if.end82.loopexit
; NOSKIP: phi-split-bb1:                                    ; preds = %if.end82.loopexit5, %phi-split-bb
; NOSKIP: phi-split-bb3:                                    ; preds = %if.end82.loopexit6, %phi-split-bb1
; NOSKIP: phi-split-bb5:                                    ; preds = %if.then7, %if.end82
; NOSKIP: phi-split-bb7:                                    ; preds = %if.then, %phi-split-bb5
; NOSKIP: ret
define void @PhiCanonCase8(i32 %arg1, i32 %arg2, float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %a, i32 %call
  %tmp2 = load float, float addrspace(1)* %arrayidx, align 4
  %cmp = icmp sgt i32 %arg1, 3
  br i1 %cmp, label %if.then, label %if.end91

if.then:                                          ; preds = %entry
  %mul = fmul float %tmp2, 2.000000e+000
  %cmp6 = icmp sgt i32 %arg2, 5
  br i1 %cmp6, label %if.then7, label %if.end91

if.then7:                                         ; preds = %if.then
  %add = fadd float %mul, 1.000000e+001
  %cmp13 = fcmp ogt float %tmp2, 1.000000e+002
  br i1 %cmp13, label %if.then14, label %if.end91

if.then14:                                        ; preds = %if.then7
  %conv = sitofp i32 %arg1 to float
  %div = fdiv float %add, %conv
  %cmp19 = icmp sgt i32 %arg1, %arg2
  br i1 %cmp19, label %if.then21, label %if.end82

if.then21:                                        ; preds = %if.then14
  %add23 = fadd float %div, 1.800000e+001
  %cmp27 = fcmp olt float %add23, %conv
  br i1 %cmp27, label %for.body.preheader, label %if.else

for.body.preheader:                               ; preds = %if.then21
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i31.0109 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %temp.2108 = phi float [ %sub, %for.body ], [ %add23, %for.body.preheader ]
  %sub = fadd float %temp.2108, -1.000000e+000
  %inc = add nsw i32 %i31.0109, 1
  %exitcond4 = icmp eq i32 %inc, %arg2
  br i1 %exitcond4, label %if.end82.loopexit6, label %for.body

if.else:                                          ; preds = %if.then21
  %conv39 = sitofp i32 %arg2 to float
  %sub41 = fsub float %add23, %conv39
  %cmp45 = fcmp ogt float %sub41, %conv39
  br i1 %cmp45, label %for.body55.preheader, label %for.cond65.preheader

for.body55.preheader:                             ; preds = %if.else
  br label %for.body55

for.cond65.preheader:                             ; preds = %if.else
  %add6996 = add nsw i32 %arg2, %arg1
  %cmp7097 = icmp sgt i32 %add6996, 0
  br i1 %cmp7097, label %for.body72.preheader, label %if.end82

for.body72.preheader:                             ; preds = %for.cond65.preheader
  br label %for.body72

for.body55:                                       ; preds = %for.body55.preheader, %for.body55
  %i49.0103 = phi i32 [ %inc60, %for.body55 ], [ 0, %for.body55.preheader ]
  %temp.3102 = phi float [ %add57, %for.body55 ], [ %sub41, %for.body55.preheader ]
  %add57 = fadd float %temp.3102, 1.000000e+000
  %inc60 = add nsw i32 %i49.0103, 1
  %exitcond = icmp eq i32 %inc60, %arg1
  br i1 %exitcond, label %if.end82.loopexit5, label %for.body55

for.body72:                                       ; preds = %for.body72.preheader, %for.body72
  %i64.099 = phi i32 [ %inc79, %for.body72 ], [ 0, %for.body72.preheader ]
  %temp.498 = phi float [ %add76, %for.body72 ], [ %sub41, %for.body72.preheader ]
  %add76 = fadd float %temp.498, %conv39
  %inc79 = add nsw i32 %i64.099, 1
  %exitcond3 = icmp eq i32 %inc79, %add6996
  br i1 %exitcond3, label %if.end82.loopexit, label %for.body72

if.end82.loopexit:                                ; preds = %for.body72
  br label %if.end82

if.end82.loopexit5:                               ; preds = %for.body55
  br label %if.end82

if.end82.loopexit6:                               ; preds = %for.body
  br label %if.end82

if.end82:                                         ; preds = %if.end82.loopexit6, %if.end82.loopexit5, %if.end82.loopexit, %for.cond65.preheader, %if.then14
  %temp.1 = phi float [ %div, %if.then14 ], [ %sub41, %for.cond65.preheader ], [ %add76, %if.end82.loopexit ], [ %add57, %if.end82.loopexit5 ], [ %sub, %if.end82.loopexit6 ]
  %add85 = add nsw i32 %arg2, %arg1
  %conv86 = sitofp i32 %add85 to float
  %div88 = fdiv float %temp.1, %conv86
  br label %if.end91

if.end91:                                         ; preds = %if.end82, %if.then7, %if.then, %entry
  %temp.0 = phi float [ %div88, %if.end82 ], [ %add, %if.then7 ], [ %mul, %if.then ], [ %tmp2, %entry ]
  %arrayidx95 = getelementptr inbounds float, float addrspace(1)* %b, i32 %call
  store float %temp.0, float addrspace(1)* %arrayidx95, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32) readnone

; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function PhiCanonCase8 --  br label %phi-split-bb1
; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function PhiCanonCase8 --  br label %phi-split-bb3
; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function PhiCanonCase8 --  br label %if.end82
; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function PhiCanonCase8 --  br label %phi-split-bb7
; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function PhiCanonCase8 --  br label %if.end91
; DEBUGIFY-ALL-NOT: WARNING

; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Verify that we do not get stuck in infinite recursion when processing this file.
; CHECK: DO i1 = 0, %i.161 + -1
; CHECK: END LOOP


; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@DestBuffer = external global [150 x i8], align 16

; Function Attrs: nounwind uwtable
define void @main() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %cond.end, %entry
  br i1 undef, label %cond.end, label %cond.false

cond.false:                                       ; preds = %for.body
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %for.body
  br i1 undef, label %for.cond.8.preheader.preheader, label %for.body

for.cond.8.preheader.preheader:                   ; preds = %cond.end
  br label %for.cond.8.preheader

for.cond.8.preheader:                             ; preds = %for.inc.24, %for.cond.8.preheader.preheader
  %i.161 = phi i64 [ %inc25, %for.inc.24 ], [ 0, %for.cond.8.preheader.preheader ]
  %add.ptr = getelementptr inbounds [150 x i8], [150 x i8]* @DestBuffer, i64 0, i64 %i.161
  br label %for.cond.12.preheader

for.cond.12.preheader:                            ; preds = %for.inc.21, %for.cond.8.preheader
  br label %for.body.15

for.body.15:                                      ; preds = %for.inc.18, %for.cond.12.preheader
  br i1 undef, label %while.cond.6.preheader.i, label %while.body.i.preheader

while.body.i.preheader:                           ; preds = %for.body.15
  br label %while.body.i

while.cond.6.preheader.i.loopexit:                ; preds = %while.body.i
  br label %while.cond.6.preheader.i

while.cond.6.preheader.i:                         ; preds = %while.cond.6.preheader.i.loopexit, %for.body.15
  br i1 false, label %while.cond.30.preheader.i, label %while.body.9.lr.ph.i.preheader

while.body.9.lr.ph.i.preheader:                   ; preds = %while.cond.6.preheader.i
  br label %while.body.9.lr.ph.i

while.body.i:                                     ; preds = %while.body.i, %while.body.i.preheader
  %ptr.082.i = phi i8* [ %incdec.ptr.i, %while.body.i ], [ getelementptr inbounds ([150 x i8], [150 x i8]* @DestBuffer, i64 0, i64 0), %while.body.i.preheader ], !in.de.ssa !1
  %incdec.ptr.i = getelementptr inbounds i8, i8* %ptr.082.i, i64 1
  %cmp1.i = icmp eq i8* %incdec.ptr.i, %add.ptr
  br i1 %cmp1.i, label %while.cond.6.preheader.i.loopexit, label %while.body.i

while.cond.30.preheader.i.loopexit:               ; preds = %if.else.i
  br label %while.cond.30.preheader.i

while.cond.30.preheader.i.loopexit76:             ; preds = %if.then.13.i
  br label %while.cond.30.preheader.i

while.cond.30.preheader.i:                        ; preds = %while.cond.30.preheader.i.loopexit76, %while.cond.30.preheader.i.loopexit, %while.cond.6.preheader.i
  br i1 undef, label %while.body.33.i.preheader, label %t_strncpy.exit

while.body.33.i.preheader:                        ; preds = %while.cond.30.preheader.i
  br label %while.body.33.i

while.body.9.i:                                   ; preds = %if.else.i, %while.body.9.lr.ph.i
  br i1 undef, label %if.else.i, label %if.then.13.i

if.then.13.i:                                     ; preds = %while.body.9.i
  br i1 undef, label %while.body.9.lr.ph.i, label %while.cond.30.preheader.i.loopexit76

while.body.9.lr.ph.i:                             ; preds = %if.then.13.i, %while.body.9.lr.ph.i.preheader
  br label %while.body.9.i

if.else.i:                                        ; preds = %while.body.9.i
  br i1 undef, label %while.body.9.i, label %while.cond.30.preheader.i.loopexit

while.body.33.i:                                  ; preds = %while.body.33.i, %while.body.33.i.preheader
  br i1 undef, label %while.body.33.i, label %t_strncpy.exit.loopexit

t_strncpy.exit.loopexit:                          ; preds = %while.body.33.i
  br label %t_strncpy.exit

t_strncpy.exit:                                   ; preds = %t_strncpy.exit.loopexit, %while.cond.30.preheader.i
  br i1 undef, label %for.inc.18, label %if.then

if.then:                                          ; preds = %t_strncpy.exit
  br label %for.inc.18

for.inc.18:                                       ; preds = %if.then, %t_strncpy.exit
  br i1 undef, label %for.inc.21, label %for.body.15

for.inc.21:                                       ; preds = %for.inc.18
  br i1 undef, label %for.inc.24, label %for.cond.12.preheader

for.inc.24:                                       ; preds = %for.inc.21
  %inc25 = add nuw nsw i64 %i.161, 1
  br i1 undef, label %for.end.26, label %for.cond.8.preheader

for.end.26:                                       ; preds = %for.inc.24
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1686) (llvm/branches/loopopt 1915)"}
!1 = !{!"ptr.082.i.de.ssa"}

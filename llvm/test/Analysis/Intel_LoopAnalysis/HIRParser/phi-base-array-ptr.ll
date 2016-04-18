; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the store which has a phi base with a pointer to array type is parsed correctly.
; CHECK: DO i1 = 0, %N + -1
; CHECK-NEXT: {al:8}(%struc.bc)[i1][2][3] = undef;
; CHECK-NEXT:  END LOOP

; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

; Function Attrs: nounwind uwtable
define void @jinit_1pass_quantizer(i8* %struc, i64 %N) {
entry:
  %struc.bc = bitcast i8* %struc to [4 x [4 x i64]]*
  br label %for.body110.i

for.body110.i:                                    ; preds = %for.body110.i, %entry
  %odither.0335.i = phi [4 x [4 x i64]]* [ %struc.bc, %entry ], [ %incdec.ptr.i, %for.body110.i ]
  %i.1334.i = phi i64 [ 0, %entry ], [ %inc187.i, %for.body110.i ]
  %arrayidx167.i = getelementptr inbounds [4 x [4 x i64]], [4 x [4 x i64]]* %odither.0335.i, i64 0, i64 2, i64 3
  store i64 undef, i64* %arrayidx167.i, align 8
  %inc187.i = add nuw nsw i64 %i.1334.i, 1
  %incdec.ptr.i = getelementptr inbounds [4 x [4 x i64]], [4 x [4 x i64]]* %odither.0335.i, i64 1
  %exitcond = icmp eq i64 %inc187.i, %N
  br i1 %exitcond, label %create_colormap.exit.loopexit, label %for.body110.i

create_colormap.exit.loopexit:                    ; preds = %for.body110.i
  br label %create_colormap.exit

create_colormap.exit:                             ; preds = %create_colormap.exit.loopexit, %if.then100.i, %for.end95.i
  ret void
}


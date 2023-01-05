; RUN: opt -S -passes='loop(loop-rotate),print-alias-sets,loop-mssa(licm)' -o - < %s 2>&1 | FileCheck %s
; RUN: opt -S -passes='loop(loop-rotate),print-alias-sets' -o - < %s 2>&1 | FileCheck %s


; CHECK: Alias sets for function 'main':
; CHECK-NEXT: Alias Set Tracker: 1 alias sets for 2 pointer values.
; CHECK-NEXT:   AliasSet[0x{{[0-9a-f]+}}, 2] may alias, Mod/Ref   Pointers: (i32* %arrayidx, LocationSize::precise(4)), (i32* %p.02, LocationSize::precise(4))

; CHECK-NOT: store i32 0, i32 *null, align 4

define dso_local i32 @main() local_unnamed_addr {
entry:
  %gc = alloca [3 x i32], align 4
  br label %for.cond

for.cond:                                         ; preds = %for.end, %entry
  %i.0 = phi i32 [ 2, %entry ], [ %inc10, %for.end ]
  %p.0 = phi i32* [ null, %entry ], [ %arrayidx, %for.end ]
  %cmp = icmp ult i32 %i.0, 4
  br i1 %cmp, label %for.body, label %for.end11

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds [3 x i32], [3 x i32]* %gc, i64 0, i64 0
  store i32 0, i32* %arrayidx, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.body3, %for.body
  %j.0 = phi i32 [ %i.0, %for.body ], [ 1, %for.body3 ]
  br i1 false, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %0 = load i32, i32* %p.0, align 4
  br label %for.cond1

for.end:                                          ; preds = %for.cond1
  %arrayidx7 = getelementptr inbounds [3 x i32], [3 x i32]* %gc, i64 0, i64 1
  %inc10 = add i32 %i.0, 1
  br label %for.cond

for.end11:                                        ; preds = %for.cond
  %p.0.lcssa = phi i32* [ %p.0, %for.cond ]
  ret i32 0
}

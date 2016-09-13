; RUN: opt < %s -analyze -hir-scc-formation | FileCheck %s

; Verify that we do not construct the SCC (%0 -> %mul -> %add10 -> %add14) (by removing intermediate temp %mul) which contains two different cycles in it (%0 -> %mul -> %add10 -> %add14) and (%0 -> %add14).
; CHECK-NOT: SCC1


; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr #3 {
entry:
  %z = alloca i32, align 4
  %l = alloca [100 x i32], align 16
  %pi = alloca [100 x i32], align 16
  store i32 62, i32* %z, align 4
  %z.promoted = load i32, i32* %z, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv285 = phi i64 [ 82, %entry ], [ %indvars.iv.next286, %for.body ]
  %0 = phi i32 [ %z.promoted, %entry ], [ %add14, %for.body ]
  %1 = trunc i64 %indvars.iv285 to i32
  %mul = mul i32 %0, %1
  %2 = add nuw nsw i64 %indvars.iv285, 1
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* %l, i64 0, i64 %2
  %3 = load i32, i32* %arrayidx9, align 4
  %add10 = add i32 %3, %mul
  store i32 %add10, i32* %arrayidx9, align 4
  %add14 = add i32 %0, %add10
  %add18 = add i32 %add10, %add14
  %arrayidx21 = getelementptr inbounds [100 x i32], [100 x i32]* %pi, i64 0, i64 %2
  %4 = load i32, i32* %arrayidx21, align 4
  %mul22 = mul i32 %4, %add18
  store i32 %mul22, i32* %arrayidx21, align 4
  %indvars.iv.next286 = add nsw i64 %indvars.iv285, -1
  %cmp = icmp ugt i64 %indvars.iv.next286, 4
  br i1 %cmp, label %for.body, label %for.end

for.end:
  ret i32 0
}

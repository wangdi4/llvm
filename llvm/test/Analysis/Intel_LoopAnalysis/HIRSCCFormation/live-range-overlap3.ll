; RUN: opt < %s -analyze -hir-scc-formation | FileCheck %s

; Verify that we do not create SCC (v_ovm.0159 -> %v_ovm.2158 -> %or -> %or.lcssa) which has live-range overlap because %v_ovm.0159 is used in %or after the definition of %v_ovm.2158 in the same bblock.

; CHECK-NOT: SCC1

@a1_c = local_unnamed_addr global [192 x i64] zeroinitializer, align 16
@g_d = local_unnamed_addr global i32 32, align 4
@g_mox = local_unnamed_addr global i32 16, align 4
@g_tu = local_unnamed_addr global i64 26, align 8

define void @foo() {
entry:
  %0 = load i32, i32* @g_d, align 4
  %1 = load i32, i32* @g_mox, align 4
  %inc = add i32 %1, 1
  store i32 %inc, i32* @g_mox, align 4
  store i32 0, i32* @g_d, align 4
  %2 = load i64, i64* @g_tu, align 8
  br label %for.body

for.body:                                         ; preds = %for.end26, %entry
  %3 = phi i32 [ 0, %entry ], [ %inc32, %for.end26 ]
  %v_ykdxfz.0160 = phi i32 [ %0, %entry ], [ %conv30, %for.end26 ]
  %v_ovm.0159 = phi i32 [ 70, %entry ], [ %or.lcssa, %for.end26 ]
  %conv = zext i32 %v_ovm.0159 to i64
  br label %for.body14

for.body14:                                       ; preds = %for.body14, %for.body
  %v_ovm.2158 = phi i32 [ %or, %for.body14 ], [ 38, %for.body ]
  %v_h.1157 = phi i64 [ %inc25, %for.body14 ], [ 0, %for.body ]
  %or = or i32 %v_ovm.2158, %v_ovm.0159
  %mul20 = shl i64 %v_h.1157, 1
  %add21 = sub nuw nsw i64 64, %v_h.1157
  %sub = add nuw nsw i64 %add21, %mul20
  %arrayidx22 = getelementptr inbounds [192 x i64], [192 x i64]* @a1_c, i64 0, i64 %sub
  %4 = load i64, i64* %arrayidx22, align 8
  %sub23 = sub i64 %4, %conv
  store i64 %sub23, i64* %arrayidx22, align 8
  %inc25 = add nuw nsw i64 %v_h.1157, 1
  %exitcond = icmp eq i64 %inc25, 64
  br i1 %exitcond, label %for.end26, label %for.body14

for.end26:                                        ; preds = %for.body14
  %or.lcssa = phi i32 [ %or, %for.body14 ]
  %conv28 = zext i32 %v_ykdxfz.0160 to i64
  %add27.neg = add nuw nsw i64 %conv28, 4294967208
  %sub29 = sub i64 %add27.neg, %2
  %conv30 = trunc i64 %sub29 to i32
  %inc32 = add nuw nsw i32 %3, 1
  %exitcond173 = icmp eq i32 %inc32, 42
  br i1 %exitcond173, label %for.end33, label %for.body

for.end33:                                        ; preds = %for.end26
  %conv30.lcssa = phi i32 [ %conv30, %for.end26 ]
  ret void
}

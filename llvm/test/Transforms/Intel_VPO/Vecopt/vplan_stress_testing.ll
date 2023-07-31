; RUN: opt < %s -passes="vplan-vec" -vplan-build-vect-candidates=1000000 -disable-output

@arr.float.1 = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@arr.float.2 = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@arr.float.3 = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16

define void @test_vectorize() local_unnamed_addr {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  %float.ld.idx = getelementptr inbounds [1024 x float], ptr @arr.float.1, i64 0, i64 %indvars.iv
  %float.ld = load float, ptr %float.ld.idx
  %float2.ld.idx = getelementptr inbounds [1024 x float], ptr @arr.float.2, i64 0, i64 %indvars.iv
  %float2.ld = load float, ptr %float2.ld.idx

  %float.add = fadd fast float %float.ld, %float2.ld
  %float.st.idx = getelementptr inbounds [1024 x float], ptr @arr.float.3, i64 0, i64 %indvars.iv
  store float %float.add, ptr %float.st.idx

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}


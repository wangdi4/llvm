; RUN: llvm-profdata merge %S/Inputs/intel-profx-indc-metadata.proftext -o %t.profdata
; RUN: opt < %s -S -passes=pgo-instr-use -pgo-test-profile-file=%t.profdata | FileCheck %s

; Test that intel_profx metadata is placed on an indirect call

; CHECK: define dso_local i32 @main()
; CHECK: call i32 %fp.1{{.*}}!intel-profx [[PROFX:![0-9]+]]
; CHECK: [[PROFX]] = !{!"intel_profx", i64 10000}

define internal i32 @func0(i32 %i) unnamed_addr #1 {
entry:
  %mul = mul nsw i32 %i, %i
  ret i32 %mul
}

define internal i32 @func1(i32 %i) unnamed_addr #1 {
entry:
  %mul = mul nsw i32 %i, %i
  %sub = sub nsw i32 %mul, %i
  ret i32 %sub
}

define internal i32 @func2(i32 %i) unnamed_addr #1 {
entry:
  %mul = mul nsw i32 %i, %i
  %add = add nsw i32 %mul, %i
  ret i32 %add
}

define dso_local i32 @main() local_unnamed_addr {
entry:
  br label %sw.bb

for.body:                                         ; preds = %sw.epilog
  %rem = urem i32 %inc, 3
  switch i32 %rem, label %sw.epilog [
    i32 0, label %sw.bb
    i32 1, label %sw.bb1
    i32 2, label %sw.bb2
  ]

sw.bb:                                            ; preds = %entry, %for.body
  %s.019 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %i.015 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  br label %sw.epilog

sw.bb1:                                           ; preds = %for.body
  br label %sw.epilog

sw.bb2:                                           ; preds = %for.body
  br label %sw.epilog

sw.epilog:                                        ; preds = %for.body, %sw.bb2, %sw.bb1, %sw.bb
  %s.018 = phi i32 [ %add, %for.body ], [ %add, %sw.bb2 ], [ %add, %sw.bb1 ], [ %s.019, %sw.bb ]
  %i.016 = phi i32 [ %inc, %for.body ], [ %inc, %sw.bb2 ], [ %inc, %sw.bb1 ], [ %i.015, %sw.bb ]
  %fp.1 = phi i32 (i32)* [ %fp.1, %for.body ], [ @func2, %sw.bb2 ], [ @func1, %sw.bb1 ], [ @func0, %sw.bb ]
  %call = call i32 %fp.1(i32 %i.016)
  %add = add nsw i32 %s.018, %call
  %inc = add nuw nsw i32 %i.016, 1
  %cmp = icmp ult i32 %inc, 10000
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %sw.epilog
  ret i32 %add
}


; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     -disable-output -vplan-cost-model-print-analysis-for-vf=4 \
; RUN:     | FileCheck %s

; The test verifies that CM doesn't crush on given input and fixes the costs
; of load/store of aggregate data type data.

; CHECK: Cost 8 for %complex_64bit = type { float, float } %vp{{[0-9]+}} = load %complex_64bit* %vp{{[0-9]+}}
; CHECK: Cost 8 for store %complex_64bit = type { float, float } %vp{{[0-9]+}} %complex_64bit* %vp{{[0-9]+}}

%complex_64bit = type { float, float }

@pR = internal unnamed_addr global [100 x %complex_64bit] zeroinitializer, align 16
@pS = internal unnamed_addr global [100 x %complex_64bit] zeroinitializer, align 16

define void @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:
  %p0 = phi i32 [ 0, %entry ], [ %add21, %for.body ]
  %int_sext17 = sext i32 %p0 to i64
  %pR = getelementptr inbounds [100 x %complex_64bit], [100 x %complex_64bit]* @pR, i32 0, i64 %int_sext17
  %pR_fetch = load %complex_64bit, %complex_64bit* %pR
  %pS = getelementptr inbounds [100 x %complex_64bit], [100 x %complex_64bit]* @pS, i32 0, i64 %int_sext17
  store %complex_64bit %pR_fetch, %complex_64bit* %pS
  %add21 = add nsw i32 %p0, 1
  %rel = icmp sle i32 %add21, 5
  br i1 %rel, label %for.body, label %for.end

for.end:
  ret void
}

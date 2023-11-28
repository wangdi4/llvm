; RUN: opt < %s -S -passes=loop-unroll -pass-remarks=loop-unroll -unroll-count=16 2>&1 | FileCheck -check-prefix=COMPLETE-UNROLL %s
; RUN: opt < %s -S -passes=loop-unroll -pass-remarks=loop-unroll -unroll-count=4 2>&1 | FileCheck -check-prefix=PARTIAL-UNROLL %s
; RUN: opt < %s -S -passes=loop-unroll -pass-remarks=loop-unroll -unroll-count=4 -unroll-runtime=true -unroll-remainder 2>&1 | FileCheck %s --check-prefix=RUNTIME-UNROLL

; INTEL_CUSTOMIZATION
; RUN: opt -passes='loop-unroll,intel-ir-optreport-emitter' -unroll-count=16 -intel-opt-report=low -disable-output -intel-opt-report-file=stdout < %s | FileCheck -check-prefixes=OPTREPORT,OPTREPORT-COMPLETE-UNROLL %s
; RUN: opt -passes='loop-unroll,intel-ir-optreport-emitter' -unroll-count=4 -intel-opt-report=low -disable-output -intel-opt-report-file=stdout < %s | FileCheck -check-prefixes=OPTREPORT,OPTREPORT-PARTIAL-UNROLL %s
; end INTEL_CUSTOMIZATION

; COMPLETE-UNROLL: remark: {{.*}}: completely unrolled loop with 16 iterations
; PARTIAL-UNROLL: remark: {{.*}}: unrolled loop by a factor of 4
; RUNTIME-UNROLL: remark: {{.*}}: unrolled loop by a factor of 4

; INTEL_CUSTOMIZATION
; OPTREPORT: Global optimization report for : sum

; OPTREPORT-COMPLETE-UNROLL: LOOP BEGIN
; OPTREPORT-COMPLETE-UNROLL:     remark #25603: Loop has been completely unrolled by LLVM LoopUnroll
; OPTREPORT-COMPLETE-UNROLL: LOOP END

; OPTREPORT-PARTIAL-UNROLL: LOOP BEGIN
; OPTREPORT-PARTIAL-UNROLL:     remark #25604: Loop has been partially unrolled with factor 4 by LLVM LoopUnroll
; OPTREPORT-PARTIAL-UNROLL: LOOP END
; end INTEL_CUSTOMIZATION

define i32 @sum() {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %s.06 = phi i32 [ 0, %entry ], [ %add1, %for.body ]
  %i.05 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nsw i32 %i.05, 4
  %call = tail call i32 @baz(i32 %add) #2
  %add1 = add nsw i32 %call, %s.06
  %inc = add nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, 16
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 %add1
}

; RUNTIME-UNROLL-NOT: remark: {{.*}}: completely unrolled loop with 3 iterations
; RUNTIME-UNROLL: remark: {{.*}}: unrolled loop by a factor of 4

define i32 @runtime(i32 %n) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %s.06 = phi i32 [ 0, %entry ], [ %add1, %for.body ]
  %i.05 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nsw i32 %i.05, 4
  %call = tail call i32 @baz(i32 %add) #2
  %add1 = add nsw i32 %call, %s.06
  %inc = add nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 %add1
}

declare i32 @baz(i32)

; RUN: llvm-profdata merge %S/Inputs/intel-profx-metadata.proftext -o %t.profdata
; RUN: opt < %s -S -passes=pgo-instr-use -pgo-test-profile-file=%t.profdata | FileCheck %s

; Test that the call to foo outside the loop is instrumented with intel-profx
; metadata with count of 1.
; Test that the call to foo inside the loop is instrumented with intel-profx
; metadata with count of 10000.

@myglobal = dso_local local_unnamed_addr global i32 0, align 4

; CHECK: define dso_local i32 @main() local_unnamed_addr
; CHECK: call i32 @foo(), !intel-profx [[X1_MAIN_CS:![0-9]+]]
; CHECK: call i32 @foo(), !intel-profx [[X2_MAIN_CS:![0-9]+]]
; CHECK: [[X1_MAIN_CS]] = !{!"intel_profx", i64 0}
; CHECK: [[X2_MAIN_CS]] = !{!"intel_profx", i64 10000}

define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  ret i32 5
}

define dso_local i32 @main() local_unnamed_addr #1 {
entry:
  %0 = load i32, i32* @myglobal, align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %for.cond, label %if.then

if.then:                                          ; preds = %entry
  %call = call i32 @foo()
  br label %cleanup

for.cond:                                         ; preds = %entry, %for.body
  %i.0 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %s.0 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %cmp1 = icmp ult i32 %i.0, 10000
  br i1 %cmp1, label %for.body, label %cleanup

for.body:                                         ; preds = %for.cond
  %call2 = call i32 @foo()
  %add = add i32 %s.0, 5
  %inc = add i32 %i.0, 1
  br label %for.cond

cleanup:                                          ; preds = %for.cond, %if.then
  %retval.0 = phi i32 [ 5, %if.then ], [ %s.0, %for.cond ]
  ret i32 %retval.0
}

; RUN: llvm-profdata merge %S/Inputs/intel-profx-zero-metadata.proftext -o %t.profdata
; RUN: opt < %s -S -passes=pgo-instr-use -pgo-test-profile-file=%t.profdata | FileCheck %s

; Test that the call to @foo gets a intel_profx metadata that shows it has an
; execution count of 0.

@global = dso_local local_unnamed_addr global i32 -50, align 4

; CHECK: define fastcc void @bar()
; CHECK: call fastcc void @foo(), !intel-profx [[X1:![0-9]+]]
; CHECK: [[X1]] = !{!"intel_profx", i64 0}

define fastcc void @foo() unnamed_addr #1 {
entry:
  ret void
}

define fastcc void @bar() unnamed_addr #1 {
entry:
  call fastcc void @foo()
  ret void
}

define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %0 = load i32, i32* @global, align 4
  %cmp = icmp sgt i32 %0, 5
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call fastcc void @bar()
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %s.0 = phi i32 [ 52, %if.then ], [ 0, %entry ]
  ret i32 %s.0
}


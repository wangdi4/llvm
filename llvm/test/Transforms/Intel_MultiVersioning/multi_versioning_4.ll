; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -passes=multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s

; This test case checks that multiversioning was applied even if the compare
; instruction was passed through a freeze instruction, and then the freeze
; instruction is used for branching.

%struct.S = type { i8 }

;                                    / Branch
;  Arg - GEP - Load - Cmp NE - Freeze
;                                    \ Select

; CHECK: %3 = freeze i1 true
; CHECK: br i1 %3, label %if.then, label %if.else
; CHECK: %5 = freeze i1 false
; CHECK: br i1 %5, label %if.then.clone, label %if.else.clone

define i32 @foo1(%struct.S* %Arg) local_unnamed_addr {
entry:
  %0 = getelementptr inbounds %struct.S, %struct.S* %Arg, i64 0, i32 0
  %1 = load i8, i8* %0, align 1
  %2 = icmp ne i8 %1, 0
  %3 = freeze i1 %2
  br i1 %3, label %if.then, label %if.else

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  %4 = select i1 %3, i32 33, i32 22
  ret i32 %4
}

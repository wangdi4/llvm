; RUN: opt -passes=dpcpp-kernel-phi-canonicalization %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY-ALL
; RUN: opt -passes=dpcpp-kernel-phi-canonicalization %s -S -o - | FileCheck %s -check-prefix=SKIP

; RUN: opt -passes=dpcpp-kernel-phi-canonicalization -dpcpp-skip-non-barrier-function=false %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefixes=DEBUGIFY-NOSKIP,DEBUGIFY-ALL
; RUN: opt -passes=dpcpp-kernel-phi-canonicalization -dpcpp-skip-non-barrier-function=false %s -S -o - | FileCheck %s -check-prefix=NOSKIP

; ModuleID = 'file.s'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"
define i32 @m33ain(i32 %x, i32 %y) nounwind {
entry:
  %tobool = icmp eq i32 %x, 0
  br i1 %tobool, label %if.else, label %if.then

if.then:                                          ; preds = %entry
  tail call void @g(i32 %y) nounwind
  br label %UnifiedReturnBlock

if.else:                                          ; preds = %entry
  %tobool3 = icmp eq i32 %y, 0
  br i1 %tobool3, label %if.else6, label %if.then4

if.then4:                                         ; preds = %if.else
  tail call void @g(i32 0) nounwind
  br label %UnifiedReturnBlock

if.else6:                                         ; preds = %if.else
  tail call void @g(i32 -5) nounwind
  br label %UnifiedReturnBlock

; SKIP-NOT: phi-split-bb

; NOSKIP: br label %UnifiedReturnBlock
; NOSKIP: phi-split-bb
; NOSKIP: UnifiedReturnBlock:
; NOSKIP: preds =
; NOSKIP: phi-split-bb

UnifiedReturnBlock:                               ; preds = %if.else6, %if.then4, %if.then
  %UnifiedRetVal = phi i32 [ 2, %if.then ], [ 1, %if.then4 ], [ 4, %if.else6 ]
  ret i32 %UnifiedRetVal
}

declare void @g(i32)

; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function m33ain --  br label %UnifiedReturnBlock
; DEBUGIFY-ALL-NOT: WARNING

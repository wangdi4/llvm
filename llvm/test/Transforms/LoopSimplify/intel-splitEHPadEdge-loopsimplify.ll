; Test checks that the critical edge to cleanuppad instruction is split appropriately.

; RUN: opt < %s -passes=loop-simplify -S | FileCheck %s

target datalayout = "e-m:x-p:32:32-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:32-n8:16:32-a:0:32-S32"
target triple = "i386-pc-windows-msvc19.23.28105"

@globalvar = external dso_local local_unnamed_addr global i32, align 4

define dso_local i32 @test() local_unnamed_addr personality i32 (...)* @__CxxFrameHandler3 {
entry:
  %loop.iv = alloca i32, align 4
  %loop.ub = alloca i32, align 4
  store volatile i32 0, i32* %loop.iv, align 4
  store volatile i32 9, i32* %loop.ub, align 4
  br label %loophead

loophead:                                         ; preds = %looplatch, %entry
  %curr.iv = load volatile i32, i32* %loop.iv, align 4
  %ub = load volatile i32, i32* %loop.ub, align 4
  %cmp = icmp sgt i32 %curr.iv, %ub
  br i1 %cmp, label %loopexit, label %loopbody

loopexit:                                         ; preds = %loophead
  ret i32 0

loopbody:                                         ; preds = %loophead
  %gl = load volatile i32, i32* @globalvar, align 4
  %cmp1 = icmp sgt i32 %gl, 2
  br i1 %cmp1, label %if.then, label %loopbb1

if.then:                                          ; preds = %loopbody
  invoke void @foo()
          to label %returnbb unwind label %ehcleanup

returnbb:                                         ; preds = %if.then
  ret i32 1

loopbb1:                                          ; preds = %loopbody
  %y = load volatile i32, i32* %loop.ub, align 4
  invoke void @bar()
          to label %looplatch unwind label %ehcleanup

looplatch:                                        ; preds = %loopbb1
  %iv = load volatile i32, i32* %loop.iv, align 4
  %add1 = add nsw i32 %iv, 1
  store volatile i32 %add1, i32* %loop.iv, align 4
  br label %loophead

; CHECK: ehcleanup.loopexit:                               ; preds = %loopbb1
; CHECK:   %cpad.loopexit = cleanuppad within none []
; CHECK:   cleanupret from %cpad.loopexit unwind label %ehcleanup
; CHECK: ehcleanup:                                        ; preds = %ehcleanup.loopexit, %if.then
; CHECK:   %val = phi i32 [ %gl, %if.then ], [ %y, %ehcleanup.loopexit ]

ehcleanup:                                        ; preds = %loopbb1, %if.then
  %val = phi i32 [ %gl, %if.then ], [ %y, %loopbb1 ]
  %cpad = cleanuppad within none []
  call void @use_val(i32 %val)
  cleanupret from %cpad unwind to caller
}

declare void @use_val(i32)

declare void @foo()

declare void @bar()

declare dso_local i32 @__CxxFrameHandler3(...)

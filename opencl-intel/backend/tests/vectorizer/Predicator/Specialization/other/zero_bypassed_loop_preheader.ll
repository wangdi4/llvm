; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -loop-simplify -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @kernel
; CHECK: header{{[0-9]*}}:
; CHECK: br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK: br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        
; CHECK: footer{{[0-9]*}}:                                        
; CHECK: header{{[0-9]*}}:
; CHECK: br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:          
; CHECK: ret

; ModuleID = 'c:\work\temp\test.ll'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @kernel(i32* nocapture %arg0, i32* nocapture %arg1) nounwind uwtable {
BB0:
  %call.10.0 = tail call i64 @_Z13get_global_idj(i32 0) nounwind
  %trunc.10.0 = trunc i64 %call.10.0 to i32
  %sext.10.0 = sext i32 %trunc.10.0 to i64
  %getelementptr.14.0 = getelementptr inbounds i32, i32* %arg1, i64 %sext.10.0
  %load.10.0 = load i32, i32* %getelementptr.14.0, align 4, !tbaa !0
  %icmp.10.0 = icmp slt i32 %trunc.10.0, 50
  br i1 %icmp.10.0, label %BB1, label %BB7

BB1:                                              ; preds = %BB0
  %icmp.10.1 = icmp sgt i32 %trunc.10.0, 10
  br i1 %icmp.10.1, label %BB2, label %BB3

BB2:                                              ; preds = %BB1
  %add.10.0 = add nsw i32 %trunc.10.0, 5
  %sext.10.1 = sext i32 %add.10.0 to i64
  %getelementptr.14.1 = getelementptr inbounds i32, i32* %arg0, i64 %sext.10.1
  %load.10.1 = load i32, i32* %getelementptr.14.1, align 4, !tbaa !0
  %sext.10.2 = sext i32 %load.10.1 to i64
  br label %BB3

BB3:                                              ; preds = %BB2, %BB1
  %phi.10.0 = phi i64 [ %sext.10.2, %BB2 ], [ 0, %BB1 ]
  br label %BB4

BB4:                                              ; preds = %BB5, %BB3
  %phi.10.1 = phi i64 [ %add.10.1, %BB5 ], [ %phi.10.0, %BB3 ]
  %getelementptr.14.2 = getelementptr inbounds i32, i32* %arg0, i64 %phi.10.1
  %load.10.2 = load i32, i32* %getelementptr.14.2, align 4, !tbaa !0
  %icmp.10.2 = icmp eq i32 %load.10.2, %load.10.0
  br i1 %icmp.10.2, label %BB6, label %BB5

BB5:                                              ; preds = %BB4
  store i32 5, i32* %getelementptr.14.2, align 4, !tbaa !0
  %add.10.1 = add i64 %phi.10.1, 1
  %trunc.10.1 = trunc i64 %add.10.1 to i32
  %icmp.10.3 = icmp slt i32 %trunc.10.1, %trunc.10.0
  br i1 %icmp.10.3, label %BB4, label %BB7.loopexit

BB6:                                              ; preds = %BB4
  store i32 0, i32* %arg1, align 4, !tbaa !0
  br label %BB8

BB7.loopexit:                                     ; preds = %BB5
  br label %BB7

BB7:                                              ; preds = %BB7.loopexit, %BB0
  %getelementptr.14.3 = getelementptr inbounds i32, i32* %arg0, i64 %sext.10.0
  store i32 0, i32* %getelementptr.14.3, align 4, !tbaa !0
  br label %BB8

BB8:                                              ; preds = %BB7, %BB6
  ret void
}

declare i64 @_Z13get_global_idj(i32)

!0 = !{!"int", !1}
!1 = !{!"omnipotent char", !2}
!2 = !{!"Simple C/C++ TBAA"}

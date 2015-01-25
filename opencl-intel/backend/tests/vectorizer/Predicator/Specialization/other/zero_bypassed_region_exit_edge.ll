; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -loop-simplify -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @kernel
; CHECK: header{{[0-9]*}}:
; CHECK: br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        
; CHECK: %BB2_in_mask_maskspec
; CHECK: ret


; ModuleID = 'c:\work\temp\test_diamond.ll'

define void @kernel(i32* nocapture %arg0, i32 %arg1) nounwind uwtable {
BB0:
  %call.10.0 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %icmp.10.0 = icmp sgt i32 %call.10.0, 10
  br i1 %icmp.10.0, label %BB1, label %BB4

BB1:                                              ; preds = %BB0
  %add.10.1 = add i32 %arg1, %call.10.0
  %icmp.10.1 = icmp sgt i32 %arg1, %add.10.1
  br i1 %icmp.10.1, label %BB2, label %BB3

BB2:                                              ; preds = %BB1
  %sext.10.0 = sext i32 %call.10.0 to i64
  %getelementptr.14.0 = getelementptr inbounds i32* %arg0, i64 %sext.10.0
  store i32 0, i32* %getelementptr.14.0, align 4
  br label %BB3

BB3:                                              ; preds = %BB2, %BB1
  br label %BB4

BB4:                                              ; preds = %BB3, %BB0
  ret void
}

declare i32 @_Z13get_global_idj(i32)

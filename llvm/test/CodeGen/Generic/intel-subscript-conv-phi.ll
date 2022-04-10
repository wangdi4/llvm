; RUN: opt < %s -S -convert-to-subscript | FileCheck %s
;
; CHECK: for.body.preheader:
; CHECK-NEXT: {{.*}} = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 0, i64 200, i32* elementtype(i32) getelementptr inbounds ([50 x i32], [50 x i32]* @A, i64 0, i64 0), i64 0)
; CHECK-NEXT; {..*}} = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 0, i64 4, i32* elementtype(i32) {{.*}}, i64 10)

; RUN: opt < %s -S -opaque-pointers -convert-to-subscript | FileCheck %s --check-prefix=OPAQUE
;
; OPAQUE: for.body.preheader:
; OPAQUE-NEXT: {{.*}} = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 200, ptr elementtype(i32) {{.*}}, i64 0)
; OPAQUE-NEXT; {..*}} = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 4, ptr elementtype(i32) {{.*}}, i64 10)

; ModuleID = 'array-init-phi-base-ptr-iv.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [50 x i32] zeroinitializer, align 16

define void @foo(i32 %n) {
entry:
  %cmp.6 = icmp sgt i32 %n, 0
  br i1 %cmp.6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.08 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %p.07 = phi i32* [ %incdec.ptr, %for.body ], [ getelementptr inbounds ([50 x i32], [50 x i32]* @A, i64 0, i64 10), %for.body.preheader ]
  store i32 %i.08, i32* %p.07, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %p.07, i64 1
  %inc = add nuw nsw i32 %i.08, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

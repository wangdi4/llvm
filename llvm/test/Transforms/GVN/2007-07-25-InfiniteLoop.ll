<<<<<<< HEAD
; RUN: opt < %s -basic-aa -gvn -S | FileCheck %s
=======
; RUN: opt < %s -passes=gvn -S | FileCheck %s
>>>>>>> 881c6c0d46ae1b72fb60bbb6a547577f79a5d14f

	%struct.INT2 = type { i32, i32 }
@blkshifts = external global %struct.INT2*		; <%struct.INT2**> [#uses=2]

define i32 @xcompact() {
entry:
	store %struct.INT2* null, %struct.INT2** @blkshifts, align 4
	br label %bb

bb:		; preds = %bb, %entry
	%tmp10 = load %struct.INT2*, %struct.INT2** @blkshifts, align 4		; <%struct.INT2*> [#uses=0]
; CHECK-NOT:  %tmp10
	br label %bb
}

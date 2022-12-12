<<<<<<< HEAD
; RUN: opt < %s -basic-aa -licm -disable-output
=======
; RUN: opt < %s -passes=licm -disable-output
>>>>>>> 80e8f2beeb954f8c241897099bb01b24da400e8a

;%MoveArray = external global [64 x ulong]

define void @InitMoveArray() {
bb3:
	%X = alloca [2 x i64]		; <ptr> [#uses=1]
	br i1 false, label %bb13, label %bb4
bb4:		; preds = %bb3
	%reg3011 = getelementptr [2 x i64], ptr %X, i64 0, i64 0		; <ptr> [#uses=1]
	br label %bb8
bb8:		; preds = %bb8, %bb4
	store i64 0, ptr %reg3011
	br i1 false, label %bb8, label %bb13
bb13:		; preds = %bb8, %bb3
	ret void
}


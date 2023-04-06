; REQUIRES: asserts
; This test checks that the argument alignment didn't pass the analysis process
; and prints the proper debug messages.
; This test is same as argument_align_4.ll except "null" is also stored into
; the malloced location in addition to return address of "calloc".

; RUN: opt < %s -passes=intel-argument-alignment -whole-program-assume -debug-only=intel-argument-alignment -disable-output 2>&1

; CHECK: Candidates for argument alignment: 0
; CHECK-NEXT: Reason: Candidates didn't pass analysis

declare noalias ptr @calloc(i64, i64)
declare noalias ptr @malloc(i64)

define internal fastcc void @foo(ptr %arg, i64 %arg1) {
entry:
  %i = ptrtoint ptr %arg to i64
  %i2 = and i64 %i, 7
  %i3 = icmp eq i64 %i2, 0
  br i1 %i3, label %if_bb, label %end

if_bb:                                            ; preds = %else_bb, %entry
  %i4 = phi ptr [ %arg, %entry ], [ %i7, %else_bb ]
  %i5 = phi i1 [ %i3, %entry ], [ %i10, %else_bb ]
  br i1 %i5, label %else_bb, label %end

else_bb:                                          ; preds = %if_bb
  %i6 = sub i64 0, %arg1
  %i7 = getelementptr inbounds i8, ptr %i4, i64 %i6
  %i8 = ptrtoint ptr %i7 to i64
  %i9 = and i64 %i8, 7
  %i10 = icmp eq i64 %i9, 0
  %i11 = lshr i64 %i6, 3
  %i12 = icmp ult i64 %i11, 7
  br i1 %i12, label %if_bb, label %end

end:                                              ; preds = %else_bb, %if_bb, %entry
  ret void
}

define void @bar(i64 %arg, i64 %arg1) {
entry:
  %call0 = tail call noalias ptr @calloc(i64 %arg, i64 8)
  %call1 = tail call noalias ptr @malloc(i64 8)
  store ptr null, ptr %call1, align 8
  store ptr %call0, ptr %call1, align 8
  %val = load ptr, ptr %call1, align 8
  %bc3 = bitcast ptr %val to ptr
  tail call fastcc void @foo(ptr %bc3, i64 %arg1)
  ret void
}

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vplan-force-vf=4 -S < %s 2>&1 | FileCheck %s

; CHECK:                + DO i1 = 0, 1023, 4 <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:           |   @llvm.assume(-1); [ align(&((%specific_integral*)(%this)[i1]), 1) ]
; CHECK-NEXT:           |   [[VP_EXTRACT0:%.*]] = extractelement &((<4 x ptr>)(%this)[i1 + <i64 0, i64 1, i64 2, i64 3>]), 1;
; CHECK-NEXT:           |   @llvm.assume(-1); [ align([[VP_EXTRACT0]], 1) ]
; CHECK-NEXT:           |   [[VP_EXTRACT1:%.*]] = extractelement &((<4 x ptr>)(%this)[i1 + <i64 0, i64 1, i64 2, i64 3>]), 2;
; CHECK-NEXT:           |   @llvm.assume(-1); [ align([[VP_EXTRACT1]], 1) ]
; CHECK-NEXT:           |   [[VP_EXTRACT2:%.*]] = extractelement &((<4 x ptr>)(%this)[i1 + <i64 0, i64 1, i64 2, i64 3>]), 3;
; CHECK-NEXT:           |   @llvm.assume(-1); [ align([[VP_EXTRACT2]], 1) ]
; CHECK-NEXT:           + END LOOP

; CHEC:         call void @llvm.assume(i1 true) [ "align"(ptr [[VP_LOAD0:%.*]], i64 1) ]
; CHECK:        [[VP_EXTRACT2:%.*]] = extractelement <4 x ptr> [[VP_GEP2:%.*]], i64 1
; CHECK-NEXT:   store ptr [[VP_EXTRACT2]], ptr [[VP_STORE2:%.*]], align 8
; CHECK-NEXT:   [[VP_LOAD2:%.*]] = load ptr, ptr [[VP_STORE2]], align 8
; CHECK-NEXT:   call void @llvm.assume(i1 true) [ "align"(ptr [[VP_LOAD2]], i64 1) ]
; CHECK:        [[VP_EXTRACT3:%.*]] = extractelement <4 x ptr> [[VP_GEP3:%.*]], i64 2
; CHECK-NEXT:   store ptr [[VP_EXTRACT3]], ptr [[VP_STORE3:%.*]], align 8
; CHECK-NEXT:   [[VP_LOAD3:%.*]] = load ptr, ptr [[VP_STORE3]], align 8
; CHECK-NEXT:   call void @llvm.assume(i1 true) [ "align"(ptr [[VP_LOAD3]], i64 1) ]
; CHECK:        [[VP_EXTRACT4:%.*]] = extractelement <4 x ptr> [[VP_GEP4:%.*]], i64 3
; CHECK-NEXT:   store ptr [[VP_EXTRACT4]], ptr [[VP_STORE4:%.*]], align 8
; CHECK-NEXT:   [[VP_LOAD4:%.*]] = load ptr, ptr [[VP_STORE4]], align 8
; CHECK-NEXT:   call void @llvm.assume(i1 true) [ "align"(ptr [[VP_LOAD4]], i64 1) ]


%"specific_integral" = type { %struct.anon }
%struct.anon = type { [4 x i8] }

define dso_local void @foo(ptr noundef nonnull align 8 %this) align 2 {
entry:                             ; preds = %entry
  br label %for.body

for.body:                                    ; preds = %for.body, %for.body.preheader
  %dec.i.i = phi i32 [ %dec.i, %for.body ], [ 0, %entry ]
  %arr.i = phi ptr [ %incdec.ptr.i, %for.body ], [ %this, %entry ]
  call void @llvm.assume(i1 true) [ "align"(ptr %arr.i, i64 1) ]
  %incdec.ptr.i = getelementptr inbounds %"specific_integral", ptr %arr.i, i64 1
  %dec.i = add i32 %dec.i.i, 1
  %cmp.not.i = icmp eq i32 %dec.i, 1024
  br i1 %cmp.not.i, label %latch, label %for.body

latch: ; preds = %for.body
  br label %exit

exit: ; preds = %latch
  ret void
}

declare void @llvm.assume(i1 noundef) 

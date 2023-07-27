; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-cg" -force-hir-cg -S %s 2>&1 | FileCheck %s

; Verify that we generate code for this addressOf ref successfully:
; &((<4 x float>*)(%reg_save_area)[%fp_offset])

; CHECK:  + DO i1 = 0, %argCount + -1, 1   <DO_LOOP>
; CHECK:  |   if (%fp_offset <u 161)
; CHECK:  |   {
; CHECK:  |      %vaarg.addr = &((%reg_save_area)[%fp_offset]);
; CHECK:  |      %fp_offset = %fp_offset  +  16;
; CHECK:  |   }
; CHECK:  |   else
; CHECK:  |   {
; CHECK:  |      %vaarg.addr = &((%i11)[0]);
; CHECK:  |   }
; CHECK:  |   (%m)[0] = (%vaarg.addr)[0];
; CHECK:  + END LOOP

; CHECK: region{{.*}}:

; CHECK:       [[GEP:%.*]] = getelementptr i8, ptr %reg_save_area, i64 {{.*}}
; CHECK-NEXT:  store ptr [[GEP]]


define i32 @test(i32 %argCount, i32 %fp_offset.pre, ptr %reg_save_area, ptr %i11, ptr %m) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc17, %entry
  %fp_offset = phi i32 [ %fp_offset53, %for.inc17 ], [ %fp_offset.pre, %entry ]
  %i.047 = phi i32 [ %inc18, %for.inc17 ], [ 0, %entry ]
  %fits_in_fp = icmp ult i32 %fp_offset, 161
  br i1 %fits_in_fp, label %vaarg.in_reg, label %vaarg.in_mem

vaarg.in_reg:                                     ; preds = %for.body
  %i4 = zext i32 %fp_offset to i64
  %i5 = getelementptr i8, ptr %reg_save_area, i64 %i4
  %i6 = bitcast ptr %i5 to ptr
  %i7 = add nuw nsw i32 %fp_offset, 16
  br label %for.inc17

vaarg.in_mem:                                     ; preds = %for.body
  br label %for.inc17

for.inc17:                                        ; preds = %vaarg.in_mem, %vaarg.in_reg
  %fp_offset53 = phi i32 [ %i7, %vaarg.in_reg ], [ %fp_offset, %vaarg.in_mem ]
  %vaarg.addr = phi ptr [ %i6, %vaarg.in_reg ], [ %i11, %vaarg.in_mem ]
  %i12 = load <4 x float>, ptr %vaarg.addr, align 16
  store <4 x float> %i12, ptr %m, align 16
  %inc18 = add nuw nsw i32 %i.047, 1
  %exitcond52.not = icmp eq i32 %inc18, %argCount
  br i1 %exitcond52.not, label %exit, label %for.body

exit:
  ret i32 0
}


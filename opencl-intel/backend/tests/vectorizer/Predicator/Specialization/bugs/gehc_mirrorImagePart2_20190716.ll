; Predicator pass hung on following code (CORC-5186).
; RUN: llvm-as %s -o %t.bc
; RUN: %oclopt -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;; The IR is dumped at the beginning of Predicator::runOnFunction()
;; when calling clBuildProgram from the following source.
;; void __kernel mirrorImagePart2()
;; {
;;     int index = (int)get_global_id(0);
;;
;;     bool exit_loop = false;
;;     bool cond1 = false;
;;
;;     int outIndex = index;
;;     while(!exit_loop)
;;     {
;;         if (outIndex)
;;         {
;;             cond1 = true;
;;         }
;;         if(cond1)
;;         {
;;         }
;;         else if((outIndex<=3)&&(outIndex>2))
;;         {
;;         }
;;         else if (outIndex>3)
;;         {
;;             outIndex = (index - outIndex)*2+1;
;;         }
;;         else
;;         {
;;             exit_loop = true;
;;         }
;;     }
;; }


; CHECK: @__Vectorized_.mirrorImagePart2
; CHECK-NOT: bypassAuxExitBB
; CHECK: ret

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent nounwind
define void @__Vectorized_.mirrorImagePart2() {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2
  %conv = trunc i64 %call to i32
  br label %while.body

while.body:                                       ; preds = %if.end15, %entry
  %outIndex.010 = phi i32 [ %conv, %entry ], [ %outIndex.1, %if.end15 ]
  %cond1.09 = phi i8 [ 0, %entry ], [ %spec.select, %if.end15 ]
  %tobool1 = icmp eq i32 %outIndex.010, 0
  %spec.select = select i1 %tobool1, i8 %cond1.09, i8 1
  %0 = and i8 %spec.select, 1
  %tobool2 = icmp eq i8 %0, 0
  br i1 %tobool2, label %if.else, label %if.end15

if.else:                                          ; preds = %while.body
  %1 = icmp eq i32 %outIndex.010, 3
  br i1 %1, label %phi-split-bb, label %if.else8

if.else8:                                         ; preds = %if.else
  %cmp9 = icmp sgt i32 %outIndex.010, 3
  br i1 %cmp9, label %if.then11, label %while.end

if.then11:                                        ; preds = %if.else8
  %sub = sub nsw i32 %conv, %outIndex.010
  %mul = shl nsw i32 %sub, 1
  %add = or i32 %mul, 1
  br label %phi-split-bb

phi-split-bb:                                     ; preds = %if.else, %if.then11
  %new_phi = phi i32 [ %add, %if.then11 ], [ 3, %if.else ]
  br label %if.end15

if.end15:                                         ; preds = %phi-split-bb, %while.body
  %outIndex.1 = phi i32 [ %outIndex.010, %while.body ], [ %new_phi, %phi-split-bb ]
  br label %while.body

while.end:                                        ; preds = %if.else8
  ret void
}


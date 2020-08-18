; RUN: opt -basic-aa -licm -S < %s | FileCheck %s
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basic-aa -licm -S | FileCheck %s
;
; struct matrix {
;   float *ptr;
;   int row_size;
; 
; public:
;   matrix(int size) {
;     ptr = (float *)malloc(sizeof(float) * size * size);
;     memset(ptr, 0, sizeof(float) * size * size);
;     row_size = size;
;   }
;   ~matrix() { free(ptr); }
;   matrix operator*(matrix &y) {
;     int size = y.row_size;
;     matrix temp(size);
;     for (int i = 0; i < size; i++) {
;       for (int j = 0; j < size; j++) {
;         temp.ptr[(i * size) + j] = 0;
;         for (int k = 0; k < size; k++)
;           temp.ptr[(i * size) + j] +=
;               (ptr[(i * size) + k] * y.ptr[(k * size) + j]);
;       }
;     }
;     return temp;
;   }
; };
; After the basic-aa identifies there is no overlap between the array reference
; temp.ptr[(i * size) + j] and the references ptr[(i * size) + k] and 
; y.ptr[(k * size) + j]), the LICM should sink the store out of the loop. 
; The store in the basic block %for.body.9.us.us will be removed.

; INTEL
; CHECK:       @_ZN6matrixmlERS_
; CHECK-LABEL: for.body.9.us.us:
; CHECK:       fmul
; CHECK:       fadd
; CHECK-NOT:   store
; CHECK-LABEL: for.body.5.lr.ph.split.us.us:


; ModuleID = 'SepiaFilterCilkPlus.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.matrix = type <{ float*, i32, [4 x i8] }>

$_ZN6matrixmlERS_ = comdat any

; Function Attrs: noinline nounwind uwtable
define weak_odr void @_ZN6matrixmlERS_(%struct.matrix* noalias nocapture sret %agg.result, %struct.matrix* nocapture readonly %this, %struct.matrix* nocapture readonly dereferenceable(16) %y) #0 comdat align 2 {
entry:
  %row_size = getelementptr inbounds %struct.matrix, %struct.matrix* %y, i64 0, i32 1
  %0 = load i32, i32* %row_size, align 4, !tbaa !1
  %conv.i = sext i32 %0 to i64
  %mul.i = shl nsw i64 %conv.i, 2
  %mul3.i = mul i64 %mul.i, %conv.i
  %call.i = tail call noalias i8* @malloc(i64 %mul3.i) #2
  %1 = bitcast %struct.matrix* %agg.result to i8**
  store i8* %call.i, i8** %1, align 8, !tbaa !7
  tail call void @llvm.memset.p0i8.i64(i8* %call.i, i8 0, i64 %mul3.i, i32 4, i1 false) #2
  %row_size.i = getelementptr inbounds %struct.matrix, %struct.matrix* %agg.result, i64 0, i32 1
  store i32 %0, i32* %row_size.i, align 4, !tbaa !1
  %cmp.61 = icmp sgt i32 %0, 0
  %2 = bitcast i8* %call.i to float*
  br i1 %cmp.61, label %for.cond.2.preheader.lr.ph.split.us, label %nrvo.skipdtor

for.cond.2.preheader.lr.ph.split.us:              ; preds = %entry
  %ptr13 = getelementptr inbounds %struct.matrix, %struct.matrix* %this, i64 0, i32 0
  %ptr18 = getelementptr inbounds %struct.matrix, %struct.matrix* %y, i64 0, i32 0
  %3 = load float*, float** %ptr18, align 8, !tbaa !7
  %4 = load float*, float** %ptr13, align 8, !tbaa !7
  br label %for.body.5.lr.ph.split.us.us

for.cond.cleanup.4.us:                            ; preds = %for.cond.cleanup.8.us.us
  %indvars.iv.next104 = add nuw nsw i64 %indvars.iv103, 1
  %lftr.wideiv106 = trunc i64 %indvars.iv.next104 to i32
  %exitcond107 = icmp eq i32 %lftr.wideiv106, %0
  br i1 %exitcond107, label %nrvo.skipdtor, label %for.body.5.lr.ph.split.us.us

for.body.9.lr.ph.us.us:                           ; preds = %for.body.5.lr.ph.split.us.us, %for.cond.cleanup.8.us.us
  %indvars.iv96 = phi i64 [ 0, %for.body.5.lr.ph.split.us.us ], [ %indvars.iv.next97, %for.cond.cleanup.8.us.us ]
  %5 = add nsw i64 %indvars.iv96, %12
  %arrayidx.us.us = getelementptr inbounds float, float* %2, i64 %5
  store float 0.000000e+00, float* %arrayidx.us.us, align 4, !tbaa !8
  %arrayidx25.us.us = getelementptr inbounds float, float* %2, i64 %5
  br label %for.body.9.us.us

for.cond.cleanup.8.us.us:                         ; preds = %for.body.9.us.us
  %indvars.iv.next97 = add nuw nsw i64 %indvars.iv96, 1
  %lftr.wideiv99 = trunc i64 %indvars.iv.next97 to i32
  %exitcond100 = icmp eq i32 %lftr.wideiv99, %0
  br i1 %exitcond100, label %for.cond.cleanup.4.us, label %for.body.9.lr.ph.us.us

for.body.9.us.us:                                 ; preds = %for.body.9.us.us, %for.body.9.lr.ph.us.us
  %6 = phi float [ %add26.us.us, %for.body.9.us.us ], [ 0.000000e+00, %for.body.9.lr.ph.us.us ]
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body.9.us.us ], [ 0, %for.body.9.lr.ph.us.us ]
  %7 = add nsw i64 %indvars.iv, %12
  %arrayidx14.us.us = getelementptr inbounds float, float* %4, i64 %7
  %8 = load float, float* %arrayidx14.us.us, align 4, !tbaa !8
  %9 = mul nsw i64 %indvars.iv, %conv.i
  %10 = add nsw i64 %9, %indvars.iv96
  %arrayidx19.us.us = getelementptr inbounds float, float* %3, i64 %10
  %11 = load float, float* %arrayidx19.us.us, align 4, !tbaa !8
  %mul20.us.us = fmul float %8, %11
  %add26.us.us = fadd float %mul20.us.us, %6
  store float %add26.us.us, float* %arrayidx25.us.us, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %0
  br i1 %exitcond, label %for.cond.cleanup.8.us.us, label %for.body.9.us.us

for.body.5.lr.ph.split.us.us:                     ; preds = %for.cond.cleanup.4.us, %for.cond.2.preheader.lr.ph.split.us
  %indvars.iv103 = phi i64 [ %indvars.iv.next104, %for.cond.cleanup.4.us ], [ 0, %for.cond.2.preheader.lr.ph.split.us ]
  %12 = mul nsw i64 %indvars.iv103, %conv.i
  br label %for.body.9.lr.ph.us.us

nrvo.skipdtor:                                    ; preds = %for.cond.cleanup.4.us, %entry
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) #1

; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) #2

attributes #0 = { noinline nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1170) (llvm/branches/ltoprof 1248)"}
!1 = !{!2, !6, i64 8}
!2 = !{!"_ZTS6matrix", !3, i64 0, !6, i64 8}
!3 = !{!"any pointer", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!"int", !4, i64 0}
!7 = !{!2, !3, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"float", !4, i64 0}

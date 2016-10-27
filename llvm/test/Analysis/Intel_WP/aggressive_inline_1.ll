; This test expects aggressive inline analysis to be kicked-in
; at LTO link-time. The analysis tracks allocation and
; usage of srcGrid and dstGrid variables to enable aggressive
; inlining. This test checks whether aggressive inlining is
; enabled or not.


; RUN: llvm-as < %s >%t1
; RUN: llvm-lto -exported-symbol=main -inline-agg-trace -o %t2 %t1  2>&1 | FileCheck %s

; CHECK:   AggInl:  Marking callsite for inline
; CHECK:   AggInl:  Marking callsite for inline
; CHECK:   AggInl:  Marking callsite for inline
; CHECK:   AggInl:  Marking callsite for inline
; CHECK:   AggInl:  All CallSites marked for inline after propagation


@srcGrid = internal global double** null, align 8
@dstGrid = internal global double** null, align 8

; Function Attrs: nounwind uwtable
define void @allocateGrid(double** nocapture %ptr) local_unnamed_addr  {
entry:
  %call = tail call noalias i8* @malloc(i64 1610612991) 
  %0 = bitcast double** %ptr to i8**
  store i8* %call, i8** %0, align 8
  %tobool = icmp eq i8* %call, null
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void @exit(i32 1) 
  unreachable

if.end:                                           ; preds = %entry
  %add.ptr = getelementptr inbounds i8, i8* %call, i64 33056
  store i8* %add.ptr, i8** %0, align 8
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) local_unnamed_addr 

; Function Attrs: noreturn nounwind
declare void @exit(i32) local_unnamed_addr 

; Function Attrs: norecurse nounwind uwtable
define void @performS(double* nocapture readonly %srcGrid, double* nocapture %dstGrid) local_unnamed_addr  {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds double, double* %srcGrid, i64 %indvars.iv
  %0 = bitcast double* %arrayidx to i64*
  %1 = load i64, i64* %0, align 8
  %arrayidx2 = getelementptr inbounds double, double* %dstGrid, i64 %indvars.iv
  %2 = bitcast double* %arrayidx2 to i64*
  store i64 %1, i64* %2, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1610612736
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nounwind uwtable
define void @main_init() local_unnamed_addr  {
entry:
  tail call void @allocateGrid(double** bitcast (double*** @srcGrid to double**))
  tail call void @allocateGrid(double** bitcast (double*** @dstGrid to double**))
  %0 = load double**, double*** @srcGrid, align 8
  %1 = load double*, double** %0, align 8
  %2 = load double**, double*** @dstGrid, align 8
  %3 = load double*, double** %2, align 8
  tail call void @performS(double* %1, double* %3)
  ret void
}

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr  {
entry:
  tail call void @main_init()
  ret i32 0
}

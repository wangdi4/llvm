; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; Check fusion with presence of Unknown Memory Access loops

; BEGIN REGION { }
;     + DO i1 = 0, 999, 1   <DO_LOOP>
;     |   %0 = (%p)[i1];
;     |   %1 = (%q)[i1];
;     |   (%q)[i1] = %0 + %1;
;     + END LOOP
;
;     + DO i1 = 0, 999, 1   <DO_LOOP>
;     |   %2 = (%p)[i1];
;     |   %3 = (%q)[i1];
;     |   (%q)[i1] = %2 + %3;
;     |   %putchar182 = @putchar(10);  << unsafe
;     + END LOOP
;
;     + DO i1 = 0, 999, 1   <DO_LOOP>
;     |   %4 = (%p)[i1];
;     |   %5 = (%q)[i1];
;     |   (%q)[i1] = %4 + %5;
;     + END LOOP
;
;     + DO i1 = 0, 999, 1   <DO_LOOP>
;     |   %6 = (%p)[i1];
;     |   %7 = (%q)[i1];
;     |   (%q)[i1] = %6 + %7;
;     + END LOOP
;
;     + DO i1 = 0, 999, 1   <DO_LOOP>
;     |   %8 = (%p)[i1];
;     |   %9 = (%q)[i1];
;     |   (%q)[i1] = %8 + %9;
;     |   %putchar181 = @putchar(10);  << unsafe
;     + END LOOP
;
;     + DO i1 = 0, 999, 1   <DO_LOOP>
;     |   %10 = (%p)[i1];
;     |   %11 = (%q)[i1];
;     |   (%q)[i1] = %10 + %11;
;     + END LOOP
;
;     + DO i1 = 0, 999, 1   <DO_LOOP>
;     |   %12 = (%p)[i1];
;     |   %13 = (%q)[i1];
;     |   (%q)[i1] = %12 + %13;
;     + END LOOP
;
;     + DO i1 = 0, 999, 1   <DO_LOOP>
;     |   %14 = (%p)[i1];
;     |   %15 = (%q)[i1];
;     |   (%q)[i1] = %14 + %15;
;     |   %putchar = @putchar(10);  << unsafe
;     + END LOOP
;
;     + DO i1 = 0, 999, 1   <DO_LOOP>
;     |   %16 = (%p)[i1];
;     |   %17 = (%q)[i1];
;     |   (%q)[i1] = %16 + %17;
;     + END LOOP
;
;     + DO i1 = 0, 999, 1   <DO_LOOP>
;     |   %18 = (%p)[i1];
;     |   %19 = (%q)[i1];
;     |   (%q)[i1] = %18 + %19;
;     + END LOOP
;
;     ret 0;
; END REGION

; CHECK: BEGIN REGION
; CHECK-SAME: modified

; CHECK: DO i1
; CHECK: ;
; CHECK: ;
; CHECK: ;

; CHECK: DO i1
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: putchar

; CHECK: DO i1
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: ;

; CHECK: DO i1
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: putchar

; CHECK: DO i1
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: ;

; CHECK: DO i1
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: putchar

; CHECK: DO i1
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: ;
; CHECK: ;

; CHECK: END REGION

;Module Before HIR; ModuleID = 'uma-chain.c'
source_filename = "uma-chain.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(ptr noalias nocapture readonly %p, ptr nocapture %q, i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv217 = phi i64 [ 0, %entry ], [ %indvars.iv.next218, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv217
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv217
  %1 = load i32, ptr %arrayidx2, align 4
  %add = add nsw i32 %1, %0
  store i32 %add, ptr %arrayidx2, align 4
  %indvars.iv.next218 = add nuw nsw i64 %indvars.iv217, 1
  %exitcond219 = icmp eq i64 %indvars.iv.next218, 1000
  br i1 %exitcond219, label %for.body7.preheader, label %for.body

for.body7.preheader:                              ; preds = %for.body
  br label %for.body7

for.body7:                                        ; preds = %for.body7.preheader, %for.body7
  %indvars.iv214 = phi i64 [ %indvars.iv.next215, %for.body7 ], [ 0, %for.body7.preheader ]
  %arrayidx9 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv214
  %2 = load i32, ptr %arrayidx9, align 4
  %arrayidx11 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv214
  %3 = load i32, ptr %arrayidx11, align 4
  %add12 = add nsw i32 %3, %2
  store i32 %add12, ptr %arrayidx11, align 4
  %putchar182 = tail call i32 @putchar(i32 10)
  %indvars.iv.next215 = add nuw nsw i64 %indvars.iv214, 1
  %exitcond216 = icmp eq i64 %indvars.iv.next215, 1000
  br i1 %exitcond216, label %for.body20.preheader, label %for.body7

for.body20.preheader:                             ; preds = %for.body7
  br label %for.body20

for.body20:                                       ; preds = %for.body20.preheader, %for.body20
  %indvars.iv211 = phi i64 [ %indvars.iv.next212, %for.body20 ], [ 0, %for.body20.preheader ]
  %arrayidx22 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv211
  %4 = load i32, ptr %arrayidx22, align 4
  %arrayidx24 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv211
  %5 = load i32, ptr %arrayidx24, align 4
  %add25 = add nsw i32 %5, %4
  store i32 %add25, ptr %arrayidx24, align 4
  %indvars.iv.next212 = add nuw nsw i64 %indvars.iv211, 1
  %exitcond213 = icmp eq i64 %indvars.iv.next212, 1000
  br i1 %exitcond213, label %for.body33.preheader, label %for.body20

for.body33.preheader:                             ; preds = %for.body20
  br label %for.body33

for.body33:                                       ; preds = %for.body33.preheader, %for.body33
  %indvars.iv208 = phi i64 [ %indvars.iv.next209, %for.body33 ], [ 0, %for.body33.preheader ]
  %arrayidx35 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv208
  %6 = load i32, ptr %arrayidx35, align 4
  %arrayidx37 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv208
  %7 = load i32, ptr %arrayidx37, align 4
  %add38 = add nsw i32 %7, %6
  store i32 %add38, ptr %arrayidx37, align 4
  %indvars.iv.next209 = add nuw nsw i64 %indvars.iv208, 1
  %exitcond210 = icmp eq i64 %indvars.iv.next209, 1000
  br i1 %exitcond210, label %for.body46.preheader, label %for.body33

for.body46.preheader:                             ; preds = %for.body33
  br label %for.body46

for.body46:                                       ; preds = %for.body46.preheader, %for.body46
  %indvars.iv205 = phi i64 [ %indvars.iv.next206, %for.body46 ], [ 0, %for.body46.preheader ]
  %arrayidx48 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv205
  %8 = load i32, ptr %arrayidx48, align 4
  %arrayidx50 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv205
  %9 = load i32, ptr %arrayidx50, align 4
  %add51 = add nsw i32 %9, %8
  store i32 %add51, ptr %arrayidx50, align 4
  %putchar181 = tail call i32 @putchar(i32 10)
  %indvars.iv.next206 = add nuw nsw i64 %indvars.iv205, 1
  %exitcond207 = icmp eq i64 %indvars.iv.next206, 1000
  br i1 %exitcond207, label %for.body60.preheader, label %for.body46

for.body60.preheader:                             ; preds = %for.body46
  br label %for.body60

for.body60:                                       ; preds = %for.body60.preheader, %for.body60
  %indvars.iv202 = phi i64 [ %indvars.iv.next203, %for.body60 ], [ 0, %for.body60.preheader ]
  %arrayidx62 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv202
  %10 = load i32, ptr %arrayidx62, align 4
  %arrayidx64 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv202
  %11 = load i32, ptr %arrayidx64, align 4
  %add65 = add nsw i32 %11, %10
  store i32 %add65, ptr %arrayidx64, align 4
  %indvars.iv.next203 = add nuw nsw i64 %indvars.iv202, 1
  %exitcond204 = icmp eq i64 %indvars.iv.next203, 1000
  br i1 %exitcond204, label %for.body73.preheader, label %for.body60

for.body73.preheader:                             ; preds = %for.body60
  br label %for.body73

for.body73:                                       ; preds = %for.body73.preheader, %for.body73
  %indvars.iv199 = phi i64 [ %indvars.iv.next200, %for.body73 ], [ 0, %for.body73.preheader ]
  %arrayidx75 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv199
  %12 = load i32, ptr %arrayidx75, align 4
  %arrayidx77 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv199
  %13 = load i32, ptr %arrayidx77, align 4
  %add78 = add nsw i32 %13, %12
  store i32 %add78, ptr %arrayidx77, align 4
  %indvars.iv.next200 = add nuw nsw i64 %indvars.iv199, 1
  %exitcond201 = icmp eq i64 %indvars.iv.next200, 1000
  br i1 %exitcond201, label %for.body86.preheader, label %for.body73

for.body86.preheader:                             ; preds = %for.body73
  br label %for.body86

for.body86:                                       ; preds = %for.body86.preheader, %for.body86
  %indvars.iv196 = phi i64 [ %indvars.iv.next197, %for.body86 ], [ 0, %for.body86.preheader ]
  %arrayidx88 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv196
  %14 = load i32, ptr %arrayidx88, align 4
  %arrayidx90 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv196
  %15 = load i32, ptr %arrayidx90, align 4
  %add91 = add nsw i32 %15, %14
  store i32 %add91, ptr %arrayidx90, align 4
  %putchar = tail call i32 @putchar(i32 10)
  %indvars.iv.next197 = add nuw nsw i64 %indvars.iv196, 1
  %exitcond198 = icmp eq i64 %indvars.iv.next197, 1000
  br i1 %exitcond198, label %for.body100.preheader, label %for.body86

for.body100.preheader:                            ; preds = %for.body86
  br label %for.body100

for.body100:                                      ; preds = %for.body100.preheader, %for.body100
  %indvars.iv193 = phi i64 [ %indvars.iv.next194, %for.body100 ], [ 0, %for.body100.preheader ]
  %arrayidx102 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv193
  %16 = load i32, ptr %arrayidx102, align 4
  %arrayidx104 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv193
  %17 = load i32, ptr %arrayidx104, align 4
  %add105 = add nsw i32 %17, %16
  store i32 %add105, ptr %arrayidx104, align 4
  %indvars.iv.next194 = add nuw nsw i64 %indvars.iv193, 1
  %exitcond195 = icmp eq i64 %indvars.iv.next194, 1000
  br i1 %exitcond195, label %for.body113.preheader, label %for.body100

for.body113.preheader:                            ; preds = %for.body100
  br label %for.body113

for.cond.cleanup112:                              ; preds = %for.body113
  ret i32 0

for.body113:                                      ; preds = %for.body113.preheader, %for.body113
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body113 ], [ 0, %for.body113.preheader ]
  %arrayidx115 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  %18 = load i32, ptr %arrayidx115, align 4
  %arrayidx117 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv
  %19 = load i32, ptr %arrayidx117, align 4
  %add118 = add nsw i32 %19, %18
  store i32 %add118, ptr %arrayidx117, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup112, label %for.body113
}

; Function Attrs: nounwind
declare i32 @putchar(i32) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }



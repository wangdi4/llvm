;  Just checking for a clean compile - no assertions
;
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" 2>&1 | FileCheck %s

; CHECK:   <Safe Reduction>
; ModuleID = 'sum22.c'
source_filename = "sum22.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type { ptr, ptr, i32 }

@.str = private unnamed_addr constant [10 x i8] c"input.txt\00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.2 = private unnamed_addr constant [21 x i8] c"%u %u %u %u %u %u %u\00", align 1
@.str.3 = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

; Function Attrs: norecurse nounwind uwtable
define void @init(ptr nocapture %a, i32 %n, i32 %seed) local_unnamed_addr #0 {
entry:
  %cmp8 = icmp eq i32 %n, 0
  br i1 %cmp8, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %rem = and i64 %indvars.iv, 1
  %cmp1 = icmp eq i64 %rem, 0
  %0 = trunc i64 %indvars.iv to i32
  %1 = sub i32 0, %0
  %2 = trunc i64 %indvars.iv to i32
  %.p = select i1 %cmp1, i32 %2, i32 %1
  %3 = add i32 %.p, %seed
  %rem2 = urem i32 %3, 101
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 %rem2, ptr %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture) #1

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @checkSum(ptr nocapture readonly %a, i32 %n) local_unnamed_addr #2 {
entry:
  %cmp10 = icmp eq i32 %n, 0
  br i1 %cmp10, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %sum.012 = phi i32 [ %add, %for.body ], [ 0, %for.body.preheader ]
  %rem = and i64 %indvars.iv, 1
  %cmp1 = icmp eq i64 %rem, 0
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !1
  %sub = sub i32 0, %0
  %1 = select i1 %cmp1, i32 %0, i32 %sub
  %add = add i32 %1, %sum.012
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.body ]
  ret i32 %sum.0.lcssa
}

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr #3 {
entry:
  %k7 = alloca i32, align 4
  %ct5 = alloca i32, align 4
  %k9 = alloca i32, align 4
  %k = alloca i32, align 4
  %k3 = alloca i32, align 4
  %kq = alloca i32, align 4
  %ir = alloca i32, align 4
  %nt = alloca [100 x i32], align 16
  %tk = alloca [100 x i32], align 16
  %x = alloca [100 x i32], align 16
  %lz = alloca [100 x i32], align 16
  %j7 = alloca [100 x i32], align 16
  %hs = alloca [100 x [100 x [100 x i32]]], align 16
  %c = alloca [100 x [100 x i32]], align 16
  %za = alloca [100 x i32], align 16
  %y = alloca [100 x i32], align 16
  %call = tail call ptr @fopen(ptr @.str, ptr @.str.1)
  call void @llvm.lifetime.start(i64 4, ptr %k7) #6
  store i32 49, ptr %k7, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %ct5) #6
  store i32 42, ptr %ct5, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %k9) #6
  store i32 31, ptr %k9, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %k) #6
  store i32 10, ptr %k, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %k3) #6
  store i32 98, ptr %k3, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %kq) #6
  store i32 82, ptr %kq, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %ir) #6
  store i32 52, ptr %ir, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 400, ptr %nt) #6
  call void @llvm.lifetime.start(i64 400, ptr %tk) #6
  call void @llvm.lifetime.start(i64 400, ptr %x) #6
  call void @llvm.lifetime.start(i64 400, ptr %lz) #6
  call void @llvm.lifetime.start(i64 400, ptr %j7) #6
  call void @llvm.lifetime.start(i64 4000000, ptr %hs) #6
  call void @llvm.lifetime.start(i64 40000, ptr %c) #6
  call void @llvm.lifetime.start(i64 400, ptr %za) #6
  call void @llvm.lifetime.start(i64 400, ptr %y) #6
  %arrayidx = getelementptr inbounds [100 x i32], ptr %j7, i64 0, i64 29
  call void @init(ptr %nt, i32 100, i32 6)
  call void @init(ptr %tk, i32 100, i32 51)
  call void @init(ptr %x, i32 100, i32 92)
  call void @init(ptr %lz, i32 100, i32 97)
  call void @init(ptr %j7, i32 100, i32 49)
  call void @init(ptr %hs, i32 1000000, i32 10)
  call void @init(ptr %c, i32 10000, i32 53)
  call void @init(ptr %za, i32 100, i32 77)
  call void @init(ptr %y, i32 100, i32 78)
  %call11 = call i32 (ptr, ptr, ...) @__isoc99_fscanf(ptr %call, ptr @.str.2, ptr nonnull %k7, ptr nonnull %ct5, ptr nonnull %k9, ptr nonnull %k, ptr nonnull %k3, ptr nonnull %kq, ptr nonnull %ir) #6
  %arrayidx12 = getelementptr inbounds [100 x i32], ptr %nt, i64 0, i64 17
  %0 = load i32, ptr %arrayidx12, align 4, !tbaa !1
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !1
  %add = add i32 %1, %0
  store i32 %add, ptr %arrayidx, align 4, !tbaa !1
  store i32 89, ptr %k7, align 4, !tbaa !1
  %ct5.promoted = load i32, ptr %ct5, align 4, !tbaa !1
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 89, %entry ], [ %indvars.iv.next, %for.body ]
  %sub157 = phi i32 [ %ct5.promoted, %entry ], [ %sub, %for.body ]
  %arrayidx13 = getelementptr inbounds [100 x i32], ptr %nt, i64 0, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx13, align 4, !tbaa !1
  %sub = sub i32 %sub157, %2
  %3 = mul nuw nsw i64 %indvars.iv, 113
  %4 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx16 = getelementptr inbounds [100 x i32], ptr %tk, i64 0, i64 %4
  %5 = trunc i64 %3 to i32
  store i32 %5, ptr %arrayidx16, align 4, !tbaa !1
  %arrayidx18 = getelementptr inbounds [100 x i32], ptr %x, i64 0, i64 %indvars.iv
  %6 = trunc i64 %indvars.iv to i32
  store i32 %6, ptr %arrayidx18, align 4, !tbaa !1
  %arrayidx25 = getelementptr inbounds [100 x i32], ptr %lz, i64 0, i64 %4
  %7 = load i32, ptr %arrayidx25, align 4, !tbaa !1
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %arrayidx28 = getelementptr inbounds [100 x i32], ptr %x, i64 0, i64 %indvars.iv.next
  %8 = load i32, ptr %arrayidx28, align 4, !tbaa !1
  %mul29 = mul i32 %8, %7
  store i32 %mul29, ptr %arrayidx28, align 4, !tbaa !1
  %arrayidx35 = getelementptr inbounds [100 x i32], ptr %lz, i64 0, i64 %indvars.iv.next
  store i32 %mul29, ptr %arrayidx35, align 4, !tbaa !1
  %cmp = icmp ugt i64 %indvars.iv.next, 4
  br i1 %cmp, label %for.body, label %for.end116

for.end116:                                       ; preds = %for.body
  store i32 %sub, ptr %ct5, align 4, !tbaa !1
  store i32 4, ptr %k7, align 4, !tbaa !1
  %9 = load i32, ptr %k9, align 4, !tbaa !1
  %10 = load i32, ptr %kq, align 4, !tbaa !1
  %11 = load i32, ptr %ir, align 4, !tbaa !1
  %call122 = call i32 @checkSum(ptr nonnull %nt, i32 100)
  %call124 = call i32 @checkSum(ptr nonnull %tk, i32 100)
  %call127 = call i32 @checkSum(ptr nonnull %x, i32 100)
  %call130 = call i32 @checkSum(ptr nonnull %lz, i32 100)
  %call133 = call i32 @checkSum(ptr %j7, i32 100)
  %call136 = call i32 @checkSum(ptr %hs, i32 1000000)
  %call139 = call i32 @checkSum(ptr %c, i32 10000)
  %call142 = call i32 @checkSum(ptr %za, i32 100)
  %call145 = call i32 @checkSum(ptr %y, i32 100)
  %add125 = add i32 %sub, 4
  %sub128 = sub i32 %add125, %9
  %add131 = add i32 %sub128, %10
  %sub134 = sub i32 %add131, %11
  %add137 = add i32 %sub134, %call122
  %sub140 = add i32 %add137, %call124
  %add143 = sub i32 %sub140, %call127
  %sub146 = add i32 %add143, %call130
  %add117 = sub i32 %sub146, %call133
  %sub118 = add i32 %add117, %call136
  %add119 = sub i32 %sub118, %call139
  %sub120 = add i32 %add119, %call142
  %add147 = sub i32 %sub120, %call145
  %call148 = call i32 (ptr, ...) @printf(ptr @.str.3, i32 %add147)
  call void @llvm.lifetime.end(i64 400, ptr %y) #6
  call void @llvm.lifetime.end(i64 400, ptr %za) #6
  call void @llvm.lifetime.end(i64 40000, ptr %c) #6
  call void @llvm.lifetime.end(i64 4000000, ptr %hs) #6
  call void @llvm.lifetime.end(i64 400, ptr %j7) #6
  call void @llvm.lifetime.end(i64 400, ptr nonnull %lz) #6
  call void @llvm.lifetime.end(i64 400, ptr nonnull %x) #6
  call void @llvm.lifetime.end(i64 400, ptr nonnull %tk) #6
  call void @llvm.lifetime.end(i64 400, ptr nonnull %nt) #6
  call void @llvm.lifetime.end(i64 4, ptr %ir) #6
  call void @llvm.lifetime.end(i64 4, ptr %kq) #6
  call void @llvm.lifetime.end(i64 4, ptr %k3) #6
  call void @llvm.lifetime.end(i64 4, ptr %k) #6
  call void @llvm.lifetime.end(i64 4, ptr %k9) #6
  call void @llvm.lifetime.end(i64 4, ptr %ct5) #6
  call void @llvm.lifetime.end(i64 4, ptr nonnull %k7) #6
  ret i32 0
}

; Function Attrs: nounwind
declare noalias ptr @fopen(ptr nocapture readonly, ptr nocapture readonly) local_unnamed_addr #4

declare i32 @__isoc99_fscanf(ptr, ptr, ...) local_unnamed_addr #5

; Function Attrs: nounwind
declare i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #4

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20662) (llvm/branches/loopopt 20668)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

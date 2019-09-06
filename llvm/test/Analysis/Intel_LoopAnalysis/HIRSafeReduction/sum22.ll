;  Just checking for a clean compile - no assertions
;
; RUN: opt < %s  -hir-ssa-deconstruction   -analyze  -hir-temp-cleanup   -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" 2>&1 | FileCheck %s

; CHECK:   <Safe Reduction>
; ModuleID = 'sum22.c'
source_filename = "sum22.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@.str = private unnamed_addr constant [10 x i8] c"input.txt\00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.2 = private unnamed_addr constant [21 x i8] c"%u %u %u %u %u %u %u\00", align 1
@.str.3 = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

; Function Attrs: norecurse nounwind uwtable
define void @init(i32* nocapture %a, i32 %n, i32 %seed) local_unnamed_addr #0 {
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
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  store i32 %rem2, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @checkSum(i32* nocapture readonly %a, i32 %n) local_unnamed_addr #2 {
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
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
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
  %call = tail call %struct._IO_FILE* @fopen(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.1, i64 0, i64 0))
  %0 = bitcast i32* %k7 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #6
  store i32 49, i32* %k7, align 4, !tbaa !1
  %1 = bitcast i32* %ct5 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) #6
  store i32 42, i32* %ct5, align 4, !tbaa !1
  %2 = bitcast i32* %k9 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %2) #6
  store i32 31, i32* %k9, align 4, !tbaa !1
  %3 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start(i64 4, i8* %3) #6
  store i32 10, i32* %k, align 4, !tbaa !1
  %4 = bitcast i32* %k3 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #6
  store i32 98, i32* %k3, align 4, !tbaa !1
  %5 = bitcast i32* %kq to i8*
  call void @llvm.lifetime.start(i64 4, i8* %5) #6
  store i32 82, i32* %kq, align 4, !tbaa !1
  %6 = bitcast i32* %ir to i8*
  call void @llvm.lifetime.start(i64 4, i8* %6) #6
  store i32 52, i32* %ir, align 4, !tbaa !1
  %7 = bitcast [100 x i32]* %nt to i8*
  call void @llvm.lifetime.start(i64 400, i8* %7) #6
  %8 = bitcast [100 x i32]* %tk to i8*
  call void @llvm.lifetime.start(i64 400, i8* %8) #6
  %9 = bitcast [100 x i32]* %x to i8*
  call void @llvm.lifetime.start(i64 400, i8* %9) #6
  %10 = bitcast [100 x i32]* %lz to i8*
  call void @llvm.lifetime.start(i64 400, i8* %10) #6
  %11 = bitcast [100 x i32]* %j7 to i8*
  call void @llvm.lifetime.start(i64 400, i8* %11) #6
  %12 = bitcast [100 x [100 x [100 x i32]]]* %hs to i8*
  call void @llvm.lifetime.start(i64 4000000, i8* %12) #6
  %13 = bitcast [100 x [100 x i32]]* %c to i8*
  call void @llvm.lifetime.start(i64 40000, i8* %13) #6
  %14 = bitcast [100 x i32]* %za to i8*
  call void @llvm.lifetime.start(i64 400, i8* %14) #6
  %15 = bitcast [100 x i32]* %y to i8*
  call void @llvm.lifetime.start(i64 400, i8* %15) #6
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %j7, i64 0, i64 29
  %arraydecay = getelementptr inbounds [100 x i32], [100 x i32]* %nt, i64 0, i64 0
  call void @init(i32* %arraydecay, i32 100, i32 6)
  %arraydecay3 = getelementptr inbounds [100 x i32], [100 x i32]* %tk, i64 0, i64 0
  call void @init(i32* %arraydecay3, i32 100, i32 51)
  %arraydecay4 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 0
  call void @init(i32* %arraydecay4, i32 100, i32 92)
  %arraydecay5 = getelementptr inbounds [100 x i32], [100 x i32]* %lz, i64 0, i64 0
  call void @init(i32* %arraydecay5, i32 100, i32 97)
  %arraydecay6 = getelementptr inbounds [100 x i32], [100 x i32]* %j7, i64 0, i64 0
  call void @init(i32* %arraydecay6, i32 100, i32 49)
  %16 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %hs, i64 0, i64 0, i64 0, i64 0
  call void @init(i32* %16, i32 1000000, i32 10)
  %17 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %c, i64 0, i64 0, i64 0
  call void @init(i32* %17, i32 10000, i32 53)
  %arraydecay9 = getelementptr inbounds [100 x i32], [100 x i32]* %za, i64 0, i64 0
  call void @init(i32* %arraydecay9, i32 100, i32 77)
  %arraydecay10 = getelementptr inbounds [100 x i32], [100 x i32]* %y, i64 0, i64 0
  call void @init(i32* %arraydecay10, i32 100, i32 78)
  %call11 = call i32 (%struct._IO_FILE*, i8*, ...) @__isoc99_fscanf(%struct._IO_FILE* %call, i8* getelementptr inbounds ([21 x i8], [21 x i8]* @.str.2, i64 0, i64 0), i32* nonnull %k7, i32* nonnull %ct5, i32* nonnull %k9, i32* nonnull %k, i32* nonnull %k3, i32* nonnull %kq, i32* nonnull %ir) #6
  %arrayidx12 = getelementptr inbounds [100 x i32], [100 x i32]* %nt, i64 0, i64 17
  %18 = load i32, i32* %arrayidx12, align 4, !tbaa !1
  %19 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add i32 %19, %18
  store i32 %add, i32* %arrayidx, align 4, !tbaa !1
  store i32 89, i32* %k7, align 4, !tbaa !1
  %ct5.promoted = load i32, i32* %ct5, align 4, !tbaa !1
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 89, %entry ], [ %indvars.iv.next, %for.body ]
  %sub157 = phi i32 [ %ct5.promoted, %entry ], [ %sub, %for.body ]
  %arrayidx13 = getelementptr inbounds [100 x i32], [100 x i32]* %nt, i64 0, i64 %indvars.iv
  %20 = load i32, i32* %arrayidx13, align 4, !tbaa !1
  %sub = sub i32 %sub157, %20
  %21 = mul nuw nsw i64 %indvars.iv, 113
  %22 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx16 = getelementptr inbounds [100 x i32], [100 x i32]* %tk, i64 0, i64 %22
  %23 = trunc i64 %21 to i32
  store i32 %23, i32* %arrayidx16, align 4, !tbaa !1
  %arrayidx18 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 %indvars.iv
  %24 = trunc i64 %indvars.iv to i32
  store i32 %24, i32* %arrayidx18, align 4, !tbaa !1
  %arrayidx25 = getelementptr inbounds [100 x i32], [100 x i32]* %lz, i64 0, i64 %22
  %25 = load i32, i32* %arrayidx25, align 4, !tbaa !1
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %arrayidx28 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 %indvars.iv.next
  %26 = load i32, i32* %arrayidx28, align 4, !tbaa !1
  %mul29 = mul i32 %26, %25
  store i32 %mul29, i32* %arrayidx28, align 4, !tbaa !1
  %arrayidx35 = getelementptr inbounds [100 x i32], [100 x i32]* %lz, i64 0, i64 %indvars.iv.next
  store i32 %mul29, i32* %arrayidx35, align 4, !tbaa !1
  %cmp = icmp ugt i64 %indvars.iv.next, 4
  br i1 %cmp, label %for.body, label %for.end116

for.end116:                                       ; preds = %for.body
  store i32 %sub, i32* %ct5, align 4, !tbaa !1
  store i32 4, i32* %k7, align 4, !tbaa !1
  %27 = load i32, i32* %k9, align 4, !tbaa !1
  %28 = load i32, i32* %kq, align 4, !tbaa !1
  %29 = load i32, i32* %ir, align 4, !tbaa !1
  %call122 = call i32 @checkSum(i32* nonnull %arraydecay, i32 100)
  %call124 = call i32 @checkSum(i32* nonnull %arraydecay3, i32 100)
  %call127 = call i32 @checkSum(i32* nonnull %arraydecay4, i32 100)
  %call130 = call i32 @checkSum(i32* nonnull %arraydecay5, i32 100)
  %call133 = call i32 @checkSum(i32* %arraydecay6, i32 100)
  %call136 = call i32 @checkSum(i32* %16, i32 1000000)
  %call139 = call i32 @checkSum(i32* %17, i32 10000)
  %call142 = call i32 @checkSum(i32* %arraydecay9, i32 100)
  %call145 = call i32 @checkSum(i32* %arraydecay10, i32 100)
  %add125 = add i32 %sub, 4
  %sub128 = sub i32 %add125, %27
  %add131 = add i32 %sub128, %28
  %sub134 = sub i32 %add131, %29
  %add137 = add i32 %sub134, %call122
  %sub140 = add i32 %add137, %call124
  %add143 = sub i32 %sub140, %call127
  %sub146 = add i32 %add143, %call130
  %add117 = sub i32 %sub146, %call133
  %sub118 = add i32 %add117, %call136
  %add119 = sub i32 %sub118, %call139
  %sub120 = add i32 %add119, %call142
  %add147 = sub i32 %sub120, %call145
  %call148 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.3, i64 0, i64 0), i32 %add147)
  call void @llvm.lifetime.end(i64 400, i8* %15) #6
  call void @llvm.lifetime.end(i64 400, i8* %14) #6
  call void @llvm.lifetime.end(i64 40000, i8* %13) #6
  call void @llvm.lifetime.end(i64 4000000, i8* %12) #6
  call void @llvm.lifetime.end(i64 400, i8* %11) #6
  call void @llvm.lifetime.end(i64 400, i8* nonnull %10) #6
  call void @llvm.lifetime.end(i64 400, i8* nonnull %9) #6
  call void @llvm.lifetime.end(i64 400, i8* nonnull %8) #6
  call void @llvm.lifetime.end(i64 400, i8* nonnull %7) #6
  call void @llvm.lifetime.end(i64 4, i8* %6) #6
  call void @llvm.lifetime.end(i64 4, i8* %5) #6
  call void @llvm.lifetime.end(i64 4, i8* %4) #6
  call void @llvm.lifetime.end(i64 4, i8* %3) #6
  call void @llvm.lifetime.end(i64 4, i8* %2) #6
  call void @llvm.lifetime.end(i64 4, i8* %1) #6
  call void @llvm.lifetime.end(i64 4, i8* nonnull %0) #6
  ret i32 0
}

; Function Attrs: nounwind
declare noalias %struct._IO_FILE* @fopen(i8* nocapture readonly, i8* nocapture readonly) local_unnamed_addr #4

declare i32 @__isoc99_fscanf(%struct._IO_FILE*, i8*, ...) local_unnamed_addr #5

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #4

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

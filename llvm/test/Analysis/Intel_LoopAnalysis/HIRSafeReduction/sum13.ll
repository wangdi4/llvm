; RUN: opt < %s  -hir-ssa-deconstruction | opt -analyze -force-hir-safe-reduction-analysis  -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -force-hir-safe-reduction-analysis -disable-output 2>&1 | FileCheck %s

; CHECK:  %hir.de.ssa.copy0.out = %15; <Safe Reduction>
;
; ModuleID = 't2278.c'
source_filename = "t2278.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@.str = private unnamed_addr constant [10 x i8] c"input.txt\00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.2 = private unnamed_addr constant [36 x i8] c"%u %u %u %u %u %u %u %u %u %u %u %u\00", align 1
@.str.3 = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

; Function Attrs: norecurse nounwind uwtable
define void @init(i32* nocapture %a, i32 %n, i32 %seed) #0 {
entry:
  %cmp8 = icmp eq i32 %n, 0
  br i1 %cmp8, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
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
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @checkSum(i32* nocapture readonly %a, i32 %n) #2 {
entry:
  %cmp10 = icmp eq i32 %n, 0
  br i1 %cmp10, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %sum.012 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %rem = and i64 %indvars.iv, 1
  %cmp1 = icmp eq i64 %rem, 0
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %sub = sub i32 0, %0
  %1 = select i1 %cmp1, i32 %0, i32 %sub
  %add = add i32 %1, %sum.012
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.body ]
  ret i32 %sum.0.lcssa
}

; Function Attrs: nounwind uwtable
define i32 @main() #3 {
entry:
  %jk = alloca i32, align 4
  %j = alloca i32, align 4
  %of6 = alloca i32, align 4
  %nb3 = alloca i32, align 4
  %kx9 = alloca i32, align 4
  %jn = alloca i32, align 4
  %zm = alloca i32, align 4
  %n7 = alloca i32, align 4
  %vw1 = alloca i32, align 4
  %kq = alloca i32, align 4
  %k = alloca i32, align 4
  %xc = alloca i32, align 4
  %c = alloca [100 x i32], align 16
  %call = tail call %struct._IO_FILE* @fopen(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.1, i64 0, i64 0))
  %0 = bitcast i32* %jk to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #6
  store i32 80, i32* %jk, align 4, !tbaa !1
  %1 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) #6
  store i32 84, i32* %j, align 4, !tbaa !1
  %2 = bitcast i32* %of6 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %2) #6
  store i32 53, i32* %of6, align 4, !tbaa !1
  %3 = bitcast i32* %nb3 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %3) #6
  store i32 98, i32* %nb3, align 4, !tbaa !1
  %4 = bitcast i32* %kx9 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #6
  store i32 29, i32* %kx9, align 4, !tbaa !1
  %5 = bitcast i32* %jn to i8*
  call void @llvm.lifetime.start(i64 4, i8* %5) #6
  store i32 25, i32* %jn, align 4, !tbaa !1
  %6 = bitcast i32* %zm to i8*
  call void @llvm.lifetime.start(i64 4, i8* %6) #6
  store i32 95, i32* %zm, align 4, !tbaa !1
  %7 = bitcast i32* %n7 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %7) #6
  store i32 16, i32* %n7, align 4, !tbaa !1
  %8 = bitcast i32* %vw1 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %8) #6
  store i32 20, i32* %vw1, align 4, !tbaa !1
  %9 = bitcast i32* %kq to i8*
  call void @llvm.lifetime.start(i64 4, i8* %9) #6
  store i32 8, i32* %kq, align 4, !tbaa !1
  %10 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start(i64 4, i8* %10) #6
  store i32 96, i32* %k, align 4, !tbaa !1
  %11 = bitcast i32* %xc to i8*
  call void @llvm.lifetime.start(i64 4, i8* %11) #6
  store i32 15, i32* %xc, align 4, !tbaa !1
  %12 = bitcast [100 x i32]* %c to i8*
  call void @llvm.lifetime.start(i64 400, i8* %12) #6
  %arraydecay = getelementptr inbounds [100 x i32], [100 x i32]* %c, i64 0, i64 0
  call void @init(i32* %arraydecay, i32 100, i32 31)
  %call1 = call i32 (%struct._IO_FILE*, i8*, ...) @__isoc99_fscanf(%struct._IO_FILE* %call, i8* getelementptr inbounds ([36 x i8], [36 x i8]* @.str.2, i64 0, i64 0), i32* nonnull %jk, i32* nonnull %j, i32* nonnull %of6, i32* nonnull %nb3, i32* nonnull %kx9, i32* nonnull %jn, i32* nonnull %zm, i32* nonnull %n7, i32* nonnull %vw1, i32* nonnull %kq, i32* nonnull %k, i32* nonnull %xc) #6
  store i32 28, i32* %jk, align 4, !tbaa !1
  %nb3.promoted = load i32, i32* %nb3, align 4, !tbaa !1
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %entry, %for.end
  %dec1380 = phi i32 [ 28, %entry ], [ %dec13, %for.end ]
  br label %for.body4

for.cond15.preheader:                             ; preds = %for.end
  %13 = add i32 %nb3.promoted, 27
  store i32 1, i32* %j, align 4, !tbaa !1
  store i32 %13, i32* %nb3, align 4, !tbaa !1
  store i32 %13, i32* %of6, align 4, !tbaa !1
  store i32 1, i32* %jk, align 4, !tbaa !1
  store i32 28, i32* %kx9, align 4, !tbaa !1
  %n7.promoted94 = load i32, i32* %n7, align 4, !tbaa !1
  %zm.promoted95 = load i32, i32* %zm, align 4, !tbaa !1
  br label %for.end24

for.body4:                                        ; preds = %for.cond2.preheader, %for.body4
  %indvars.iv92 = phi i64 [ 28, %for.cond2.preheader ], [ %indvars.iv.next93, %for.body4 ]
  %indvars.iv.next93 = add nsw i64 %indvars.iv92, -1
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %c, i64 0, i64 %indvars.iv.next93
  %14 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %sub5 = sub i32 0, %14
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* %c, i64 0, i64 %indvars.iv92
  store i32 %sub5, i32* %arrayidx7, align 4, !tbaa !1
  %cmp3 = icmp ugt i64 %indvars.iv.next93, 1
  br i1 %cmp3, label %for.body4, label %for.end

for.end:                                          ; preds = %for.body4
  %dec13 = add nsw i32 %dec1380, -1
  %cmp = icmp ugt i32 %dec13, 1
  br i1 %cmp, label %for.cond2.preheader, label %for.cond15.preheader

for.end24:                                        ; preds = %for.end24, %for.cond15.preheader
  %15 = phi i32 [ %zm.promoted95, %for.cond15.preheader ], [ %25, %for.end24 ]
  %16 = phi i32 [ %n7.promoted94, %for.cond15.preheader ], [ %26, %for.end24 ]
  %indvars.iv89 = phi i32 [ 28, %for.cond15.preheader ], [ %indvars.iv.next90, %for.end24 ]
  %indvars.iv87 = phi i32 [ 27, %for.cond15.preheader ], [ %indvars.iv.next88, %for.end24 ]
  %indvars.iv85 = phi i32 [ 25, %for.cond15.preheader ], [ %indvars.iv.next86, %for.end24 ]
  %indvars.iv83 = phi i33 [ 26, %for.cond15.preheader ], [ %indvars.iv.next84, %for.end24 ]
  %indvars.iv81 = phi i32 [ 26, %for.cond15.preheader ], [ %indvars.iv.next82, %for.end24 ]
  %17 = add i32 %15, %16
  %18 = add i32 %16, 1
  %19 = mul i32 %indvars.iv81, %18
  %20 = add i32 %17, %19
  %21 = zext i32 %indvars.iv85 to i33
  %22 = mul i33 %indvars.iv83, %21
  %23 = lshr i33 %22, 1
  %24 = trunc i33 %23 to i32
  %25 = add i32 %20, %24
  %26 = add i32 %16, %indvars.iv87
  %indvars.iv.next90 = add nsw i32 %indvars.iv89, -1
  %cmp16 = icmp ugt i32 %indvars.iv.next90, 1
  %indvars.iv.next82 = add nsw i32 %indvars.iv81, -1
  %indvars.iv.next84 = add nsw i33 %indvars.iv83, -1
  %indvars.iv.next86 = add nsw i32 %indvars.iv85, -1
  %indvars.iv.next88 = add nsw i32 %indvars.iv87, -1
  br i1 %cmp16, label %for.end24, label %for.end35

for.end35:                                        ; preds = %for.end24
  store i32 %26, i32* %n7, align 4, !tbaa !1
  store i32 %25, i32* %zm, align 4, !tbaa !1
  store i32 2, i32* %jn, align 4, !tbaa !1
  store i32 1, i32* %kx9, align 4, !tbaa !1
  %arrayidx36 = getelementptr inbounds [100 x i32], [100 x i32]* %c, i64 0, i64 34
  %27 = load i32, i32* %arrayidx36, align 8, !tbaa !1
  %28 = load i32, i32* %kq, align 4, !tbaa !1
  %add37 = add i32 %28, %27
  store i32 %add37, i32* %kq, align 4, !tbaa !1
  store i32 2, i32* %k, align 4, !tbaa !1
  %29 = load i32, i32* %zm, align 4, !tbaa !1
  %xc.promoted = load i32, i32* %xc, align 4, !tbaa !1
  br label %for.body40

for.body40:                                       ; preds = %for.body40, %for.end35
  %indvars.iv = phi i64 [ 2, %for.end35 ], [ %indvars.iv.next, %for.body40 ]
  %mul67 = phi i32 [ %xc.promoted, %for.end35 ], [ %mul, %for.body40 ]
  %arrayidx42 = getelementptr inbounds [100 x i32], [100 x i32]* %c, i64 0, i64 %indvars.iv
  %30 = load i32, i32* %arrayidx42, align 4, !tbaa !1
  %add43 = add i32 %29, %30
  %mul = mul i32 %mul67, %add43
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 68
  br i1 %exitcond, label %for.end46, label %for.body40

for.end46:                                        ; preds = %for.body40
  store i32 %mul, i32* %xc, align 4, !tbaa !1
  store i32 68, i32* %k, align 4, !tbaa !1
  %31 = load i32, i32* %of6, align 4, !tbaa !1
  %32 = load i32, i32* %nb3, align 4, !tbaa !1
  %add47 = add i32 %32, %31
  %33 = load i32, i32* %zm, align 4, !tbaa !1
  %sub48 = sub i32 %add47, %33
  %34 = load i32, i32* %n7, align 4, !tbaa !1
  %add49 = add i32 %sub48, %34
  %35 = load i32, i32* %vw1, align 4, !tbaa !1
  %sub50 = sub i32 %add49, %35
  %36 = load i32, i32* %kq, align 4, !tbaa !1
  %add51 = add i32 %sub50, %36
  %37 = load i32, i32* %xc, align 4, !tbaa !1
  %sub52 = sub i32 %add51, %37
  %call54 = call i32 @checkSum(i32* nonnull %arraydecay, i32 100)
  %add55 = add i32 %sub52, %call54
  %call56 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.3, i64 0, i64 0), i32 %add55)
  call void @llvm.lifetime.end(i64 400, i8* nonnull %12) #6
  call void @llvm.lifetime.end(i64 4, i8* %11) #6
  call void @llvm.lifetime.end(i64 4, i8* nonnull %10) #6
  call void @llvm.lifetime.end(i64 4, i8* %9) #6
  call void @llvm.lifetime.end(i64 4, i8* %8) #6
  call void @llvm.lifetime.end(i64 4, i8* %7) #6
  call void @llvm.lifetime.end(i64 4, i8* %6) #6
  call void @llvm.lifetime.end(i64 4, i8* %5) #6
  call void @llvm.lifetime.end(i64 4, i8* %4) #6
  call void @llvm.lifetime.end(i64 4, i8* %3) #6
  call void @llvm.lifetime.end(i64 4, i8* %2) #6
  call void @llvm.lifetime.end(i64 4, i8* %1) #6
  call void @llvm.lifetime.end(i64 4, i8* %0) #6
  ret i32 0
}

; Function Attrs: nounwind
declare noalias %struct._IO_FILE* @fopen(i8* nocapture readonly, i8* nocapture readonly) #4

declare i32 @__isoc99_fscanf(%struct._IO_FILE*, i8*, ...) #5

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) #4

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { norecurse nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15842) (llvm/branches/loopopt 15857)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

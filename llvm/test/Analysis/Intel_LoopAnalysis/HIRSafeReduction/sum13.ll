; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s

; CHECK:  %hir.de.ssa.copy0.out = %2; <Safe Reduction>
;
; ModuleID = 't2278.c'
source_filename = "t2278.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type { ptr, ptr, i32 }

@.str = private unnamed_addr constant [10 x i8] c"input.txt\00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.2 = private unnamed_addr constant [36 x i8] c"%u %u %u %u %u %u %u %u %u %u %u %u\00", align 1
@.str.3 = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

; Function Attrs: norecurse nounwind uwtable
define void @init(ptr nocapture %a, i32 %n, i32 %seed) #0 {
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
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 %rem2, ptr %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture) #1

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @checkSum(ptr nocapture readonly %a, i32 %n) #2 {
entry:
  %cmp10 = icmp eq i32 %n, 0
  br i1 %cmp10, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %sum.012 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %rem = and i64 %indvars.iv, 1
  %cmp1 = icmp eq i64 %rem, 0
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !1
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
  %call = tail call ptr @fopen(ptr @.str, ptr @.str.1)
  call void @llvm.lifetime.start(i64 4, ptr %jk) #6
  store i32 80, ptr %jk, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %j) #6
  store i32 84, ptr %j, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %of6) #6
  store i32 53, ptr %of6, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %nb3) #6
  store i32 98, ptr %nb3, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %kx9) #6
  store i32 29, ptr %kx9, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %jn) #6
  store i32 25, ptr %jn, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %zm) #6
  store i32 95, ptr %zm, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %n7) #6
  store i32 16, ptr %n7, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %vw1) #6
  store i32 20, ptr %vw1, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %kq) #6
  store i32 8, ptr %kq, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %k) #6
  store i32 96, ptr %k, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 4, ptr %xc) #6
  store i32 15, ptr %xc, align 4, !tbaa !1
  call void @llvm.lifetime.start(i64 400, ptr %c) #6
  call void @init(ptr %c, i32 100, i32 31)
  %call1 = call i32 (ptr, ptr, ...) @__isoc99_fscanf(ptr %call, ptr @.str.2, ptr nonnull %jk, ptr nonnull %j, ptr nonnull %of6, ptr nonnull %nb3, ptr nonnull %kx9, ptr nonnull %jn, ptr nonnull %zm, ptr nonnull %n7, ptr nonnull %vw1, ptr nonnull %kq, ptr nonnull %k, ptr nonnull %xc) #6
  store i32 28, ptr %jk, align 4, !tbaa !1
  %nb3.promoted = load i32, ptr %nb3, align 4, !tbaa !1
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %entry, %for.end
  %dec1380 = phi i32 [ 28, %entry ], [ %dec13, %for.end ]
  br label %for.body4

for.cond15.preheader:                             ; preds = %for.end
  %0 = add i32 %nb3.promoted, 27
  store i32 1, ptr %j, align 4, !tbaa !1
  store i32 %0, ptr %nb3, align 4, !tbaa !1
  store i32 %0, ptr %of6, align 4, !tbaa !1
  store i32 1, ptr %jk, align 4, !tbaa !1
  store i32 28, ptr %kx9, align 4, !tbaa !1
  %n7.promoted94 = load i32, ptr %n7, align 4, !tbaa !1
  %zm.promoted95 = load i32, ptr %zm, align 4, !tbaa !1
  br label %for.end24

for.body4:                                        ; preds = %for.cond2.preheader, %for.body4
  %indvars.iv92 = phi i64 [ 28, %for.cond2.preheader ], [ %indvars.iv.next93, %for.body4 ]
  %indvars.iv.next93 = add nsw i64 %indvars.iv92, -1
  %arrayidx = getelementptr inbounds [100 x i32], ptr %c, i64 0, i64 %indvars.iv.next93
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !1
  %sub5 = sub i32 0, %1
  %arrayidx7 = getelementptr inbounds [100 x i32], ptr %c, i64 0, i64 %indvars.iv92
  store i32 %sub5, ptr %arrayidx7, align 4, !tbaa !1
  %cmp3 = icmp ugt i64 %indvars.iv.next93, 1
  br i1 %cmp3, label %for.body4, label %for.end

for.end:                                          ; preds = %for.body4
  %dec13 = add nsw i32 %dec1380, -1
  %cmp = icmp ugt i32 %dec13, 1
  br i1 %cmp, label %for.cond2.preheader, label %for.cond15.preheader

for.end24:                                        ; preds = %for.end24, %for.cond15.preheader
  %2 = phi i32 [ %zm.promoted95, %for.cond15.preheader ], [ %12, %for.end24 ]
  %3 = phi i32 [ %n7.promoted94, %for.cond15.preheader ], [ %13, %for.end24 ]
  %indvars.iv89 = phi i32 [ 28, %for.cond15.preheader ], [ %indvars.iv.next90, %for.end24 ]
  %indvars.iv87 = phi i32 [ 27, %for.cond15.preheader ], [ %indvars.iv.next88, %for.end24 ]
  %indvars.iv85 = phi i32 [ 25, %for.cond15.preheader ], [ %indvars.iv.next86, %for.end24 ]
  %indvars.iv83 = phi i33 [ 26, %for.cond15.preheader ], [ %indvars.iv.next84, %for.end24 ]
  %indvars.iv81 = phi i32 [ 26, %for.cond15.preheader ], [ %indvars.iv.next82, %for.end24 ]
  %4 = add i32 %2, %3
  %5 = add i32 %3, 1
  %6 = mul i32 %indvars.iv81, %5
  %7 = add i32 %4, %6
  %8 = zext i32 %indvars.iv85 to i33
  %9 = mul i33 %indvars.iv83, %8
  %10 = lshr i33 %9, 1
  %11 = trunc i33 %10 to i32
  %12 = add i32 %7, %11
  %13 = add i32 %3, %indvars.iv87
  %indvars.iv.next90 = add nsw i32 %indvars.iv89, -1
  %cmp16 = icmp ugt i32 %indvars.iv.next90, 1
  %indvars.iv.next82 = add nsw i32 %indvars.iv81, -1
  %indvars.iv.next84 = add nsw i33 %indvars.iv83, -1
  %indvars.iv.next86 = add nsw i32 %indvars.iv85, -1
  %indvars.iv.next88 = add nsw i32 %indvars.iv87, -1
  br i1 %cmp16, label %for.end24, label %for.end35

for.end35:                                        ; preds = %for.end24
  store i32 %13, ptr %n7, align 4, !tbaa !1
  store i32 %12, ptr %zm, align 4, !tbaa !1
  store i32 2, ptr %jn, align 4, !tbaa !1
  store i32 1, ptr %kx9, align 4, !tbaa !1
  %arrayidx36 = getelementptr inbounds [100 x i32], ptr %c, i64 0, i64 34
  %14 = load i32, ptr %arrayidx36, align 8, !tbaa !1
  %15 = load i32, ptr %kq, align 4, !tbaa !1
  %add37 = add i32 %15, %14
  store i32 %add37, ptr %kq, align 4, !tbaa !1
  store i32 2, ptr %k, align 4, !tbaa !1
  %16 = load i32, ptr %zm, align 4, !tbaa !1
  %xc.promoted = load i32, ptr %xc, align 4, !tbaa !1
  br label %for.body40

for.body40:                                       ; preds = %for.body40, %for.end35
  %indvars.iv = phi i64 [ 2, %for.end35 ], [ %indvars.iv.next, %for.body40 ]
  %mul67 = phi i32 [ %xc.promoted, %for.end35 ], [ %mul, %for.body40 ]
  %arrayidx42 = getelementptr inbounds [100 x i32], ptr %c, i64 0, i64 %indvars.iv
  %17 = load i32, ptr %arrayidx42, align 4, !tbaa !1
  %add43 = add i32 %16, %17
  %mul = mul i32 %mul67, %add43
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 68
  br i1 %exitcond, label %for.end46, label %for.body40

for.end46:                                        ; preds = %for.body40
  store i32 %mul, ptr %xc, align 4, !tbaa !1
  store i32 68, ptr %k, align 4, !tbaa !1
  %18 = load i32, ptr %of6, align 4, !tbaa !1
  %19 = load i32, ptr %nb3, align 4, !tbaa !1
  %add47 = add i32 %19, %18
  %20 = load i32, ptr %zm, align 4, !tbaa !1
  %sub48 = sub i32 %add47, %20
  %21 = load i32, ptr %n7, align 4, !tbaa !1
  %add49 = add i32 %sub48, %21
  %22 = load i32, ptr %vw1, align 4, !tbaa !1
  %sub50 = sub i32 %add49, %22
  %23 = load i32, ptr %kq, align 4, !tbaa !1
  %add51 = add i32 %sub50, %23
  %24 = load i32, ptr %xc, align 4, !tbaa !1
  %sub52 = sub i32 %add51, %24
  %call54 = call i32 @checkSum(ptr nonnull %c, i32 100)
  %add55 = add i32 %sub52, %call54
  %call56 = call i32 (ptr, ...) @printf(ptr @.str.3, i32 %add55)
  call void @llvm.lifetime.end(i64 400, ptr nonnull %c) #6
  call void @llvm.lifetime.end(i64 4, ptr %xc) #6
  call void @llvm.lifetime.end(i64 4, ptr nonnull %k) #6
  call void @llvm.lifetime.end(i64 4, ptr %kq) #6
  call void @llvm.lifetime.end(i64 4, ptr %vw1) #6
  call void @llvm.lifetime.end(i64 4, ptr %n7) #6
  call void @llvm.lifetime.end(i64 4, ptr %zm) #6
  call void @llvm.lifetime.end(i64 4, ptr %jn) #6
  call void @llvm.lifetime.end(i64 4, ptr %kx9) #6
  call void @llvm.lifetime.end(i64 4, ptr %nb3) #6
  call void @llvm.lifetime.end(i64 4, ptr %of6) #6
  call void @llvm.lifetime.end(i64 4, ptr %j) #6
  call void @llvm.lifetime.end(i64 4, ptr %jk) #6
  ret i32 0
}

; Function Attrs: nounwind
declare noalias ptr @fopen(ptr nocapture readonly, ptr nocapture readonly) #4

declare i32 @__isoc99_fscanf(ptr, ptr, ...) #5

; Function Attrs: nounwind
declare i32 @printf(ptr nocapture readonly, ...) #4

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

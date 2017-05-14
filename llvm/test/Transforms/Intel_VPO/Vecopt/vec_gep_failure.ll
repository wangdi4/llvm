; RUN: opt -S -O2 -loopopt=0 -vplan-build-stress-test -vplan-driver -vplan-build-vect-candidates=100000 -vplan-enable-subregions -vplan-predicator -vpo-codegen %s > /dev/null
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.s_net = type { i8*, i32, i32*, float, float }
%struct.s_block = type { i8*, i32, i32*, i32, i32 }
%struct.s_subblock = type { i8*, i32, i32, i32* }

@num_nets = external local_unnamed_addr global i32, align 4
@net = external local_unnamed_addr global %struct.s_net*, align 8
@num_blocks = external local_unnamed_addr global i32, align 4
@block = external local_unnamed_addr global %struct.s_block*, align 8
@is_global = external local_unnamed_addr global i32*, align 8
@net_pin_class = external local_unnamed_addr global i32**, align 8
@pins_per_clb = external local_unnamed_addr global i32, align 4
@num_subblocks_per_block = external local_unnamed_addr global i32*, align 8
@subblock_inf = external local_unnamed_addr global %struct.s_subblock**, align 8

; Function Attrs: nounwind uwtable
define void @read_net(i8* nocapture readnone %net_file) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %init_parse.exit, %entry
  %doall.011 = phi i32 [ 0, %entry ], [ %inc, %init_parse.exit ]
  %0 = load i32, i32* @num_nets, align 4, !tbaa !1
  %conv.i = sext i32 %0 to i64
  %mul.i = shl nsw i64 %conv.i, 5
  %call.i = tail call i8* @my_malloc(i64 %mul.i) #2
  store i8* %call.i, i8** bitcast (%struct.s_net** @net to i8**), align 8, !tbaa !5
  %1 = load i32, i32* @num_blocks, align 4, !tbaa !1
  %conv1.i = sext i32 %1 to i64
  %mul2.i = shl nsw i64 %conv1.i, 5
  %call3.i = tail call i8* @my_malloc(i64 %mul2.i) #2
  store i8* %call3.i, i8** bitcast (%struct.s_block** @block to i8**), align 8, !tbaa !5
  %2 = load i32, i32* @num_nets, align 4, !tbaa !1
  %conv4.i = sext i32 %2 to i64
  %call5.i = tail call i8* @my_calloc(i64 %conv4.i, i64 4) #2
  store i8* %call5.i, i8** bitcast (i32** @is_global to i8**), align 8, !tbaa !7
  %3 = load i32, i32* @num_nets, align 4, !tbaa !1
  %conv6.i = sext i32 %3 to i64
  %mul7.i = shl nsw i64 %conv6.i, 2
  %call8.i = tail call i8* @my_malloc(i64 %mul7.i) #2
  %4 = load i32, i32* @num_nets, align 4, !tbaa !1
  %conv9.i = sext i32 %4 to i64
  %mul10.i = shl nsw i64 %conv9.i, 2
  %call11.i = tail call i8* @my_malloc(i64 %mul10.i) #2
  %5 = load i32, i32* @num_nets, align 4, !tbaa !1
  %conv12.i = sext i32 %5 to i64
  %mul13.i = shl nsw i64 %conv12.i, 3
  %call14.i = tail call i8* @my_malloc(i64 %mul13.i) #2
  store i8* %call14.i, i8** bitcast (i32*** @net_pin_class to i8**), align 8, !tbaa !9
  %6 = load i32, i32* @pins_per_clb, align 4, !tbaa !1
  %7 = load i32, i32* @num_blocks, align 4, !tbaa !1
  %mul15.i = mul nsw i32 %7, %6
  %conv16.i = sext i32 %mul15.i to i64
  %mul17.i = shl nsw i64 %conv16.i, 2
  %call18.i = tail call i8* @my_malloc(i64 %mul17.i) #2
  %8 = bitcast i8* %call18.i to i32*
  %9 = load i32, i32* @num_blocks, align 4, !tbaa !1
  %cmp.i7 = icmp sgt i32 %9, 0
  br i1 %cmp.i7, label %for.body.i.lr.ph, label %init_parse.exit

for.body.i.lr.ph:                                 ; preds = %for.body
  %10 = load i32, i32* @pins_per_clb, align 4, !tbaa !1
  %11 = load %struct.s_block*, %struct.s_block** @block, align 8, !tbaa !5
  %12 = sext i32 %10 to i64
  %13 = sext i32 %9 to i64
  %14 = add nsw i64 %13, -1
  %xtraiter = and i64 %13, 7
  %lcmp.mod = icmp eq i64 %xtraiter, 0
  br i1 %lcmp.mod, label %for.body.i.prol.loopexit, label %for.body.i.prol.preheader

for.body.i.prol.preheader:                        ; preds = %for.body.i.lr.ph
  br label %for.body.i.prol

for.body.i.prol:                                  ; preds = %for.body.i.prol, %for.body.i.prol.preheader
  %indvars.iv.prol = phi i64 [ 0, %for.body.i.prol.preheader ], [ %indvars.iv.next.prol, %for.body.i.prol ]
  %prol.iter = phi i64 [ %xtraiter, %for.body.i.prol.preheader ], [ %prol.iter.sub, %for.body.i.prol ]
  %15 = mul nsw i64 %12, %indvars.iv.prol
  %add.ptr.i.prol = getelementptr inbounds i32, i32* %8, i64 %15
  %nets.i.prol = getelementptr inbounds %struct.s_block, %struct.s_block* %11, i64 %indvars.iv.prol, i32 2
  store i32* %add.ptr.i.prol, i32** %nets.i.prol, align 8, !tbaa !11
  %indvars.iv.next.prol = add nuw nsw i64 %indvars.iv.prol, 1
  %prol.iter.sub = add i64 %prol.iter, -1
  %prol.iter.cmp = icmp eq i64 %prol.iter.sub, 0
  br i1 %prol.iter.cmp, label %for.body.i.prol.loopexit.unr-lcssa, label %for.body.i.prol, !llvm.loop !14

for.body.i.prol.loopexit.unr-lcssa:               ; preds = %for.body.i.prol
  br label %for.body.i.prol.loopexit

for.body.i.prol.loopexit:                         ; preds = %for.body.i.lr.ph, %for.body.i.prol.loopexit.unr-lcssa
  %indvars.iv.unr = phi i64 [ 0, %for.body.i.lr.ph ], [ %indvars.iv.next.prol, %for.body.i.prol.loopexit.unr-lcssa ]
  %16 = icmp ult i64 %14, 7
  br i1 %16, label %for.body24.i.lr.ph, label %for.body.i.lr.ph.new

for.body.i.lr.ph.new:                             ; preds = %for.body.i.prol.loopexit
  br label %for.body.i

for.body24.i.lr.ph.unr-lcssa:                     ; preds = %for.body.i
  br label %for.body24.i.lr.ph

for.body24.i.lr.ph:                               ; preds = %for.body.i.prol.loopexit, %for.body24.i.lr.ph.unr-lcssa
  %17 = load i32*, i32** @num_subblocks_per_block, align 8, !tbaa !7
  %xtraiter17 = and i64 %13, 3
  %lcmp.mod18 = icmp eq i64 %xtraiter17, 0
  br i1 %lcmp.mod18, label %for.body24.i.prol.loopexit, label %for.body24.i.prol.preheader

for.body24.i.prol.preheader:                      ; preds = %for.body24.i.lr.ph
  br label %for.body24.i.prol

for.body24.i.prol:                                ; preds = %for.inc31.i.prol, %for.body24.i.prol.preheader
  %indvars.iv13.prol = phi i64 [ 0, %for.body24.i.prol.preheader ], [ %indvars.iv.next14.prol, %for.inc31.i.prol ]
  %prol.iter19 = phi i64 [ %xtraiter17, %for.body24.i.prol.preheader ], [ %prol.iter19.sub, %for.inc31.i.prol ]
  %arrayidx26.i.prol = getelementptr inbounds i32, i32* %17, i64 %indvars.iv13.prol
  %18 = load i32, i32* %arrayidx26.i.prol, align 4, !tbaa !1
  %cmp27.i.prol = icmp eq i32 %18, 0
  br i1 %cmp27.i.prol, label %if.then.i.prol, label %for.inc31.i.prol

if.then.i.prol:                                   ; preds = %for.body24.i.prol
  %19 = load %struct.s_subblock**, %struct.s_subblock*** @subblock_inf, align 8, !tbaa !5
  %arrayidx30.i.prol = getelementptr inbounds %struct.s_subblock*, %struct.s_subblock** %19, i64 %indvars.iv13.prol
  store %struct.s_subblock* null, %struct.s_subblock** %arrayidx30.i.prol, align 8, !tbaa !5
  br label %for.inc31.i.prol

for.inc31.i.prol:                                 ; preds = %if.then.i.prol, %for.body24.i.prol
  %indvars.iv.next14.prol = add nuw nsw i64 %indvars.iv13.prol, 1
  %prol.iter19.sub = add i64 %prol.iter19, -1
  %prol.iter19.cmp = icmp eq i64 %prol.iter19.sub, 0
  br i1 %prol.iter19.cmp, label %for.body24.i.prol.loopexit.unr-lcssa, label %for.body24.i.prol, !llvm.loop !16

for.body24.i.prol.loopexit.unr-lcssa:             ; preds = %for.inc31.i.prol
  br label %for.body24.i.prol.loopexit

for.body24.i.prol.loopexit:                       ; preds = %for.body24.i.lr.ph, %for.body24.i.prol.loopexit.unr-lcssa
  %indvars.iv13.unr = phi i64 [ 0, %for.body24.i.lr.ph ], [ %indvars.iv.next14.prol, %for.body24.i.prol.loopexit.unr-lcssa ]
  %20 = icmp ult i64 %14, 3
  br i1 %20, label %init_parse.exit.loopexit, label %for.body24.i.lr.ph.new

for.body24.i.lr.ph.new:                           ; preds = %for.body24.i.prol.loopexit
  br label %for.body24.i

for.body.i:                                       ; preds = %for.body.i, %for.body.i.lr.ph.new
  %indvars.iv = phi i64 [ %indvars.iv.unr, %for.body.i.lr.ph.new ], [ %indvars.iv.next.7, %for.body.i ]
  %21 = mul nsw i64 %12, %indvars.iv
  %add.ptr.i = getelementptr inbounds i32, i32* %8, i64 %21
  %nets.i = getelementptr inbounds %struct.s_block, %struct.s_block* %11, i64 %indvars.iv, i32 2
  store i32* %add.ptr.i, i32** %nets.i, align 8, !tbaa !11
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %22 = mul nsw i64 %12, %indvars.iv.next
  %add.ptr.i.1 = getelementptr inbounds i32, i32* %8, i64 %22
  %nets.i.1 = getelementptr inbounds %struct.s_block, %struct.s_block* %11, i64 %indvars.iv.next, i32 2
  store i32* %add.ptr.i.1, i32** %nets.i.1, align 8, !tbaa !11
  %indvars.iv.next.1 = add nsw i64 %indvars.iv, 2
  %23 = mul nsw i64 %12, %indvars.iv.next.1
  %add.ptr.i.2 = getelementptr inbounds i32, i32* %8, i64 %23
  %nets.i.2 = getelementptr inbounds %struct.s_block, %struct.s_block* %11, i64 %indvars.iv.next.1, i32 2
  store i32* %add.ptr.i.2, i32** %nets.i.2, align 8, !tbaa !11
  %indvars.iv.next.2 = add nsw i64 %indvars.iv, 3
  %24 = mul nsw i64 %12, %indvars.iv.next.2
  %add.ptr.i.3 = getelementptr inbounds i32, i32* %8, i64 %24
  %nets.i.3 = getelementptr inbounds %struct.s_block, %struct.s_block* %11, i64 %indvars.iv.next.2, i32 2
  store i32* %add.ptr.i.3, i32** %nets.i.3, align 8, !tbaa !11
  %indvars.iv.next.3 = add nsw i64 %indvars.iv, 4
  %25 = mul nsw i64 %12, %indvars.iv.next.3
  %add.ptr.i.4 = getelementptr inbounds i32, i32* %8, i64 %25
  %nets.i.4 = getelementptr inbounds %struct.s_block, %struct.s_block* %11, i64 %indvars.iv.next.3, i32 2
  store i32* %add.ptr.i.4, i32** %nets.i.4, align 8, !tbaa !11
  %indvars.iv.next.4 = add nsw i64 %indvars.iv, 5
  %26 = mul nsw i64 %12, %indvars.iv.next.4
  %add.ptr.i.5 = getelementptr inbounds i32, i32* %8, i64 %26
  %nets.i.5 = getelementptr inbounds %struct.s_block, %struct.s_block* %11, i64 %indvars.iv.next.4, i32 2
  store i32* %add.ptr.i.5, i32** %nets.i.5, align 8, !tbaa !11
  %indvars.iv.next.5 = add nsw i64 %indvars.iv, 6
  %27 = mul nsw i64 %12, %indvars.iv.next.5
  %add.ptr.i.6 = getelementptr inbounds i32, i32* %8, i64 %27
  %nets.i.6 = getelementptr inbounds %struct.s_block, %struct.s_block* %11, i64 %indvars.iv.next.5, i32 2
  store i32* %add.ptr.i.6, i32** %nets.i.6, align 8, !tbaa !11
  %indvars.iv.next.6 = add nsw i64 %indvars.iv, 7
  %28 = mul nsw i64 %12, %indvars.iv.next.6
  %add.ptr.i.7 = getelementptr inbounds i32, i32* %8, i64 %28
  %nets.i.7 = getelementptr inbounds %struct.s_block, %struct.s_block* %11, i64 %indvars.iv.next.6, i32 2
  store i32* %add.ptr.i.7, i32** %nets.i.7, align 8, !tbaa !11
  %indvars.iv.next.7 = add nsw i64 %indvars.iv, 8
  %exitcond15.7 = icmp eq i64 %indvars.iv.next.7, %13
  br i1 %exitcond15.7, label %for.body24.i.lr.ph.unr-lcssa, label %for.body.i

for.body24.i:                                     ; preds = %for.inc31.i.3, %for.body24.i.lr.ph.new
  %indvars.iv13 = phi i64 [ %indvars.iv13.unr, %for.body24.i.lr.ph.new ], [ %indvars.iv.next14.3, %for.inc31.i.3 ]
  %arrayidx26.i = getelementptr inbounds i32, i32* %17, i64 %indvars.iv13
  %29 = load i32, i32* %arrayidx26.i, align 4, !tbaa !1
  %cmp27.i = icmp eq i32 %29, 0
  br i1 %cmp27.i, label %if.then.i, label %for.inc31.i

if.then.i:                                        ; preds = %for.body24.i
  %30 = load %struct.s_subblock**, %struct.s_subblock*** @subblock_inf, align 8, !tbaa !5
  %arrayidx30.i = getelementptr inbounds %struct.s_subblock*, %struct.s_subblock** %30, i64 %indvars.iv13
  store %struct.s_subblock* null, %struct.s_subblock** %arrayidx30.i, align 8, !tbaa !5
  br label %for.inc31.i

for.inc31.i:                                      ; preds = %if.then.i, %for.body24.i
  %indvars.iv.next14 = add nuw nsw i64 %indvars.iv13, 1
  %arrayidx26.i.1 = getelementptr inbounds i32, i32* %17, i64 %indvars.iv.next14
  %31 = load i32, i32* %arrayidx26.i.1, align 4, !tbaa !1
  %cmp27.i.1 = icmp eq i32 %31, 0
  br i1 %cmp27.i.1, label %if.then.i.1, label %for.inc31.i.1

init_parse.exit.loopexit.unr-lcssa:               ; preds = %for.inc31.i.3
  br label %init_parse.exit.loopexit

init_parse.exit.loopexit:                         ; preds = %for.body24.i.prol.loopexit, %init_parse.exit.loopexit.unr-lcssa
  br label %init_parse.exit

init_parse.exit:                                  ; preds = %init_parse.exit.loopexit, %for.body
  %inc = add nuw nsw i32 %doall.011, 1
  %exitcond = icmp eq i32 %inc, 2
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %init_parse.exit
  ret void

if.then.i.1:                                      ; preds = %for.inc31.i
  %32 = load %struct.s_subblock**, %struct.s_subblock*** @subblock_inf, align 8, !tbaa !5
  %arrayidx30.i.1 = getelementptr inbounds %struct.s_subblock*, %struct.s_subblock** %32, i64 %indvars.iv.next14
  store %struct.s_subblock* null, %struct.s_subblock** %arrayidx30.i.1, align 8, !tbaa !5
  br label %for.inc31.i.1

for.inc31.i.1:                                    ; preds = %if.then.i.1, %for.inc31.i
  %indvars.iv.next14.1 = add nsw i64 %indvars.iv13, 2
  %arrayidx26.i.2 = getelementptr inbounds i32, i32* %17, i64 %indvars.iv.next14.1
  %33 = load i32, i32* %arrayidx26.i.2, align 4, !tbaa !1
  %cmp27.i.2 = icmp eq i32 %33, 0
  br i1 %cmp27.i.2, label %if.then.i.2, label %for.inc31.i.2

if.then.i.2:                                      ; preds = %for.inc31.i.1
  %34 = load %struct.s_subblock**, %struct.s_subblock*** @subblock_inf, align 8, !tbaa !5
  %arrayidx30.i.2 = getelementptr inbounds %struct.s_subblock*, %struct.s_subblock** %34, i64 %indvars.iv.next14.1
  store %struct.s_subblock* null, %struct.s_subblock** %arrayidx30.i.2, align 8, !tbaa !5
  br label %for.inc31.i.2

for.inc31.i.2:                                    ; preds = %if.then.i.2, %for.inc31.i.1
  %indvars.iv.next14.2 = add nsw i64 %indvars.iv13, 3
  %arrayidx26.i.3 = getelementptr inbounds i32, i32* %17, i64 %indvars.iv.next14.2
  %35 = load i32, i32* %arrayidx26.i.3, align 4, !tbaa !1
  %cmp27.i.3 = icmp eq i32 %35, 0
  br i1 %cmp27.i.3, label %if.then.i.3, label %for.inc31.i.3

if.then.i.3:                                      ; preds = %for.inc31.i.2
  %36 = load %struct.s_subblock**, %struct.s_subblock*** @subblock_inf, align 8, !tbaa !5
  %arrayidx30.i.3 = getelementptr inbounds %struct.s_subblock*, %struct.s_subblock** %36, i64 %indvars.iv.next14.2
  store %struct.s_subblock* null, %struct.s_subblock** %arrayidx30.i.3, align 8, !tbaa !5
  br label %for.inc31.i.3

for.inc31.i.3:                                    ; preds = %if.then.i.3, %for.inc31.i.2
  %indvars.iv.next14.3 = add nsw i64 %indvars.iv13, 4
  %exitcond16.3 = icmp eq i64 %indvars.iv.next14.3, %13
  br i1 %exitcond16.3, label %init_parse.exit.loopexit.unr-lcssa, label %for.body24.i
}

declare i8* @my_malloc(i64) local_unnamed_addr #1

declare i8* @my_calloc(i64, i64) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+aes,+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+rtm,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+aes,+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+rtm,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 21234)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"unspecified pointer", !3, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"pointer@_ZTSPi", !3, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"pointer@_ZTSPPi", !3, i64 0}
!11 = !{!12, !8, i64 16}
!12 = !{!"struct@s_block", !13, i64 0, !3, i64 8, !8, i64 16, !2, i64 24, !2, i64 28}
!13 = !{!"pointer@_ZTSPc", !3, i64 0}
!14 = distinct !{!14, !15}
!15 = !{!"llvm.loop.unroll.disable"}
!16 = distinct !{!16, !15}

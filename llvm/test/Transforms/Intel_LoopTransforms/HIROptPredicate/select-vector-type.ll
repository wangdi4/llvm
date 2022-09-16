; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate" -aa-pipeline="basic-aa" -disable-output -print-before=hir-opt-predicate -print-after=hir-opt-predicate < %s -disable-output -xmain-opt-level=3 < %s 2>&1 | FileCheck %s

; This test case checks that the Select instruction wasn't converted into
; and If/Else because the type of the predicate's operands is a vector
; type.

; HIR before and after opt-predicate should be the same

; CHECK: IR Dump Before HIROptPredicatePass
; CHECK       : BEGIN REGION { }
;                  + DO i1 = 0, 49, 1   <DO_LOOP>
;                  |   %3 = (<8 x double>*)(@dsrc1)[0][2 * i1];
;                  |   %add.i94 = %3  +  %3;
;                  |   (<8 x double>*)(@ddest_zmm)[0][2 * i1] = %add.i94;
;                  |   %5 = (<8 x double>*)(@dsrc1)[0][2 * i1 + 1];
;                  |   %6 = (<8 x double>*)(@ddest_zmm)[0][2 * i1 + 1];
;                  |   %add.i.i = %5  +  %5;
; CHECK       :    |   %7 = (%2 != 0) ? %add.i.i : %6;
;                  |   (<8 x double>*)(@ddest_zmm)[0][2 * i1 + 1] = %7;
;                  + END LOOP
;              END REGION

; CHECK: IR Dump After HIROptPredicatePass
; CHECK      :  BEGIN REGION { }
;                  + DO i1 = 0, 49, 1   <DO_LOOP>
;                  |   %3 = (<8 x double>*)(@dsrc1)[0][2 * i1];
;                  |   %add.i94 = %3  +  %3;
;                  |   (<8 x double>*)(@ddest_zmm)[0][2 * i1] = %add.i94;
;                  |   %5 = (<8 x double>*)(@dsrc1)[0][2 * i1 + 1];
;                  |   %6 = (<8 x double>*)(@ddest_zmm)[0][2 * i1 + 1];
;                  |   %add.i.i = %5  +  %5;
; CHECK      :     |   %7 = (%2 != 0) ? %add.i.i : %6;
;                  |   (<8 x double>*)(@ddest_zmm)[0][2 * i1 + 1] = %7;
;                  + END LOOP
;              END REGION

%union.V512 = type { <16 x float> }

@full_mask = dso_local global i16 -1
@ddest_xmm = dso_local global [100 x %union.V512] zeroinitializer
@dsrc1 = dso_local local_unnamed_addr global [100 x %union.V512] zeroinitializer
@ddest_zmm = dso_local global [100 x %union.V512] zeroinitializer
@n_errs = dso_local local_unnamed_addr global i32 0
@str.2 = private unnamed_addr constant [17 x i8] c"checkdest FAILED\00"
@dsrc2 = dso_local local_unnamed_addr global [100 x %union.V512] zeroinitializer

; Function Attrs: nofree nounwind uwtable
define dso_local void @do_add_pd() local_unnamed_addr #0 {
entry:
  %0 = load volatile i16, i16* @full_mask
  store i64 0, i64* bitcast ([100 x %union.V512]* @ddest_xmm to i64*)
  br label %for.cond3.preheader

for.cond3.preheader:                              ; preds = %entry, %for.inc19
  %indvars.iv108 = phi i64 [ 0, %entry ], [ %indvars.iv.next109, %for.inc19 ]
  %arrayidx = getelementptr inbounds [100 x %union.V512], [100 x %union.V512]* @dsrc1, i64 0, i64 %indvars.iv108
  %xmmd = bitcast %union.V512* %arrayidx to [4 x <2 x double>]*
  %arrayidx15 = getelementptr inbounds [100 x %union.V512], [100 x %union.V512]* @ddest_xmm, i64 0, i64 %indvars.iv108
  %xmmd16 = bitcast %union.V512* %arrayidx15 to [4 x <2 x double>]*
  br label %for.body6

for.body6:                                        ; preds = %for.cond3.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond3.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx8 = getelementptr inbounds [4 x <2 x double>], [4 x <2 x double>]* %xmmd, i64 0, i64 %indvars.iv
  %1 = load <2 x double>, <2 x double>* %arrayidx8
  %add.i = fadd <2 x double> %1, %1
  %arrayidx18 = getelementptr inbounds [4 x <2 x double>], [4 x <2 x double>]* %xmmd16, i64 0, i64 %indvars.iv
  store <2 x double> %add.i, <2 x double>* %arrayidx18
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %for.inc19, label %for.body6

for.inc19:                                        ; preds = %for.body6
  %indvars.iv.next109 = add nuw nsw i64 %indvars.iv108, 1
  %exitcond110.not = icmp eq i64 %indvars.iv.next109, 100
  br i1 %exitcond110.not, label %for.end21, label %for.cond3.preheader

for.end21:                                        ; preds = %for.inc19
  %conv = trunc i16 %0 to i8
  store i64 0, i64* bitcast ([100 x %union.V512]* @ddest_zmm to i64*)
  %2 = bitcast i8 %conv to <8 x i1>
  br label %for.body26

for.body26:                                       ; preds = %for.end21, %for.body26
  %indvars.iv111 = phi i64 [ 0, %for.end21 ], [ %indvars.iv.next112, %for.body26 ]
  %arrayidx28 = getelementptr inbounds [100 x %union.V512], [100 x %union.V512]* @dsrc1, i64 0, i64 %indvars.iv111
  %zmmd = bitcast %union.V512* %arrayidx28 to <8 x double>*
  %3 = load <8 x double>, <8 x double>* %zmmd
  %add.i94 = fadd <8 x double> %3, %3
  %arrayidx34 = getelementptr inbounds [100 x %union.V512], [100 x %union.V512]* @ddest_zmm, i64 0, i64 %indvars.iv111
  %zmmd35 = bitcast %union.V512* %arrayidx34 to <8 x double>*
  store <8 x double> %add.i94, <8 x double>* %zmmd35
  %4 = or i64 %indvars.iv111, 1
  %arrayidx37 = getelementptr inbounds [100 x %union.V512], [100 x %union.V512]* @dsrc1, i64 0, i64 %4
  %zmmd38 = bitcast %union.V512* %arrayidx37 to <8 x double>*
  %5 = load <8 x double>, <8 x double>* %zmmd38
  %arrayidx45 = getelementptr inbounds [100 x %union.V512], [100 x %union.V512]* @ddest_zmm, i64 0, i64 %4
  %zmmd46 = bitcast %union.V512* %arrayidx45 to <8 x double>*
  %6 = load <8 x double>, <8 x double>* %zmmd46
  %add.i.i = fadd <8 x double> %5, %5
  %7 = select <8 x i1> %2, <8 x double> %add.i.i, <8 x double> %6
  store <8 x double> %7, <8 x double>* %zmmd46
  %indvars.iv.next112 = add nuw nsw i64 %indvars.iv111, 2
  %cmp24 = icmp ult i64 %indvars.iv111, 98
  br i1 %cmp24, label %for.body26, label %for.end54

for.end54:                                        ; preds = %for.body26
  %call.i = tail call i32 @memcmp(i8* noundef nonnull dereferenceable(6400) bitcast ([100 x %union.V512]* @ddest_xmm to i8*), i8* noundef nonnull dereferenceable(6400) bitcast ([100 x %union.V512]* @ddest_zmm to i8*), i64 noundef 6400)
  %cmp.not.i = icmp eq i32 %call.i, 0
  br i1 %cmp.not.i, label %checkddest.exit, label %if.then.i

if.then.i:                                        ; preds = %for.end54
  %puts.i = tail call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([17 x i8], [17 x i8]* @str.2, i64 0, i64 0))
  %8 = load i32, i32* @n_errs
  %inc.i = add nsw i32 %8, 1
  store i32 %inc.i, i32* @n_errs
  br label %checkddest.exit

checkddest.exit:                                  ; preds = %for.end54, %if.then.i
  br label %for.body58

for.body58:                                       ; preds = %checkddest.exit, %for.body58
  %indvars.iv114 = phi i64 [ 0, %checkddest.exit ], [ %indvars.iv.next115, %for.body58 ]
  %arrayidx60 = getelementptr inbounds [4 x <2 x double>], [4 x <2 x double>]* bitcast ([100 x %union.V512]* @dsrc2 to [4 x <2 x double>]*), i64 0, i64 %indvars.iv114
  %9 = load <2 x double>, <2 x double>* %arrayidx60
  %arrayidx62 = getelementptr inbounds [4 x <2 x double>], [4 x <2 x double>]* bitcast ([100 x %union.V512]* @dsrc1 to [4 x <2 x double>]*), i64 0, i64 %indvars.iv114
  %10 = load <2 x double>, <2 x double>* %arrayidx62
  %add.i95 = fadd <2 x double> %9, %10
  %arrayidx65 = getelementptr inbounds [4 x <2 x double>], [4 x <2 x double>]* bitcast ([100 x %union.V512]* @ddest_xmm to [4 x <2 x double>]*), i64 0, i64 %indvars.iv114
  store <2 x double> %add.i95, <2 x double>* %arrayidx65
  %indvars.iv.next115 = add nuw nsw i64 %indvars.iv114, 1
  %exitcond116.not = icmp eq i64 %indvars.iv.next115, 4
  br i1 %exitcond116.not, label %for.end68, label %for.body58

for.end68:                                        ; preds = %for.body58
  %11 = load <8 x double>, <8 x double>* bitcast ([100 x %union.V512]* @dsrc2 to <8 x double>*)
  %12 = load <8 x double>, <8 x double>* bitcast ([100 x %union.V512]* @dsrc1 to <8 x double>*)
  %add.i96 = fadd <8 x double> %11, %12
  store <8 x double> %add.i96, <8 x double>* bitcast ([100 x %union.V512]* @ddest_zmm to <8 x double>*)
  %call.i97 = tail call i32 @memcmp(i8* noundef nonnull dereferenceable(6400) bitcast ([100 x %union.V512]* @ddest_xmm to i8*), i8* noundef nonnull dereferenceable(6400) bitcast ([100 x %union.V512]* @ddest_zmm to i8*), i64 noundef 6400)
  %cmp.not.i98 = icmp eq i32 %call.i97, 0
  br i1 %cmp.not.i98, label %checkddest.exit102, label %if.then.i101

if.then.i101:                                     ; preds = %for.end68
  %puts.i99 = tail call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([17 x i8], [17 x i8]* @str.2, i64 0, i64 0))
  %13 = load i32, i32* @n_errs
  %inc.i100 = add nsw i32 %13, 1
  store i32 %inc.i100, i32* @n_errs
  br label %checkddest.exit102

checkddest.exit102:                               ; preds = %for.end68, %if.then.i101
  ret void
}

declare dso_local i32 @memcmp(i8* nocapture noundef, i8* nocapture noundef, i64 noundef)

declare noundef i32 @puts(i8* nocapture noundef readonly)

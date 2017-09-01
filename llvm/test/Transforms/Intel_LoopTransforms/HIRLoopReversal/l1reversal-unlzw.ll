; Sanity Test(s) on HIR Loop Reversal: reversal case extracted from spec2000/164.gzip/unlzw.c
; l1reversal-unlzw.ll
; 
; [REASONS]
; - Applicable: YES
; - Profitable: YES
; - Legal:      YES
; This testcase helps to zap bug(s) from the HIR Framework.
;
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
; for (i = 0; i<=255; ++i){
;   A[255-i] = 255-i;
;
; 
; [AFTER LOOP REVERSAL]
; 
; for(i = 0; i<=255; ++i) {
;  A[i] = i;
; }
;
; [Note]
; The original loop in the unlzw.c file is below:
;243   for (code = 255 ; code >= 0 ; --code) {
;244     tab_suffixof(code) = (char_type)code;
;245   }
;
; Demo code shown on #15 and #21 are normalized version from HIR level. 
;  
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=BEFORE 
;
; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal, DOESN'T REVERSE anything ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=AFTER 
;
;
; === -------------------------------------- ===
; *** Tests0:  Output                        ***
; === -------------------------------------- ===
; Expected output before Loop Reversal
; 
;          BEGIN REGION { }
;<11>            + DO i1 = 0, 255, 1   <DO_LOOP>
;<4>             |   (@window)[0][-1 * i1 + 255] = -1 * i1 + 255;
;<11>            + END LOOP
;          END REGION
; 
; BEFORE:  BEGIN REGION { }
; BEFORE:        + DO i1 = 0, 255, 1   <DO_LOOP>
; BEFORE:        |   (@window)[0][-1 * i1 + 255] = -1 * i1 + 255;
; BEFORE:        + END LOOP
; BEFORE:  END REGION
;
;
; === -------------------------------------- ===
; *** Tests1:  Output                        ***
; === -------------------------------------- ===
; Expected output AFTER	 Loop Reversal
; 
;          BEGIN REGION { modified }
;<11>            + DO i1 = 0, 255, 1   <DO_LOOP>
;<4>             |   (@window)[0][i1] = i1;
;<11>            + END LOOP
;          END REGION
;
;
; AFTER:   BEGIN REGION { modified }
; AFTER:         + DO i1 = 0, 255, 1   <DO_LOOP>
; AFTER:         |   (@window)[0][i1] = i1;
; AFTER:         + END LOOP
; AFTER:   END REGION
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "unlzw.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@block_mode = global i32 128, align 4
@inptr = external global i32, align 4
@insize = external global i32, align 4
@inbuf = external global [0 x i8], align 1
@maxbits = external global i32, align 4
@quiet = external global i32, align 4
@stderr = external global %struct._IO_FILE*, align 8
@.str = private unnamed_addr constant [38 x i8] c"\0A%s: %s: warning, unknown flags 0x%x\0A\00", align 1
@progname = external global i8*, align 8
@ifname = external global [0 x i8], align 1
@exit_code = external global i32, align 4
@.str.1 = private unnamed_addr constant [59 x i8] c"\0A%s: %s: compressed with %d bits, can only handle %d bits\0A\00", align 1
@prev = external global [0 x i16], align 2
@window = external global [0 x i8], align 1
@bytes_in = external global i64, align 8
@.str.2 = private unnamed_addr constant [15 x i8] c"corrupt input.\00", align 1
@outbuf = external global [0 x i8], align 1
@d_buf = external global [0 x i16], align 2
@test = external global i32, align 4
@bytes_out = external global i64, align 8
@to_stdout = external global i32, align 4
@.str.3 = private unnamed_addr constant [46 x i8] c"corrupt input. Use zcat to recover some data.\00", align 1

; Function Attrs: nounwind uwtable
define i32 @unlzw(i32 %in, i32 %out) #0 {
entry:
  %0 = load i32, i32* @inptr, align 4, !tbaa !1
  %1 = load i32, i32* @insize, align 4, !tbaa !1
  %cmp = icmp ult i32 %0, %1
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %inc = add i32 %0, 1
  store i32 %inc, i32* @inptr, align 4, !tbaa !1
  %idxprom = zext i32 %0 to i64
  %arrayidx = getelementptr inbounds [0 x i8], [0 x i8]* @inbuf, i64 0, i64 %idxprom
  %2 = load i8, i8* %arrayidx, align 1, !tbaa !5
  %conv = zext i8 %2 to i32
  br label %cond.end

cond.false:                                       ; preds = %entry
  %call = tail call i32 @fill_inbuf(i32 0) #4
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %conv, %cond.true ], [ %call, %cond.false ]
  store i32 %cond, i32* @maxbits, align 4, !tbaa !1
  %and = and i32 %cond, 128
  store i32 %and, i32* @block_mode, align 4, !tbaa !1
  %and1 = and i32 %cond, 96
  %cmp2 = icmp eq i32 %and1, 0
  br i1 %cmp2, label %if.end11, label %if.then

if.then:                                          ; preds = %cond.end
  %3 = load i32, i32* @quiet, align 4, !tbaa !1
  %tobool = icmp eq i32 %3, 0
  br i1 %tobool, label %if.then4, label %if.end

if.then4:                                         ; preds = %if.then
  %4 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !6
  %5 = load i8*, i8** @progname, align 8, !tbaa !6
  %call6 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %4, i8* getelementptr inbounds ([38 x i8], [38 x i8]* @.str, i64 0, i64 0), i8* %5, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @ifname, i64 0, i64 0), i32 %and1) #5
  br label %if.end

if.end:                                           ; preds = %if.then, %if.then4
  %6 = load i32, i32* @exit_code, align 4, !tbaa !1
  %cmp7 = icmp eq i32 %6, 0
  br i1 %cmp7, label %if.then9, label %if.end11

if.then9:                                         ; preds = %if.end
  store i32 2, i32* @exit_code, align 4, !tbaa !1
  br label %if.end11

if.end11:                                         ; preds = %cond.end, %if.end, %if.then9
  %7 = load i32, i32* @maxbits, align 4, !tbaa !1
  %and12 = and i32 %7, 31
  store i32 %and12, i32* @maxbits, align 4, !tbaa !1
  %sh_prom = zext i32 %and12 to i64
  %shl = shl i64 1, %sh_prom
  %cmp13 = icmp ugt i32 %and12, 16
  br i1 %cmp13, label %if.then15, label %if.end17

if.then15:                                        ; preds = %if.end11
  %8 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !6
  %9 = load i8*, i8** @progname, align 8, !tbaa !6
  %call16 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %8, i8* getelementptr inbounds ([59 x i8], [59 x i8]* @.str.1, i64 0, i64 0), i8* %9, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @ifname, i64 0, i64 0), i32 %and12, i32 16) #5
  store i32 1, i32* @exit_code, align 4, !tbaa !1
  br label %cleanup

if.end17:                                         ; preds = %if.end11
  %10 = load i32, i32* @insize, align 4, !tbaa !1
  %11 = load i32, i32* @inptr, align 4, !tbaa !1
  %shl19 = shl i32 %11, 3
  %12 = load i32, i32* @block_mode, align 4, !tbaa !1
  %tobool21 = icmp ne i32 %12, 0
  tail call void @llvm.memset.p0i8.i64(i8* bitcast ([0 x i16]* @prev to i8*), i8 0, i64 256, i32 2, i1 false)
  br label %for.body

for.body:                                         ; preds = %if.end17, %for.body
  %code.0436 = phi i64 [ 255, %if.end17 ], [ %dec, %for.body ]
  %conv26 = trunc i64 %code.0436 to i8
  %arrayidx27 = getelementptr inbounds [0 x i8], [0 x i8]* @window, i64 0, i64 %code.0436
  store i8 %conv26, i8* %arrayidx27, align 1, !tbaa !5
  %dec = add nsw i64 %code.0436, -1
  %cmp24 = icmp sgt i64 %code.0436, 0
  br i1 %cmp24, label %for.body, label %do.body.preheader

do.body.preheader:                                ; preds = %for.body
  %conv20 = zext i32 %shl19 to i64
  %conv23 = select i1 %tobool21, i64 257, i64 256
  br label %do.body

do.body:                                          ; preds = %do.body.preheader, %while.end228
  %oldcode.0 = phi i64 [ %oldcode.2.lcssa, %while.end228 ], [ -1, %do.body.preheader ]
  %posbits.0 = phi i64 [ %posbits.2.lcssa, %while.end228 ], [ %conv20, %do.body.preheader ]
  %outpos.0 = phi i32 [ %outpos.2.lcssa, %while.end228 ], [ 0, %do.body.preheader ]
  %bitmask.0 = phi i32 [ %bitmask.1, %while.end228 ], [ 511, %do.body.preheader ]
  %free_ent.0 = phi i64 [ %free_ent.2.ph.lcssa377, %while.end228 ], [ %conv23, %do.body.preheader ]
  %maxcode.0 = phi i64 [ %maxcode.1, %while.end228 ], [ 511, %do.body.preheader ]
  %finchar.0 = phi i32 [ %finchar.2.lcssa, %while.end228 ], [ 0, %do.body.preheader ]
  %n_bits.0 = phi i32 [ %n_bits.1, %while.end228 ], [ 9, %do.body.preheader ]
  %rsize.0 = phi i32 [ %rsize.2, %while.end228 ], [ %10, %do.body.preheader ]
  br label %resetbuf

resetbuf:                                         ; preds = %if.then129, %if.end92, %do.body
  %oldcode.1 = phi i64 [ %oldcode.0, %do.body ], [ %oldcode.2.ph411, %if.end92 ], [ %oldcode.2386, %if.then129 ]
  %posbits.1 = phi i64 [ %posbits.0, %do.body ], [ %add84, %if.end92 ], [ %add141, %if.then129 ]
  %outpos.1 = phi i32 [ %outpos.0, %do.body ], [ %outpos.2.ph413, %if.end92 ], [ %outpos.2388, %if.then129 ]
  %bitmask.1 = phi i32 [ %bitmask.0, %do.body ], [ %sub94, %if.end92 ], [ 511, %if.then129 ]
  %free_ent.1 = phi i64 [ %free_ent.0, %do.body ], [ %free_ent.2.ph414, %if.end92 ], [ 256, %if.then129 ]
  %maxcode.1 = phi i64 [ %maxcode.0, %do.body ], [ %maxcode.2, %if.end92 ], [ 511, %if.then129 ]
  %finchar.1 = phi i32 [ %finchar.0, %do.body ], [ %finchar.2.ph415, %if.end92 ], [ %finchar.2389, %if.then129 ]
  %n_bits.1 = phi i32 [ %n_bits.0, %do.body ], [ %inc85, %if.end92 ], [ 9, %if.then129 ]
  %rsize.1 = phi i32 [ %rsize.0, %do.body ], [ %rsize.2, %if.end92 ], [ %rsize.2, %if.then129 ]
  %13 = load i32, i32* @insize, align 4, !tbaa !1
  %shr358 = lshr i64 %posbits.1, 3
  %conv28 = trunc i64 %shr358 to i32
  %sub29 = sub i32 %13, %conv28
  %cmp31378 = icmp sgt i32 %sub29, 0
  br i1 %cmp31378, label %for.body33.preheader, label %for.end40

for.body33.preheader:                             ; preds = %resetbuf
  %sext486 = shl i64 %shr358, 32
  %14 = ashr exact i64 %sext486, 32
  br label %for.body33

for.body33:                                       ; preds = %for.body33, %for.body33.preheader
  %indvars.iv = phi i64 [ 0, %for.body33.preheader ], [ %indvars.iv.next, %for.body33 ]
  %15 = add nsw i64 %indvars.iv, %14
  %arrayidx35 = getelementptr inbounds [0 x i8], [0 x i8]* @inbuf, i64 0, i64 %15
  %16 = load i8, i8* %arrayidx35, align 1, !tbaa !5
  %arrayidx37 = getelementptr inbounds [0 x i8], [0 x i8]* @inbuf, i64 0, i64 %indvars.iv
  store i8 %16, i8* %arrayidx37, align 1, !tbaa !5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %sub29
  br i1 %exitcond, label %for.end40, label %for.body33

for.end40:                                        ; preds = %for.body33, %resetbuf
  store i32 %sub29, i32* @insize, align 4, !tbaa !1
  %cmp41 = icmp ult i32 %sub29, 64
  br i1 %cmp41, label %if.then43, label %if.end52

if.then43:                                        ; preds = %for.end40
  %idx.ext = zext i32 %sub29 to i64
  %add.ptr = getelementptr inbounds [0 x i8], [0 x i8]* @inbuf, i64 0, i64 %idx.ext
  %call44 = tail call i32 (i32, i8*, i32, ...) bitcast (i32 (...)* @spec_read to i32 (i32, i8*, i32, ...)*)(i32 %in, i8* %add.ptr, i32 32768) #4
  %cmp45 = icmp eq i32 %call44, -1
  br i1 %cmp45, label %if.then47, label %if.end48

if.then47:                                        ; preds = %if.then43
  tail call void @read_error() #4
  br label %if.end48

if.end48:                                         ; preds = %if.then47, %if.then43
  %17 = load i32, i32* @insize, align 4, !tbaa !1
  %add49 = add i32 %17, %call44
  store i32 %add49, i32* @insize, align 4, !tbaa !1
  %conv50 = sext i32 %call44 to i64
  %18 = load i64, i64* @bytes_in, align 8, !tbaa !8
  %add51 = add i64 %18, %conv50
  store i64 %add51, i64* @bytes_in, align 8, !tbaa !8
  br label %if.end52

if.end52:                                         ; preds = %if.end48, %for.end40
  %19 = phi i32 [ %add49, %if.end48 ], [ %sub29, %for.end40 ]
  %rsize.2 = phi i32 [ %call44, %if.end48 ], [ %rsize.1, %for.end40 ]
  %cmp53 = icmp ne i32 %rsize.2, 0
  %conv56 = zext i32 %19 to i64
  br i1 %cmp53, label %cond.true55, label %cond.false60

cond.true55:                                      ; preds = %if.end52
  %rem = urem i32 %19, %n_bits.1
  %conv57 = zext i32 %rem to i64
  %sub58 = sub nsw i64 %conv56, %conv57
  %shl59 = shl nsw i64 %sub58, 3
  br label %cond.end66

cond.false60:                                     ; preds = %if.end52
  %shl62 = shl nuw nsw i64 %conv56, 3
  %sub63 = add nsw i32 %n_bits.1, -1
  %conv64 = sext i32 %sub63 to i64
  %sub65 = sub nsw i64 %shl62, %conv64
  br label %cond.end66

cond.end66:                                       ; preds = %cond.false60, %cond.true55
  %cond67 = phi i64 [ %shl59, %cond.true55 ], [ %sub65, %cond.false60 ]
  %cmp68385410 = icmp sgt i64 %cond67, 0
  br i1 %cmp68385410, label %while.body.lr.ph.lr.ph, label %while.end228

while.body.lr.ph.lr.ph:                           ; preds = %cond.end66
  %conv109 = zext i32 %bitmask.1 to i64
  %conv111 = sext i32 %n_bits.1 to i64
  br label %while.body.lr.ph

while.body.lr.ph:                                 ; preds = %while.body.lr.ph.lr.ph, %if.then221
  %finchar.2.ph415 = phi i32 [ %finchar.1, %while.body.lr.ph.lr.ph ], [ %conv173, %if.then221 ]
  %free_ent.2.ph414 = phi i64 [ %free_ent.1, %while.body.lr.ph.lr.ph ], [ %add226, %if.then221 ]
  %outpos.2.ph413 = phi i32 [ %outpos.1, %while.body.lr.ph.lr.ph ], [ %outpos.6, %if.then221 ]
  %posbits.2.ph412 = phi i64 [ 0, %while.body.lr.ph.lr.ph ], [ %add112, %if.then221 ]
  %oldcode.2.ph411 = phi i64 [ %oldcode.1, %while.body.lr.ph.lr.ph ], [ %and110, %if.then221 ]
  %cmp70 = icmp sgt i64 %free_ent.2.ph414, %maxcode.1
  %cmp219 = icmp slt i64 %free_ent.2.ph414, %shl
  br label %while.body

while.body:                                       ; preds = %while.body.lr.ph, %while.cond.backedge
  %finchar.2389 = phi i32 [ %finchar.2.ph415, %while.body.lr.ph ], [ %finchar.2.be, %while.cond.backedge ]
  %outpos.2388 = phi i32 [ %outpos.2.ph413, %while.body.lr.ph ], [ %outpos.2.be, %while.cond.backedge ]
  %posbits.2387 = phi i64 [ %posbits.2.ph412, %while.body.lr.ph ], [ %add112, %while.cond.backedge ]
  %oldcode.2386 = phi i64 [ %oldcode.2.ph411, %while.body.lr.ph ], [ %and110, %while.cond.backedge ]
  br i1 %cmp70, label %if.then72, label %if.end95

if.then72:                                        ; preds = %while.body
  %sub73 = add nsw i64 %posbits.2.ph412, -1
  %shl74 = shl i32 %n_bits.1, 3
  %conv75 = sext i32 %shl74 to i64
  %add79 = add i64 %sub73, %conv75
  %rem82 = srem i64 %add79, %conv75
  %add84 = sub i64 %add79, %rem82
  %inc85 = add nsw i32 %n_bits.1, 1
  %20 = load i32, i32* @maxbits, align 4, !tbaa !1
  %cmp86 = icmp eq i32 %inc85, %20
  br i1 %cmp86, label %if.end92, label %if.else

if.else:                                          ; preds = %if.then72
  %sh_prom89 = zext i32 %inc85 to i64
  %shl90 = shl i64 1, %sh_prom89
  %sub91 = add nsw i64 %shl90, -1
  br label %if.end92

if.end92:                                         ; preds = %if.then72, %if.else
  %maxcode.2 = phi i64 [ %sub91, %if.else ], [ %shl, %if.then72 ]
  %shl93 = shl i32 1, %inc85
  %sub94 = add nsw i32 %shl93, -1
  br label %resetbuf

if.end95:                                         ; preds = %while.body
  %shr96 = ashr i64 %posbits.2387, 3
  %arrayidx97 = getelementptr inbounds [0 x i8], [0 x i8]* @inbuf, i64 0, i64 %shr96
  %21 = load i8, i8* %arrayidx97, align 1, !tbaa !5
  %conv99 = zext i8 %21 to i64
  %arrayidx100 = getelementptr inbounds i8, i8* %arrayidx97, i64 1
  %22 = load i8, i8* %arrayidx100, align 1, !tbaa !5
  %conv101 = zext i8 %22 to i64
  %shl102 = shl nuw nsw i64 %conv101, 8
  %or = or i64 %shl102, %conv99
  %arrayidx103 = getelementptr inbounds i8, i8* %arrayidx97, i64 2
  %23 = load i8, i8* %arrayidx103, align 1, !tbaa !5
  %conv104 = zext i8 %23 to i64
  %shl105 = shl nuw nsw i64 %conv104, 16
  %or106 = or i64 %or, %shl105
  %and107 = and i64 %posbits.2387, 7
  %shr108 = lshr i64 %or106, %and107
  %and110 = and i64 %shr108, %conv109
  %add112 = add nsw i64 %posbits.2387, %conv111
  %cmp113 = icmp eq i64 %oldcode.2386, -1
  br i1 %cmp113, label %if.then115, label %if.end125

if.then115:                                       ; preds = %if.end95
  %cmp116 = icmp ugt i64 %and110, 255
  br i1 %cmp116, label %if.then118, label %if.end119

if.then118:                                       ; preds = %if.then115
  tail call void @error(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.2, i64 0, i64 0)) #4
  br label %if.end119

if.end119:                                        ; preds = %if.then118, %if.then115
  %conv120 = trunc i64 %and110 to i32
  %conv121 = trunc i64 %and110 to i8
  %inc122 = add nsw i32 %outpos.2388, 1
  %idxprom123 = sext i32 %outpos.2388 to i64
  %arrayidx124 = getelementptr inbounds [0 x i8], [0 x i8]* @outbuf, i64 0, i64 %idxprom123
  store i8 %conv121, i8* %arrayidx124, align 1, !tbaa !5
  br label %while.cond.backedge

while.cond.backedge:                              ; preds = %if.end119, %if.end218
  %outpos.2.be = phi i32 [ %inc122, %if.end119 ], [ %outpos.6, %if.end218 ]
  %finchar.2.be = phi i32 [ %conv120, %if.end119 ], [ %conv173, %if.end218 ]
  %cmp68 = icmp sgt i64 %cond67, %add112
  br i1 %cmp68, label %while.body, label %while.end228

if.end125:                                        ; preds = %if.end95
  %cmp126 = icmp eq i64 %and110, 256
  %24 = load i32, i32* @block_mode, align 4
  %tobool128 = icmp ne i32 %24, 0
  %or.cond = and i1 %cmp126, %tobool128
  br i1 %or.cond, label %if.then129, label %if.end144

if.then129:                                       ; preds = %if.end125
  tail call void @llvm.memset.p0i8.i64(i8* bitcast ([0 x i16]* @prev to i8*), i8 0, i64 256, i32 2, i1 false)
  %sub130 = add nsw i64 %add112, -1
  %shl131 = shl i32 %n_bits.1, 3
  %conv132 = sext i32 %shl131 to i64
  %add136 = add i64 %sub130, %conv132
  %rem139 = srem i64 %add136, %conv132
  %add141 = sub i64 %add136, %rem139
  br label %resetbuf

if.end144:                                        ; preds = %if.end125
  %cmp145 = icmp slt i64 %and110, %free_ent.2.ph414
  br i1 %cmp145, label %while.cond164.preheader, label %if.then147

if.then147:                                       ; preds = %if.end144
  %cmp148 = icmp sgt i64 %and110, %free_ent.2.ph414
  br i1 %cmp148, label %if.then150, label %if.end161

if.then150:                                       ; preds = %if.then147
  %25 = load i32, i32* @test, align 4, !tbaa !1
  %tobool151 = icmp eq i32 %25, 0
  %cmp153 = icmp sgt i32 %outpos.2388, 0
  %or.cond254 = and i1 %cmp153, %tobool151
  br i1 %or.cond254, label %if.then155, label %if.end158

if.then155:                                       ; preds = %if.then150
  tail call void @write_buf(i32 %out, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @outbuf, i64 0, i64 0), i32 %outpos.2388) #4
  %conv156 = sext i32 %outpos.2388 to i64
  %26 = load i64, i64* @bytes_out, align 8, !tbaa !8
  %add157 = add i64 %26, %conv156
  store i64 %add157, i64* @bytes_out, align 8, !tbaa !8
  br label %if.end158

if.end158:                                        ; preds = %if.then150, %if.then155
  %27 = load i32, i32* @to_stdout, align 4, !tbaa !1
  %tobool159 = icmp ne i32 %27, 0
  %cond160 = select i1 %tobool159, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.2, i64 0, i64 0), i8* getelementptr inbounds ([46 x i8], [46 x i8]* @.str.3, i64 0, i64 0)
  tail call void @error(i8* %cond160) #4
  br label %if.end161

if.end161:                                        ; preds = %if.end158, %if.then147
  %conv162 = trunc i32 %finchar.2389 to i8
  store i8 %conv162, i8* getelementptr (i8, i8* bitcast (i16* getelementptr inbounds ([0 x i16], [0 x i16]* @d_buf, i64 0, i64 32767) to i8*), i64 -1), align 1, !tbaa !5
  br label %while.cond164.preheader

while.cond164.preheader:                          ; preds = %if.end161, %if.end144
  %code.1.ph = phi i64 [ %and110, %if.end144 ], [ %oldcode.2386, %if.end161 ]
  %stackp.0.ph = phi i8* [ bitcast (i16* getelementptr inbounds ([0 x i16], [0 x i16]* @d_buf, i64 0, i64 32767) to i8*), %if.end144 ], [ getelementptr (i8, i8* bitcast (i16* getelementptr inbounds ([0 x i16], [0 x i16]* @d_buf, i64 0, i64 32767) to i8*), i64 -1), %if.end161 ]
  %cmp165380 = icmp ugt i64 %code.1.ph, 255
  %arrayidx168381 = getelementptr inbounds [0 x i8], [0 x i8]* @window, i64 0, i64 %code.1.ph
  %28 = load i8, i8* %arrayidx168381, align 1, !tbaa !5
  br i1 %cmp165380, label %while.body167, label %while.end

while.body167:                                    ; preds = %while.cond164.preheader, %while.body167
  %29 = phi i8 [ %31, %while.body167 ], [ %28, %while.cond164.preheader ]
  %stackp.0383 = phi i8* [ %incdec.ptr169, %while.body167 ], [ %stackp.0.ph, %while.cond164.preheader ]
  %code.1382 = phi i64 [ %conv171, %while.body167 ], [ %code.1.ph, %while.cond164.preheader ]
  %incdec.ptr169 = getelementptr inbounds i8, i8* %stackp.0383, i64 -1
  store i8 %29, i8* %incdec.ptr169, align 1, !tbaa !5
  %arrayidx170 = getelementptr inbounds [0 x i16], [0 x i16]* @prev, i64 0, i64 %code.1382
  %30 = load i16, i16* %arrayidx170, align 2, !tbaa !10
  %conv171 = zext i16 %30 to i64
  %cmp165 = icmp ugt i16 %30, 255
  %arrayidx168 = getelementptr inbounds [0 x i8], [0 x i8]* @window, i64 0, i64 %conv171
  %31 = load i8, i8* %arrayidx168, align 1, !tbaa !5
  br i1 %cmp165, label %while.body167, label %while.end

while.end:                                        ; preds = %while.body167, %while.cond164.preheader
  %.lcssa = phi i8 [ %28, %while.cond164.preheader ], [ %31, %while.body167 ]
  %stackp.0.lcssa = phi i8* [ %stackp.0.ph, %while.cond164.preheader ], [ %incdec.ptr169, %while.body167 ]
  %conv173 = zext i8 %.lcssa to i32
  %incdec.ptr175 = getelementptr inbounds i8, i8* %stackp.0.lcssa, i64 -1
  store i8 %.lcssa, i8* %incdec.ptr175, align 1, !tbaa !5
  %sub.ptr.rhs.cast = ptrtoint i8* %incdec.ptr175 to i64
  %sub.ptr.sub = sub i64 ptrtoint (i16* getelementptr inbounds ([0 x i16], [0 x i16]* @d_buf, i64 0, i64 32767) to i64), %sub.ptr.rhs.cast
  %conv177 = trunc i64 %sub.ptr.sub to i32
  %add178 = add nsw i32 %conv177, %outpos.2388
  %cmp179 = icmp sgt i32 %add178, 16383
  br i1 %cmp179, label %do.body182, label %if.else213

do.body182:                                       ; preds = %while.end, %if.end205
  %outpos.3 = phi i32 [ %outpos.5, %if.end205 ], [ %outpos.2388, %while.end ]
  %stackp.1 = phi i8* [ %add.ptr207, %if.end205 ], [ %incdec.ptr175, %while.end ]
  %i176.0 = phi i32 [ %conv210, %if.end205 ], [ %conv177, %while.end ]
  %sub183 = sub nsw i32 16384, %outpos.3
  %cmp184 = icmp sgt i32 %i176.0, %sub183
  %sub183.i176.0 = select i1 %cmp184, i32 %sub183, i32 %i176.0
  %cmp189 = icmp sgt i32 %sub183.i176.0, 0
  br i1 %cmp189, label %if.then191, label %if.end196

if.then191:                                       ; preds = %do.body182
  %idx.ext192 = sext i32 %outpos.3 to i64
  %add.ptr193 = getelementptr inbounds [0 x i8], [0 x i8]* @outbuf, i64 0, i64 %idx.ext192
  %conv194 = sext i32 %sub183.i176.0 to i64
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %add.ptr193, i8* %stackp.1, i64 %conv194, i32 1, i1 false)
  %add195 = add nsw i32 %sub183.i176.0, %outpos.3
  br label %if.end196

if.end196:                                        ; preds = %if.then191, %do.body182
  %outpos.4 = phi i32 [ %add195, %if.then191 ], [ %outpos.3, %do.body182 ]
  %cmp197 = icmp sgt i32 %outpos.4, 16383
  br i1 %cmp197, label %if.then199, label %if.end205

if.then199:                                       ; preds = %if.end196
  %32 = load i32, i32* @test, align 4, !tbaa !1
  %tobool200 = icmp eq i32 %32, 0
  br i1 %tobool200, label %if.then201, label %if.end205

if.then201:                                       ; preds = %if.then199
  tail call void @write_buf(i32 %out, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @outbuf, i64 0, i64 0), i32 %outpos.4) #4
  %conv202 = sext i32 %outpos.4 to i64
  %33 = load i64, i64* @bytes_out, align 8, !tbaa !8
  %add203 = add i64 %33, %conv202
  store i64 %add203, i64* @bytes_out, align 8, !tbaa !8
  br label %if.end205

if.end205:                                        ; preds = %if.then201, %if.then199, %if.end196
  %outpos.5 = phi i32 [ %outpos.4, %if.end196 ], [ 0, %if.then199 ], [ 0, %if.then201 ]
  %idx.ext206 = sext i32 %sub183.i176.0 to i64
  %add.ptr207 = getelementptr inbounds i8, i8* %stackp.1, i64 %idx.ext206
  %sub.ptr.rhs.cast208 = ptrtoint i8* %add.ptr207 to i64
  %sub.ptr.sub209 = sub i64 ptrtoint (i16* getelementptr inbounds ([0 x i16], [0 x i16]* @d_buf, i64 0, i64 32767) to i64), %sub.ptr.rhs.cast208
  %conv210 = trunc i64 %sub.ptr.sub209 to i32
  %cmp211 = icmp sgt i32 %conv210, 0
  br i1 %cmp211, label %do.body182, label %if.end218

if.else213:                                       ; preds = %while.end
  %idx.ext214 = sext i32 %outpos.2388 to i64
  %add.ptr215 = getelementptr inbounds [0 x i8], [0 x i8]* @outbuf, i64 0, i64 %idx.ext214
  %sext = shl i64 %sub.ptr.sub, 32
  %conv216 = ashr exact i64 %sext, 32
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %add.ptr215, i8* %incdec.ptr175, i64 %conv216, i32 1, i1 false)
  br label %if.end218

if.end218:                                        ; preds = %if.end205, %if.else213
  %outpos.6 = phi i32 [ %add178, %if.else213 ], [ %outpos.5, %if.end205 ]
  br i1 %cmp219, label %if.then221, label %while.cond.backedge

if.then221:                                       ; preds = %if.end218
  %conv222 = trunc i64 %oldcode.2386 to i16
  %arrayidx223 = getelementptr inbounds [0 x i16], [0 x i16]* @prev, i64 0, i64 %free_ent.2.ph414
  store i16 %conv222, i16* %arrayidx223, align 2, !tbaa !10
  %arrayidx225 = getelementptr inbounds [0 x i8], [0 x i8]* @window, i64 0, i64 %free_ent.2.ph414
  store i8 %.lcssa, i8* %arrayidx225, align 1, !tbaa !5
  %add226 = add nsw i64 %free_ent.2.ph414, 1
  %cmp68385 = icmp sgt i64 %cond67, %add112
  br i1 %cmp68385, label %while.body.lr.ph, label %while.end228

while.end228:                                     ; preds = %cond.end66, %if.then221, %while.cond.backedge
  %free_ent.2.ph.lcssa377 = phi i64 [ %free_ent.2.ph414, %while.cond.backedge ], [ %add226, %if.then221 ], [ %free_ent.1, %cond.end66 ]
  %finchar.2.lcssa = phi i32 [ %finchar.2.be, %while.cond.backedge ], [ %conv173, %if.then221 ], [ %finchar.1, %cond.end66 ]
  %outpos.2.lcssa = phi i32 [ %outpos.2.be, %while.cond.backedge ], [ %outpos.6, %if.then221 ], [ %outpos.1, %cond.end66 ]
  %posbits.2.lcssa = phi i64 [ %add112, %while.cond.backedge ], [ %add112, %if.then221 ], [ 0, %cond.end66 ]
  %oldcode.2.lcssa = phi i64 [ %and110, %while.cond.backedge ], [ %and110, %if.then221 ], [ %oldcode.1, %cond.end66 ]
  br i1 %cmp53, label %do.body, label %do.end232

do.end232:                                        ; preds = %while.end228
  %34 = load i32, i32* @test, align 4, !tbaa !1
  %tobool233 = icmp eq i32 %34, 0
  %cmp235 = icmp sgt i32 %outpos.2.lcssa, 0
  %or.cond255 = and i1 %cmp235, %tobool233
  br i1 %or.cond255, label %if.then237, label %cleanup

if.then237:                                       ; preds = %do.end232
  tail call void @write_buf(i32 %out, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @outbuf, i64 0, i64 0), i32 %outpos.2.lcssa) #4
  %conv238 = sext i32 %outpos.2.lcssa to i64
  %35 = load i64, i64* @bytes_out, align 8, !tbaa !8
  %add239 = add i64 %35, %conv238
  store i64 %add239, i64* @bytes_out, align 8, !tbaa !8
  br label %cleanup

cleanup:                                          ; preds = %if.then237, %do.end232, %if.then15
  %retval.0 = phi i32 [ 1, %if.then15 ], [ 0, %do.end232 ], [ 0, %if.then237 ]
  ret i32 %retval.0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare i32 @fill_inbuf(i32) #2

; Function Attrs: nounwind
declare i32 @fprintf(%struct._IO_FILE* nocapture, i8* nocapture readonly, ...) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) #1

declare i32 @spec_read(...) #2

declare void @read_error() #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

declare void @error(i8*) #2

declare void @write_buf(i32, i8*, i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { cold }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 12506) (llvm/branches/loopopt 15673)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!3, !3, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"any pointer", !3, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"long", !3, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"short", !3, i64 0}

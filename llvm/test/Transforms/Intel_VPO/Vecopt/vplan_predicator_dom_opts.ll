; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -disable-predicator-opts -S -o /dev/null | FileCheck %s -check-prefix=NOOPT
; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -S -o /dev/null | FileCheck %s -check-prefix=OPT

; region1
; -------
;  BB19
;   |
;   v
; loop23
;   |
;   v
;  BB20


; loop23
; ------
;  BB18
;   |
;   v
;  BB2<----+
;   |      |
;   v      |
; region24 |
;   |      |
;   v      |
;  BB22----+
;   |
;   v
;  BB17


; region24
; --------
;   BB21---+
;    |     |
;    v     |
; region25 |
;    |     |
;    v     |
;   BB10   |
;    |     |
;    v     |
; region26 |
;    |     |
;    v     |
;   BB5<---+


; region25
; --------
; BB4
;  | \
;  v  v
; BB7 BB8
;  |  /
;  v v
; BB9


; region26
; --------
; BB11
;  | \
;  v  v
; BB13 BB14
;  |   /
;  v  v
; BB15


; NOOPT: [[loop_23:loop[0-9]+]]:
; NOOPT:   [[BB_18:BB[0-9]+]]:
; NOOPT:     [[BP_27:BP[0-9]+]] = 
; NOOPT:   [[BB_2:BB[0-9]+]]:
; NOOPT:     [[BP_28:BP[0-9]+]] = [[BP_27]]
; NOOPT:   [[region_24:region[0-9]+]]:
; NOOPT:     [[BP_28]] = [[BP_27]]
; NOOPT:   [[BB_22:BB[0-9]+]]:
; NOOPT:     [[BP_29:BP[0-9]+]] = [[BP_28]]
; NOOPT:   [[BB_17:BB[0-9]+]]:
; NOOPT:     [[BP_30:BP[0-9]+]] = [[BP_27]]

; NOOPT: [[region_24]]:
; NOOPT:   [[BB_21:BB[0-9]+]]:
; NOOPT:     [[BP_32:BP[0-9]+]] = [[BP_28]]
; NOOPT:     [[IfT_36:IfT[0-9]+]] = [[BP_32]] && [[VBR_35:VBR[0-9]+]]
; NOOPT:     [[IfF_37:IfF[0-9]+]] = [[BP_32]] && ![[VBR_35]]
; NOOPT:   [[region_25:region[0-9]+]]:
; NOOPT:     [[IfT_36]] = [[BP_32]] && [[VBR_35]]
; NOOPT:   [[BB_10:BB[0-9]+]]:
; NOOPT:     [[BP_33:BP[0-9]+]] = [[IfT_36]]
; NOOPT:   [[region_26:region[0-9]+]]:
; NOOPT:     [[BP_33]] = [[IfT_36]]
; NOOPT:   [[BB_5:BB[0-9]+]]:
; NOOPT:     [[BP_34:BP[0-9]+]] = [[BP_33]] || [[IfF_37]]

; NOOPT: [[region_25]]:
; NOOPT:   [[BB_4:BB[0-9]+]]:
; NOOPT:     [[BP_38:BP[0-9]+]] = [[IfT_36]]
; NOOPT:     [[IfF_43:IfF[0-9]+]] = [[BP_38]] && ![[VBR_42:VBR[0-9]+]]
; NOOPT:     [[IfT_44:IfT[0-9]+]] = [[BP_38]] && [[VBR_42]]
; NOOPT:   [[BB_8:BB[0-9]+]]:
; NOOPT:     [[BP_39:BP[0-9]+]] = [[IfF_43]]
; NOOPT:   [[BB_7:BB[0-9]+]]:
; NOOPT:     [[BP_40:BP[0-9]+]] = [[IfT_44]]
; NOOPT:   [[BB_9:BB[0-9]+]]:
; NOOPT:     [[BP_41:BP[0-9]+]] = [[BP_39]] || [[BP_40]]

; NOOPT: [[region_26]]:
; NOOPT:   [[BB_11:BB[0-9]+]]:
; NOOPT:     [[BP_47:BP[0-9]+]] = [[BP_33]]
; NOOPT:     [[IfF_52:IfF[0-9]+]] = [[BP_47]] && ![[VBR_51:VBR[0-9]+]]
; NOOPT:     [[IfT_53:IfT[0-9]+]] = [[BP_47]] && [[VBR_51]]
; NOOPT:   [[BB_14:BB[0-9]+]]:
; NOOPT:     [[BP_48:BP[0-9]+]] = [[IfF_52]]
; NOOPT:   [[BB_13:BB[0-9]+]]:
; NOOPT:     [[BP_49:BP[0-9]+]] = [[IfT_53]]
; NOOPT:   [[BB_15:BB[0-9]+]]:
; NOOPT:     [[BP_50:BP[0-9]+]] = [[BP_48]] || [[BP_49]]



; OPT: [[region_25:region[0-9]+]]:
; OPT:   [[BB_4:BB[0-9]+]]:
; OPT:     [[BP_38:BP[0-9]+]] = [[IfT_36:IfT[0-9]+]]
; OPT:     [[IfF_43:IfF[0-9]+]] = [[BP_38]] && ![[VBR_42:VBR[0-9]+]]
; OPT:     [[IfT_44:IfT[0-9]+]] = [[BP_38]] && [[VBR_42]]
; OPT:   [[BB_8:BB[0-9]+]]:
; OPT:     [[BP_39:BP[0-9]+]] = [[IfF_43]]
; OPT:   [[BB_7:BB[0-9]+]]:
; OPT:     [[BP_40:BP[0-9]+]] = [[IfT_44]]
; OPT:   [[BB_9:BB[0-9]+]]:
; OPT:     [[BP_41:BP[0-9]+]] = [[BP_38]]

; OPT: [[region_26:region[0-9]+]]:
; OPT:   [[BB_11:BB[0-9]+]]:
; OPT:     [[BP_47:BP[0-9]+]] = [[BP_33:BP[0-9]+]]
; OPT:     [[IfF_52:IfF[0-9]+]] = [[BP_47]] && ![[VBR_51:VBR[0-9]+]]
; OPT:     [[IfT_53:IfT[0-9]+]] = [[BP_47]] && [[VBR_51]]
; OPT:   [[BB_14:BB[0-9]+]]:
; OPT:     [[BP_48:BP[0-9]+]] = [[IfF_52]]
; OPT:   [[BB_13:BB[0-9]+]]:
; OPT:     [[BP_49:BP[0-9]+]] = [[IfT_53]]
; OPT:   [[BB_15:BB[0-9]+]]:
; OPT:     [[BP_50:BP[0-9]+]] = [[BP_47]]

; OPT: [[region_24:region[0-9]+]]:
; OPT:   [[BB_21:BB[0-9]+]]:
; OPT:     [[IfT_36]] = [[VBR_35:VBR[0-9]+]]
; OPT:     [[IfF_37:IfF[0-9]+]] = ![[VBR_35]]
; OPT:   [[region_25]]:
; OPT:     [[IfT_36]] = [[VBR_35]]
; OPT:   [[BB_10:BB[0-9]+]]:
; OPT:     [[BP_33]] = [[IfT_36]]
; OPT:   [[region_26]]:
; OPT:     [[BP_33]] = [[IfT_36]]
; OPT:   [[BB_5:BB[0-9]+]]:

; OPT: [[loop_23:loop[0-9]+]]:
; OPT:   [[BB_18:BB[0-9]+]]:
; OPT:   [[BB_2:BB[0-9]+]]:
; OPT:   [[region_24]]:
; OPT:   [[BB_22:BB[0-9]+]]:
; OPT:   [[BB_17:BB[0-9]+]]:


source_filename = "none"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture %c, i32 %N) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %if.end0 ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 0
  %0 = load i32, i32* %arrayidx, align 4
  %cmp0 = icmp sgt i64 %iv, 0
  br i1 %cmp0, label %if.then0, label %if.end0

if.then0:
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %if.then1, label %if.else1

if.then1:
  br label %if.end1

if.else1:
  br label %if.end1

if.end1:
  br label %bb.mid1

bb.mid1:
  br label %bb.mid2

bb.mid2:
  %cmp2 = icmp sgt i32 %0, 0
  br i1 %cmp2, label %if.then2, label %if.else2

if.then2:                                          ; preds = %if.end1
  br label %if.end2

if.else2:                                          ; preds = %if.end1
  br label %if.end2

if.end2:                                           ; preds = %if.else2, %if.then2
  br label %if.end0

if.end0:
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { noinline nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }




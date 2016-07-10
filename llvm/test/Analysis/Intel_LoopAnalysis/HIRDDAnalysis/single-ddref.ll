;
;
;  Loop has 1 single DDREF for a symbase
;  Still need to call DDTest and let it decide if Edge is needed  
;  
;  e.g. 
;   Loop i
;     VAR = i; 
; Obviously we need to build an edge for VAR with output dep
;
; RUN:  opt < %s -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK-DAG: (%gd)[0] OUTPUT (*)
;
define i32 @main() {
entry:
  %gd = alloca i32, align 4
  %m = alloca [64 x i32], align 16

  br label %for.cond12.preheader
 
for.cond12.preheader:                             ; preds = %entry, %for.inc16
  %indvars.iv546 = phi i64 [ 62, %entry ], [ %indvars.iv.next547, %for.inc16 ]
  %0 = phi i32 [ 61, %entry ], [ %dec17, %for.inc16 ]
  %cmp13488 = icmp ult i32 %0, 28
  br i1 %cmp13488, label %for.body14.preheader, label %for.inc16

for.body14.preheader:                             ; preds = %for.cond12.preheader
  %1 = add nuw nsw i64 %indvars.iv546, 1
  %arrayidx15 = getelementptr inbounds [64 x i32], [64 x i32]* %m, i64 0, i64 %1
  %2 = load i32, i32* %arrayidx15, align 4
  store i32 %2, i32* %gd, align 4
  br label %for.inc16

for.inc16:                                        ; preds = %for.body14.preheader, %for.cond12.preheader
  %dec17 = add nsw i32 %0, -1
  %cmp = icmp ugt i32 %dec17, 1
  %indvars.iv.next547 = add nsw i64 %indvars.iv546, -1
  br i1 %cmp, label %for.cond12.preheader, label %exit

exit:
  ret i32 0
}

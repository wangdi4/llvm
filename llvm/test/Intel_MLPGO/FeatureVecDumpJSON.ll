; RUN: env MLPGO_OUTPUT=%t.profile.mlpgo opt -passes=pgo-instr-use -pgo-test-profile-file=%S/code.profdata %s -S -o %t.out.ll
; The output file will have PID postfix
; RUN: cat %t.profile.mlpgo* | FileCheck %s --check-prefixes=FEATURE_DUMP
; Remove the output to avoid collision with subsequent runs of the test
; RUN: rm %t.profile.mlpgo*
; UNSUPPORTED: intel_use_sanitizers

; The IR is generated from
; int TripCount = 64;
;
; int main(int ARGC, char **ARGV) {
;
;   int Res = 0;
;   for (int I = 0; I < TripCount; ++I) {
;     if (ARGC * Res > 2)
;       Res += ARGC;
;     else
;       --Res;
;
;     for (int J = 0; J < I; ++J)
;       Res += J - TripCount;
;   }
;   return Res;
; }

; FEATURE_DUMP:[
; FEATURE_DUMP:  {
; FEATURE_DUMP:      "SrcBBFeatures": {
; FEATURE_DUMP:        "srcBranchPredicate": 40,
; FEATURE_DUMP:        "srcBranchOperandOpcode": 53,
; FEATURE_DUMP:        "srcBranchOperandFunc": 53,
; FEATURE_DUMP:        "srcBranchOperandType": 9,
; FEATURE_DUMP:        "srcRAOpCode": 55,
; FEATURE_DUMP:        "srcRAFunc": 55,
; FEATURE_DUMP:        "srcRAType": 12,
; FEATURE_DUMP:        "srcRBOpCode": 32,
; FEATURE_DUMP:        "srcRBFunc": 77,
; FEATURE_DUMP:        "srcRBType": 12,
; FEATURE_DUMP:        "srcLoopHeader": 1,
; FEATURE_DUMP:        "srcProcedureType": 0,
; FEATURE_DUMP:        "srcLoopDepth": 1,
; FEATURE_DUMP:        "srcLoopBlockSize": 10,
; FEATURE_DUMP:        "srcTotalSubLoopSize": 1,
; FEATURE_DUMP:        "srcTotalSubLoopBlockSize": 3,
; FEATURE_DUMP:        "srcLoopExitingSize": 1,
; FEATURE_DUMP:        "srcLoopExitSize": 1,
; FEATURE_DUMP:        "srcLoopExitEdgesSize": 1,
; FEATURE_DUMP:        "srcTriangle": 0,
; FEATURE_DUMP:        "srcDiamond": 0,
; FEATURE_DUMP:        "srcFunctionStartWithRet": 0,
; FEATURE_DUMP:        "srcFunctionInstructionSize": 29,
; FEATURE_DUMP:        "srcFunctionBlockSize": 12,
; FEATURE_DUMP:        "srcFunctionEdgesSize": 14,
; FEATURE_DUMP:        "srcNumberOfSuccessors": 2
; FEATURE_DUMP:      },
; FEATURE_DUMP:      "SuccBBFeatures_0": {
; FEATURE_DUMP:        "SuccessorsRank": 0,
; FEATURE_DUMP:        "SuccessorBranchDirection": 0,
; FEATURE_DUMP:        "SuccessorLoopHeader": 0,
; FEATURE_DUMP:        "SuccesorLoopBack": 0,
; FEATURE_DUMP:        "SuccessorExitEdge": 2,
; FEATURE_DUMP:        "SuccessorsCall": 0,
; FEATURE_DUMP:        "SuccessorsEnd": 2,
; FEATURE_DUMP:        "SuccessorsUseDef": 0,
; FEATURE_DUMP:        "SuccessorBranchDominate": 1,
; FEATURE_DUMP:        "SuccessorsBranchPostDominate": 0,
; FEATURE_DUMP:        "SuccessorUnlikely": 0,
; FEATURE_DUMP:        "SuccessorNumberOfSiblingExitSuccessors": 1,
; FEATURE_DUMP:        "SuccessorEstimatedWeight": 3,
; FEATURE_DUMP:        "SuccessorTotalWeight": 6,
; FEATURE_DUMP:        "SuccessorInstructionSize": 3,
; FEATURE_DUMP:        "SuccessorStore": 0,
; FEATURE_DUMP:        "SuccessorLLVMHeuristicProb": 2080374784,
; FEATURE_DUMP:        "SuccessorPGOProb": 2114445438
; FEATURE_DUMP:      },
; FEATURE_DUMP:      "SuccBBFeatures_1": {
; FEATURE_DUMP:        "SuccessorsRank": 1,
; FEATURE_DUMP:        "SuccessorBranchDirection": 0,
; FEATURE_DUMP:        "SuccessorLoopHeader": 0,
; FEATURE_DUMP:        "SuccesorLoopBack": 0,
; FEATURE_DUMP:        "SuccessorExitEdge": 1,
; FEATURE_DUMP:        "SuccessorsCall": 0,
; FEATURE_DUMP:        "SuccessorsEnd": 8,
; FEATURE_DUMP:        "SuccessorsUseDef": 0,
; FEATURE_DUMP:        "SuccessorBranchDominate": 1,
; FEATURE_DUMP:        "SuccessorsBranchPostDominate": 1,
; FEATURE_DUMP:        "SuccessorUnlikely": 0,
; FEATURE_DUMP:        "SuccessorNumberOfSiblingExitSuccessors": 0,
; FEATURE_DUMP:        "SuccessorEstimatedWeight": 3,
; FEATURE_DUMP:        "SuccessorTotalWeight": 6,
; FEATURE_DUMP:        "SuccessorInstructionSize": 1,
; FEATURE_DUMP:        "SuccessorStore": 0,
; FEATURE_DUMP:        "SuccessorLLVMHeuristicProb": 67108864,
; FEATURE_DUMP:        "SuccessorPGOProb": 33038210
; FEATURE_DUMP:      }
; FEATURE_DUMP:    },
; FEATURE_DUMP:      "SrcBBFeatures": {
; FEATURE_DUMP:        "srcBranchPredicate": 38,
; FEATURE_DUMP:        "srcBranchOperandOpcode": 53,
; FEATURE_DUMP:        "srcBranchOperandFunc": 53,
; FEATURE_DUMP:        "srcBranchOperandType": 9,
; FEATURE_DUMP:        "srcRAOpCode": 17,
; FEATURE_DUMP:        "srcRAFunc": 17,
; FEATURE_DUMP:        "srcRAType": 12,
; FEATURE_DUMP:        "srcRBOpCode": 69,
; FEATURE_DUMP:        "srcRBFunc": 71,
; FEATURE_DUMP:        "srcRBType": 12,
; FEATURE_DUMP:        "srcLoopHeader": 0,
; FEATURE_DUMP:        "srcProcedureType": 0,
; FEATURE_DUMP:        "srcLoopDepth": 1,
; FEATURE_DUMP:        "srcLoopBlockSize": 10,
; FEATURE_DUMP:        "srcTotalSubLoopSize": 1,
; FEATURE_DUMP:        "srcTotalSubLoopBlockSize": 3,
; FEATURE_DUMP:        "srcLoopExitingSize": 1,
; FEATURE_DUMP:        "srcLoopExitSize": 1,
; FEATURE_DUMP:        "srcLoopExitEdgesSize": 1,
; FEATURE_DUMP:        "srcTriangle": 0,
; FEATURE_DUMP:        "srcDiamond": 1,
; FEATURE_DUMP:        "srcFunctionStartWithRet": 0,
; FEATURE_DUMP:        "srcFunctionInstructionSize": 29,
; FEATURE_DUMP:        "srcFunctionBlockSize": 12,
; FEATURE_DUMP:        "srcFunctionEdgesSize": 14,
; FEATURE_DUMP:        "srcNumberOfSuccessors": 2,
; FEATURE_DUMP:      },
; FEATURE_DUMP:      "SuccBBFeatures_0": {
; FEATURE_DUMP:        "SuccessorsRank": 0,
; FEATURE_DUMP:        "SuccessorBranchDirection": 0,
; FEATURE_DUMP:        "SuccessorLoopHeader": 0,
; FEATURE_DUMP:        "SuccesorLoopBack": 0,
; FEATURE_DUMP:        "SuccessorExitEdge": 0,
; FEATURE_DUMP:        "SuccessorsCall": 0,
; FEATURE_DUMP:        "SuccessorsEnd": 5,
; FEATURE_DUMP:        "SuccessorsUseDef": 0,
; FEATURE_DUMP:        "SuccessorBranchDominate": 1,
; FEATURE_DUMP:        "SuccessorsBranchPostDominate": 0,
; FEATURE_DUMP:        "SuccessorUnlikely": 0,
; FEATURE_DUMP:        "SuccessorNumberOfSiblingExitSuccessors": 0,
; FEATURE_DUMP:        "SuccessorEstimatedWeight": 3,
; FEATURE_DUMP:        "SuccessorTotalWeight": 6,
; FEATURE_DUMP:        "SuccessorInstructionSize": 2,
; FEATURE_DUMP:        "SuccessorStore": 0,
; FEATURE_DUMP:        "SuccessorLLVMHeuristicProb": 1073741824,
; FEATURE_DUMP:        "SuccessorPGOProb": 0
; FEATURE_DUMP:      },
; FEATURE_DUMP:      "SuccBBFeatures_1": {
; FEATURE_DUMP:        "SuccessorsRank": 1,
; FEATURE_DUMP:        "SuccessorBranchDirection": 0,
; FEATURE_DUMP:        "SuccessorLoopHeader": 0,
; FEATURE_DUMP:        "SuccesorLoopBack": 0,
; FEATURE_DUMP:        "SuccessorExitEdge": 0,
; FEATURE_DUMP:        "SuccessorsCall": 0,
; FEATURE_DUMP:        "SuccessorsEnd": 1,
; FEATURE_DUMP:        "SuccessorsUseDef": 0,
; FEATURE_DUMP:        "SuccessorBranchDominate": 1,
; FEATURE_DUMP:        "SuccessorsBranchPostDominate": 0,
; FEATURE_DUMP:        "SuccessorUnlikely": 0,
; FEATURE_DUMP:        "SuccessorNumberOfSiblingExitSuccessors": 0,
; FEATURE_DUMP:        "SuccessorEstimatedWeight": 3,
; FEATURE_DUMP:        "SuccessorTotalWeight": 6,
; FEATURE_DUMP:        "SuccessorInstructionSize": 2,
; FEATURE_DUMP:        "SuccessorStore": 0,
; FEATURE_DUMP:        "SuccessorLLVMHeuristicProb": 1073741824,
; FEATURE_DUMP:        "SuccessorPGOProb": 2147483648
; FEATURE_DUMP:      }
; FEATURE_DUMP:    },
; FEATURE_DUMP:      "SrcBBFeatures": {
; FEATURE_DUMP:        "srcBranchPredicate": 40,
; FEATURE_DUMP:        "srcBranchOperandOpcode": 53,
; FEATURE_DUMP:        "srcBranchOperandFunc": 53,
; FEATURE_DUMP:        "srcBranchOperandType": 9,
; FEATURE_DUMP:        "srcRAOpCode": 55,
; FEATURE_DUMP:        "srcRAFunc": 55,
; FEATURE_DUMP:        "srcRAType": 12,
; FEATURE_DUMP:        "srcRBOpCode": 55,
; FEATURE_DUMP:        "srcRBFunc": 55,
; FEATURE_DUMP:        "srcRBType": 12,
; FEATURE_DUMP:        "srcLoopHeader": 1,
; FEATURE_DUMP:        "srcProcedureType": 0,
; FEATURE_DUMP:        "srcLoopDepth": 2,
; FEATURE_DUMP:        "srcLoopBlockSize": 3,
; FEATURE_DUMP:        "srcTotalSubLoopSize": 0,
; FEATURE_DUMP:        "srcTotalSubLoopBlockSize": 0,
; FEATURE_DUMP:        "srcLoopExitingSize": 1,
; FEATURE_DUMP:        "srcLoopExitSize": 1,
; FEATURE_DUMP:        "srcLoopExitEdgesSize": 1,
; FEATURE_DUMP:        "srcTriangle": 0,
; FEATURE_DUMP:        "srcDiamond": 0,
; FEATURE_DUMP:        "srcFunctionStartWithRet": 0,
; FEATURE_DUMP:        "srcFunctionInstructionSize": 29,
; FEATURE_DUMP:        "srcFunctionBlockSize": 12,
; FEATURE_DUMP:        "srcFunctionEdgesSize": 14,
; FEATURE_DUMP:        "srcNumberOfSuccessors": 2
; FEATURE_DUMP:      },
; FEATURE_DUMP:      "SuccBBFeatures_0": {
; FEATURE_DUMP:        "SuccessorsRank": 0,
; FEATURE_DUMP:        "SuccessorBranchDirection": 0,
; FEATURE_DUMP:        "SuccessorLoopHeader": 0,
; FEATURE_DUMP:        "SuccesorLoopBack": 0,
; FEATURE_DUMP:        "SuccessorExitEdge": 2,
; FEATURE_DUMP:        "SuccessorsCall": 0,
; FEATURE_DUMP:        "SuccessorsEnd": 1,
; FEATURE_DUMP:        "SuccessorsUseDef": 1,
; FEATURE_DUMP:        "SuccessorBranchDominate": 1,
; FEATURE_DUMP:        "SuccessorsBranchPostDominate": 0,
; FEATURE_DUMP:        "SuccessorUnlikely": 0,
; FEATURE_DUMP:        "SuccessorNumberOfSiblingExitSuccessors": 1,
; FEATURE_DUMP:        "SuccessorEstimatedWeight": 3,
; FEATURE_DUMP:        "SuccessorTotalWeight": 6,
; FEATURE_DUMP:        "SuccessorInstructionSize": 4,
; FEATURE_DUMP:        "SuccessorStore": 0,
; FEATURE_DUMP:        "SuccessorLLVMHeuristicProb": 2080374784,
; FEATURE_DUMP:        "SuccessorPGOProb": 2081407228
; FEATURE_DUMP:      },
; FEATURE_DUMP:      "SuccBBFeatures_1": {
; FEATURE_DUMP:        "SuccessorsRank": 1,
; FEATURE_DUMP:        "SuccessorBranchDirection": 0,
; FEATURE_DUMP:        "SuccessorLoopHeader": 0,
; FEATURE_DUMP:        "SuccesorLoopBack": 0,
; FEATURE_DUMP:        "SuccessorExitEdge": 1,
; FEATURE_DUMP:        "SuccessorsCall": 0,
; FEATURE_DUMP:        "SuccessorsEnd": 1,
; FEATURE_DUMP:        "SuccessorsUseDef": 0,
; FEATURE_DUMP:        "SuccessorBranchDominate": 1,
; FEATURE_DUMP:        "SuccessorsBranchPostDominate": 1,
; FEATURE_DUMP:        "SuccessorUnlikely": 0,
; FEATURE_DUMP:        "SuccessorNumberOfSiblingExitSuccessors": 0,
; FEATURE_DUMP:        "SuccessorEstimatedWeight": 3,
; FEATURE_DUMP:        "SuccessorTotalWeight": 6,
; FEATURE_DUMP:        "SuccessorInstructionSize": 1,
; FEATURE_DUMP:        "SuccessorStore": 0,
; FEATURE_DUMP:        "SuccessorLLVMHeuristicProb": 67108864,
; FEATURE_DUMP:        "SuccessorPGOProb": 66076420
; FEATURE_DUMP:      }
; FEATURE_DUMP:    }
; FEATURE_DUMP:  }
; FEATURE_DUMP:] 

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@TripCount = dso_local global i32 64, align 4

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %ARGC, ptr noundef %ARGV) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc6, %entry
  %Res.0 = phi i32 [ 0, %entry ], [ %Res.2, %for.inc6 ]
  %I.0 = phi i32 [ 0, %entry ], [ %inc7, %for.inc6 ]
  %0 = load i32, ptr @TripCount, align 4
  %cmp = icmp slt i32 %I.0, %0
  br i1 %cmp, label %for.body, label %for.end8

for.body:                                         ; preds = %for.cond
  %mul = mul nsw i32 %ARGC, %Res.0
  %cmp1 = icmp sgt i32 %mul, 2
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %add = add nsw i32 %Res.0, %ARGC
  br label %if.end

if.else:                                          ; preds = %for.body
  %dec = add nsw i32 %Res.0, -1
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %Res.1 = phi i32 [ %add, %if.then ], [ %dec, %if.else ]
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %if.end
  %Res.2 = phi i32 [ %Res.1, %if.end ], [ %add5, %for.inc ]
  %J.0 = phi i32 [ 0, %if.end ], [ %inc, %for.inc ]
  %cmp3 = icmp slt i32 %J.0, %I.0
  br i1 %cmp3, label %for.body4, label %for.end

for.body4:                                        ; preds = %for.cond2
  %1 = load i32, ptr @TripCount, align 4
  %sub = sub nsw i32 %J.0, %1
  %add5 = add nsw i32 %Res.2, %sub
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %inc = add nsw i32 %J.0, 1
  br label %for.cond2, !llvm.loop !4

for.end:                                          ; preds = %for.cond2
  br label %for.inc6

for.inc6:                                         ; preds = %for.end
  %inc7 = add nsw i32 %I.0, 1
  br label %for.cond, !llvm.loop !6

for.end8:                                         ; preds = %for.cond
  ret i32 %Res.0
}

attributes #0 = { mustprogress noinline norecurse nounwind uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
!6 = distinct !{!6, !5}

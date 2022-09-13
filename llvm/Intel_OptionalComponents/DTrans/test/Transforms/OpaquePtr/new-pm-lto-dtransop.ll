; Test DTrans integration in the new pass manager.
;
; This test is to verify that the DTrans opaque pointer pipeline
; passes are run.
;
; Ideally, we'd like to have this test integrated with the main test for the
; new pass manager pipeline (test/Other/new-pm-lto-default.ll) but in order
; to exclude the use of the dtrans option for builds that exclude dtrans, the
; test must be separate.

; RUN: opt -disable-verify -debug-pass-manager -whole-program-assume    \
; RUN:     -passes='lto<O2>' -internalize-public-api-list main          \
; RUN:     -S  %s -enable-npm-dtrans -dtransop-allow-typed-pointers     \
; RUN:     2>&1                                                         \
; RUN:     | FileCheck %s

; Basic orientation checks.
; CHECK: Running pass: Annotation2MetadataPass
; CHECK-NEXT: Running pass: CrossDSOCFIPass
; CHECK-NEXT: Running pass: InlineReportSetupPass
; CHECK-NEXT: Running pass: XmainOptLevelAnalysisInit
; CHECK-NEXT: Running analysis: XmainOptLevelAnalysis
; CHECK-NEXT: Running pass: InternalizePass
; CHECK: Running analysis: WholeProgramAnalysis
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*}}Module{{.*}}>
; CHECK-NEXT: Running analysis: TargetLibraryAnalysis on bar
; CHECK-NEXT: Running analysis: TargetIRAnalysis on foo
; CHECK-NEXT: Running analysis: TargetIRAnalysis on main
; CHECK-NEXT: Running pass: OpenMPOptPass
; CHECK-NEXT: Running pass: GlobalDCEPass
; CHECK-NEXT: Running pass: IntelIPOPrefetchPass
; CHECK-NEXT: Running pass: IntelFoldWPIntrinsicPass
; CHECK: Running pass: ForceFunctionAttrsPass
; CHECK-NEXT: Running pass: InferFunctionAttrsPass
; CHECK: Running pass: OptimizeDynamicCastsPass
; CHECK: Running pass: {{.*}}SimplifyCFGPass{{.*}}
; CHECK-NEXT: Running pass: {{.*}}SimplifyCFGPass{{.*}}
; CHECK-NEXT: Running pass: GlobalSplitPass
; CHECK-NEXT: Running pass: WholeProgramDevirtPass

; Verify the DTrans opaque pointer passes get run in the proper order

; CHECK: Running pass: dtransOP::RemoveDeadDTransTypeMetadataPass on [module]
; CHECK-NEXT: Running pass: dtransOP::DTransNormalizeOPPass
; CHECK-NEXT: Running pass: dtransOP::CommuteCondOPPass
; CHECK-NEXT: Running analysis: dtransOP::DTransSafetyAnalyzer
; CHECK-NEXT: Running pass: dtransOP::MemInitTrimDownOPPass
; CHECK-NEXT: Running pass: dtransOP::SOAToAOSOPPreparePass
; CHECK-NEXT: Running pass: dtransOP::MemManageTransOPPass
; CHECK-NEXT: Running pass: dtransOP::CodeAlignPass
; CHECK-NEXT: Running pass: dtransOP::DeleteFieldOPPass
; CHECK-NEXT: Running pass: dtransOP::ReorderFieldsOPPass
; CHECK-NEXT: Running pass: dtransOP::AOSToSOAOPPass
; CHECK-NEXT: Running pass: dtransOP::ReuseFieldOPPass
; CHECK-NEXT: Running pass: dtransOP::DeleteFieldOPPass
; CHECK-NEXT: Running pass: dtrans::EliminateROFieldAccessPass
; CHECK-NEXT: Running pass: dtransOP::DynClonePass on
; CHECK-NEXT: Running pass: dtrans::AnnotatorCleanerPass

; CHECK: Running pass: DopeVectorConstProp
; CHECK: Running pass: ArgumentPromotionPass on (foo)
; CHECK: Running pass: ArgumentPromotionPass on (main)

; Make sure we get the IR back out without changes when we print the module.
; CHECK-LABEL: define internal fastcc void @foo(i32 %n) unnamed_addr #0 {
; CHECK-NEXT: entry:
; CHECK-NEXT:   br label %loop
; CHECK:      loop:
; CHECK-NEXT:   %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
; CHECK-NEXT:   %iv.next = add i32 %iv, 1
; CHECK-NEXT:   tail call void @bar()
; CHECK-NEXT:   %cmp = icmp eq i32 %iv, %n
; CHECK-NEXT:   br i1 %cmp, label %exit, label %loop
; CHECK:      exit:
; CHECK-NEXT:   ret void
; CHECK-NEXT: }
;
; CHECK-LABEL: define i32 @main(i32 %n) local_unnamed_addr #1 {
; CHECK-NEXT:    call fastcc void @foo(i32 %n)
; CHECK-NEXT:    ret i32 0
; CHECK-NEXT:  }
;
; CHECK-LABEL: attributes #0 = { noinline norecurse uwtable }
; CHECK-LABEL: attributes #1 = { norecurse }

declare void @bar() local_unnamed_addr

; Function Attrs: noinline uwtable
define void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  br label %loop
loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
  %iv.next = add i32 %iv, 1
  tail call void @bar()
  %cmp = icmp eq i32 %iv, %n
  br i1 %cmp, label %exit, label %loop
exit:
  ret void
}

define i32 @main(i32 %n) {
  call void @foo(i32 %n)
  ret i32 0
}

attributes #0 = { noinline uwtable }

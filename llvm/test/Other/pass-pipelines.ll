; Test the particular pass pipelines have the expected structure. This is
; particularly important in order to check that the implicit scheduling of the
; legacy pass manager doesn't introduce unexpected structural changes in the
; pass pipeline.
;
; INTEL - Disabled Intel Andersen's Alias Analysis and loopopt so as to not
; INTEL - break the pass pipeline this is trying to check for.
; INTEL - Disabled svml translation pass to prevent this test from breaking
; INTEL - Enabled loop vectorizer to prevent this test from breaking
; INTEL_CUSTOMIZATION
; INTEL_FEATURE_CSA
; INTEL - Disable test for CSA compiler due to extra passes in pipeline.
; UNSUPPORTED: csa-registered-target
; end INTEL_FEATURE_CSA
; end INTEL_CUSTOMIZATION
;
; RUN: opt -disable-output -disable-verify -debug-pass=Structure \
; INTEL CUSTOMIZATION
; RUN:     -enable-andersen=false -loopopt=0 -O2 -enable-lv %s 2>&1 \
; END INTEL CUSTOMIZATION
; RUN:     | FileCheck %s --check-prefix=CHECK-O2
; RUN: llvm-profdata merge %S/Inputs/pass-pipelines.proftext -o %t.profdata
; RUN: opt -disable-output -disable-verify -debug-pass=Structure \
; RUN:     -pgo-kind=pgo-instr-use-pipeline -profile-file='%t.profdata' \
; INTEL CUSTOMIZATION
; RUN:     -enable-andersen=false -loopopt=0 -O2 -enable-lv %s 2>&1 \
; END INTEL CUSTOMIZATION
; RUN:     | FileCheck %s --check-prefix=CHECK-O2 --check-prefix=PGOUSE
; RUN: opt -disable-output -disable-verify -debug-pass=Structure \
; RUN:     -pgo-kind=pgo-instr-use-pipeline -profile-file='%t.profdata' \
; RUN:     -hot-cold-split \
; INTEL CUSTOMIZATION
; RUN:     -enable-andersen=false -loopopt=0 -O2 -enable-lv %s 2>&1 \
; END INTEL CUSTOMIZATION
; RUN:     | FileCheck %s --check-prefix=CHECK-O2 --check-prefix=PGOUSE --check-prefix=SPLIT
;
; In the first pipeline there should just be a function pass manager, no other
; pass managers.
; CHECK-O2: Pass Arguments:
; CHECK-O2-NOT: Manager
; CHECK-O2: FunctionPass Manager
; CHECK-O2-NOT: Manager
;
; CHECK-O2: Pass Arguments:
; CHECK-O2: ModulePass Manager
; CHECK-O2-NOT: Manager
; First function pass pipeline just does early opts.
; CHECK-O2-COUNT-3: FunctionPass Manager
; CHECK-O2-NOT: Manager
; FIXME: It's a bit odd to do dead arg elim in the middle of early opts...
; CHECK-O2: Dead Argument Elimination
; CHECK-O2-NEXT: FunctionPass Manager
; CHECK-O2-NOT: Manager
; Very carefully assert the CGSCC pass pipeline as it is fragile and unusually
; susceptible to phase ordering issues.
; CHECK-O2: CallGraph Construction
; PGOUSE: Call Graph SCC Pass Manager
; PGOUSE:      Function Integration/Inlining
; PGOUSE: PGOInstrumentationUsePass
; PGOUSE: PGOIndirectCallPromotion
; PGOUSE: CallGraph Construction
; CHECK-O2-NEXT: Globals Alias Analysis
; INTEL -- Added two passes: setting inline report and [no]inline list attributes
; CHECK-O2-NEXT: Setup inlining report
; CHECK-O2-NEXT: Set attributes for callsites in [no]inline list
; CHECK-O2-NEXT: Call Graph SCC Pass Manager
; CHECK-O2-NEXT: Remove unused exception handling info
; CHECK-O2-NEXT: Function Integration/Inlining
; CHECK-O2-NEXT: Deduce and propagate attributes (CGSCC pass)
; CHECK-O2-NEXT: OpenMP specific optimizations
; CHECK-O2-NEXT: Deduce function attributes
; Next up is the main function pass pipeline. It shouldn't be split up and
; should contain the main loop pass pipeline as well.
; CHECK-O2-NEXT: FunctionPass Manager
; CHECK-O2-NOT: Manager
; CHECK-O2: Loop Pass Manager
; CHECK-O2-NOT: Manager
; FIXME: We shouldn't be pulling out to simplify-cfg and instcombine and
; causing new loop pass managers.
; CHECK-O2: Simplify the CFG
; CHECK-O2-NOT: Manager
; CHECK-O2: Combine redundant instructions
; CHECK-O2-NOT: Manager
; CHECK-O2: Loop Pass Manager
; CHECK-O2-NOT: Manager
; FIXME: It isn't clear that we need yet another loop pass pipeline
; and run of LICM here.
; CHECK-O2-NOT: Manager
; CHECK-O2: Loop Pass Manager
; CHECK-O2-NEXT: Loop Invariant Code Motion
; CHECK-O2-NOT: Manager
; Next we break out of the main Function passes inside the CGSCC pipeline with
; a barrier pass.
; CHECK-O2: A No-Op Barrier Pass
; INTEL -- StdContainerOpt and CleanupFakeLoads insert a function pass manager
; CHECK-O2: FunctionPass Manager
; CHECK-O2: StdContainerOpt
; CHECK-O2: Cleanup fake loads 
; End INTEL
; CHECK-O2-NEXT: Eliminate Available Externally
; Inferring function attribute should be right after the CGSCC pipeline, before
; any other optimizations/analyses.
; CHECK-O2-NEXT: CallGraph
; CHECK-O2-NEXT: Deduce function attributes in RPO
; CHECK-O2-NOT: Manager
; Reduce the size of the IR ASAP after the inliner.
; CHECK-O2-NEXT: Global Variable Optimizer
; CHECK-O2: Dead Global Elimination
; Next is the late function pass pipeline.
; CHECK-O2: FunctionPass Manager
; CHECK-O2-NOT: Manager
; We rotate loops prior to vectorization.
; CHECK-O2: Loop Pass Manager
; CHECK-O2-NEXT: Rotate Loops
; CHECK-O2-NOT: Manager
; CHECK-O2: Loop Vectorization
; CHECK-O2-NOT: Manager
; CHECK-O2: SLP Vectorizer
; CHECK-O2-NOT: Manager
; After vectorization we do partial unrolling.
; CHECK-O2: Loop Pass Manager
; CHECK-O2-NEXT: Unroll loops
; CHECK-O2-NOT: Manager
; After vectorization and unrolling we try to do any cleanup of inserted code,
; including a run of LICM. This shouldn't run in the same loop pass manager as
; the runtime unrolling though.
; CHECK-O2: Loop Pass Manager
; CHECK-O2-NEXT: Loop Invariant Code Motion
; SPLIT: Hot Cold Splitting
; CHECK-O2: FunctionPass Manager
; CHECK-O2: Loop Pass Manager
; CHECK-O2-NEXT: Loop Sink
; CHECK-O2: Simplify the CFG
; CHECK-O2-NOT: Manager
;
; FIXME: There really shouldn't be another pass manager, especially one that
; just builds the domtree. It doesn't even run the verifier.
; CHECK-O2: Pass Arguments:
; CHECK-O2: FunctionPass Manager
; CHECK-O2-NEXT: Dominator Tree Construction

define void @foo() {
  ret void
}

.. toctree::
   :hidden:

   OpenSourceDecisionFlow

==========================================
CPU2017 Open Source Performance Guidelines
==========================================

**Intel Proprietary Compilers Goals**

- Continue to produce industry leading compilers that demonstrate best
  performance on Intel and AMD processors, compared to all relevant compilers.

   - Currently ICC, soon to be ICX.
   - Leadership on Intel Processors should be > 10% over best alternative
     compilers.
   - Leadership on AMD processors should be 1-5%, or more, depending on H/W
     competitive standing. Ultimate goal is to be even to 1% ahead and never
     behind. This means we may need to overshoot a little in anticipation of
     competitive gains.

**Open Source Goals**

- Provide the best performance to LLVM Open Source on Intel Architectures
  without compromising our competitive standpoint.

- Encourage CSPs to shift from GCC to LLVM.

   - For two main reasons:

      - Since we're investing most of our resources into LLVM, this is how
        we will best impact CSP performance on Intel architectures.
      - Once switched to LLVM, we will have a better chance of convincing
        CSPs to switch to our own LLVM based proprietary compiler.

   - This requires demonstrating leadership in open source LLVM performance
     over GCC.

      - On at least CPU2017

         - Estimated lead over GCC will be CSP specific, but should be at least
           ~1% or more.

         - Today, LLVM and GCC are fairly close.
         - Implement simple SPEC improvements in LLVM that already exist in AOCC
           (preferred).
         - Optimizations already in GCC may be considered for open sourcing if:
            - They are simple extensions to existing frameworks.
            - They don’t give AMD more performance than they do Intel.
         - When goals have been reached, additional improvements may be made
           based on strategic decisions per CSP
         - We don’t want to put all of our optimizations into LLVM because:
            - AMD and ARM will realize gains that will reduce our HW competitive
              advantage.
            - We will lose our ability to differentiate.
	 - For additional information, please refer to this flow diagram:
	   :doc:`Open Source Decision Flow <OpenSourceDecisionFlow>`

      - On other benchmarks which will be identified per CSP (ongoing).
         - SPEC2006 may also be relevant here, as many CSPs have yet to adopt
           CPU2017.

**Ultimate Open Source Goals**

- Encourage CSPs to shift from LLVM open source to ICX Proprietary Intel
  Compiler.

   - The criteria for this will vary between the CSPs, if possible at all.

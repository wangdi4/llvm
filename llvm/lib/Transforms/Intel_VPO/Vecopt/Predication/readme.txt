
The predicator is made of two main components:
    1. The PhiCanon class. This class simply implements the phi canonicalization pass. After this pass
        each PHINode has only two entries.

    3. The predicator itself. The predicator has 4 major steps:
        A. Predication:
           1. Preperation
           2. Mask placement
           3. Select insertion
           4. Linearization - Scheduling based on loop and specialization constraints

        B. Specialization:
            Collect dominance information and scheduling constraints (before predication)
            Perform specialization (after predication)

    The entire process is documented in a presentation with pictures, charts and text. See shared-point.

    Each of the steps is documented separately in the code. The main driver of the pass is called from
    the runOnFunction call. This is a good place to start looking at this pass.

The predicator is based on the following works:

1. Automatic Packetization / Ralf Karrenberg
    URL: http://www.prog.uni-saarland.de/people/karrenberg/content/karrenberg_automatic_packetization.pdf

    Description: This work describes automatic vectorization of data parallel works. We base our vectorizer
      implementation on this work. We implement the predication of arbitrart control flow using the methods
      described in this paper. However, this work is not optimal in several casese. We address these cases
      by implementing the optimizations described in the papers below.

2. Introducing Control Flow into Vectorized Code / Jaewook Shin
    URL: http://portal.acm.org/citation.cfm?id=1299055

    Description: Both this work and the following work describe introduction of control flow after the code
      had been predicated. In the work Karrenberg, the resulting code is predicated, but not optimized
      for cases where all of the branches of the program agree. These works address this problem by introducing
      control flow.

3. Superword-Level Parallelism in the Presence of Control Flow / Jacqueline Chame et al.
    URL: http://portal.acm.org/citation.cfm?id=1048983

    Description: See above.

4. Analysis of Predicated Code / Johnson, Richard et al.
   URL: http://www.hpl.hp.com/techreports/96/HPL-96-119.html

   Description: For insertion of control-flow (which comes from predicated code), may be unoptimized
     if selected arbitrarily due to the fact that certain ifs may contain other ifs. In order to address
     this issue PQ analysis is often used. This classic paper describes the analysis of predicated code
     and the generation of a query system which allows us to detect if two conditions always negate, always agree, etc.

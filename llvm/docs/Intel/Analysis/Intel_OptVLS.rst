===========================================
OptVLS: Optimization of Vector Loads/Stores
===========================================

.. contents::
   :local:

Introduction
============

Since the latency of a gather/scatter is typically longer than that of a contiguous load/store, the compiler
attempts to perform an optimization, that:
- Replaces a set of adjacent gathers/scatters by a set of contiguous loads/stores followed by a sequence of
register-to-register re-arrangement instructions

Example-1 - Here is an example of this optimization:

.. code-block::  c++

  double x[1000];
  for(int k = 0; k < n; k++) {
    double p = x[2 * k];
    double q = x[2 * k + 1];
    ...
  }

The loads from multiple consecutive iterations are laid out in the linear memory space of
x such as: p1 q1 p2 q2 p3 q3 p4 q4 ..

A naive vectorization will generate 2 gathers for the two strided memory references. With vector length 4,
it will try to load p(p1, p2, p3, p4) using one gather, q(q1, q2, q3, q4) using another gather and so on.

Pseudo vector code:

.. code-block:: none

  for.body: // vector length is 4
    ...
    %gather1address = getelementptr <4 x double*> %i, <4 x i64> %strided_index
    %gather2address = getelementptr ..
    // two gathers are generated for two strided accesses x[2 * k], x[2 * k + 1]

    // gather-1 gathering p1, p2, p3, p4
    %p_v = call <4 x double> @llvm.masked.gather.v4f64(%gather1address, ..);

    // gather-2 gathering q1, q2, q3, q4
    %q_v = call <4 x double> @llvm.masked.gather.v4f64(%gather2address, ..);

    ...

As we can see adjacent gathers/scatters access a piece-wise contiguous range of memory, we can issue a contiguous
load to load each contiguous chunk and finally in order to produce the gather output we can shuffle the loaded
value. Even though we execute more instructions this way, the total execution time of these instructions is usualy
less than the total execution time of gathers/scatters. Therefore, this optimization is generally profitable.

Optimized pseudo vector code:

.. code-block::  none

  for.body: // vector length is 4
    ...
    // load p1 q1 p2 q2
    %1 = @llvm.masked.load.v4f64(&x[i], ..);

    // load p3 q3 p4 q4
    %2 = @llvm.masked.load.v4f64(&x[i+4], ..);

    // move p1 q1 p3 q3
    %i1_v = shufflevector <4 x f64> %1, <4 x f64> %2, <4 x i32> <i32 0, i32 1, i32 4, i32 5>;

    // move p2 q2 p4 q4
    %i2_v = shufflevector <4 x f64> %1, <4 x f64> %2, <4 x i32> <i32 2, i32 3, i32 6, i32 7>;

    // move p1 p2 p3 p4
    %p_v = shufflevector <4 x f64> %i1_v, <4 x f64> %i2_v, <4 x i32> <i32 0, i32 4, i32 2, i32 6>;

    // move q1 q2 q3 q4
    %q_v = shufflevector <4 x f64> %i1_v, <4 x f64> %i2_v, <4 x i32> <i32 1, i32 5, i32 3, i32 7>;

    ...

OptVLS is designed to follow the client-server model. The server is responsible for doing the optimization on a set
of abstract memrefs and providing a set of abstract optimized sequences. The client is responsible for providing
mechanisms to reason about these memrefs (for example, computing a distance between a pair of memrefs). One example
client is the OptVLS pass, which operates on (vector) gathers/scatters. Another example client is the vectorizer,
which operates on scalar loads/stores.  The server is responsible for doing the optimization and providing services
to its different clients (both scalar and vectorizer optimizer). It supports both vector and scalar memrefs seamlessly
(as long as they are abstracted as OVLSMemrefs).

This document focuses on the server part of the optimization and is broken down into the following 5
sections as follows:

...Section 1: OptVLS server core functionalities

...Section 2: Key design considerations

...Section 3: Documentation of the public interface that the server provided to the clients

...Section 4: How to use OptVLS in a compiler by writing an OptVLS client.

...Section 5: Important implementation details.

...Section 6: Overall scenerio of OptVLS server-client system


Section 1: OptVLS server core functionalities:
==============================================

#. Takes a set of abstracted memory references such as gathers/scatters and separates them into multiple
   groups where each group includes only adjacent gathers/scatters.

#. Estimates the relative cost/benefit of replacing the adjacent gathers/scatters in a group by a
   semantically equivalent set of contiguous loads/stores and re-arrangement instructions.

#. Generates the abstract optimized sequence for a group of adjacent gathers/scatters

Section 2: Key Design Considerations
====================================

In order to facilitate different clients, such as vectorizer client, or scalar (optimizer) client,
this optimization is implemented following a client/server model where the server is agnostic of the
IR used by the client and the client and server communicate using abstract data types. Consequently,
the optimization can be called from anywhere by simply implementing new clients, and little to no
changes should be required in the server.

Section 3: Public Interface
===========================

The OptVLS Core functionalities discussed in Section 1 are exposed through the public interfaces that operate on
abstract data types (discussed in Section 2).

These abstract data types are as follows:

#. OVLSAccessType- Allows representing different vector access type such as [stride|index][load|store].
#. OVLSType- Allows representing a vector type as <# elements> x <element-size in bits>
#. OVLSMemref- Allows representing a vector memory reference such as gather/scatter
#. OVLSGroup- Allows representing a group of OVLSMemrefs
#. OVLSLoad- Represents a load instruction
#. OVLSShuffle- Represents a shuffle instruction

The client_to_server interface that operates on the above abstract data types is defined
by the OptVLS client_to_server interface class which is described here:

OptVLS public interface class:

.. code-block::  c++

  class OptVLSInterface {
    static void getGroups(const OVLSMemrefVector &Memrefs,
                          uint32_t VectorLength,
                          OVLSGroupVector &Grps,
                          OVLSMemrefToGroupMap *MemrefToGroupMap = nullptr);

    static bool getSequence(const OVLSGroup& Group,
                            OVLSInstructionVector& InstVector);
  };

... A quick description of the public member functions is as follows:

getGroups(..) -  Takes a set of OVLSMemrefs and a vector length that is the maximum
allowed vector register size (in bytes) on the underlying architecture. Returns a set of OVLSGroups
where each group contains the OVLSMemrefs that are adjacent and a mapping from OVLSMemref
to an OVLSGroup.

getSequence(..) - In order to get an optimized instruction sequence for a set of adjacent memrefs(gather/scatter)
client needs to provide an OVLSGroup comprising the set of adjacent memrefs. getSequence() then returns
true and a vector containing the instruction sequence in InstVector. It returns false if it is unable to
generate an optimized sequence. The way to generate the OVLSGroup is to call the getGroups() with the set of
adjacent memrefs which would return a vector containing OVLSGroups. There might be more than one OVLSGroup for
the specified set of memrefs. In that case, getSequence() is supposed to be called for each of them in turn.
The sequence returned here is topologically-ordered where the producer instructions appear before the consumer
instructions. At this point, this sequence has been OptVLS-optimized, i.e. the sequence has been replaced multiple
gathers/scatters by a faster sequence of instructions that uses only loads/stores and register/register rearrangement
instructions. Each instruction in this OptVLS-optimized sequence is an abstract OVLSInstruction which needs to be
converted to an LLVM-IR instruction by the client.


Section 4: How to use OptVLS in a compiler by writing an OptVLS client
======================================================================

To start out, a client needs to include the Intel_OptVLS.h header file into its program.

Please note that, OptVLS does not take into account either program context nor any underlying
architecture. In order to access certain information regarding the program context or underlying
architecture such as alias-information or instruction costs on a particular architecture it needs
to call back to the client. That communication is done through a server_to_client call-back
interface which the client is expected to implement. This interface allows the server to request
the necessary information to complete its core functionalities. The code block below shows how the
server defines this interface but leaves it to the client to implement.

.. code-block::  c++

  class OVLSMemref {

  public:
    virtual bool isAConstDistanceFrom(const OVLSMemref& Memref, int64_t *Dist) = 0;

    virtual bool haveSameNumElements(const OVLSMemref& Memref) = 0;

    virtual bool canMoveTo(const OVLSMemref& Memref) = 0;

    virtual bool hasAConstStride(int64_t *Stride) = 0;

  }

... Here is quick description of the semantics of the callback functions that need to
... be implemented by the client:

  isAConstDistanceFrom()- queries whether two memrefs are a constant distance apart.

  haveSameNumElements()- queries whether two memrefs have same number of elements.

  canMoveTo()- FIXME: We are still discussing whether it's the server or the client is responsible
               for code placement, which will affect this interface.

  hasAConstStride()-returns true if a memref has a constant distance between its vector elements.

The code below shows how the client would extend the virtual class to implement these methods.

.. code-block::  c++

  // A code snippet of client header file.
  #include "llvm/Analysis/Intel_OptVLS.h"

  class ClientMemref : public OVLSMemref {
  public:
    bool isAConstDistanceFrom(const OVLSMemref& Memref, int64_t *Dist) {
       // Client implements this
    }
    bool haveSameNumElements(const OVLSMemref& Memref) {
      // client implements this
    }
    bool canMoveTo(const OVLSMemref& Memref) {
      // client implements this
    }
    bool hasAConstStride(int64_t *Stride) {
      // client implements this
    }
 }

The code below shows how the client can process each memref into OVLSMemref and push
it to the OVLSMemrefVector and finally call the getGroups() using the memref vector
and a vector length.

.. code-block::  c++

  // A code snippet of client.cpp
  OVLSMemrefVector Mrfs;
  for each memref {
    OVLSMemref mrf = new ClientMemref(..);
    Mrfs.push_back(mrf);
  }
  OVLSGroupVector Grps;
  OptVLSInterface::getGroups(Mrfs, Grps, 32 /*maximum vector size on HSW*/);

Section 5: Implementation Details
=================================

This section describes more details for each interface function and abstract type.

1. getGroups()
--------------

  a) The input vector length is the maximum allowed vector size in the underlying architecture.
     This determines how many adjacent memrefs can be put together in a group. In addition, it
     tells us how many memrefs can be processed at a time using a single vector register.

  b) Currently, grouping is done using a greedy algorithm. It sorts out the memrefs based
     on their distance from the base address. Then it keeps putting the memref starting at
     the lowest address until the group is full. Doing it this way, it's possible for a memref
     to be put in a group where it has a bigger distance between memrefs than if it were put
     in a different group which would have different performance implications.

     As an example that uses maximum vector length of 16:
       memref1- distance from base is 0 bytes

       memref2- distance from base is 4 bytes

       memref3- distance from base is 12 bytes

       memref4- distance from base is 16 bytes

       memref5- distance from base is 20 bytes

     The best grouping should be:
        Group1: memref1, memref2

        Group2: memref2, memref4, memref5

     Using current approach the groups we will get are:
        Group1: memref1, memref2, memref3

        Group2: memref4, memref5


  c) canMoveTo()- FIXME: We are still discussing whether it's the server or the client is responsible
                   for code placement, which will affect this interface.

2. getSequence()
----------------

  Optimized sequence generation for a group of gathers is split into two parts:

  a) Generate loads - This part is very straightforward, it generates loads to load each contiguous chunk
     of memory created by a group of adjacent gathers.

     For our example, the following two loads get generated

     %1 = mask.load.64.4 (<Base:0xf7ced0 Offset:0>, 1111)

     %2 = mask.load.64.4 (<Base:0xf7ced0 Offset:32>, 1111)

  b) Generate shuffles - The result of (a) is that the elements of each gather have been loaded but are distributed
     across multiple registers. In order to produce the actual gather-output, we need to move (/rearrange) all those
     distributed elements (of each gather) back to the single destination register where the gather is expected to
     have deposited them. To maximize speedup, the challenge is to generate efficient code for the rearrangement.

     genShuffles() uses a directed graph to automatically find an efficient sequence of rearrangement instructions.
     In this directed graph, an edge represents a move of a source bit-range, and a node can be thought of as the
     result of some logical rearrangement of those incoming bit-ranges/edges. An initial version of the graph gets
     drawn by the load-generator and is passed to the genShuffles() as an input. Initially, it only has nodes for
     the loaded data, and final gather results, and edges between loaded and gather results show which loaded
     elements contribute to which gather results. The total number of edges of a gather-node needs to match its total number
     of elements where each edge moves its element size of bits.

     This initial graph represents doing all rearrangement in 1 logic operation for each gather result.  In most cases,
     no single instruction exists that can do such logical operations. It is the responsibility of genShuffles() to
     expand the graph, breaking such complex logical operations into multiple simpler logical operations for which
     instructions exist. The rest of the content talks about how genShuffles() does this graph expansion that results
     in efficient and legal rearrangement instruction sequences.

     This is how the initial graph looks like coming out of the load-generator for the above example,
     load-nodes:{V3, V4}, gather-nodes{V1, V2}:

.. graphviz::

   digraph Initial_Graph {

      graph[ordering=in];

      V3 -> V1[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V3 -> V1[fontcolor=red, color=red, label="128:191",weight="128:191"];

      V4 -> V1[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V4 -> V1[fontcolor=red, color=red, label="128:191",weight="128:191"];

      V3 -> V2[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V3 -> V2[fontcolor=blue, color=blue, label="192:255",weight="192:255"];

      V4 -> V2[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V4 -> V2[fontcolor=blue, color=blue, label="192:255",weight="192:255"];

   }


...

     And, this is how it gets printed by OptVLS-server:

     Initial Graph:

       V3: Load

       V4: Load

       V1:
        [0:63] = V3[0:63]

        [64:127] = V3[128:191]

        [128:191] = V4[0:63]

        [192:255] = V4[128:191]

       V2:
        [0:63] = V3[64:127]

        [64:127] = V3[192:255]

        [128:191] = V4[64:127]

        [192:255] = V4[192:255]


     In the above graph, each gather-node has four incoming edges which matches its total number of elements,
     and each edge moves exactly 64 bits which is its element-size.
     Below shows the auxiliary data-structures that help building this graph:


.. code-block::  c++

  /// Represents a range of bits using a bit-location of the leftmost bit and
  /// a number of consecutive bits immediately to the right that are included
  /// in the range. {0, 0} means undefined bit-range.
  ///
  struct BitRange {
    uint32_t BIndex;
    uint32_t NumBits;
    ...
  };

  /// Edge represents a move of a specified bit-range 'BR' from 'Src' GraphNode.
  /// 'Src' can be nullptr, which means an undefined source. For an undefined
  /// source, BR still represents a valid bitrange. A bit-range with an undefined
  /// source is used to represent a gap in the destination GraphNode.
  ///
  struct Edge {
    GraphNode *Src;
    BitRange BR;
  };

  /// GraphNode can be thought of as a result of some logical instruction
  /// (mainly rearrangement instruction such as shift, shuffle, etc) on
  /// its ‘IncomingEdges’(/source bit-ranges). These ‘IncomingEdges’
  /// particularly show which source bit-range maps to which bit-index of this (which helps
  /// defining (/elaborates on) the logical instruction semantics). A ‘GraphNode’ basically
  /// allows us to define an expected behavior (/semantic) first which then evolves into a
  /// particular valid OVLSinstruction ‘Inst’ if there is any for that semantic.
  ...
  class GraphNode {
    /// Provides a unique id to each instruction node. It helps printing
    /// tracable node information.
    uint32_t Id;

    /// Initially when a GraphNode is created, Inst can be nullptr
    /// which means undefined instruction. An undefined instruction can
    /// still have valid IncomingEdges which would define the semantics of
    /// this logical instruction (GraphNode), helps specifying the actual
    /// instruction later.
    /// A GraphNode is also used for holding the result of a load/store
    /// instruction, in such case, Inst should point to a valid load/store
    /// instruction.
    OVLSInstruction *Inst;

    /// A ‘GraphNode’ is a result of some logical instruction on its incoming edges where ‘IncomingEdges’
    /// contains that result. The output value of the GraphNode is the concatenation of the source bit-ranges
    /// which shows which source bit-range maps to which bit index of this node. Depending on the order of the edges
    /// (in IncomingEdges) that bitindex gets determined. Multiple edges can be drawn between two nodes with
    /// different bit ranges. When there are no edges to a certain bit-index, a dummy edge
    /// (an edge with Src=nullptr) gets inserted into IncomingEdges to represent the whole.
    /// IncomingEdges for a memory instruction can be empty.
    OVLSVector<Edge *> IncomingEdges;
  };

  /// This directed graph is used to automatically build the network (of
  /// required instructions) of computing the result of a set of adjacent
  /// gathers from a set of contiguous loads. In this directed graph, an edge
  /// represents a move of a bit-range, and a node can be thought of as a result
  /// of some logical operation on its incoming (edges/)bit-ranges.
  ///
  /// NEXT: describe how the graph is used to automatically compute the
  /// rearrangement instructions.
  class Graph {
    /// When a node is created, it gets pushed into the NodeVector. Therefore,
    /// nodes in the NodeVector don't maintain any order. A destination node could
    /// appear before a source node in the NodeVector.
    GraphNodeVector Nodes;
    ...
  };

...

     In order to find an efficient sequence of rearrangement instructions genShuffles()
     performs two primary tasks on the initial graph:

     1. Simplify(Split) - simplify the graph into a singular mode where each element
        in a gather/scatter has a single unique source.

     2. Optimize(Merge) - selectively merge the single unique nodes within the
        singular-mode graph into a new optimized graph.


1. Simplify
^^^^^^^^^^^

     While the initial graph shows how bit fields from loads need to be rearranged to produce
     each gather result, it often leaves the graph in an un-optimized form. The logical operations
     that can be derived from this initial graph may not be efficient. Each move of a bit-field
     may end up requiring a single instruction. To take advantage of data-parallelism(having to
     move multiple bit-fields by a single instruction) we may need to pack(create a node)
     different bit-fields of different gather nodes together. To facilitate this efficient packing
     (done by the optimizer) over the multiple gather results, the simplifier creates a
     single intermediate node for each bit-field. The way it does it by splitting the
     incoming edges of each gather result.

     This is how the graph looks like after simplification.

.. graphviz::

   digraph Initial_Graph {

      graph[ordering=in];

      V5 -> V1[fontcolor=red, color=red, label="0:64",weight="0:64"];

      V6 -> V1[fontcolor=red, color=red, label="0:64",weight="0:64"];

      V7 -> V1[fontcolor=red, color=red, label="0:64",weight="0:64"];

      V8 -> V1[fontcolor=red, color=red, label="0:64",weight="0:64"];

      V9 -> V2[fontcolor=blue, color=blue, label="0:64",weight="0:64"];

      V10 -> V2[fontcolor=blue, color=blue, label="0:64",weight="0:64"];

      V11 -> V2[fontcolor=blue, color=blue, label="0:64",weight="0:64"];

      V12 -> V2[fontcolor=blue, color=blue, label="0:64",weight="0:64"];

      V3 -> V5[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V3 -> V6[fontcolor=red, color=red, label="128:191",weight="128:191"];

      V4 -> V7[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V4 -> V8[fontcolor=red, color=red, label="128:191",weight="128:191"];

      V3 -> V9[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V3 -> V10[fontcolor=blue, color=blue, label="192:255",weight="192:255"];

      V4 -> V11[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V4 -> V12[fontcolor=blue, color=blue, label="192:255",weight="192:255"];

   }


2. merge()-
^^^^^^^^^^^

     Before trying to find the exact (opcodes/) instructions we perform an additional optimization step that attempts to exploit
     data parallelism available in the rearrangement operations. We do this by merging similar nodes, which we do by test-merging
     different combination of nodes. A test-merge is deemed successful, if an instruction(/a set of instructions)
     exits that performs the merged function and that instruction has minimum instruction cost. Minimum instruction cost is determined
     by server querying back to the client and asking for a cost of the instructions. The client is responsible for using the TTI cost-model
     (or something better) that gives us a target specific instruction cost.

     Primarily we perform 3 tasks in this phase:
       a. Simulation of test-merges

       b. Cost estimation of test-merges

       c. Commit the test-merge with the lowest cost.

a. Simulation of test-merges:
"""""""""""""""""""""""""""""

     A test merge is simulated by computing a mask for the merge.

     Two nodes, N1 and N2 are eligible to be merged if:
       #. The total number of unique sources of N1 and N2 is no more than 2
       #. Sources need to have the same type
       #. Total size of N1 and N2 fits into the vector register
       #. elem_size of N1 matches the elem_size of N2


     E.g.
       N1:
          [0:63] = V5[0:63]

          [64:127] = V6[0:63]
       N2:
          [0:63] = V5[64:127]

          [64:127] = V6[64:127]

     There are many ways N1 and N2 can be merged such as <N1 N2 N1 N2> <N1 N1.. N2 N2..> <N2 N2 .. N1 N1 ..>
     <N2 N1 .. N2 N1 ..> etc. Right now it makes sense to concatenate N2 to N1 which will most likely lead to
     vperm or vunpck. But this ordering is subject to change in the future considering some other scenerios.
     Under the consideration, we get the following choices for our example:

       V5 can be merged with V6

         Incoming-Mask <0, 2, -1, -1>

         Outgoing-Mask <0, 1, -1, -1>

       V5 can be merged with V7

         Incoming-Mask <0, 4, -1, -1>

         Outgoing-Mask <0, -1, 1, -1>

       V5 can be merged with V8

         Incoming-Mask <0, 6, -1, -1>

         Outgoing-Mask <0, -1, -1, 1>

       V5 can be merged with V9

         Incoming-Mask <0, 1, -1, -1>

         Outgoing-Mask <0, -1, -1, -1>

         Outgoing-Mask <1, -1, -1, -1>

       V5 can be merged with V10

         Incoming-Mask <0, 3, -1, -1>

         Outgoing-Mask <0, -1, -1, -1>

         Outgoing-Mask <-1, 1, -1, -1>

       V5 can be merged with V11

         Incoming-Mask <0, 5, -1, -1>

         Outgoing-Mask <0, -1, -1, -1>

         Outgoing-Mask <-1, -1, 1, -1>

       V5 can be merged with V12

         Incoming-Mask <0, 7, -1, -1>

         Outgoing-Mask <0, -1, -1, -1>

         Outgoing-Mask <-1, -1, -1, 1>

       V6 can be merged with V7

         Incoming-Mask: <2, 4, -1, -1>

         Outgoing-Mask <-1, 0, 1, -1>

       V6 can be merged with V8

         Incoming-Mask: <2, 6, -1, -1>

         Outgoing-Mask <-1, 0, -1, 1>

       V6 can be merged with V9

         Incoming-Mask <2, 1, -1, -1>

         Outgoing-Mask <-1, 0, -1, -1>

         Outgoing-Mask <1, -1, -1, -1>

       V6 can be merged with V10

         Incoming-Mask: <2, 3, -1, -1>

         Outgoing-Mask <-1, 0, -1, -1>

         Outgoing-Mask <-1, 1, -1, -1>

       V6 can be merged with V11

         Incoming-Mask <2, 5, -1, -1>

         Outgoing-Mask <-1, 0, -1, -1>

         Outgoing-Mask <-1, -1, 1, -1>

       V6 can be merged with V12

         Incoming-Mask <2, 7, -1, -1>

         Outgoing-Mask <-1, 0, -1, -1>

         Outgoing-Mask <-1, -1, -1, 1>

       V7 can be merged with V8

         Incoming-Mask <0, 2, -1, -1>

         Outgoing-Mask <-1, -1, 0, 1>

       V7 can be merged with V9

         Incoming-Mask <4, 1, -1, -1>

         Outgoing-Mask <-1, -1, 0, -1>

         Outgoing-Mask <1, -1, -1, -1>

       V7 can be merged with V10

         Incoming-Mask <4, 3, -1, -1>

         Outgoing-Mask <-1, -1, 0, -1>

         Outgoing-Mask <-1, 1, -1, -1>

       V7 can be merged with V11

         Incoming-Mask <0, 1, -1, -1>

         Outgoing-Mask <-1, -1, 0, -1>

         Outgoing-Mask <-1, -1, 1, -1>

       V7 can be merged with V12

         Incoming-Mask <0, 3, -1, -1>

         Outgoing-Mask <-1, -1, 0, -1>

         Outgoing_Mask <-1, -1, -1, 1>

       V8 can be merged with V9

         Incoming-Mask <6, 1, -1, -1>

         Outgoing-Mask <-1, -1, -1, 0>

         Outgoing-Mask <1, -1, -1, -1>

       V8 can be merged with V10

         Incoming-Mask <6, 3, -1, -1>

         Outgoing-Mask <-1, -1, -1, 0>

         Outgoing-Mask <-1, 1, -1, -1>

       V8 can be merged with V11

         Incoming-Mask <2, 1, -1, -1>

         Outgoing-Mask <-1, -1, -1, 0>

         Outgoing-Mask <-1, -1, 1, -1>

       V8 can be merged with V12

         Incoming <2, 3, -1, -1>

         Outgoing <-1, -1, -1, 0>

         Outgoing <-1, -1, -1, 1>

     Now that we have a couple of choices to merge two nodes we decide to commit the
     merges that shows the lowest total cost.

b. Cost estimation of test-merges:
""""""""""""""""""""""""""""""""""

     In order to compute the cost of a mask first we identify the 'kind' of a mask.
     Depending on their kind we call the TTI getShuffleCost(). Currently we get the
     following cost for a target with avx2:

      V5 can be merged with V6

        Incoming-Mask <0, 2, -1, -1> Cost 4

        Outgoing-Mask <0, 1, -1, -1> Cost 1

        --Total-Cost-- 5

      V5 can be merged with V7

        Incoming-Mask <0, 4, -1, -1> Cost 4

        Outgoing-Mask <0, -1, 1, -1> Cost 4

        --Total-Cost-- 8

      V5 can be merged with V8

        Incoming-Mask <0, 6, -1, -1> Cost 4

        Outgoing-Mask <0, -1, -1, 1> Cost 4

        --Total-Cost-- 8

      V5 can be merged with V9

        Incoming-Mask <0, 1, -1, -1> Cost 1

        Outgoing-Mask <0, -1, -1, -1> Cost 1

        Outgoing-Mask <1, -1, -1, -1> Cost 1

        --Total-Cost-- 3

      V5 can be merged with V10

        Incoming-Mask <0, 3, -1, -1> Cost 4

        Outgoing-Mask <0, -1, -1, -1> Cost 1

        Outgoing-Mask <-1, 1, -1, -1> Cost 2

        --Total-Cost-- 7

      V5 can be merged with V11

        Incoming-Mask <0, 5, -1, -1> Cost 4

        Outgoing-Mask <0, -1, -1, -1> Cost 1

        Outgoing-Mask <-1, -1, 1, -1> Cost 3

        --Total-Cost-- 8

      V5 can be merged with V12

        Incoming-Mask <0, 7, -1, -1> Cost 4

        Outgoing-Mask <0, -1, -1, -1> Cost 1

        Outgoing-Mask <-1, -1, -1, 1> Cost 2

       --Total-Cost-- 7

      V6 can be merged with V7

        Incoming-Mask <2, 4, -1, -1> Cost 4

        Outgoing-Mask <-1, 0, 1, -1> Cost 4

        --Total-Cost-- 8

      V6 can be merged with V8

        Incoming-Mask <2, 6, -1, -1> Cost 4

        Outgoing-Mask <-1, 0, -1, 1> Cost 4

        --Total-Cost-- 8

      V6 can be merged with V9
        Incoming-Mask <2, 1, -1, -1> Cost 4

        Outgoing-Mask <-1, 0, -1, -1> Cost 2

        Outgoing-Mask <1, -1, -1, -1> Cost 1

        --Total-Cost-- 7

      V6 can be merged with V10

        Incoming-Mask <2, 3, -1, -1> Cost 1

        Outgoing-Mask <-1, 0, -1, -1> Cost 2

        Outgoing-Mask <-1, 1, -1, -1> Cost 2

        --Total-Cost-- 5

      V6 can be merged with V11

        Incoming-Mask <2, 5, -1, -1> Cost 4

        Outgoing-Mask <-1, 0, -1, -1> Cost 2

        Outgoing-Mask <-1, -1, 1, -1> Cost 3

        --Total-Cost-- 9

      V6 can be merged with V12

        Incoming-Mask <2, 7, -1, -1> Cost 4

        Outgoing-Mask <-1, 0, -1, -1> Cost 2

        Outgoing-Mask <-1, -1, -1, 1> Cost 2

        --Total-Cost-- 8

      V7 can be merged with V8

        Incoming-Mask <0, 2, -1, -1> Cost 4

        Outgoing-Mask <-1, -1, 0, 1> Cost 4

        --Total-Cost-- 8

      V7 can be merged with V9

        Incoming-Mask <4, 1, -1, -1> Cost 4

        Outgoing-Mask <-1, -1, 0, -1> Cost 2

        Outgoing-Mask <1, -1, -1, -1> Cost 1

        --Total-Cost-- 7

      V7 can be merged with V10
        Incoming-Mask <4, 3, -1, -1> Cost 4

        Outgoing-Mask <-1, -1, 0, -1> Cost 2

        Outgoing-Mask <-1, 1, -1, -1> Cost 2

        --Total-Cost-- 8

      V7 can be merged with V11

        Incoming-Mask <0, 1, -1, -1> Cost 1

        Outgoing-Mask <-1, -1, 0, -1> Cost 2

        Outgoing-Mask <-1, -1, 1, -1> Cost 3

        --Total-Cost-- 6

      V7 can be merged with V12

        Incoming-Mask <0, 3, -1, -1> Cost 4

        Outgoing-Mask <-1, -1, 0, -1> Cost 2

        Outgoing-Mask <-1, -1, -1, 1> Cost 2

        --Total-Cost-- 8


      V8 can be merged with V9

        Incoming-Mask <6, 1, -1, -1> Cost 4

        Outgoing-Mask <-1, -1, -1, 0> Cost 3

        Outgoing-Mask <1, -1, -1, -1> Cost 1

        --Total-Cost-- 8

      V8 can be merged with V10

        Incoming-Mask <6, 3, -1, -1> Cost 4

        Outgoing-Mask <-1, -1, -1, 0> Cost 3

        Outgoing-Mask <-1, 1, -1, -1> Cost 2

        --Total-Cost-- 9

      V8 can be merged with V11

       Incoming-Mask <2, 1, -1, -1> Cost 4

       Outgoing-Mask <-1, -1, -1, 0> Cost 3

       Outgoing-Mask <-1, -1, 1, -1> Cost 3

       --Total-Cost-- 10

      V8 can be merged with V12

        Incoming-Mask <2, 3, -1, -1> Cost 1

        Outgoing-Mask <-1, -1, -1, 0> Cost 3

        Outgoing-Mask <-1, -1, -1, 1> Cost 2

        --Total-Cost-- 6


c. Commit test-merge:
"""""""""""""""""""""

     Now that we have computed the cost for all test-merges, we commit the one with the lowest cost. There
     might be multiple merging options with the same cost. In that case, we choose the one that comes first.

     So, for the above node-set, we got to merge:

         Merge 5 to 9

         Merge 6 to 10 (This decision does not consider 9, since it alredy got merged with 5)

         Merge 7 to 11

         Merge 8 with 12


     This is how the graph looks after the first round of merging:

.. graphviz::

   digraph Initial_Graph {

      graph[ordering=in];

      V3 -> V5[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V4 -> V7[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V3 -> V5[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V4 -> V7[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V3 -> V6[fontcolor=red, color=red, label="128:191",weight="128:191"];

      V4 -> V8[fontcolor=red, color=red, label="128:191",weight="128:191"];

      V3 -> V6[fontcolor=blue, color=blue, label="192:255",weight="192:255"];

      V4 -> V8[fontcolor=blue, color=blue, label="192:255",weight="192:255"];

      V5 -> V1[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V6 -> V1[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V7 -> V1[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V8 -> V1[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V5 -> V2[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V6 -> V2[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V7 -> V2[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V8 -> V2[fontcolor=blue, color=blue, label="64:127",weight="64:127"];
   }

...

     We keep optimizing until there are no further opportunities for merge. After another round of merging
     the following final graph:



.. graphviz::

   digraph Initial_Graph {

      graph[ordering=in];

      V3 -> V5[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V3 -> V5[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V4 -> V5[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V4 -> V5[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V3 -> V6[fontcolor=red, color=red, label="128:191",weight="128:191"];

      V3 -> V6[fontcolor=blue, color=blue, label="192:255",weight="192:255"];

      V4 -> V6[fontcolor=red, color=red, label="128:191",weight="128:191"];

      V4 -> V6[fontcolor=blue, color=blue, label="192:255",weight="192:255"];

      V5 -> V1[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V6 -> V1[fontcolor=red, color=red, label="0:63",weight="0:63"];

      V5 -> V1[fontcolor=red, color=red, label="128:191",weight="128:191"];

      V6 -> V1[fontcolor=red, color=red, label="128:191",weight="128:191"];

      V5 -> V2[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V6 -> V2[fontcolor=blue, color=blue, label="64:127",weight="64:127"];

      V5 -> V2[fontcolor=blue, color=blue, label="192:255",weight="192:255"];

      V6 -> V2[fontcolor=blue, color=blue, label="192:255",weight="192:255"];
   }


...

     At this point we are done optimizing the nodes. We generate an instruction for
     each node: two contigous loads (v3, v4)followed by 4 shuffle instructions(v5, v6, v1, v2).

     %1 = mask.load.64.4 (<Base:0x3e6dab0 Offset:0>, 1111)

     %2 = mask.load.64.4 (<Base:0x3e6dab0 Offset:32>, 1111)

     %3 = shufflevector <4 x 64> %1, <4 x 64> %2, <4 x 32><0, 1, 4, 5>

     %4 = shufflevector <4 x 64> %1, <4 x 64> %2, <4 x 32><2, 3, 6, 7>

     %5 = shufflevector <4 x 64> %3, <4 x 64> %4, <4 x 32><0, 4, 2, 6>

     %6 = shufflevector <4 x 64> %3, <4 x 64> %4, <4 x 32><1, 5, 3, 7>

...

     At the end, it is possible we end up having an invalid graph. A graph that has nodes with
     more than two source nodes. These nodes do not represent any valid instructions. And this
     could happen because the initial graph can have nodes(i.e. the output nodes) with more
     than 2 sources, and the merge algorithm might but is not guranteed to reduce all these
     nodes to 2 sources. Currently we bail out in such situations.


     NEXT: provide more details on the instruction cost, merging, instruction generation and complete the example.

     NEXT: provide details on the graph-verification.


Section 6: Overall scenerio of OptVLS server-client system
==========================================================

This section demonstrates the overall scenerio of OptVLS; how OptVLS plays in LLVM compiler and
how it works as a server-client system.

The way it works in the LLVM compilation-process is:

 1. Vectorizer detects a group of adjacent gathers/scatters and generates a sequence of wide-load
    plus shuffles or a sequence of shuffles followed by a wide-store. These shuffles are the
    abstract representation of gathers/scatters (they basically represent the gather/scatter result).
    They are often semi-optimal and they can be further optimized; this what OptVLS does.

    For example-1, on AVX2 using vector length 4 vectorizer generates:

.. code-block:: none

    ** IR Dump After Loop Vectorization **

    vector.body:

     ...

     %wide.vec = load<8 x double> , <8 x double> * %4, align 8

     %strided.vec1 = shufflevector <8 x double> %wide.vec, <8 x double> undef, <4 x i32> <i32 0, i32 2, i32 4, i32 6>

     %strided.vec2 = shufflevector <8 x double> %wide.vec, <8 x double> undef, <4 x i32> <i32 1, i32 3, i32 5, i32 7>

     ...

...

    Please note that, currently vectorizer can only generate wide-load/store plus shuffles for strided accesses
    with constant strides. In order to generate this wide-load/store plus shuffles for indirect accesses or
    strided accesses with variable and large strides, we need to define an instruction(TODO).

    FIXME: Vectorizer's interaction with OptVLS for cost calculation needs to be documented.


 2. After going through many other intermediate optimizations, finaly the load/store plus shuffle sequence reaches
    the pre-codeGen passes. LLVM's InterleavedAccessPass is one of the pre-CodeGen passes that is responsible for
    lowering this wide-load/store plus shuffle sequence into further optimized sequence.

    OptVLS server is responsible for computing the optimized sequence and for details on how OptVLS server computes
    optimized sequence please refer to section-5. In this flow, X86InterleavedAccess can be
    thought of as client of OptVLS server which processes the wide-load/store plus shuffle sequence into OptVLS
    abstract types and communicates with the OptVLS server for collecting the optimized sequence and translating
    the optimized sequence into LLVM-IR. This whole process is deccribed in the following steps:

    a) X86InterleavedAccess forms an X86InterleavedAccessGroup for each wide-load/store plus shuffle sequence.
       The members of this X86InterleavedAccessGroup are the shufflevectors.

    b) In order to get the optimized sequence it needs to generate an OVLSGroup. The way it generates an OVLSGroup
       is: it represents each shufflevector into X86InterleavedClientMemref first and then calls the getGroups() method.

      This is the debug output of getGroups() for example-1:

.. code-block:: none

      Received a request from Client---FORM GROUPS

         Recieved a vector of memrefs:
           #1 <4 x 64> SLoad

           #2 <4 x 64> SLoad

         Split the vector memrefs into sub groups of adjacent memrefs:
            Distance is (in bytes) from the first memref of the set

         Set #1

         #1 <4 x 64> SLoad   Dist: 0

         #2 <4 x 64> SLoad   Dist: 8

         Printing Groups- Total Groups 1

         Group#1
           Vector Length(in bytes): 32

           AccType: SLoad

           AccessMask(per byte, R to L): 1111111111111111
           #1 <4 x 64> SLoad

           #2 <4 x 64> SLoad

...

    c) Once the OVLSGroup is formed, it calls the getSequence() method for collecting
       the optimized sequence.

       Here is the output sequence for Example-1 returned by getSequence() in abstract OVLSInstruction type.


.. code-block:: none

      %3 = mask.load.64.4 (<Base:0x5fc95e0 Offset:0>, 1111)

      %4 = mask.load.64.4 (<Base:0x5fc95e0 Offset:32>, 1111)

      %7 = shufflevector <4 x 64> %3, <4 x 64> %4, <4 x 32><0, 1, 4, 5>

      %10 = shufflevector <4 x 64> %3, <4 x 64> %4, <4 x 32><2, 3, 6, 7>

      %13 = shufflevector <4 x 64> %7, <4 x 64> %10, <4 x 32><0, 4, 2, 6>

      %16 = shufflevector <4 x 64> %7, <4 x 64> %10, <4 x 32><1, 5, 3, 7>

...


    d) The optimized sequence is in OVLS abstract type, it needs to be represented in LLVM-IR.
       So, X86InterleavedAccess then calls genLLVMIR() method, a client utility function that
       translates the OVLSInstruction into LLVM-IR instruction.

      Finally, this is how the optimized sequence looks like in LLVM-IR:

.. code-block:: none

      %1 = bitcast <8 x double>* %ptr to <4 x double>*

      %2 = getelementptr inbounds <4 x double>, <4 x double>* %1, i32 0

      %3 = load <4 x double>, <4 x double>* %2, align 16

      %4 = bitcast <8 x double>* %ptr to <4 x double>*

      %5 = getelementptr inbounds <4 x double>, <4 x double>* %4, i32 1

      %6 = load <4 x double>, <4 x double>* %5, align 16

      %7 = shufflevector <4 x double> %3, <4 x double> %6, <4 x i32> <i32 0, i32 1, i32 4, i32 5>

      %8 = shufflevector <4 x double> %3, <4 x double> %6, <4 x i32> <i32 2, i32 3, i32 6, i32 7>

      %9 = shufflevector <4 x double> %7, <4 x double> %8, <4 x i32> <i32 0, i32 4, i32 2, i32 6>

      %10 = shufflevector <4 x double> %7, <4 x double> %8, <4 x i32> <i32 1, i32 5, i32 3, i32 7>


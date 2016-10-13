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

Here is an example of this optimization:

.. code-block::  c++

  double x[1000];
  for(int k = 0; k < n; k++) {
    const int j = n[k];
    double p = x[j  ];
    double q = x[j + 1];
    ...
  }

One possible scenario of the loads from multiple consecutive iterations could be thought of laid out in the
linear memory space of x such as: p1 q1 ... p2 q2 ...

A naive vectorization will generate 2 gathers for the two indirect memory references. With vector length 2,
it will try to load p(p1, p2) using one gather, q(q1, q2) using another gather and so on.

Pseudo vector code:

.. code-block:: none

  for.body: // vector length is 2
    ...
    %nidx = getelementptr inbounds ..@n, i64 0, i64 %k
    // a contiguous load is generated for n[k]
    %j_v   = @llvm.masked.load.v2i64(nidx, ..);

    %gather1address = getelementptr(@x,   %j_v);
    %gather2address = getelementptr(@x+1, %j_v);

    // two gathers are generated for two indirect accesses x[j], x[j+1]

    // gather-1 gathering p1, p2
    %p_v = call <2 x double> @llvm.masked.gather.v2f64(%gather1address, ..);

    // gather-2 gathering q1, q2
    %q_v = call <2 x double> @llvm.masked.gather.v2f64(%gather2address, ..);

    ...

As we can see adjacent gathers/scatters access a piece-wise contiguous range of memory, we can issue a contiguous
load to load each contiguous chunk and finally in order to produce the gather output we can shuffle the loaded
value. Even though we execute more instructions this way, the total execution time of these instructions is usualy
less than the total execution time of gathers/scatters. Therefore, this optimization is generally profitable.

Optimized pseudo vector code:

.. code-block::  none

  for.body: // vector length is 2
    ...
    // load p1 q1
    %1 = @llvm.masked.load.v2f64(&x[n[k]], ..);

    // load p2 q2
    %2 = @llvm.masked.load.v2f64(&x[n[k+1]], ..);

    // move p1 p2 to %p_v
    %p_v = shufflevector <2 x f64> %1, <2 x f64> %2, <2 x i32> <i32 0, i32 2>;

    // move q1 q2 to %q_v
    %q_v = shufflevector <2 x f64> %1, <2 x f64> %2, <2 x i32> <i32 1, i32 3>;

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

     %1 = mask.load.64.2 (<Base:0xf7ced0 Offset:0>, 11)

     %2 = mask.load.64.2 (<Base:0xf7ced0 Offset:32>, 11)

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
     load-nodes:{V2, V3}, gather-nodes{V0, V1}:

.. graphviz::

   digraph Initial_Graph {

      V2 -> V0[label="0:63",weight="0:63"];

      V2 -> V1[label="64:127",weight="64:127"];

      V3 -> V0[label="0:63",weight="0:63"];

      V3 -> V1[label="64:127",weight="64:127"];
   }

...

     And, this is how it gets printed by OptVLS-server:

     Initial Graph:

       V3: Load

       V4: Load

       V1:
        [0:63] = V3[0:63]

        [64:127] = V4[0:63]

       V2:
        [0:63] = V3[64:127]

        [64:127] = V4[64:127]


     In the above graph, each gather-node has two incoming edges which matches its total number of elements,
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

     In order to find an efficient sequence of rearrangement instructions genShuffles() performs two primary tasks on the initial
     graph:

     1. Splitting

     2. Merging


1. split()-
^^^^^^^^^^^

     While the initial graph shows how bit fields from loads need to be rearranged to produce each gather result, the logical
     operations needed to do the rearrangement may not correspond to any real single machine instructions or LLVM-IR(/OVLS)-Instructions.
     A valid instruction generally have maximum 2 inputs, and this initial graph allows any number of inputs to feed a gather result,
     thus it would take many real 2-input instruction to compute each final output result.

     Here is an example whose initial graph would contain gather nodes with more than 2-input source nodes:

.. code-block::  c++

  double x[1000];
  for(int k = 0; k < n; k++) {
    const int j = n[k];
    double p = x[j  ];
    double q = x[j + 1];
    ...
  }

...

     One possible scenario of the loads from multiple consecutive iterations could be thought of laid out in the
     linear memory space of x such as: p1 q1 ... p2 q2 ... p3 q3 ... p4 q4 ...
     With VF = 4, each gather will contain 4 elements.

     genLoads() will generate 4 contiguous loads and the following initial graph:

     %1 = mask.load.64.4 (<Base:0xf7ced0 Offset:0>, 11)

     %2 = mask.load.64.4 (<Base:0xf7cdd0 Offset:0>, 11)

     %3 = mask.load.64.4 (<Base:0xf7cde0 Offset:0>, 11)

     %4 = mask.load.64.4 (<Base:0xf7eed0 Offset:0>, 11)


.. graphviz::

   digraph Initial_Graph {

      V2 -> V0[label="0:63",weight="0:63"];

      V2 -> V1[label="64:127",weight="64:127"];

      V3 -> V0[label="0:63",weight="0:63"];

      V3 -> V1[label="64:127",weight="64:127"];

      V4 -> V0[label="0:63",weight="0:63"];

      V4 -> V1[label="64:127",weight="64:127"];

      V5 -> V0[label="0:63",weight="0:63"];

      V5 -> V1[label="64:127",weight="64:127"];

   }

...

     The first job of genShuffles() is to simplify the graph so it can be optimized. We simplify the graph by
     splitting source nodes recursively until each node has no more than two source nodes. Each step of the
     recursive split replaces a single node by 3 nodes, where 2 nodes each has half the source nodes of the
     original node, and those two nodes feed the third node. Once this has been done for all nodes we have
     transformed the initial graph into a new graph where every node operates on maximum 2 sources.

     Here is the output graph after splitting:

.. graphviz::

   digraph Initial_Graph {

      V2 -> V6[label="0:63",weight="0:63"];

      V2 -> V8[label="64:127",weight="64:127"];

      V3 -> V6[label="0:63",weight="0:63"];

      V6 -> V0[label="0:63",weight="0:63"];

      V6 -> V0[label="64:127",weight="64:127"];

      V3 -> V8[label="64:127",weight="64:127"];

      V4 -> V7[label="0:63",weight="0:63"];

      V4 -> V9[label="64:127",weight="64:127"];

      V5 -> V9[label="64:127",weight="64:127"];

      V5 -> V7[label="0:63",weight="0:63"];

      V7 -> V0[label="0:63",weight="0:63"];

      V7 -> V0[label="64:127",weight="64:127"];

      V8 -> V1[label="0:63",weight="0:63"];

      V8 -> V1[label="64:127",weight="64:127"];

      V9 -> V1[label="0:63",weight="0:63"];

      V9 -> V1[label="64:127",weight="64:127"];

   }

...

     These nodes are now quite similar to instructions (they have 1 or 2 inputs and a single output), though they are not quite
     instructions yet because we haven't yet found precise instructions (including opcodes and immediate values) that perform
     the needed operations.


2. merge()-
^^^^^^^^^^^

     Before trying to find the exact (opcodes/) instructions we perform an additional optimization step that attempts to exploit
     data parallelism available in the rearrangement operations. We do this by merging similar nodes, which we do by test-merging
     different combination of nodes that have the same sources. A merge is deemed successful, if an instruction(/a set of instructions)
     exits that performs the merged function and that instruction has minimum instruction cost. Minimum instruction cost is determined
     by server querying back to the client and asking for a cost of the instructions. The client is responsible for using the TTI cost-model
     (or something better) that gives us a target specific instruction cost.

     For our simple example, splitting is not required since each node in the graph has maximum two input nodes. There are no
     intermediate nodes other than the load/gather-nodes, so no room for exploiting data parallelism or additional optimization.
     After a successful graph-verification genShuffles() traverses the graph in a topological order and translates each node (each
     logical instruction other than the load-nodes) into an OVLSInstruction(shuffle instruction) using its incoming edges. More
     specifically, input operands of the shuffle instruction are the set of 'sources' identified by the incoming edges. We compute
     the shuffle mask by combining the incoming bits where each element in the mask gets specified by the bit-index of the
     incoming bits of its input nodes. At this final stage, the graph has only two non-load nodes. Consequently, the following
     two shuffle instructions get generated:

     %3 = shufflevector <2 x 64> %1, <2 x 64> %2, <2 x 32><0, 2>;

     %4 = shufflevector <2 x 64> %1, <2 x 64> %2, <2 x 32><1, 3>;

     NEXT: provide more details on the instruction cost, merging, instruction generation and complete the example.

     NEXT: provide details on the graph-verification.

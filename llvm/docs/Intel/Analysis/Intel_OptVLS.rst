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
  double r = x[j + 2];
  double s = x[j + 3];
  ..
}

One possible scenario of the loads from multiple consecutive iterations could be thought of laid out in the
linear memory space of x such as: p1 q1 r1 s1 ... p3 q3 r3 s3 ... p2 q2 r2 s2 ... p4 q4 r4 s4 ...

A naive vectorization will generate 4 gathers for the four indirect memory references. With vector length 4, it
will try to load p(p1, p2, p3, p4) using one gather, q(q1, q2, q3, q4) using another gather and so on.

.. code-block::  pseudo vector code

for.body: // vector length is 4
  ..
  %nidx = getelementptr inbounds ..@n, i64 0, i64 %k
  // a contiguous load is generated for n[k]
  %j_v   = @llvm.masked.load.v4i32(nidx, ..);

  %gather1address = getelementptr(@x,   %j_v);
  %gather2address = getelementptr(@x+1, %j_v);
  %gather3address = getelementptr(@x+2, %j_v);
  %gather3address = getelementptr(@x+3, %j_v);

  // four gathers are generated for four indirect accesses x[j], x[j+1], x[j+2], x[j+3]
  // gathering p1, p2, p3, p4
  %p_v = @llvm.masked.gather.v16f32(%gather1address, ..);

  // gathering q1, q2, q3, q4
  %q_v = @llvm.masked.gather.v16f32(%gather2address, ..);

  // gathering r1, r2, r3, r4
  %r_v = @llvm.masked.gather.v16f32(%gather3address, ..);

  // gathering s1, s2, s3, s4
  %s_v = @llvm.masked.gather.v16f32(%gather4address, ..);
  ..

As we can see adjacent gathers/scatters access a piece-wise contiguous range of memory, we can issue a contiguous
load to load each contiguous chunk and finally in order to produce the gather output we can shuffle the loaded
value. Even though we execute more instructions this way, the total execution time of these instructions is usualy
less than the total execution time of gathers/scatters. Therefore, this optimization is generally profitable.

.. code-block::  optimized pseudo vector code

for.body: // vector length is 4
  ..
  // load p1 q1 r1 s1
  %1 = @llvm.masked.load.v4f64(&x[n[k]], ..);

  // load p2 q2 r2 s2
  %2 = @llvm.masked.load.v4f64(&x[n[k+1]], ..);

  // load p3 q3 r3 s3
  %3 = @llvm.masked.load.v4f64(&x[n[k+2]], ..);

  // load p4 q4 r4 s4
  %4 = @llvm.masked.load.v4f64((&x[n[k+3]], ..);

  // move p1 q1 p3 q3 to %5
  %5 = shufflevector <4 x f64> %1, <4 x f64> %3, <4 x i32> <i32 0, i32 1, i32 4, i32 5>;

  // move p2 q2 p4 q4 to %6
  %6 = shufflevector <4 x f64> %2, <4 x f64> %4, <4 x i32> <i32 0, i32 1, i32 4, i32 5>;

  // move p1 p2 p3 p4 to %p_v
  %p_v = shufflevector <4 x f64> %5, <4 x f64> %6, <4 x i32> <i32 0, i32 4, i32 2, i32 6>;

  // move q1 q2 q3 q4 to %q_v
  %q_v = shufflevector <4 x f64> %5, <4 x f64> %6, <4 x i32> <i32 1, i32 5, i32 3, i32 7>;

  // move q1 r1 q3 s3
  %7 = shufflevector <4 x f64> %1, <4 x f64> %3, <4 x i32> <i32 2, i32 3, i32 6, i32 7>;

  // move q2 r2 q4 s4
  %8 = shufflevector <4 x f64> %2, <4 x f64> %4, <4 x i32> <i32 2, i32 3, i32 6, i32 7>;

  // move q3 q2 q3 q4
  %r_v = shufflevector <4 x f64> %7, <4 x f64> %8, <4 x i32> <i32 0, i32 4, i32 2, i32 6>;

  // move r1 r2 r3 r4
  %s_v = shufflevector <4 x f64> %7, <4 x f64> %8, <4 x i32> <i32 1, i32 5, i32 3, i32 7>;
  ..

OptVLS is designed to follow the client-server model. The server is responsible for doing the optimization on a set
of abstract memrefs and providing a set of abstract optimized sequences. The client is responsible for providing
mechanisms to reason about these memrefs (for example, computing a distance between a pair of memrefs). One example
client is the OptVLS pass, which operates on (vector) gathers/scatters. Another example client is the vectorizer,
which operates on scalar loads/stores.  The server is responsible for doing the optimization and providing services
to its different clients (both scalar and vectorizer optimizer). It supports both vector and scalar memrefs seamlessly
(as long as they are abstracted as OVLSMemrefs).

This document focuses on the server part of the optimization and is broken down into the following 5
sections as follows:
  Section 1: OptVLS server core functionalities
  Section 2: Key design considerations
  Section 3: Documentation of the public interface that the server provided to the clients
  Section 4: How to use OptVLS in a compiler by writing an OptVLS client.
  Section 5: Important implementation details.

`Section 1: OptVLS server core functionalities:`
  i. Takes a set of abstracted memory references such as gathers/scatters and separates them into multiple
     groups where each group includes only adjacent gathers/scatters.

  ii. Estimates the relative cost/benefit of replacing the adjacent gathers/scatters in a group by a
      semantically equivalent set of contiguous loads/stores and re-arrangement instructions.

  iii. Generates the abstract optimized sequence for a group of adjacent gathers/scatters

`Section 2: Key Design Considerations`
    In order to facilitate different clients, such as vectorizer client, or scalar (optimizer) client,
  this optimization is implemented following a client/server model where the server is agnostic of the
  IR used by the client and the client and server communicate using abstract data types. Consequently,
  the optimization can be called from anywhere by simply implementing new clients, and little to no
  changes should be required in the server.

`Section 3: Public Interface`
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
  }

  ... A quick description of the public member functions is as follows:
     getGroups(..) .  Takes a set of OVLSMemrefs and a vector length that is the maximum
     allowed vector register size (in bytes) on the underlying architecture. Returns a set of OVLSGroups
     where each group contains the OVLSMemrefs that are adjacent and a mapping from OVLSMemref
     to an OVLSGroup.

`Section 4: How to use OptVLS in a compiler by writing an OptVLS client`

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

`Section 5: Implementation Details`
  This section describes more details for each interface function and abstract type.

  i. getGroups()

    #. The input vector length is the maximum allowed vector size in the underlying architecture.
    This determines how many adjacent memrefs can be put together in a group. In addition, it
    tells us how many memrefs can be processed at a time using a single vector register.

    #. Currently, grouping is done using a greedy algorithm. It sorts out the memrefs based
       on their distance from the base address. Then it keeps putting the memref starting at
       the lowest address until the group is full. Doing it this way, it's possible for a memref
       to be put in a group where it has a bigger distance between memrefs than if it were put
       in a different group which would have different performance implications.

       As an example that uses maximum vector length of 16:
        memref1- distance from base is 0 bytes
        memref2- distance from base is 4 bytes
        memref3 distance from base is 12 bytes
        memref4 distance from base is 16 bytes
        memref5 distance from base is 20 bytes

       The best grouping should be:
        Group1: memref1, memref2
        Group2: memref2, memref4, memref5

       Using current approach the groups we will get are:
        Group1: memref1, memref2, memref3
        Group2: memref4, memref5


    #. canMoveTo()- FIXME: We are still discussing whether it's the server or the client is responsible
                   for code placement, which will affect this interface.


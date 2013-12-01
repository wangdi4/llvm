// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES{ } LOSS OF USE, DATA, OR
// PROFITS{ } OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directlytypedef uint _PIPE_TYPE;

#define INTEL_PIPE_HEADER_RESERVED_SPACE    32

#ifdef KERNEL_CODE

// Total size:  16 bytes + sizeof(void*)
typedef struct _tag_pipe_control_intel_t
{
    // The pipe packet size is known at compile-time, and is
    // sizeof(_PIPE_TYPE).  

    // Total number of packets in the pipe.  This value must be 
    // set by the host when the pipe is created.
    uint pipe_max_packets;

    // The pipe head and tail must be set by the host when 
    // the pipe is created.  They will probably be set to zero,
    // though as long as head equals tail, it doesn't matter
    // what they are initially set to.
    uint head;  // Head Index, for reading: [0, pipe_max_packets)
    uint tail;  // Tail Index, for writing: [0, pipe_max_packets)

    // This controls whether the pipe is unlocked, locked for
    // reading, or locked for writing.  If it is zero, the pipe
    // is unlocked.  If it is positive, it is locked for writing.
    // If it is negative, it is locked for reading.  This is only 
    // needed if we want to support concurrent pipe reads and 
    // writes. This must be set to zero by the host when the pipe
    // is created.
    int lock;

    // This guy should be at the end, since sizeof(void*) may
    // be different on host and device... at least for OpenCL
    // 1.2 and pre-SVM.  Once we get SVM this could be set by
    // the host, too.
    __global _PIPE_TYPE* base;
} pipe_control_intel_t;

#endif

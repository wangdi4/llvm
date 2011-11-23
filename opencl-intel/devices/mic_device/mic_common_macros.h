
// Copyright (c) 2006-2008 Intel Corporation
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
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  ProgramService.h
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#pragma once

// return number of elements in array with size known at compile time
#define ARRAY_ELEMENTS( ar ) (sizeof(ar) / sizeof((ar)[0]))

// atomic assign to prevent Out-of-Order CPU optimizations on volatile vars
// if such vars are used for testing outside of critical sections, ex:
//
//    Double-Checked Locking pattern:
//
//  volatile int var;
//  if (0 != var)
//  {
//      return var;
//  }
//  lock();
//  var = XXX;
//  unlock();
//  return var;
//
#define ATOMIC_ASSIGN( var_name, value )                                                      \
    {                                                                                         \
        bool ok;                                                                              \
        ok = __sync_bool_compare_and_swap( &(var_name), (var_name), (value) );                \
        assert( ok && ("ATOMIC_ASSIGN of value " # value " to the " # var_name " failed") );  \
    }

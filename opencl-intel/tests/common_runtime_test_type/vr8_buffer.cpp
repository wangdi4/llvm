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

#include "vr8_buffer.h"
#include "common_runtime_tests.h"
#include <iostream>
#include <sstream>

//|  VR8Buffer
//|
//| Runs tests in VR8BufferTests for all OpenCL types defined in
//|    typedef ::testing::Types<... TYPES DEFINED HERE .. >
// bufferTypes;
//|    TYPED_TEST_CASE(TypedCommonRuntime, bufferTypes); (defined in
// common_runtime_tests.h)

//|  TEST: VR8Buffer.BufferTypedUseHostPtr (TC-40, TC-41, TC-42)
//|
//|  Purpose
//|  -------
//|
//|  Verify that the content of an OpenCL buffer is visible to all devices in
// the context.

//|
//|  Method
//|  ------
//|
//|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
// and GPU) with CL_MEM_USE_HOST_PTR |  2. For each device create separate
// output buffer |  2. Create for each device a kernel which copies all
// elements of input buffer into output buffer |  3. Run kernels on CPU
// and GPU |  4. Validate that all output buffers contain elements of input
// buffer
//|
//|  Pass criteria
//|  -------------
//|
//|  Validate that each device is able to read all buffer elements
//|
TYPED_TEST(VR8Buffer, BufferTypedUseHostPtr) {
  ASSERT_NO_FATAL_FAILURE(
      this->testBufferTypedUseHostPtr(this->ocl_descriptor));
}

//|  TEST: VR8Buffer.BufferTypedAllocHostPtr (TC-43, TC-44, TC-45)
//|
//|  Purpose
//|  -------
//|
//|  Verify that the content of an OpenCL buffer is visible to all devices in
// the context
//|
//|  Method
//|  ------
//|
//|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
// and GPU) with CL_MEM_ALLOC_HOST_PTR |  2. For each device create
// separate output buffer |  2. Create for each device a kernel which copies
// all
// elements of input buffer into output buffer |  3. Run kernels on CPU
// and GPU |  4. Validate that all output buffers contain elements of input
// buffer
//|
//|  Pass criteria
//|  -------------
//|
//|  Validate that each device is able to read all buffer elements
//|
TYPED_TEST(VR8Buffer, BufferTypedAllocHostPtr) {
  ASSERT_NO_FATAL_FAILURE(
      this->testBufferTypedAllocHostPtr(this->ocl_descriptor));
}

//|  TEST: VR8Buffer.BufferTypedCopyHostPtr (TC-43, TC-44, TC-45)
//|
//|  Purpose
//|  -------
//|
//|  Verify that the content of an OpenCL buffer is visible to all devices in
// the context
//|
//|  Method
//|  ------
//|
//|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
// and GPU) with CL_MEM_COPY_HOST_PTR |  2. For each device create separate
// output buffer |  2. Create for each device a kernel which copies all
// elements of input buffer into output buffer |  3. Run kernels on CPU
// and GPU |  4. Validate that all output buffers contain elements of input
// buffer
//|
//|  Pass criteria
//|  -------------
//|
//|  Validate that each device is able to read all buffer elements
//|
TYPED_TEST(VR8Buffer, BufferTypedCopyHostPtr) {
  ASSERT_NO_FATAL_FAILURE(
      this->testBufferTypedCopyHostPtr(this->ocl_descriptor));
}

//|  TEST: VR8Buffer.SubBufferUseHostPtr (TC-46, TC-47, TC-48)
//|
//|  Purpose
//|  -------
//|
//|  Verify that the content of an OpenCL sub-buffer is visible to all
// devices in the context
//|
//|  Method
//|  ------
//|
//|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
// and GPU) with CL_MEM_USE_HOST_PTR |  2. Create shared sub-buffer of buffer in
// step 1 with CL_MEM_USE_HOST_PTR |  2. For each device create separate
// output buffer |  2. Create for each device a kernel which copies all
// elements of input buffer into output buffer |  3. Run kernels on CPU
// and GPU |  4. Validate that all output buffers contain elements of input
// buffer
//|
//|  Pass criteria
//|  -------------
//|
//|  Validate that each device is able to read all sub-buffer elements
//|
TYPED_TEST(VR8Buffer, SubBufferUseHostPtr) {
  ASSERT_NO_FATAL_FAILURE(this->testSubBufferUseHostPtr(this->ocl_descriptor));
}

//|  TEST: VR8Buffer.SubBufferAllocHostPtr (TC-43, TC-44, TC-45)
//|
//|  Purpose
//|  -------
//|
//|  Verify that the content of an OpenCL sub-buffer is visible to all
// devices in the context
//|
//|  Method
//|  ------
//|
//|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
// and GPU) with CL_MEM_ALLOC_HOST_PTR |  2. Create shared sub-buffer of
// buffer in
// step 1 with CL_MEM_ALLOC_HOST_PTR |  2. For each device create separate
// output buffer |  2. Create for each device a kernel which copies all
// elements of input buffer into output buffer |  3. Run kernels on CPU
// and GPU |  4. Validate that all output buffers contain elements of input
// buffer
//|
//|  Pass criteria
//|  -------------
//|
//|  Validate that each device is able to read all sub-buffer elements
//|
TYPED_TEST(VR8Buffer, SubBufferAllocHostPtr) {
  ASSERT_NO_FATAL_FAILURE(
      this->testSubBufferAllocHostPtr(this->ocl_descriptor));
}

//|  TEST: VR8Buffer.SubBufferCopyHostPtr (TC-43, TC-44, TC-45)
//|
//|  Purpose
//|  -------
//|
//|  Verify that the content of an OpenCL sub-buffer is visible to all
// devices in the context
//|
//|  Method
//|  ------
//|
//|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
// and GPU) with CL_MEM_COPY_HOST_PTR |  2. Create shared sub-buffer of buffer
// in step 1 with CL_MEM_COPY_HOST_PTR |  2. For each device create separate
// output buffer |  2. Create for each device a kernel which copies all
// elements of input buffer into output buffer |  3. Run kernels on CPU
// and GPU |  4. Validate that all output buffers contain elements of input
// buffer
//|
//|  Pass criteria
//|  -------------
//|
//|  Validate that each device is able to read all sub-buffer elements
//|
TYPED_TEST(VR8Buffer, SubBufferCopyHostPtr) {
  ASSERT_NO_FATAL_FAILURE(this->testSubBufferCopyHostPtr(this->ocl_descriptor));
}

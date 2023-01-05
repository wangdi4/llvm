#include "CL_BASE.h"
#include "TestsHelpClasses.h"

class BarrierCommandTest : public CL_base {};

TEST_F(BarrierCommandTest, Profiling) {
  cl_int err;
  cl_event event;
  err = clEnqueueBarrierWithWaitList(m_queue, 0, nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueBarrierWithWaitList");

  cl_ulong start, end;
  err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                                sizeof(cl_ulong), &start, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetEventProfilingInfo CL_PROFILING_COMMAND_START");
  err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                                sizeof(cl_ulong), &end, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetEventProfilingInfo CL_PROFILING_COMMAND_END");
  EXPECT_NE(start, end) << "Command elapsed time should not be zero";
}

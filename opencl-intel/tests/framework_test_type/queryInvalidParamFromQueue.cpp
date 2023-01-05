#include "CL.h"

TEST_F(CL, QueryInvalidParamNameFromCQueue_Negative) {
  cl_int iRet = CL_SUCCESS;

  size_t ret_size = -1;
  std::vector<char> buf(20, '\0');

  iRet =
      clGetCommandQueueInfo(m_queue, /*InvalidParamName*/ CL_CONTEXT_PLATFORM,
                            buf.size(), &buf[0], &ret_size);
  ASSERT_EQ(CL_INVALID_VALUE, iRet) << " clGetCommandQueueInfo failed. ";
  ASSERT_EQ((size_t)0, ret_size)
      << " ret_size is not 0 for clGetCommandQueueInfo with invalid param "
         "name. ";
}

// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __LINKDATA_H__
#define __LINKDATA_H__

#include <string>
#include <vector>

namespace Intel {
namespace OpenCL {
namespace ClangFE {
struct IOCLFEBinaryResult;
}
} // namespace OpenCL
} // namespace Intel

namespace Intel {
namespace OpenCL {
namespace Frontend {
//
// Description:
//  Represents the data used by a link operation.
class LinkData {
  // typedefs
public:
  typedef std::pair<const void *, size_t> BufferInfo;

  // methods
public:
  virtual ~LinkData() {}

  void addInputBuffer(const void *pBuffer, size_t size) {
    m_inputBuffers.push_back(BufferInfo(pBuffer, size));
  }

  size_t inputBuffersCount() const { return m_inputBuffers.size(); }

  std::vector<BufferInfo>::const_iterator beginInputBuffers() const {
    return m_inputBuffers.begin();
  }

  std::vector<BufferInfo>::const_iterator endInputBuffers() const {
    return m_inputBuffers.end();
  }

  void setOptions(const char *pszOptions) {
    if (pszOptions) {
      m_options = pszOptions;
    }
  }

  void setBinaryResult(ClangFE::IOCLFEBinaryResult *pResult) {
    m_pResult = pResult;
  }

  ClangFE::IOCLFEBinaryResult *getBinaryResult() const { return m_pResult; }

private:
  std::vector<BufferInfo> m_inputBuffers;
  ClangFE::IOCLFEBinaryResult *m_pResult;
  std::string m_options;
};
} // namespace Frontend
} // namespace OpenCL
} // namespace Intel

#endif

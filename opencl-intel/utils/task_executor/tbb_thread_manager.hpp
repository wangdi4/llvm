// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

template <class Data>
TBB_ThreadManager<Data>::TBB_ThreadManager()
    : m_DescriptorsArray(nullptr), m_uiNumberOfStaticEntries(0),
      m_bOverflowed(false) {}

template <class Data>
bool TBB_ThreadManager<Data>::Init(unsigned int uiNumberOfThreads) {
  assert(false == m_object_exists);

  if (m_object_exists) {
    return false;
  }

  m_object_exists = true;
  m_uiNumberOfStaticEntries = uiNumberOfThreads;

  // allocate data
  if (m_uiNumberOfStaticEntries > 0) {
    m_DescriptorsArray =
        new TBB_ThreadDescriptor<Data>[m_uiNumberOfStaticEntries];
  }

  return true;
}

template <class Data> TBB_ThreadManager<Data>::~TBB_ThreadManager() {
  m_object_exists = false;
  // Intentionally leak everything. The objects are small.
  // If you want to be a hero, you can:
  // 0. Track "overflow" entries
  // 1. Invalidate the thread descriptors held by the threads
  // 2. Delete[] the descriptor array
  // 3. Iterate over the overflow entries and delete them
}

template <class Data> Data *TBB_ThreadManager<Data>::RegisterCurrentThread() {
  assert(m_object_exists);
  if (!m_object_exists) {
    return nullptr;
  }

  unsigned int myEntry = m_uiNumberOfStaticEntries;
  if (!m_bOverflowed) {
    myEntry = (unsigned int)m_nextFreeEntry++;
    if (myEntry >= m_uiNumberOfStaticEntries) {
      m_bOverflowed = true;
    }
  }
  // m_bOverflowed is a sticky flag indicating entries need to be allocated on
  // demand The point is to prevent write accesses to a shared location and save
  // atomic operations Once we've overflowed, we don't use the atomic counter
  // anymore, nor do we write to the bool itself

  if (myEntry < m_uiNumberOfStaticEntries) {
    m_CurrentThreadGlobalID = m_DescriptorsArray + myEntry;
  } else {
    // Overflow, allocate on demand
    // These are not tracked anywhere so they leak, but we don't have to
    // synchronize anything so hurray
    m_CurrentThreadGlobalID = new TBB_ThreadDescriptor<Data>;
  }

  m_CurrentThreadGlobalID->m_data.thread_attach();
  return &(m_CurrentThreadGlobalID->m_data);
}

template <class Data> void TBB_ThreadManager<Data>::UnregisterCurrentThread() {
  // Intentional leak to avoid allocate/deallocate whenever threads join/leave
  // an arena.
}

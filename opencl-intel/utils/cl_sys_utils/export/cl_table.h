// INTEL CONFIDENTIAL
//
// Copyright 2008-2018 Intel Corporation.
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

#if !defined(__CL_TABLE_H__)
#define __CL_TABLE_H__

#include "cl_synch_objects.h"

namespace Intel { namespace OpenCL { namespace Utils {

/***********************************************************************
 * This class implements table for void pointers.
 * Objects can be inserted to the table. New object is assigned with unique
 * key that represents its id in the list and the table id.
 * Object can be queried by its id and return in O(1).
 * An object that is removed, is marked as deleted and will never be used.
 * Any query on this key will return 0 value.
 * To iterate the map, use GetIndexByID. This function return false when Key is bigger
 * then the last available key
 * If a new object exceeds the table initial size load factor, the table is rehashed.
 * and its size is doubled.
 * This object is thread safe!
 *
 ***********************************************************************/
class ClTable
{
public:
    ClTable( unsigned short usTableSize = 0xffe  , float fLoadFactor = 0.9 );
    virtual ~ClTable();

    unsigned long   Insert    ( void* pObject );
    bool            Remove    ( unsigned long  key);
    bool            Contains  ( unsigned long  key);
    void*           Get       ( unsigned long  key );
    bool            GetByIndex( unsigned short idx, void** ppObject );
    unsigned short  GetTableId() const  { return m_usTableId; }

private:
    void**          m_table;        // Pointer to the hash table
    unsigned short  m_usTableSize;  // The table size
    unsigned short  m_usTableId;    // Table unique ID
    unsigned short  m_usObjectIdx;  // The index of the next object to be inserted
    float           m_fLoadFactor;  // The rehase load factor
    OclSpinMutex	m_muTable;      // Mutex that lock the table for thread safe implementation.

    static unsigned short m_susTableCounter;  // holds the id of the next table

    // Private methods
    void            Rehase();

    // Prevent compiler from generating defaults
    ClTable(const ClTable&);           // copy constructor
    ClTable& operator=(const ClTable&);// assignment operator
};

}}}    // Intel::OpenCL::Utils
#endif // __CL_TABLE_H__

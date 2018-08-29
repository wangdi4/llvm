// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
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

#include "cl_table.h"
#include <malloc.h>

using namespace Intel::OpenCL::Utils;

#define KEY_2_INDEX(key)  \
    (0x0000ffff & (key))

#define KEY_2_TABLE_ID(key)  \
    ((unsigned short)((0xffff0000 & (key)) >> 16 ))

#define INDEX_2_KEY(idx)  \
    ((m_usTableId << 16) | idx )

unsigned short ClTable::m_susTableCounter = 1;

/************************************************************************
 *
 ************************************************************************/
ClTable::ClTable( unsigned short usTableSize, float fLoadFactor):
    m_usTableSize(usTableSize),
    m_usTableId(m_susTableCounter),
    m_usObjectIdx(0),
    m_fLoadFactor(fLoadFactor)
{
    m_susTableCounter++;
    m_table = (void**)calloc(m_usTableSize, sizeof(void*));
}

/************************************************************************
 *
 ************************************************************************/
ClTable::~ClTable()
{
    free(m_table);
    m_usTableSize = 0;
    m_usTableId = 0;
    m_usObjectIdx = 0;
    m_fLoadFactor = 0;
}

/************************************************************************
 *
 ************************************************************************/
unsigned long ClTable::Insert( void* pObject )
{
    OclAutoMutex CS(&m_muTable);
    unsigned long key = INDEX_2_KEY(m_usObjectIdx);
    m_table[m_usObjectIdx] = pObject;
    m_usObjectIdx++;

    if( ((float)m_usObjectIdx/(float)m_usTableSize) > m_fLoadFactor )
    {
        Rehase();
    }

    return key;
}

/************************************************************************
 * Remove item according to key. Return true on success
 ************************************************************************/
bool ClTable::Remove( unsigned long key)
{
    OclAutoMutex CS(&m_muTable);
    unsigned short idx = KEY_2_INDEX(key);
    unsigned short tableId = KEY_2_TABLE_ID(key);
    if ( idx >= m_usObjectIdx || tableId != m_usTableId)
    {
        return false;
    }
    // Remove only by setting to 0;
    m_table[idx] = 0;
    return true;
}

/************************************************************************
 * Return true only if table contains the input key
 ************************************************************************/
bool ClTable::Contains( unsigned long key)
{
    OclAutoMutex CS(&m_muTable);
    unsigned short idx = KEY_2_INDEX(key);
    unsigned short tableId = KEY_2_TABLE_ID(key);
    if ( idx >= m_usObjectIdx || tableId != m_usTableId)
    {
        return false;
    }
    if ( 0 == m_table[idx])
    {
        return false;
    }
    return true;
}

/************************************************************************
 * Returns the object in slot "key", if slot is empty or key doesn't exists
 * 0 value is returned.
 ************************************************************************/
void* ClTable::Get( unsigned long key )
{
    OclAutoMutex CS(&m_muTable);
    unsigned short idx = KEY_2_INDEX(key);
    unsigned short tableId = KEY_2_TABLE_ID(key);
    if ( idx >= m_usObjectIdx || tableId != m_usTableId)
    {
        return 0;
    }
    return m_table[idx];
}

/************************************************************************
 *
 ************************************************************************/
bool ClTable::GetByIndex( unsigned short idx, void** ppObject )
{
    OclAutoMutex CS(&m_muTable);
    if ( idx >= m_usObjectIdx )
    {
        *ppObject = 0;
        return false;
    }

    *ppObject = m_table[idx];
    return true;
}

/************************************************************************
 *
 ************************************************************************/
void ClTable::Rehase()
{
    unsigned short usOldSize = m_usTableSize;
    void** oldTable = m_table;
    m_usTableSize <<= 1;
    m_table = (void**)calloc(m_usTableSize, sizeof(void*));
	if(0 == m_table)
	{
		return;
	}
	for( unsigned short i=0; i<usOldSize; i++)
    {
         m_table[i] = oldTable[i];
    }
    free(oldTable);
}

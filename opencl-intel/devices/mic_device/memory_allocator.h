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
//  MemoryAllocator.h
//  Implementation of the Class MemoryAllocator
//  Created on:      16-Dec-2008 4:54:53 PM
//  Original author: efiksman
///////////////////////////////////////////////////////////

#pragma once

#include <cl_device_api.h>
#include <cl_types.h>
#include <cl_heap.h>
#include <cl_synch_objects.h>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <common/COITypes_common.h>

#include "cl_synch_objects.h"
#include "mic_config.h"
#include "notification_port.h"

using namespace Intel::OpenCL::Utils;


namespace Intel { namespace OpenCL { namespace MICDevice {

// used by commands, Struct which represent single map handle.
struct SMemMapParams
{
	SMemMapParams() {}

	virtual ~SMemMapParams()
	{
		m_mapHandlesInstances.clear();
	}

	// Insert a new COIMAPINSTANCE to the set. (In case of regular map command will be only one, In case of rectangular map can be more than one)
	bool insertMapHandle(COIMAPINSTANCE& mapInstance)
	{
		pair<set<COIMAPINSTANCE>::iterator,bool> ret = m_mapHandlesInstances.insert(mapInstance);
		return ret.second;
	}

	// Initialize iterator to the beginning of this COIMAPINSTANCE set.
	void initMapHandleIterator()
	{
		m_mapHandlesIterator = m_mapHandlesInstances.begin();
	}

	// Return true if the current location of the iterator is not located at the end of this set.
	// Must call to "initMapHandleIterator()" before using this method.
	bool hasNextMapHandle()
	{
		return m_mapHandlesIterator != m_mapHandlesInstances.end();
	}

	// Return COIMAPINSTANCE which pointed by current iterator location and advance the iterator to the next item.
	// Must validate that there is next map handle by calling to "hasNextMapHandle()".
	COIMAPINSTANCE getNextMapHandle()
	{
		assert(m_mapHandlesIterator != m_mapHandlesInstances.end());
		set<COIMAPINSTANCE>::iterator pCurrInstance = m_mapHandlesIterator;
		m_mapHandlesIterator ++;
		return (*pCurrInstance);
	}

	// Return the size of the COIMAPINSTANCEs set.
	size_t getNumMapInstances() 
	{
		return m_mapHandlesInstances.size();
	}

private:
	set<COIMAPINSTANCE> m_mapHandlesInstances;
	set<COIMAPINSTANCE>::iterator m_mapHandlesIterator;
};

// Struct which contains a list of "SMemMapParams" because many map operations with the same address use the same handle so We should keep all of them.
struct SMemMapParamsList
{
	SMemMapParamsList() {}

	virtual ~SMemMapParamsList()
	{
		m_memMapParamsList.clear();
	}

	// push SMemMapParams to the list
	void push(SMemMapParams& memMapParams)
	{
		m_spinMutex.Lock();
		m_memMapParamsList.push_front(memMapParams);
		m_spinMutex.Unlock();
	}

	// remove and return one of the instances in the list. (If exists otherwise return false)
	bool pop(SMemMapParams* pMemMapParams)
	{
		assert(pMemMapParams);
		m_spinMutex.Lock();
		bool ret = (m_memMapParamsList.size() > 0);
		if (ret)
		{
			*pMemMapParams = m_memMapParamsList.back();
			m_memMapParamsList.pop_back();
		}
		m_spinMutex.Unlock();
		return ret;
	}

private:
	list<SMemMapParams> m_memMapParamsList;
	// Guard to the list, use spin mutex because the accessibility to this list from more than one thread is rare.
	OclSpinMutex m_spinMutex;
};


class DeviceServiceCommunication;
class MICDevice;
class CommandList;

class MemoryAllocator
{

public:
    // singleton
    static MemoryAllocator* getMemoryAllocator( cl_int devId,
                                                IOCLDevLogDescriptor *pLogDesc,
                                                unsigned long long maxBufferAllocSize );

    // delete singleton if required
    void Release(void);

    //Image Info Function
    cl_dev_err_code GetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet);
    cl_dev_err_code GetAllocProperties( cl_mem_object_type IN memObjType,    cl_dev_alloc_prop* OUT pAllocProp );
    // Create/Release functions
    cl_dev_err_code CreateObject( cl_dev_subdevice_id node_id,
                            cl_mem_flags flags, const cl_image_format* format,
                            size_t    dim_count, const size_t* dim,
                            IOCLDevRTMemObjectService*    pRTMemObjService,
                            IOCLDevMemoryObject* *memObj );

    // Utility functions
    static size_t CalculateOffset(cl_uint dim_count, const size_t* origin, const size_t* pitch, size_t elemSize);
    static SMemMapParamsList* GetCoiMapParams( cl_dev_cmd_param_map* pMapParams ) { return (SMemMapParamsList*)pMapParams->map_handle; };

    bool Use_2M_Pages_Enabled( void ) const { return (m_2M_BufferMinSize > 0); };
    bool Use_2M_Pages( size_t size ) const { return Use_2M_Pages_Enabled() && (size >= m_2M_BufferMinSize); };
    bool ImmediateTransferForced( void ) const { return m_force_immediate_transfer; };

private:
    friend class MICDevMemoryObject;
    friend class MICDevMemorySubObject;

    IOCLDevLogDescriptor*       GetLogDescriptor( void ) { return m_pLogDescriptor; };
    cl_int                      GetLogHandle( void ) { return m_iLogHandle; };

    size_t GetElementSize(const cl_image_format* format);
    cl_int                   m_iDevId;
    IOCLDevLogDescriptor*    m_pLogDescriptor;
    cl_int                   m_iLogHandle;

    size_t                   m_maxAllocSize;

    size_t                   m_2M_BufferMinSize;
    bool                     m_force_immediate_transfer;

    // singleton
    MemoryAllocator(cl_int devId, IOCLDevLogDescriptor *pLogDesc, unsigned long long maxAllocSize );
    virtual ~MemoryAllocator();

    static OclMutex*         m_instance_guard;
    static MemoryAllocator*  m_the_instance;
    static cl_uint           m_instance_referencies; // release when reaches 0

    class StaticInitializer;
    static StaticInitializer init_statics;
};

class MICDevMemoryObject : public IOCLDevMemoryObject, public NotificationPort::CallBack
{
public:
    friend class MICDevMemorySubObject;

    MICDevMemoryObject(MemoryAllocator& allocator,
                       cl_dev_subdevice_id nodeId, cl_mem_flags memFlags,
                       IOCLDevRTMemObjectService*    pRTMemObjService);

    cl_dev_err_code Init();

    // CreateMappedRegion should not perform real data mapping - just return a pointer to the returned memory
    // to be filled later
    cl_dev_err_code clDevMemObjCreateMappedRegion( cl_dev_cmd_param_map*  pMapParams );
    cl_dev_err_code clDevMemObjReleaseMappedRegion( cl_dev_cmd_param_map* pMapParams );

    cl_dev_err_code clDevMemObjUpdateBackingStore( 
	                            void* operation_handle, cl_dev_bs_update_state* pUpdateState );
    cl_dev_err_code clDevMemObjUpdateFromBackingStore(
                                void* operation_handle, cl_dev_bs_update_state* pUpdateState );
    cl_dev_err_code clDevMemObjInvalidateData( );

    cl_dev_err_code clDevMemObjRelease();
    cl_dev_err_code clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle *handle);
    const cl_mem_obj_descriptor& clDevMemObjGetDescriptorRaw() const { return m_objDescr; };

    cl_dev_err_code clDevMemObjCreateSubObject( cl_mem_flags mem_flags,
                    const size_t *origin, const size_t *size, 
                    IOCLDevRTMemObjectService IN *pBSService,
                    IOCLDevMemoryObject** ppSubObject );

    const COIBUFFER& clDevMemObjGetCoiBufferHandler() const { return m_coi_buffer; };
    const COIBUFFER& clDevMemObjGetTopLevelCoiBufferHandler() const { return m_coi_top_level_buffer; };

    const cl_mem_flags& clDevMemObjGetMemoryFlags() const { return m_memFlags; };

    // NotificationPort::CallBack
    void fireCallBack(void* arg);

    //
    // internal methods
    //

    // return new count in both inc/dec
    unsigned long   IncWriteMapsCount( void )       { return (unsigned long)(++m_write_maps_count); };
    unsigned long   DecWriteMapsCount( void )       { return (unsigned long)(--m_write_maps_count); };
    unsigned long   GetWriteMapsCount( void ) const { return (unsigned long)(m_write_maps_count); };
    bool            ImmediateTransferForced( void ) const { return m_Allocator.ImmediateTransferForced(); };
    size_t          GetRawDataSize( void )    const { return m_raw_size; };

    typedef vector<COIPROCESS>  COI_ProcessesArray;
    COI_ProcessesArray&	get_active_processes( void ) { return m_buffActiveProcesses; };

    /* Check if ptr is mapped OCL Buffer, if yes store in *ppOutMemObj the appropriate MICDevMemoryObject */
    static bool getMemObjFromMapBuffersPool(void* ptr, size_t size, MICDevMemoryObject** ppOutMemObj);

    /* If u get MICDevMemoryObject from 'getMemObjFromMapBuffersPool()' return it back when u finish to use it. */
    void returnMemObjToMapBuffersPool() { decRefCounter(); };

    /* Add each OCL Bufer that mapped to m_buffersMemoryPool */
    void addMemObjToMapBuffersPool() { m_buffersMemoryPool.addBufferToPool(this); };

    /* Remove this object from m_buffersMemoryPool */
    void removeMemObjFromMapBuffersPool() { m_buffersMemoryPool.removeBufferFromPool(this); };

    /* Set this memObj that already inserted to m_buffersMemoryPool, ready to use */
    void setMemObjInMapBuffersPoolReady( CommandList* cur_queue ) { m_buffersMemoryPool.setBufferReady(this, cur_queue); };

    /* Return true if this mem object is root buffer */
    virtual bool isRootBuffer() { return true; };
    
protected:
    MICDevMemoryObject(MemoryAllocator& allocator) : m_Allocator(allocator),
            m_nodeId(NULL), m_memFlags(0), m_coi_buffer(0) {}

    MemoryAllocator&            m_Allocator;

    // Object Management
    cl_dev_subdevice_id         m_nodeId;

    cl_mem_flags                m_memFlags;
    IOCLDevRTMemObjectService*  m_pRTMemObjService;
    IOCLDevBackingStore*        m_pBackingStore;

    cl_mem_obj_descriptor       m_objDescr;
    size_t                      m_raw_size;

    COIBUFFER                   m_coi_buffer;
    AtomicCounter               m_write_maps_count;

    // sub-buffer support
    // TODO: Why it's not part of the sub-buffer?
    COIBUFFER                   m_coi_top_level_buffer;
    size_t                      m_coi_top_level_buffer_offset;

    COI_ProcessesArray          get_active_processes_int( void );
    MICDevice*                  get_owning_device( void );

private:

    // Inc this buffer memory counter.
    void incRefCounter();

    // Dec this buffer memory counter.
    void decRefCounter();

    ~MICDevMemoryObject() {};

    struct MapBuffersMemoryPool
    {
    public:

        bool getBuffer(void* ptr, size_t size, MICDevMemoryObject** ppOutMemObj);

        void addBufferToPool(MICDevMemoryObject* pMicMemObj);

        void removeBufferFromPool(MICDevMemoryObject* pMicMemObj);
    
        void setBufferReady(MICDevMemoryObject* pMicMemObj, CommandList* cur_queue);

    private:

        struct mem_obj_directive
        {
          mem_obj_directive(MICDevMemoryObject* memObj) : pMemObj(memObj), refCounter(1), isReady(false) {};
          MICDevMemoryObject* pMemObj;
          size_t				refCounter;
          bool				isReady;
        };

        map<void*, mem_obj_directive>	m_addressToMemObj;
        OclReaderWriterLock		m_multiReadSingleWriteMutex;
    };

    COI_ProcessesArray	m_buffActiveProcesses;

    AtomicCounter	m_bufferRefCounter;

    static MapBuffersMemoryPool m_buffersMemoryPool;
};

class MICDevMemorySubObject : public MICDevMemoryObject
{
public:
    MICDevMemorySubObject(MemoryAllocator& allocator, MICDevMemoryObject& pParent);

    cl_dev_err_code Init(cl_mem_flags mem_flags, const size_t *origin, const size_t *size, IOCLDevRTMemObjectService IN *pBSService);

    virtual bool isRootBuffer() { return false; };

protected:
    MICDevMemoryObject& m_Parent;
};

}}}


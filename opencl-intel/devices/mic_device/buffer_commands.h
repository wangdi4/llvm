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

#pragma once

#include "command.h"
#include "source/COIEvent_source.h"

#include <vector>

using namespace std;

namespace Intel { namespace OpenCL { namespace MICDevice {

/* Template class which process memory chunks, T is a chunk which must have the following methods:
      - bool isReadyToFire() - return true if this chunk is ready for process.
	  - void reset() - reset the current chunk. (Call it after firing this chunk) */
template <class T>
class ProcessMemoryChunk
{
public:
    /* The first call does not call to 'fire_current_chunk()', it only set the values in 'm_current_chunk'.
	   That means that We must call to 'process_finish()' as the last command in order to execute the last chunk. */
    virtual void process_chunk( T& chunk ) = 0;
	/* Must call 'process_finish()' in order to execute the last chunk. */
    virtual void process_finish( void );

    // called after dispatching all chunks
	// Return the last COIEVENT, created by this memory command.
    COIEVENT get_last_event( void ) const { return m_last_dependency; };
    bool     error_occured( void ) const { return m_error_occured; };
	// Return true if some work dispatched.
    bool     work_dispatched( void ) const { return (NOT_INIT != m_state); };

    ProcessMemoryChunk( const COIEVENT* external_dependency ) :
        m_external_dependency( external_dependency ), m_state(NOT_INIT), m_error_occured(false) {};

    virtual ~ProcessMemoryChunk() {};

protected:

    /* function to fire actual COI calls
       return success */
    virtual bool fire_action( const T& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event  ) = 0;

	/* Fire the last registered chunk.
	   force - If true fire the chunk with dependency on all previous dependecies, otherwise dependent only on 'm_external_dependency'. */
	void fire_current_chunk( bool force );

	// The last registered chunk.
	T	                m_current_chunk;

private:
    enum State
    {
        NOT_INIT = 0,
        SINGLE_CHUNK_MODE,
        MULTI_CHUNK_MODE
    };

    typedef vector<COIEVENT>   DependeciesArray;

    DependeciesArray    m_dependencies;
    COIEVENT            m_last_dependency;
    const COIEVENT*     m_external_dependency;
    State               m_state;
    bool                m_error_occured;
};

// Chunk struct for common buffer commands.
namespace CommonMemoryChunk
{
	struct Chunk
	{
		size_t from_offset;
		size_t to_offset;
		size_t size;

		Chunk() : from_offset(0), to_offset(0), size(0) {};

		Chunk(size_t from, size_t to, size_t sz) : from_offset(from), to_offset(to), size(sz) {};

		bool isReadyToFire() { return (size > 0); };

		void reset() { size = 0; };
	};
}

/* Class for processing memory chunks of common buffer commands. */
class ProcessCommonMemoryChunk : public ProcessMemoryChunk<CommonMemoryChunk::Chunk>
{
public:
    /* called from inside calculateCopyRegion.
	   The first call does not call to 'fire_current_chunk()', it only set the values in 'm_current_chunk'.
	   That means that We must call to 'process_finish()' as the last command in order to execute the last chunk. */
    virtual void process_chunk( CommonMemoryChunk::Chunk& chunk );

	ProcessCommonMemoryChunk( const COIEVENT* external_dependency ) : ProcessMemoryChunk<CommonMemoryChunk::Chunk>(external_dependency), 
        m_readyToFireChunk(false) {};

	virtual ~ProcessCommonMemoryChunk() {};

private:

	bool				m_readyToFireChunk;

};


// Chunk struct for unmap buffer command.
namespace UnmapMemoryChunkStruct
{
	struct Chunk
	{
		COIMAPINSTANCE coi_map_instance;

		Chunk() : coi_map_instance(NULL) {};

		Chunk(COIMAPINSTANCE map_instamce) : coi_map_instance(map_instamce) {};

		bool isReadyToFire() { return (NULL != coi_map_instance); };

		void reset() { coi_map_instance = NULL; };
	};
}


/* Class for processing memory chunks of unmap buffer command. */
class ProcessUnmapMemoryChunk : public ProcessMemoryChunk<UnmapMemoryChunkStruct::Chunk>
{
public:
    /* called from inside UnmapMemObject::execute().
	   The first call does not call to 'fire_current_chunk()', it only set the values in 'm_current_chunk'.
	   That means that We must call to 'process_finish()' as the last command in order to execute the last chunk. */
    virtual void process_chunk( UnmapMemoryChunkStruct::Chunk& chunk );

	ProcessUnmapMemoryChunk( const COIEVENT* external_dependency ) : ProcessMemoryChunk<UnmapMemoryChunkStruct::Chunk>(external_dependency), 
        m_readyToFireChunk(false) {};

	virtual ~ProcessUnmapMemoryChunk() {};

private:

	bool				m_readyToFireChunk;

};


class BufferCommands : public Command
{
protected:

	// struct that define the copy info in order to calculate the offsets
	struct mem_copy_info_struct
	{
		cl_uint	    uiDimCount;
		size_t		from_Offset;
		size_t		vFromPitch[MAX_WORK_DIM-1];
		size_t		to_Offset;
		size_t		vToPitch[MAX_WORK_DIM-1];
		size_t		vRegion[MAX_WORK_DIM];
	};

	BufferCommands(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

	virtual ~BufferCommands();

	void CopyRegion( mem_copy_info_struct* pMemCopyInfo, ProcessCommonMemoryChunk* chunk_consumer );
private:
	void CopyRegionInternal( mem_copy_info_struct* pMemCopyInfo, ProcessCommonMemoryChunk* chunk_consumer );

};

class ReadWriteMemObject : public BufferCommands
{

public:

	/* static function for ReadWriteMemObject Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand);

	cl_dev_err_code execute();

private:

	/* Private constructor because We like to create Commands only by the factory method */
    ReadWriteMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

};

class CopyMemObject : public BufferCommands
{

public:

	/* static function for CopyMemObject Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand);

	cl_dev_err_code execute();

private:

	/* Private constructor because We like to create Commands only by the factory method */
    CopyMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

};

class MapMemObject : public BufferCommands
{

public:

	/* static function for MapMemObject Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand);

	cl_dev_err_code execute();

private:

	/* Private constructor because We like to create Commands only by the factory method */
    MapMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

};

class UnmapMemObject : public BufferCommands
{

public:

	/* static function for UnmapMemObject Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand);

	cl_dev_err_code execute();

private:

	/* Private constructor because We like to create Commands only by the factory method */
    UnmapMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

};

}}}


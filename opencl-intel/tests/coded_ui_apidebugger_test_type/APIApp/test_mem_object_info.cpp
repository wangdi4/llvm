/******************************************************************
 //
 //  OpenCL Conformance Tests
 // 
 //  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
 //
 ******************************************************************/

#include "testBase.h"
#include "harness/testHarness.h"

extern cl_uint gRandomSeed;


#define TEST_MEM_OBJECT_PARAM( mem, paramName, val, expected, name, type, cast )    \
error = clGetMemObjectInfo( mem, paramName, sizeof( val ), &val, &size );   \
test_error( error, "Unable to get mem object " name );  \
if( val != expected )   \
{   \
log_error( "ERROR: Mem object " name " did not validate! (expected " type ", got " type " from %s:%d)\n",   \
expected, (cast)val, __FILE__, __LINE__ );   \
return -1;  \
}   \
if( size != sizeof( val ) ) \
{   \
log_error( "ERROR: Returned size of mem object " name " does not validate! (expected %d, got %d from %s:%d)\n", \
(int)sizeof( val ), (int)size , __FILE__, __LINE__ );   \
return -1;  \
}

static void CL_CALLBACK mem_obj_destructor_callback( cl_mem, void * data )
{
    free( data );
}



int test_get_buffer_info( cl_device_id deviceID, cl_context context, cl_command_queue ignoreQueue, int num_elements )
{
    int error;
    size_t size;
    void * buffer = NULL;
    
    clMemWrapper bufferObject;
    clMemWrapper subBufferObject;
    
    cl_mem_flags bufferFlags[] = {
        /*CL_MEM_READ_WRITE,
        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,*/
        CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
        /*CL_MEM_READ_ONLY,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
        CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        CL_MEM_WRITE_ONLY,
        CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
        CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
        CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_WRITE,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_ONLY,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY,
        CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_WRITE,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_WRITE_ONLY,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_WRITE,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_ONLY,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_WRITE_ONLY,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,*/
    };
    
    cl_mem_flags subBufferFlags[] = {
        CL_MEM_READ_WRITE,
        /*CL_MEM_READ_ONLY,
        CL_MEM_WRITE_ONLY,
        0,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_WRITE,
        CL_MEM_HOST_READ_ONLY | CL_MEM_READ_ONLY,
        CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY,
        CL_MEM_HOST_READ_ONLY | 0,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_WRITE,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY,
        CL_MEM_HOST_WRITE_ONLY | CL_MEM_WRITE_ONLY,
        CL_MEM_HOST_WRITE_ONLY | 0,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_WRITE,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_ONLY,
        CL_MEM_HOST_NO_ACCESS | CL_MEM_WRITE_ONLY,
        CL_MEM_HOST_NO_ACCESS | 0,*/
    };
    
    
    // Get the address alignment, so we can make sure the sub-buffer test later works properly.
    cl_uint addressAlignBits;
    error = clGetDeviceInfo( deviceID, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(addressAlignBits), &addressAlignBits, NULL );
  
    size_t addressAlign = addressAlignBits/8;
    if ( addressAlign < 128 )
    {
        addressAlign = 128;
    }
    
    for ( unsigned int i = 0; i < sizeof(bufferFlags) / sizeof(cl_mem_flags); ++i )
    {
        //printf("@@@ bufferFlags[%u]=0x%x\n", i, bufferFlags[ i ]);
        if ( bufferFlags[ i ] & CL_MEM_USE_HOST_PTR )
        {
            // Create a buffer object to test against.
            buffer = malloc( addressAlign * 4 );
            bufferObject = clCreateBuffer( context, bufferFlags[ i ], addressAlign * 4, buffer, &error );
            if ( error )
            {
                free( buffer );
                test_error( error, "Unable to create buffer (CL_MEM_USE_HOST_PTR) to test with" );
            }
            
            // Make sure buffer is cleaned up appropriately if we encounter an error in the rest of the calls.
            error = clSetMemObjectDestructorCallback( bufferObject, NULL, buffer );
            error = clSetMemObjectDestructorCallback( bufferObject, mem_obj_destructor_callback, buffer );
            test_error( error, "Unable to set mem object destructor callback" );
            
            void * ptr;
            TEST_MEM_OBJECT_PARAM( bufferObject, CL_MEM_HOST_PTR, ptr, buffer, "host pointer", "%p", void * )
        }
        else if ( (bufferFlags[ i ] & CL_MEM_ALLOC_HOST_PTR) && (bufferFlags[ i ] & CL_MEM_COPY_HOST_PTR) )
        {
            // Create a buffer object to test against.
            buffer = malloc( addressAlign * 4 );
            bufferObject = clCreateBuffer( context, bufferFlags[ i ], addressAlign * 4, buffer, &error );
            if ( error )
            {
                free( buffer );
                test_error( error, "Unable to create buffer (CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR) to test with" );
            }
            
            // Make sure buffer is cleaned up appropriately if we encounter an error in the rest of the calls.
            error = clSetMemObjectDestructorCallback( bufferObject, mem_obj_destructor_callback, buffer );
            test_error( error, "Unable to set mem object destructor callback" );
        }
        else if ( bufferFlags[ i ] & CL_MEM_ALLOC_HOST_PTR )
        {
            // Create a buffer object to test against.
            bufferObject = clCreateBuffer( context, bufferFlags[ i ], addressAlign * 4, NULL, &error );
            test_error( error, "Unable to create buffer (CL_MEM_ALLOC_HOST_PTR) to test with" );
        }
        else if ( bufferFlags[ i ] & CL_MEM_COPY_HOST_PTR )
        {
            // Create a buffer object to test against.
            buffer = malloc( addressAlign * 4 );
            bufferObject = clCreateBuffer( context, bufferFlags[ i ], addressAlign * 4, buffer, &error );
            if ( error )
            {
                free( buffer );
                test_error( error, "Unable to create buffer (CL_MEM_COPY_HOST_PTR) to test with" );
            }
            
            // Make sure buffer is cleaned up appropriately if we encounter an error in the rest of the calls.
            error = clSetMemObjectDestructorCallback( bufferObject, mem_obj_destructor_callback, buffer );
            test_error( error, "Unable to set mem object destructor callback" );
        }
        else
        {
            // Create a buffer object to test against.
            bufferObject = clCreateBuffer( context, bufferFlags[ i ], addressAlign * 4, NULL, &error );
            test_error( error, "Unable to create buffer to test with" );
        }
        
        // Perform buffer object queries.
        cl_mem_object_type type;
        TEST_MEM_OBJECT_PARAM( bufferObject, CL_MEM_TYPE, type, CL_MEM_OBJECT_BUFFER, "type", "%d", int )
        
        cl_mem_flags flags;
        TEST_MEM_OBJECT_PARAM( bufferObject, CL_MEM_FLAGS, flags, (unsigned int)bufferFlags[ i ], "flags", "%d", unsigned int )
        
        size_t sz;
        TEST_MEM_OBJECT_PARAM( bufferObject, CL_MEM_SIZE, sz, (size_t)( addressAlign * 4 ), "size", "%ld", size_t )
        
        cl_uint mapCount;
        error = clGetMemObjectInfo( bufferObject, CL_MEM_MAP_COUNT, NULL, &mapCount, &size );
        error = clGetMemObjectInfo( bufferObject, CL_MEM_MAP_COUNT, sizeof( mapCount ), &mapCount, &size );
        test_error( error, "Unable to get mem object map count" );
        if( size != sizeof( mapCount ) )
        {
            log_error( "ERROR: Returned size of mem object map count does not validate! (expected %d, got %d from %s:%d)\n",
                      (int)sizeof( mapCount ), (int)size, __FILE__, __LINE__ );
            return -1;
        }
        
        cl_uint refCount;
        error = clGetMemObjectInfo( bufferObject, CL_MEM_REFERENCE_COUNT, sizeof( refCount ), &refCount, &size );
        test_error( error, "Unable to get mem object reference count" );							
        if( size != sizeof( refCount ) )
        {
            log_error( "ERROR: Returned size of mem object reference count does not validate! (expected %d, got %d from %s:%d)\n",
                      (int)sizeof( refCount ), (int)size, __FILE__, __LINE__ );
            return -1;	
        }
        
        cl_context otherCtx;
        TEST_MEM_OBJECT_PARAM( bufferObject, CL_MEM_CONTEXT, otherCtx, context, "context", "%p", cl_context )
        
        cl_mem origObj;
        TEST_MEM_OBJECT_PARAM( bufferObject, CL_MEM_ASSOCIATED_MEMOBJECT, origObj, (void *)NULL, "associated mem object", "%p", void * )
        
        size_t offset;
        TEST_MEM_OBJECT_PARAM( bufferObject, CL_MEM_OFFSET, offset, 0L, "offset", "%ld", size_t )
        
        cl_buffer_region region;
        region.origin = addressAlign;
        region.size = addressAlign;
        
        // Loop over possible sub-buffer objects to create.
        for ( unsigned int j = 0; j < sizeof(subBufferFlags) / sizeof(cl_mem_flags); ++j )
        {
            if ( subBufferFlags[ j ] & CL_MEM_READ_WRITE )
            {
                if ( !(bufferFlags[ i ] & CL_MEM_READ_WRITE) )
                    continue; // Buffer must be read_write for sub-buffer to be read_write.
            }
            if ( subBufferFlags[ j ] & CL_MEM_READ_ONLY )
            {
                if ( !(bufferFlags[ i ] & CL_MEM_READ_WRITE) && !(bufferFlags[ i ] & CL_MEM_READ_ONLY) )
                    continue; // Buffer must be read_write or read_only for sub-buffer to be read_only
            }
            if ( subBufferFlags[ j ] & CL_MEM_WRITE_ONLY )
            {
                if ( !(bufferFlags[ i ] & CL_MEM_READ_WRITE) && !(bufferFlags[ i ] & CL_MEM_WRITE_ONLY) )
                    continue; // Buffer must be read_write or write_only for sub-buffer to be write_only
            }
            if ( subBufferFlags[ j ] & CL_MEM_HOST_READ_ONLY )
            {
                if ( (bufferFlags[ i ] & CL_MEM_HOST_NO_ACCESS) || (bufferFlags[ i ] & CL_MEM_HOST_WRITE_ONLY) )
                    continue; // Buffer must be host all access or host read_only for sub-buffer to be host read_only
            }
            if ( subBufferFlags[ j ] & CL_MEM_HOST_WRITE_ONLY )
            {
                if ( (bufferFlags[ i ] & CL_MEM_HOST_NO_ACCESS) || (bufferFlags[ i ] & CL_MEM_HOST_READ_ONLY) )
                    continue; // Buffer must be host all access or host write_only for sub-buffer to be host write_only
            }
            //printf("@@@ bufferFlags[%u]=0x%x subBufferFlags[%u]=0x%x\n", i, bufferFlags[ i ], j, subBufferFlags[ j ]);
            
            subBufferObject = clCreateSubBuffer( bufferObject, subBufferFlags[ j ], CL_BUFFER_CREATE_TYPE_REGION, NULL, &error );
            subBufferObject = clCreateSubBuffer( bufferObject, subBufferFlags[ j ], CL_BUFFER_CREATE_TYPE_REGION, &region, &error );
            test_error( error, "Unable to create sub-buffer to test against" );
            
            // Perform sub-buffer object queries.
            cl_mem_object_type type;
            TEST_MEM_OBJECT_PARAM( subBufferObject, CL_MEM_TYPE, type, CL_MEM_OBJECT_BUFFER, "type", "%d", int )
            
            cl_mem_flags flags;
            cl_mem_flags inheritedFlags = subBufferFlags[ j ];
            if ( (subBufferFlags[ j ] & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)) == 0 )
            {
              inheritedFlags |= bufferFlags[ i ] & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY);
            }
            inheritedFlags |= bufferFlags[ i ] & (CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR);
            if ( (subBufferFlags[ j ] & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) == 0)
            {
              inheritedFlags |= bufferFlags[ i ] & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS);
            }
            TEST_MEM_OBJECT_PARAM( subBufferObject, CL_MEM_FLAGS, flags, (unsigned int)inheritedFlags, "flags", "%d", unsigned int )
            
            TEST_MEM_OBJECT_PARAM( subBufferObject, CL_MEM_SIZE, sz, (size_t)( addressAlign ), "size", "%ld", size_t )
            
            if ( bufferFlags[ i ] & CL_MEM_USE_HOST_PTR )
            {
                void * ptr;
                void * offsetInBuffer = (char *)buffer + addressAlign;
                
                TEST_MEM_OBJECT_PARAM( subBufferObject, CL_MEM_HOST_PTR, ptr, offsetInBuffer, "host pointer", "%p", void * )
            }
            
            cl_uint mapCount;
            error = clGetMemObjectInfo( subBufferObject, CL_MEM_MAP_COUNT, sizeof( mapCount ), &mapCount, &size );
            test_error( error, "Unable to get mem object map count" );
            if( size != sizeof( mapCount ) )
            {
                log_error( "ERROR: Returned size of mem object map count does not validate! (expected %d, got %d from %s:%d)\n",
                          (int)sizeof( mapCount ), (int)size, __FILE__, __LINE__ );
                return -1;
            }
            
            cl_uint refCount;
            error = clGetMemObjectInfo( subBufferObject, CL_MEM_REFERENCE_COUNT, sizeof( refCount ), &refCount, &size );
            test_error( error, "Unable to get mem object reference count" );
            if( size != sizeof( refCount ) )
            {
                log_error( "ERROR: Returned size of mem object reference count does not validate! (expected %d, got %d from %s:%d)\n",
                          (int)sizeof( refCount ), (int)size, __FILE__, __LINE__ );
                return -1;
            }
            
            cl_context otherCtx;
            TEST_MEM_OBJECT_PARAM( subBufferObject, CL_MEM_CONTEXT, otherCtx, context, "context", "%p", cl_context )
            
            TEST_MEM_OBJECT_PARAM( subBufferObject, CL_MEM_ASSOCIATED_MEMOBJECT, origObj, (cl_mem)bufferObject, "associated mem object", "%p", void * )
            
            TEST_MEM_OBJECT_PARAM( subBufferObject, CL_MEM_OFFSET, offset, (size_t)( addressAlign ), "offset", "%ld", size_t )
            
            clReleaseMemObject( subBufferObject );
            subBufferObject = NULL;
            
        }
        
        clReleaseMemObject( bufferObject );
        bufferObject = NULL;
    }
    
    return CL_SUCCESS;
}
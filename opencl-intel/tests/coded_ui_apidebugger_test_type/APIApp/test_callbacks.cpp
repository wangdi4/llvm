/******************************************************************
 //
 //  OpenCL Conformance Tests
 // 
 //  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
 //
 ******************************************************************/

#include "testBase.h"
#include "action_classes.h"
#include "harness/conversions.h"

#if !defined (_MSC_VER)
#include <unistd.h>
#endif // !_MSC_VER

extern const char *IGetStatusString( cl_int status );

#define PRINT_OPS 0

// Yes, this is somewhat nasty, in that we're relying on the CPU (the real CPU, not the OpenCL device)
// to be atomic w.r.t. boolean values. Although if it isn't, we'll just miss the check on this bool
// until the next time around, so it's not that big of a deal. Ideally, we'd be using a semaphore with
// a trywait on it, but then that introduces the fun issue of what to do on Win32, etc. This way is
// far more portable, and worst case of failure is a slightly longer test run.
static bool sCallbackTriggered = false;


#define EVENT_CALLBACK_TYPE_TOTAL 3
static bool sCallbackTriggered_flag[ EVENT_CALLBACK_TYPE_TOTAL ] ={ false,false, false };
cl_int event_callback_types[EVENT_CALLBACK_TYPE_TOTAL] ={ CL_SUBMITTED, CL_RUNNING, CL_COMPLETE};

// Our callback function
/*void CL_CALLBACK single_event_callback_function( cl_event event, cl_int commandStatus, void * userData )
{
     int i=*static_cast<int *>(userData);        
	log_info( "\tEvent callback  %d   triggered\n",  i);
	sCallbackTriggered_flag [ i ] = true;
}*/

/*   use struct as call back para */
typedef struct { cl_int enevt_type; int index; } CALL_BACK_USER_DATA;

void CL_CALLBACK single_event_callback_function_flags( cl_event event, cl_int commandStatus, void * userData )
{
   // int i=*static_cast<int *>(userData);
    CALL_BACK_USER_DATA *pdata= static_cast<CALL_BACK_USER_DATA *>(userData);
    
	log_info( "\tEvent callback  %d  of type %d triggered\n",  pdata->index, pdata->enevt_type);
	sCallbackTriggered_flag [pdata->index ] = true;
}

int test_callback_event_single( cl_device_id device, cl_context context, cl_command_queue queue, Action *actionToTest )
{
	// Note: we don't use the waiting feature here. We just want to verify that we get a callback called
	// when the given event finishes
	
	cl_int error = actionToTest->Setup( device, context, queue );
	test_error( error, "Unable to set up test action" );
	
	// Set up a user event, which we use as a gate for the second event
	clEventWrapper gateEvent = clCreateUserEvent( context, &error );
	test_error( error, "Unable to set up user gate event" );

	// Set up the execution of the action with its actual event
	clEventWrapper actualEvent;
	error = actionToTest->Execute( queue, 1, &gateEvent, &actualEvent );
	test_error( error, "Unable to set up action execution" );

	// Set up the callback on the actual event
	  
  /*  use struct as call back para */
  CALL_BACK_USER_DATA user_data[EVENT_CALLBACK_TYPE_TOTAL];
  int index [EVENT_CALLBACK_TYPE_TOTAL]={ 0,1,2};

  user_data[0].enevt_type=event_callback_types[0];
  user_data[0].index =0;
	error = clSetEventCallback( actualEvent, event_callback_types[0], NULL, user_data+0 );
  error = clSetEventCallback( actualEvent, event_callback_types[0], single_event_callback_function_flags, user_data+0 );

  for( int i=1;i< EVENT_CALLBACK_TYPE_TOTAL; i++)
  {
       user_data[i].enevt_type=event_callback_types[i];
       user_data[i].index =i;
       error = clSetEventCallback( actualEvent, event_callback_types[i], single_event_callback_function_flags, user_data+i );
      
  }
	
	// Now release the user event, which will allow our actual action to run
	error = clSetUserEventStatus( gateEvent, CL_COMPLETE );
	test_error( error, "Unable to trigger gate event" );

	// Now we wait for completion. Note that we can actually wait on the event itself, at least at first
	error = clWaitForEvents( 1, &actualEvent );
	test_error( error, "Unable to wait for actual test event" );

	// Note: we can check our callback now, and it MIGHT have been triggered, but that's not guaranteed
	if( sCallbackTriggered )
	{
		// We're all good, so return success
		return 0;
	}
	
	// The callback has not yet been called, but that doesn't mean it won't be. So wait for it
	log_info( "\tWaiting for callback..." );
	fflush( stdout );
	for( int i = 0; i < 10 * 10; i++ )
	{
		usleep( 100000 );	// 1/10th second
        
    int cc=0;
    for( int k=0;k< EVENT_CALLBACK_TYPE_TOTAL;k++)
        if (sCallbackTriggered_flag[k]) {
            cc++;
        }
        
		if  (cc== EVENT_CALLBACK_TYPE_TOTAL  )
		{
			log_info( "\n" );
			return 0;
		}
		log_info( "." );
		fflush( stdout );
	}
	
	// If we got here, we never got the callback
	log_error( "\nCallback not called within 10 seconds! (assuming failure)\n" );
	return -1;
}

#define TEST_ACTION( name ) \
{	\
	name##Action action;	\
	log_info( "-- Testing " #name "...\n" );	\
	if( ( error = test_callback_event_single( deviceID, context, queue, &action ) ) != CL_SUCCESS )	\
		retVal++;			\
	clFinish( queue ); \
}

int test_callbacks( cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements )
{
	cl_int error;
	int retVal = 0;

	log_info( "\n" );
	
	//TEST_ACTION( NDRangeKernel )
	
	TEST_ACTION( ReadBuffer )
	/*TEST_ACTION( WriteBuffer )
	TEST_ACTION( MapBuffer )
	TEST_ACTION( UnmapBuffer )
	
	if( checkForImageSupport( deviceID ) == CL_IMAGE_FORMAT_NOT_SUPPORTED )
	{
		log_info( "\nNote: device does not support images. Skipping remainder of callback tests...\n" );
	}
	else
	{
		TEST_ACTION( ReadImage2D )
        TEST_ACTION( WriteImage2D )
        TEST_ACTION( CopyImage2Dto2D )
        TEST_ACTION( Copy2DImageToBuffer )
        TEST_ACTION( CopyBufferTo2DImage )
        TEST_ACTION( MapImage )
 
        if( checkFor3DImageSupport( deviceID ) == CL_IMAGE_FORMAT_NOT_SUPPORTED )
            log_info( "\nNote: device does not support 3D images. Skipping remainder of waitlist tests...\n" );
		else
        {
			TEST_ACTION( ReadImage3D )
			TEST_ACTION( WriteImage3D )
			TEST_ACTION( CopyImage2Dto3D )
			TEST_ACTION( CopyImage3Dto2D )
			TEST_ACTION( CopyImage3Dto3D )
			TEST_ACTION( Copy3DImageToBuffer )
			TEST_ACTION( CopyBufferTo3DImage )
		}
	}*/
	
	return retVal;
}

#define  SIMUTANEOUS_ACTION_TOTAL  18
static bool sSimultaneousFlags[ 54 ];// for 18 actions with 3 callback status
static volatile int sSimultaneousCount;

Action * actions[ 19 ] = { 0 };

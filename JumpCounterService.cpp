#include "JumpCounterService.h"
#include "app-resources/resources.h"
#include "common/core/debug.h"
#include "meas_acc/resources.h"
#include "whiteboard/builtinTypes/UnknownStructure.h"
#include "whiteboard/integration/bsp/shared/debug.h"

#include <float.h>
#include <math.h>

#define ASSERT WB_DEBUG_ASSERT
#define ACC_SAMPLERATE 104

const char* const JumpCounterService::LAUNCHABLE_NAME = "JumpC";

static const whiteboard::ExecutionContextId sExecutionContextId =
    WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_JUMPCOUNT::EXECUTION_CONTEXT;

static const whiteboard::LocalResourceId sProviderResources[] = {
    WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_JUMPCOUNT::LID,
    WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_LASTJUMPHEIGHT::LID,
};

JumpCounterService::JumpCounterService()
    : ResourceClient(WBDEBUG_NAME(__FUNCTION__), sExecutionContextId),
      ResourceProvider(WBDEBUG_NAME(__FUNCTION__), sExecutionContextId),
      LaunchableModule(LAUNCHABLE_NAME, sExecutionContextId),
      isRunning(false)
{
    mJumpCounter = 0;
    mLastJumpHeight = 0.0f;
}

JumpCounterService::~JumpCounterService()
{
}

bool JumpCounterService::initModule()
{
    if (registerProviderResources(sProviderResources) != whiteboard::HTTP_CODE_OK)
    {
        return false;
    }

    mModuleState = WB_RES::ModuleStateValues::INITIALIZED;
    return true;
}

void JumpCounterService::deinitModule()
{
    unregisterProviderResources(sProviderResources);
    mModuleState = WB_RES::ModuleStateValues::UNINITIALIZED;
}

/** @see whiteboard::ILaunchableModule::startModule */
bool JumpCounterService::startModule()
{
    mModuleState = WB_RES::ModuleStateValues::STARTED;
    return true;
}

void JumpCounterService::onGetRequest(const whiteboard::Request& request,
                                      const whiteboard::ParameterList& parameters)
{
    DEBUGLOG("JumpCounterService::onGetRequest() called.");

    if (mModuleState != WB_RES::ModuleStateValues::STARTED)
    {
        return returnResult(request, wb::HTTP_CODE_SERVICE_UNAVAILABLE);
    }

    switch (request.getResourceConstId())
    {
    case WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_JUMPCOUNT::ID:
    {
        return returnResult(request, whiteboard::HTTP_CODE_OK,
                            ResponseOptions::Empty, mJumpCounter);
    }

    break;

    case WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_LASTJUMPHEIGHT::ID:
    {
        return returnResult(request, whiteboard::HTTP_CODE_OK,
                            ResponseOptions::Empty, mLastJumpHeight);
    }

    break;

    default:
        // Return error
        return returnResult(request, whiteboard::HTTP_CODE_NOT_IMPLEMENTED);
    }
}

void JumpCounterService::onUnsubscribeResult(whiteboard::RequestId requestId,
                                             whiteboard::ResourceId resourceId,
                                             whiteboard::Result resultCode,
                                             const whiteboard::Value& rResultData)
{
    DEBUGLOG("JumpCounterService::onUnsubscribeResult() called.");
}

void JumpCounterService::onSubscribeResult(whiteboard::RequestId requestId,
                                           whiteboard::ResourceId resourceId,
                                           whiteboard::Result resultCode,
                                           const whiteboard::Value& rResultData)
{
    DEBUGLOG("JumpCounterService::onSubscribeResult() called. resourceId: %u, result: %d", resourceId.localResourceId, (uint32_t)resultCode);

    whiteboard::Request relatedIncomingRequest;
    bool relatedRequestFound = mOngoingRequests.get(requestId, relatedIncomingRequest);

    if (relatedRequestFound)
    {
        returnResult(relatedIncomingRequest, wb::HTTP_CODE_OK);
    }
}

whiteboard::Result JumpCounterService::startRunning(whiteboard::RequestId& remoteRequestId)
{
    DEBUGLOG("JumpCounterService::startRunning()");

    // Reset 0G detection count
    mSamplesSince0GStart = 0;

    // Subscribe to LinearAcceleration resource (updates at ACC_SAMPLERATE Hz), when subscribe is done, we get callback
    wb::Result result = asyncSubscribe(WB_RES::LOCAL::MEAS_ACC_SAMPLERATE::ID, AsyncRequestOptions(&remoteRequestId, 0, true), ACC_SAMPLERATE);
    if (!wb::RETURN_OKC(result))
    {
        DEBUGLOG("asyncSubscribe threw error: %u", result);
        return whiteboard::HTTP_CODE_BAD_REQUEST;
    }
    isRunning = true;

    return whiteboard::HTTP_CODE_OK;
}

whiteboard::Result JumpCounterService::stopRunning()
{
    if (!isRunning) {
        return whiteboard::HTTP_CODE_OK;
    }

    DEBUGLOG("JumpCounterService::stopRunning()");
    if (isResourceSubscribed(WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_JUMPCOUNT::ID) == wb::HTTP_CODE_OK ||
        isResourceSubscribed(WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_LASTJUMPHEIGHT::ID) == wb::HTTP_CODE_OK) {
            DEBUGLOG("JumpCounterService::stopRunning() skipping. Subscribers still exist.");
            return whiteboard::HTTP_CODE_OK;
    }

    // Unsubscribe the LinearAcceleration resource, when unsubscribe is done, we get callback
    wb::Result result = asyncUnsubscribe(WB_RES::LOCAL::MEAS_ACC_SAMPLERATE::ID, NULL, ACC_SAMPLERATE);
    if (!wb::RETURN_OKC(result))
    {
        DEBUGLOG("asyncUnsubscribe threw error: %u", result);
    }
    isRunning = false;

    return whiteboard::HTTP_CODE_OK;
}

void JumpCounterService::punchFound(float punchStrength)
{
    mJumpCounter++;

    // update jump count
    updateResource(WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_JUMPCOUNT(),
                   ResponseOptions::Empty, mJumpCounter);

    // update jump height
    updateResource(WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_LASTJUMPHEIGHT(),
                   ResponseOptions::Empty, punchStrength);
}

/* Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
 *    Command line: /www/usr/fisher/helpers/mkfilter -Be -Hp -o 4 -a 9.6153846154e-02 0.0000000000e+00 -l */

#define NZEROS 4
#define NPOLES 4
#define GAIN   1.873702775e+00
#define PUNCH_THRESHOLD 36000000 

static float hpf(float input_value) { 
	static float xv[NZEROS+1] = {};
        static float yv[NPOLES+1] = {};
	xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4]; 
	xv[4] = input_value / GAIN;
	yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4]; 
	yv[4] =   (xv[0] + xv[4]) - 4 * (xv[1] + xv[3]) + 6 * xv[2]
		+ ( -0.2678183622 * yv[0]) + (  1.4569143755 * yv[1])
		+ ( -3.0094584394 * yv[2]) + (  2.8050500595 * yv[3]);
	return yv[4];
 }

// This callback is called when the acceleration resource notifies us of new data
void JumpCounterService::onNotify(whiteboard::ResourceId resourceId, const whiteboard::Value& value,
                                  const whiteboard::ParameterList& parameters)
{
    // Confirm that it is the correct resource
    switch (resourceId.getConstId())
    {
    case WB_RES::LOCAL::MEAS_ACC_SAMPLERATE::ID:
    {
        const WB_RES::AccData& linearAccelerationValue =
            value.convertTo<const WB_RES::AccData&>();

        if (linearAccelerationValue.arrayAcc.size() <= 0)
        {
            // No value, do nothing...
            return;
        }

        const whiteboard::Array<whiteboard::FloatVector3D>& arrayData = linearAccelerationValue.arrayAcc;

        uint32_t relativeTime = linearAccelerationValue.timestamp;

        for (size_t i = 0; i < arrayData.size(); i++)
        {
            whiteboard::FloatVector3D accValue = arrayData[i];
            float accelerationSq = accValue.mX * accValue.mX +
                                   accValue.mY * accValue.mY +
                                   accValue.mZ * accValue.mZ;
	    
	    float filtered_punch = hpf(accelerationSq);
            if (filtered_punch > PUNCH_THRESHOLD) {
	    	punchFound(filtered_punch);	
	    }
        }
    }
    break;
    }
}

void JumpCounterService::onSubscribe(const whiteboard::Request& request,
                                     const whiteboard::ParameterList& parameters)
{
    DEBUGLOG("JumpCounterService::onSubscribe()");

    switch (request.getResourceConstId())
    {
    case WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_JUMPCOUNT::ID:
    case WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_LASTJUMPHEIGHT::ID:
    {
        // Someone subscribed to our service. If no subscribers before, start running
        if (!isRunning)
        {
            whiteboard::RequestId remoteRequestId;
            whiteboard::Result result = startRunning(remoteRequestId);

            if (!whiteboard::RETURN_OK(result))
            {
                return returnResult(request, result);
            }

            bool queueResult = mOngoingRequests.put(remoteRequestId, request);
            WB_ASSERT(queueResult);
        }
        else{
            return returnResult(request, whiteboard::HTTP_CODE_OK);
        }

        break;
    }
    default:
        ASSERT(0); // Should not happen
    }
}

void JumpCounterService::onUnsubscribe(const whiteboard::Request& request,
                                       const whiteboard::ParameterList& parameters)
{
    DEBUGLOG("JumpCounterService::onUnsubscribe()");

    switch (request.getResourceConstId())
    {
    case WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_JUMPCOUNT::ID:
    case WB_RES::LOCAL::SAMPLE_JUMPCOUNTER_LASTJUMPHEIGHT::ID:
        returnResult(request, stopRunning());
        break;

    default:
        returnResult(request, wb::HTTP_CODE_BAD_REQUEST);
        break;
    }
}

void JumpCounterService::onRemoteWhiteboardDisconnected(whiteboard::WhiteboardId whiteboardId)
{
    DEBUGLOG("JumpCounterService::onRemoteWhiteboardDisconnected()");
    stopRunning();
}

void JumpCounterService::onClientUnavailable(whiteboard::ClientId clientId)
{
    DEBUGLOG("JumpCounterService::onClientUnavailable()");
    stopRunning();
}

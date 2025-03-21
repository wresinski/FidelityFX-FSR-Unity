#pragma once

#include "IUnityInterface.h"
#include "IUnityLog.h"
#include "IUnityGraphics.h"


class FSRUnityPlugin
{
public:
    enum PassEvent
    {
        INVALID = 0,
        INITIALIZE = 1,
        DISPATCH,
        REACTIVEMASK,
        DESTROY,
        MAX
    };

public:
    static IUnityInterfaces* UnityInterfaces;
    static IUnityGraphics* UnityGraphics;
    static IUnityLog* UnityLog;
};

#define FSR_LOG(msg) UNITY_LOG(FSRUnityPlugin::UnityLog, msg);
#define FSR_ERROR(msg) UNITY_LOG_ERROR(FSRUnityPlugin::UnityLog, msg);
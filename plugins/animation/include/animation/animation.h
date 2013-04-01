#ifndef _ANIMATION_H
#define _ANIMATION_H

#define ANIMATION_ABI 20091205

#include <core/core.h>
#include <core/pluginclasshandler.h>

typedef enum
{
    WindowEventOpen = 0,
    WindowEventClose,
    WindowEventMinimize,
    WindowEventUnminimize,
    WindowEventShade,
    WindowEventUnshade,
    WindowEventFocus,
    WindowEventNum,
    WindowEventNone
} WindowEvent;

typedef enum
{
    AnimEventOpen = 0,
    AnimEventClose,
    AnimEventMinimize,
    AnimEventUnminimize,
    AnimEventShade,
    AnimEventFocus,
    AnimEventNum
} AnimEvent;

typedef enum
{
    AnimDirectionDown = 0,
    AnimDirectionUp,
    AnimDirectionLeft,
    AnimDirectionRight,
    AnimDirectionRandom,
    AnimDirectionAuto
} AnimDirection;

extern const unsigned short LAST_ANIM_DIRECTION;

class PrivateAnimScreen;
class PrivateAnimWindow;
class Animation;
class AnimWindow;
class AnimEffectInfo;

typedef AnimEffectInfo * AnimEffect;

#define RAND_FLOAT() ((float)rand() / RAND_MAX)

#define sigmoid(fx) (1.0f/(1.0f+exp(-5.0f*2*((fx)-0.5))))
#define sigmoid2(fx, s) (1.0f/(1.0f+exp(-(s)*2*((fx)-0.5))))

#define NUM_OPTIONS(s) (sizeof ((s)->opt) / sizeof (CompOption))

#include "extensionplugin.h"
#include "animeffect.h"
#include "point3d.h"
#include "persistent.h"
#include "grid.h"
#include "screen.h"
#include "window.h"
#include "transform.h"
#include "fade.h"
#include "partialwindow.h"
#include "gridtransform.h"
#include "zoom.h"
#include "multi.h"


// ratio of perceived length of animation compared to real duration
// to make it appear to have the same speed with other animation effects

#endif


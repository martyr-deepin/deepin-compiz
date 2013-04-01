#ifndef ANIMATION_EXTENSIONPLUGIN_H
#define ANIMATION_EXTENSIONPLUGIN_H
#include "animation.h"

class ExtensionPluginInfo
{
public:
     ExtensionPluginInfo (const CompString &name,
			  unsigned int nEffects,
			  AnimEffect *effects,
			  CompOption::Vector *effectOptions,
			  unsigned int firstEffectOptionIndex);
     
     CompString name;
     unsigned int nEffects;
     AnimEffect *effects;
     
     /// Plugin options to be used in "effect options" strings.
     CompOption::Vector *effectOptions;
     
     /// Index of first effect option.
     unsigned int firstEffectOptionIndex;
     
     // More general and/or non-window functions (including functions that access
     // persistent animation data) to be overriden
     
     /// To be run at the beginning of glPaintOutput.
     virtual void prePaintOutput (CompOutput *output) {}
     
     /// To be run at the beginning of preparePaint.
     virtual void prePreparePaintGeneral () {}
     
     /// To be run at the end of preparePaint.
     virtual void postPreparePaintGeneral () {}
     
     /// To be run when a CompWindowNotifyRestack is handled.
     virtual void handleRestackNotify (AnimWindow *aw) {}
     
     /// To be run at the beginning of initiateOpenAnim.
     virtual void preInitiateOpenAnim (AnimWindow *aw) {}
     
     /// To be run at the beginning of initiateCloseAnim.
     virtual void preInitiateCloseAnim (AnimWindow *aw) {}
     
     /// To be run at the beginning of initiateMinimizeAnim.
     virtual void preInitiateMinimizeAnim (AnimWindow *aw) {}
     
     /// To be run at the beginning of initiateUnminimizeAnim.
     virtual void preInitiateUnminimizeAnim (AnimWindow *aw) {}
     
     /// Initializes plugin's persistent animation data for a window (if any).
     virtual void initPersistentData (AnimWindow *aw) {}
     
     /// Destroys plugin's persistent animation data for a window (if any).
     virtual void destroyPersistentData (AnimWindow *aw) {}
     
     /// To be run at the end of updateEventEffects.
     virtual void postUpdateEventEffects (AnimEvent e,
					  bool forRandom) {}
					  
    /// To be run after the startup countdown ends.
    virtual void postStartupCountdown () {}

    virtual bool paintShouldSkipWindow (CompWindow *w) { return false; }

    virtual void cleanUpAnimation (bool closing,
				    bool destructing) {}

    virtual void processAllRestacks () {}
};
#endif

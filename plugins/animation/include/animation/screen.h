#ifndef ANIMATION_SCREEN_H
#define ANIMATION_SCREEN_H
#include "animation.h"

class AnimScreen;
extern template class PluginClassHandler<AnimScreen, CompScreen, ANIMATION_ABI>;

class AnimScreen :
    public PluginClassHandler<AnimScreen, CompScreen, ANIMATION_ABI>,
    public CompOption::Class
{
    friend class ExtensionPluginAnimation;
    friend class PrivateAnimScreen;
    friend class PrivateAnimWindow;
    
public:
    AnimScreen (CompScreen *);
    ~AnimScreen ();
    
    void addExtension (ExtensionPluginInfo *extensionPluginInfo);
    void removeExtension (ExtensionPluginInfo *extensionPluginInfo);
    bool getMousePointerXY (short *x, short *y);
    CompOption::Vector &getOptions ();
    bool setOption (const CompString &name, CompOption::Value &value);
    CompOutput &output ();
    AnimEffect getMatchingAnimSelection (CompWindow *w,
					 AnimEvent e,
					 int *duration);
    void enableCustomPaintList (bool enabled);
    bool isRestackAnimPossible ();
    bool isAnimEffectPossible (AnimEffect theEffect);
    bool otherPluginsActive ();
    bool initiateFocusAnim (AnimWindow *aw);
    
private:
    PrivateAnimScreen *priv;
    
};
#endif

#ifndef ANIMATION_PERSISTENT_H
#define ANIMATION_PERSISTENT_H

#include "animation.h"
class PersistentData
{
};

typedef std::map<std::string, PersistentData *> PersistentDataMap;

class MultiPersistentData :
public PersistentData
{
public:
    MultiPersistentData () : num (0) {}
    int num;
};
#endif

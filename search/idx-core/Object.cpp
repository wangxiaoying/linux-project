#include <stdio.h>
#include "Object.h"

long long Object::NumObjectCount = 0;
long long Object::NumObjectCreated = 0;

void Object::onExit()
{
    if (Object::NumObjectCount!=0)
    {
        fprintf(stderr,"Memory leak detected: %lld objects haven't been released.\n", Object::NumObjectCount);
    }
}

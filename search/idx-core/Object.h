#ifndef OBJECT_H
#define OBJECT_H

class Object
{
    public:
        Object()
        {
            ++NumObjectCount;
            ++NumObjectCreated;
        }

        virtual ~Object()
        {
            --NumObjectCount;
        }

        static void onExit();

    private:
        static long long NumObjectCount;
        static long long NumObjectCreated;
};
#endif

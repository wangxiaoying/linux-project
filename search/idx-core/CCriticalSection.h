#ifndef CCRITICALSECTION_H_INCLUDED
#define CCRITICALSECTION_H_INCLUDED

class CCriticalSection
{
private:
	pthread_mutex_t mutex_lock;

	CCriticalSection(const CCriticalSection&);

public:
	CCriticalSection()
	{
        pthread_mutex_init(&mutex_lock,NULL);
	}

	~CCriticalSection()
	{
		pthread_mutex_destroy(&mutex_lock);
	}

	bool inline tryEnter()
	{
		return pthread_mutex_trylock(&mutex_lock);
	}

	void inline enter()
	{
		pthread_mutex_lock(&mutex_lock);
	}

	void inline leave()
	{
		pthread_mutex_unlock(&mutex_lock);
	}
};

class CDummyCriticalSection
{
public:
	void inline enter() const{};
	void inline leave() const{};
	bool inline tryEnter() const{return true;}
};

class CAutoCriticalSection
{
    private:
        CCriticalSection& lock;

    public:
        CAutoCriticalSection(CCriticalSection& mutex):lock(mutex)
        {
            lock.enter();
        }

        ~CAutoCriticalSection()
        {
            lock.leave(); // leave when being deleted
        }
};

#define AUTO_LOCK(mutex) CAutoCriticalSection __auto__lock##__LINE__(mutex)

#endif // CCRITICALSECTION_H_INCLUDED

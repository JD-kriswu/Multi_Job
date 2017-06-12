#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <pthread.h>
#include <stdlib.h>
//#include <sys/types.h>

/*
    线程安全的单例模式
*/

#define SINGLETON(__CLASS)      boss::CSingleton<__CLASS>::Instance()

#define TSD_SINGLETON(__CLASS)  boss::CTSDSingleton<__CLASS>::Instance()

namespace boss
{
    template <typename T>
    class CSingleton
    {
    public:
        static T * Instance()
        {
            if ( !m_pInstance )
            {
                ++m_pInstance; //防止优化
                --m_pInstance;
                pthread_mutex_lock(&m_mutex) ;
                if ( !m_pInstance )
                {
                    if ( m_destroyed )
                    {
                        OnDeadReference();
                    } else
                    {
                        Create();
                    }
                }
                pthread_mutex_unlock(&m_mutex);
            }
            return m_pInstance;
        }
    private:
        static T * m_pInstance;
        static bool m_destroyed;
        static pthread_mutex_t m_mutex;
    private:
        CSingleton();
        CSingleton(const CSingleton&);
        CSingleton& operator=(const CSingleton&);
        ~CSingleton()
        {
            m_pInstance = 0;
            m_destroyed = true;
        }
        static void Create()
        {
            static T sInstance;
            m_pInstance = &sInstance;
        }
        static void OnDeadReference()
        {
            Create();
            new (m_pInstance) T;
            atexit(KillPhoenixSingleton);
            m_destroyed = false;
        }
        static void KillPhoenixSingleton()
        {
            m_pInstance->~T();
        }
    };


    template <typename T>
    T * CSingleton<T>::m_pInstance = NULL;

    template <typename T>
    bool CSingleton<T>::m_destroyed = false;

    template <typename T>
    pthread_mutex_t CSingleton<T>::m_mutex = PTHREAD_MUTEX_INITIALIZER;


    template <typename T>
    class CTSDSingleton
    {
    public:
        static T * Instance()
        {
            T * ptr = NULL;
            pthread_once(&key_once, make_key);
            if ( (ptr = (T *)pthread_getspecific(key))  == NULL )
            {
                pthread_setspecific(key, new T);
                ptr = (T *)pthread_getspecific(key);
                //printf("thread %lu create %p\n",pthread_self(),ptr);
            }
            //printf("thread %lu returns %p\n",pthread_self(),ptr);
            return ptr;
        }

    private:
        static void  make_key()
        {
            (void) pthread_key_create(&key, destroy);
        }

        static void  destroy(void * p)
        {
            //printf("thread %lu returns %p\n",pthread_self(),p);
            delete (T*)p;
        }

        static pthread_key_t key;
        static pthread_once_t key_once;
    };
    
    template <typename T>
    pthread_key_t CTSDSingleton<T>::key = 0;

    template <typename T>
    pthread_once_t CTSDSingleton<T>::key_once = PTHREAD_ONCE_INIT;
}
#endif

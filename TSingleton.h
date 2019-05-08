#pragma once

#include <stdlib.h>

/* TSingleton: is to make the singleton instance */

template<typename T>
class TSingleton
{
public:
    static T* Instance()
    {
        if (NULL == m_pInstance)
        {
            m_pInstance = new T;
            atexit(Destroy);
        }
        return m_pInstance;
    }

private:
    TSingleton();
    ~TSingleton();
    TSingleton(TSingleton&);
    TSingleton& operator = (TSingleton&);
    
    static void Destroy()
    {
        if (NULL != m_pInstance)
        {
            delete m_pInstance;
            m_pInstance = NULL;
        }
    }

private:
    static T* m_pInstance;
};

template<typename T>
T* TSingleton<T>::m_pInstance = NULL;
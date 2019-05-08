#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

#include "TaskManager.h"
#include "HttpHandler.h"


TaskManager::TaskManager()
        : m_nTotalTaskCount(1024)
        , m_thrMgrId(0)
        , m_bShutDown(false)
{
    pthread_mutex_init(&m_mutex, NULL);

    /* Initialize thread creation attributes */
    pthread_attr_t attr;

    int s = pthread_attr_init(&attr);
    if (s == 0)
    {
        /* Create the tasks manager */
        s = pthread_create(&m_thrMgrId, &attr, &ManagerThread, (void*)this);
    }
    pthread_attr_destroy(&attr);
}

TaskManager::~TaskManager()
{
    m_bShutDown = true;
    void *retCode = NULL;
    pthread_join(m_thrMgrId, &retCode);
    pthread_mutex_destroy(&m_mutex);
}

void* TaskManager::ManagerThread(void* pArg)
{
    TaskManager* pThis = (TaskManager*)pArg;

    do
    {
        if (pThis->m_bShutDown)
        {
            break;
        }
        if (pThis->m_vecTaskId.size() >= pThis->m_nTotalTaskCount)
        {
            for(size_t i = pThis->m_vecTaskId.size()-1; i > 0; i--)
            {
                pthread_t id = pThis->m_vecTaskId.at(i);
                if (ESRCH == pthread_kill(id,0))
                {
                    pThis->m_vecTaskId.pop_back();
                    i++;
                }
            }            
        }
        if (pThis->m_vecTaskId.size() >= pThis->m_nTotalTaskCount)
        {
            sleep(1);
            continue;
        }
        if (pThis->m_vecSocksConn.size() == 0)
        {
            sleep(1);
            continue;
        }

        pthread_mutex_lock(&pThis->m_mutex);
        int* pClientSock = new int;
        *pClientSock = pThis->m_vecSocksConn.at(0);
        pThis->m_vecSocksConn.erase(pThis->m_vecSocksConn.begin());
        pthread_mutex_unlock(&pThis->m_mutex);

        /* Initialize thread creation attributes */
        pthread_attr_t attr;

        int s = pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);// so that the system resource would be free when the thread exit

        if (s == 0)
        {
            /* Create connection thread handler */
            pthread_t thread_id;
            
            s = pthread_create(&thread_id, &attr, &doTask, (void*)pClientSock);
            pThis->m_vecTaskId.push_back(thread_id);
        }
        else
        {
            close(*pClientSock);
        }
        
        pthread_attr_destroy(&attr);

    } while (true);

    return 0;
}

void* TaskManager::doTask(void* pArg)
{
    int sockClient = *(int*)pArg;
    delete (int*)pArg;
    pArg = NULL;

    HttpHandler* pHandler = new HttpHandler(sockClient);
    pHandler->ClientConnHandler();
    delete pHandler;

    return 0;
}

bool TaskManager::addTask(int sockConn, int ipClient, int portClient)
{
    in_addr ip = { ipClient };
    printf("Client connected [%s:%d]\r\n", inet_ntoa(ip), ntohs(portClient));

    /* add the connected client socket as a task */
    pthread_mutex_lock(&m_mutex);
    m_vecSocksConn.push_back(sockConn);
    pthread_mutex_unlock(&m_mutex);

    return true;
}
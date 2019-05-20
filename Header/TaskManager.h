#pragma once

#include "TSingleton.h"
#include <pthread.h>
#include <vector>


class TaskManager
{
public:
    TaskManager();
    ~TaskManager();

public:
    bool addTask(int sockConn, int ipClient, int portClient);

protected:
    static void* doTask(void* pArg);            // do work
    static void* ManagerThread(void* pArg);     // The tasks manager

private:
    bool                            m_bShutDown;
    const size_t                    m_nTotalTaskCount;  // total count of the task
    std::vector<int>                m_vecSocksConn;     // the sockets of connected
    std::vector<pthread_t>          m_vecTaskId;        // tasks's ids
    pthread_t                       m_thrMgrId;         // manager thread id
    pthread_mutex_t                 m_mutex;            // mutex for m_vecSocksConn;
};

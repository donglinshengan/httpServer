#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#include "../Header/HttpHandler.h"
#include "../Header/config.h"

HttpHandler::HttpHandler(int sockConn)
        : m_sockConn(sockConn)
{
}

HttpHandler::~HttpHandler()
{
}

int HttpHandler::ClientConnHandler()
{
    struct timeval nTime = {6, 0};
    setsockopt(m_sockConn, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTime, sizeof(nTime));

    /* communicate */
    do {
        char data[2048] = {0};
        int nLen = recv(m_sockConn, (void*)data, 2048, 0);
        if (-1 == nLen)
        {
            printf("recv() error with errno=%d\r\n", errno);
            break;
        }
        if (0 == nLen)
        {
            printf("nothing received from client with errno=%d\r\n", errno);
            break;
        }
        printf("[sock: %d] recevie data[len=%d]:\r\n%s\r\n", m_sockConn, nLen, data);

        char szPath[256] = {0};
        GetRequestPath(data, szPath, 256);
        SendResponseContent(szPath);
    } while (true);

    shutdown(m_sockConn, SHUT_RDWR);
    close(m_sockConn);

    return 0;
}

int HttpHandler::SendResponseContent(char* pPath)
{
    if (NULL == pPath)
    {
        return -1;
    }

    FILE*   pFile       = NULL;
    long    size        = 0;
    char    szPath[256] = Work_Dir "/webPage";

    if (0 == strcmp("/", pPath))
    {
        strcat(szPath, "/index.html");
    }
    else
    {
        strcat(szPath, pPath);
    }    

    pFile = fopen (szPath,"rb");
    if (pFile==NULL)
    {
        return SendFailureResponse(404);
    }
    else
    {
        fseek (pFile, 0, SEEK_END);
        size=ftell (pFile);
        SendResponseHeader(size);

        fseek(pFile, 0, SEEK_SET);
        char szBuf[1024] = {0};

        do {
            size = fread(szBuf, 1, 1024, pFile);
            send(m_sockConn, (void*)szBuf, size, 0);
        } while (size == 1024);
    }
    fclose (pFile);
}

int HttpHandler::SendFailureResponse(int nErrCode)
{
    char pResponse[] = "HTTP/1.1 404 Not Found\r\n"
            "Server : HttpServer/1.0 (Linux)\r\n"
            "Pragma : no-cache\r\n"
            "Connection : Keep-Alive\r\n"
            "Content-Type : text/html; application/x-www-form-urlencoded; Language=UTF-8\r\n"
            "Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\n"
            "Access-Control-Allow-Credentials: true\r\n"
            "Access-Control-Allow-Methods: GET, POST, PUT,DELETE\r\n\r\n"
            "<html><title>Page Not Found</title><body><h1>Page Not Found</body></html>";
    
    return send(m_sockConn, (void*)pResponse, strlen(pResponse), 0);
}

int HttpHandler::SendResponseHeader(int nContentLength)
{
    int nLen = 0;
    char* pHeader = new char[1024];

    memset(pHeader, 0, 1024);

    sprintf(pHeader,
        "HTTP/1.1 200 OK\r\n"
		"Server : HttpServer/1.0 (Linux)\r\n"
		"Pragma : no-cache\r\n"
		"Connection : Keep-Alive\r\n"
        "Content-Length: %d\r\n"
		"Content-Type : text/html; application/x-www-form-urlencoded; Language=UTF-8\r\n"
		"Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\n"
		"Access-Control-Allow-Credentials: true\r\n"
		"Access-Control-Allow-Methods: GET, POST, PUT,DELETE\r\n\r\n", 
        nContentLength);
    
    nLen = send(m_sockConn, (void*)pHeader, strlen(pHeader), 0);
    delete pHeader;

    return nLen;
}

char* HttpHandler::GetRequestPath(const char* pData, char* pBuf, int nSize)
{
    GetHeaderContent(pData, "GET", pBuf, nSize);
    if (NULL == pBuf || 0 == *pBuf)
    {
        GetHeaderContent(pData, "POST", pBuf, nSize);
    }
    return pBuf;
}

char* HttpHandler::GetHeaderContent(const char* pData, const char* pName, char* pBuf, int nSize)
{
    if (NULL == pData || NULL == pBuf || NULL == pName)
    {
        return NULL;
    }

    memset(pBuf, 0, nSize);

    const char* posBegin = strstr(pData, pName);
    if (NULL != posBegin)
    {
        if (strcmp("GET", pName) == 0 || strcmp("POST", pName) == 0)
        {
            posBegin += strlen(pName);
        }
        else
        {
            posBegin += strlen(pName) + 1;
        }

        while (*posBegin == ' ') ++posBegin;

        const char* posEnd = posBegin;
        while (' ' != *posEnd && '\r' != *posEnd && '\n' != *posEnd) ++posEnd;
        
        int nLenCpy = posEnd - posBegin;
        nLenCpy = nLenCpy < nSize ? nLenCpy : nSize - 1;
        strncpy(pBuf, posBegin, nLenCpy);
    }
    
    return pBuf;
}
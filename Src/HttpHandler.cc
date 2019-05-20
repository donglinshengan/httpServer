#include <stdio.h>
#include <stdlib.h>
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

    char* pData = new char[10240];

    /* communicate */
    while (true)
    {
        memset(pData, 0, 10240);
        int nLen = recv(m_sockConn, (void*)pData, 10240, 0);

        if (-1 == nLen) {
            if (errno == EINTR) {
                continue;
            }
            printf("recv() error with errno=%d\r\n", errno);
            break;
        }
        if (0 == nLen) {
            printf("nothing received from client with errno=%d\r\n", errno);
            break;
        }
        
        printf("[sock: %d] recevie data[len=%d]:\r\n%s\r\n", m_sockConn, nLen, pData);        

        if (-1 == HandleRequest(pData)) {
            break;
        }
    }

    delete pData;
    shutdown(m_sockConn, SHUT_RDWR);
    close(m_sockConn);

    return 0;
}

int HttpHandler::HandleRequest(const char* pDataRequest)
{
    int nRet = 0;
    char szPath[256] = {0};
    tagReqMethod method = GetRequestPath(pDataRequest, szPath, 256);

    switch (method)
    {
    case Method_GET: {
        nRet = SendResponseContent(szPath);
        break;
    }
    case Method_HEAD: {
        nRet = SendResponseContent(szPath, true);
        break;
    }
    case Method_POST:{
        char szExpect[128] = {0};
        GetHeaderContent(pDataRequest, "Expect", szExpect, 128);
        if (0 != szExpect[0]) {
            /* tell the client that server could handle post data larger than 1024 bytes*/
            SendExpectResponse();
        }

        char szContentType[128] = {0};
        char szBoundary[128] = {0};
        char szContentLen[64] = {0};

        GetHeaderContent(pDataRequest, "Content-Length", szContentLen, 64);
        if (atoll(szContentLen) > 10 * 2<<20) {
            /* content could not large than 10M */
            return -1;
        }

        GetHeaderContent(pDataRequest, "Content-Type", szContentType, 128);
        if (NULL == strstr(szContentType, "multipart/form-data")) {
            /* just the multipart/form-data */
            return -1;
        }

        ParseBoundaryArg(szContentType, szBoundary, 128);
        if (0 == szBoundary[0]) {
            return -1;
        }

        /* handle the post data */
        nRet = HandlePostData(szPath, szBoundary);
    }
    case Method_PUT:
    case Method_DELETE:
    case Method_CONNECT:
    case Method_OPTION:
    case Method_TRACE:
    default:
        break;
    }

    return nRet;
}

int HttpHandler::SendResponseContent(const char* pPath, bool bOnlyHeader)
{
    if (NULL == pPath) {
        return -1;
    }

    FILE*   pFile       = NULL;
    long    size        = 0;
    char    szPath[256] = Work_Dir "/webPage";

    if (0 == strcmp("/", pPath)) {
        strcat(szPath, "/index.html");
    }
    else {
        strcat(szPath, pPath);
    }    

    pFile = fopen (szPath,"rb");

    if (pFile==NULL) {
        return SendFailureResponse(404, pPath);
    }
    else {
        fseek (pFile, 0, SEEK_END);
        size=ftell (pFile);
        SendResponseHeader(size);

        if (!bOnlyHeader) {
            fseek(pFile, 0, SEEK_SET);
            char szBuf[1024] = {0};

            do {
                size = fread(szBuf, 1, 1024, pFile);
                send(m_sockConn, (void*)szBuf, size, 0);
            } while (size == 1024);
        }
    }

    fclose (pFile);

    return 0;
}

int HttpHandler::SendExpectResponse()
{
    int nLen = 0;
    char szHeader[] = "HTTP/1.1 100 Continue\r\n"
		"Server : HttpServer/1.0 (Linux)\r\n"
		"Pragma : no-cache\r\n"
		"Connection : Keep-Alive\r\n"
		"Content-Type : text/html; application/x-www-form-urlencoded; Language=UTF-8\r\n\r\n";
    
    nLen = send(m_sockConn, (void*)szHeader, strlen(szHeader), 0);

    return nLen;
}

int HttpHandler::SendFailureResponse(int nErrCode, const char* pPath)
{
    char szResponse[512] = {0};
    sprintf(szResponse,
            "HTTP/1.1 %d Not Found\r\n"
            "Server : HttpServer/1.0 (Linux)\r\n"
            "Pragma : no-cache\r\n"
            "Connection : Keep-Alive\r\n"
            "Content-Type : text/html; application/x-www-form-urlencoded; Language=UTF-8\r\n"
            "Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\n"
            "Access-Control-Allow-Credentials: true\r\n"
            "Access-Control-Allow-Methods: GET, POST, PUT,DELETE\r\n\r\n"
            "<html><title>Page Not Found</title><body><h1>%s Page Not Found</body></html>",
            nErrCode, pPath);
    
    return send(m_sockConn, (void*)szResponse, strlen(szResponse), 0);
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

 HttpHandler::tagReqMethod HttpHandler::GetRequestPath(const char* pDataRequest, char* pBuf, int nSize)
{
    const char* pBeg = pDataRequest;
    const char* pEnd = NULL;
    int nLen = 0;
    tagReqMethod method = Method_NULL;

    memset(pBuf, 0, nSize);

    do {
        pEnd = strstr(pBeg, "\r\n");

        if (pEnd == NULL || pBeg == pEnd) {
            return Method_NULL;
        }

        if (strstr(pBeg, "GET") == pBeg
            || strstr(pBeg, "Get") == pBeg
            || strstr(pBeg, "get") == pBeg)
        {
            method = Method_GET;
            nLen = 3;
            break;
        }
        else if (strstr(pBeg, "POST") == pBeg
            || strstr(pBeg, "Post") == pBeg
            || strstr(pBeg, "post") == pBeg)
        {
            method = Method_POST;
            nLen = 4;
            break;
        }
        else if (strstr(pBeg, "HEAD") == pBeg
            || strstr(pBeg, "Head") == pBeg
            || strstr(pBeg, "head") == pBeg)
        {
            method = Method_HEAD;
            nLen = 4;
            break;
        }
        else if (strstr(pBeg, "OPTION") == pBeg
            || strstr(pBeg, "Option") == pBeg
            || strstr(pBeg, "option") == pBeg)
        {
            method = Method_OPTION;
            nLen = 6;
            break;
        }
        else if (strstr(pBeg, "DELETE") == pBeg
            || strstr(pBeg, "Delete") == pBeg
            || strstr(pBeg, "delete") == pBeg)
        {
            method = Method_DELETE;
            nLen = 6;
            break;
        }
        else if (strstr(pBeg, "PUT") == pBeg
            || strstr(pBeg, "Put") == pBeg
            || strstr(pBeg, "put") == pBeg)
        {
            method = Method_PUT;
            nLen = 3;
            break;
        }
        else if (strstr(pBeg, "TRACE") == pBeg
            || strstr(pBeg, "Trace") == pBeg
            || strstr(pBeg, "trace") == pBeg)
        {
            method = Method_TRACE;
            nLen = 5;
            break;
        }
        else if (strstr(pBeg, "CONNECT") == pBeg
            || strstr(pBeg, "Connect") == pBeg
            || strstr(pBeg, "connect") == pBeg)
        {
            method = Method_CONNECT;
            nLen = 7;
            break;
        }
        pBeg = pEnd + 2;
        
    } while (true);

    if (nLen > 0) {
        pBeg += nLen;
        while (*pBeg == ' ') ++pBeg;

        pEnd = strstr(pBeg, "\r\n");

        if (NULL != pEnd)
        {
            while (*pEnd != ' ') --pEnd;

            int nLenCpy = pEnd - pBeg;
            nLenCpy = nLenCpy < nSize ? nLenCpy : nSize - 1;
            strncpy(pBuf, pBeg, nLenCpy);
        }
    }    

    return method;
}

char* HttpHandler::GetHeaderContent(const char* pData, const char* pName, char* pBuf, int nSize)
{
    if (NULL == pData || NULL == pBuf || NULL == pName) {
        return NULL;
    }

    memset(pBuf, 0, nSize);

    const char* posBegin = strstr(pData, pName);
    if (NULL != posBegin) {
        posBegin += strlen(pName) + 1;

        while (*posBegin == ' ') ++posBegin;

        const char* posEnd = posBegin;
        while ('\r' != *posEnd && '\n' != *posEnd) ++posEnd;
        
        int nLenCpy = posEnd - posBegin;
        nLenCpy = nLenCpy < nSize ? nLenCpy : nSize - 1;
        strncpy(pBuf, posBegin, nLenCpy);
    }
    
    return pBuf;
}
int HttpHandler::HandlePostData(const char* pPath, const char* pBoundary)
{
    char* pRecvBuf = new char[1024];

    while (true)
    {
        memset(pRecvBuf, 0, 1024);
        int nLen = recv(m_sockConn, (void*)pRecvBuf, 1024, 0);

        if (-1 == nLen) {
            if (errno == EINTR) {
                continue;
            }
            printf("recv() error with errno=%d\r\n", errno);
            break;
        }
        if (0 == nLen) {
            printf("nothing received from client with errno=%d\r\n", errno);
            break;
        }
        
        // handle the recv data
    }

    char szHeader[] = "HTTP/1.1 200 OK\r\n"
		"Server : HttpServer/1.0 (Linux)\r\n"
		"Pragma : no-cache\r\n"
		"Connection : Keep-Alive\r\n"
		"Content-Type : text/html; application/x-www-form-urlencoded; Language=UTF-8\r\n\r\n";
    
    send(m_sockConn, (void*)szHeader, strlen(szHeader), 0);

    delete pRecvBuf;
    return 0;
}

char* HttpHandler::ParseBoundaryArg(const char* pData, char* pBuf, int nSize)
{
    const char* pBeg = strstr(pData, "boundary=");
    const char* pEnd = NULL;

    memset(pBuf, 0, nSize);

    if (NULL != pBeg) {
        pBeg += 9;
        pEnd = strchr(pBeg, ';');

        if (NULL == pEnd) {
            strcpy(pBuf, pBeg);
        } else {
            strncpy(pBuf, pBeg, pEnd - pBeg);
        }
    }

    return pBuf;
}
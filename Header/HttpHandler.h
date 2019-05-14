#pragma once

/**********************************************************/
/* HttpHandler: handle the client http request            */
/**********************************************************/

class HttpHandler
{
public:
    HttpHandler(int sockConn);
    ~HttpHandler();

    enum tagReqMethod {Method_NULL, Method_GET, Method_HEAD,
        Method_POST, Method_OPTION,Method_PUT, Method_DELETE,
        Method_CONNECT, Method_TRACE};

public:
    int ClientConnHandler(); // the handler to the connections

protected:
    /* handle the request */
    int HandleRequest(const char* pDataRequest);
    /* send the response content to the client */
    int SendResponseContent(const char* pPath, bool bOnlyHeader=false);
    /* tell client that 1024 bytes could be post (expect header is enable) */
    int SendExpectResponse();
    /* handle the error request */
    int SendFailureResponse(int nErrCode, const char* pPath);
    /* send the http header if the request should be handled correctly */
    int SendResponseHeader(int nContentLength);
    /* get the request path */
    tagReqMethod GetRequestPath(const char* pDataRequest, char* pBuf, int nSize);
    /* get the http header sent by the client */
    char* GetHeaderContent(const char* pData, const char* pName, char* pBuf, int nSize);
    /* handle the post request data */
    int HandlePostData(const char* pPath, const char* pBoundary);

protected:
    char* ParseBoundaryArg(const char* pData, char* pBuf, int nSize);

private:
    int m_sockConn; // socket the client request connected
};

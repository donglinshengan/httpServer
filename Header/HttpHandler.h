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
    int RequestHandle(tagReqMethod method, const char* pPath);
    /* send the response content to the client */
    int SendResponseContent(const char* pPath, bool bOnlyHeader=false);
    /* handle the error request */
    int SendFailureResponse(int nErrCode, const char* pPath);
    /* send the http header if the request should be handled correctly */
    int SendResponseHeader(int nContentLength);
    /* get the request path */
    tagReqMethod GetRequestPath(const char* pDataRequest, char* pBuf, int nSize);
    /* get the http header sent by the client */
    char* GetHeaderContent(const char* pData, const char* pName, char* pBuf, int nSize);

private:
    int m_sockConn; // socket the client request connected
};

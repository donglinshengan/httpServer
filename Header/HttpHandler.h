#pragma once

/**********************************************************/
/* HttpHandler: handle the client http request            */
/**********************************************************/
class HttpHandler
{
public:
    HttpHandler(int sockConn);
    ~HttpHandler();

public:
    int ClientConnHandler(); // the handler to the connections

protected:
    /* send the response content to the client */
    int SendResponseContent(char* pPath);
    /* handle the error request */
    int SendFailureResponse(int nErrCode);
    /* send the http header if the request should be handled correctly */
    int SendResponseHeader(int nContentLength);
    /* get the request path */
    char* GetRequestPath(const char* pData, char* pBuf, int nSize);
    /* get the http header sent by the client */
    char* GetHeaderContent(const char* pData, const char* pName, char* pBuf, int nSize);

private:
    int m_sockConn;     // socket the client request connected
};

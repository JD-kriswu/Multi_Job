#ifndef _HTTP_ADAPTER_H_
#define _HTTP_ADAPTER_H_

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include "tlog.h"


class HttpServiceAdapter
{
    private:
        std::string m_strURL;
        std::string m_strConnProxy;
        
        std::string m_strRequest;
        std::string m_strResponce;
        std::string m_strLastErrMsg;
        CURL * m_ptrCurlConn;

    public:
        explicit HttpServiceAdapter(const char * szURL)
            :m_strURL(szURL)
        {
            m_ptrCurlConn =  curl_easy_init();
            if ( m_ptrCurlConn == NULL)
            {
                TLOG_ERR("HttpServiceAdapter() CURL Init Faild!");
            }
        };
        
        HttpServiceAdapter(const char * szURL,const char* szConnProxy)
            :m_strURL(szURL),m_strConnProxy(szConnProxy)
        {
            m_ptrCurlConn =  curl_easy_init();
            if ( m_ptrCurlConn == NULL)
            {
                TLOG_ERR("HttpServiceAdapter() CURL Init Faild!");
            };
        };
        
        ~HttpServiceAdapter()
        {
             /* always cleanup */ 
            curl_easy_cleanup(m_ptrCurlConn); 
            curl_global_cleanup();
        };
        
        std::string encodeUrl(const char * szSourc);
        std::string decodeUrl(const char * szSourc);

        const std::string& getRequest() const
        {
            return m_strRequest;
        };
        
        const std::string& getResponce() const
        {
            return m_strResponce;
        };
        
        const std::string& getLastErrMsg() const
        {
            return m_strLastErrMsg;
        };
        
        int32_t callHttpMethodByGet(const std::vector<std::string>& vecHttpHeader,const char* szAgrv);
        
        int32_t callHttpMethodByPut(const std::vector<std::string>& vecHttpHeader,const char* szAgrv);
        
        int32_t callHttpMethodByPost(const std::vector<std::string>& vecHttpHeader,const char* szAgrv);
        
};

#endif

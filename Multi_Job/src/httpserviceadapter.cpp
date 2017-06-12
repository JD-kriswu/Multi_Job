#include "httpserviceadapter.h"

/* Auxiliary function that waits on the socket. */
static size_t _callbackWriteData(void *ptr, size_t size, size_t nmemb, std::string* ptrStrBuffer) {
	//std::cout << (char* ) ptr << std::endl;
	uint32_t sizes = size * nmemb;
	if (ptrStrBuffer == NULL)
	{
		return 0;
	}
	ptrStrBuffer->append(reinterpret_cast<const char *>(ptr), sizes);
	return sizes;
}

int32_t HttpServiceAdapter::callHttpMethodByGet(const std::vector<std::string>& vecHttpHeader, const char * szAgrv) {
	struct curl_slist *slistHeader = NULL;
	
	std::ostringstream ossUrl;
	ossUrl << "http://" << m_strURL << '?' << szAgrv;
	m_strRequest = ossUrl.str();
	
	if (m_ptrCurlConn) {
		CURLcode resCode;
		
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_URL,m_strRequest.c_str());
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set Url Faild: URL=[%s]", m_strRequest.c_str());
			
			return resCode;
		}

		if (!m_strConnProxy.empty()) {
			curl_easy_setopt(m_ptrCurlConn, CURLOPT_PROXY, m_strConnProxy.c_str());
		}

		//设置http头部
		if (vecHttpHeader.empty()) {
			for (size_t idx = 0; idx < vecHttpHeader.size(); ++idx) {
				slistHeader = curl_slist_append(slistHeader, vecHttpHeader[idx].c_str());
			}
			curl_easy_setopt(m_ptrCurlConn, CURLOPT_HTTPHEADER, slistHeader);
		}

		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_CONNECTTIMEOUT, 2);
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_TIMEOUT, 10);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set TimeOut Faild: ");
			
			return resCode;
		}
		//curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_WRITEFUNCTION, _callbackWriteData);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set CURLOPT_WRITEFUNCTION Faild: ");
			
			return resCode;
		}
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_WRITEDATA, &m_strResponce);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set CURLOPT_WRITEDATA Faild: ");
			
			return resCode;
		}

		resCode = curl_easy_perform(m_ptrCurlConn);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL curl_easy_perform Faild: URL=[%s] ErrMsg=[%s] ", m_strRequest.c_str(), curl_easy_strerror(resCode));
			
			return resCode;
		}
	}

	if (slistHeader != NULL)
	{
		curl_slist_free_all(slistHeader); /* free the list again */
	}
	return m_ptrCurlConn == NULL?-1 : 0;
}

int32_t HttpServiceAdapter::callHttpMethodByPut(const std::vector<std::string>& vecHttpHeader, const char* szAgrv) {
	std::ostringstream ossUrl;
	
	ossUrl << m_strURL << '?' << szAgrv;
	
	m_strRequest = ossUrl.str();
	struct curl_slist *slistHeader = NULL;
	
	if (m_ptrCurlConn) {
		CURLcode resCode;
		
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_URL,m_strRequest.c_str());
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set Url Faild: URL=[%s]", m_strRequest.c_str());
			
			return resCode;
		}

		//设置http代理
		if (!m_strConnProxy.empty()) {
			curl_easy_setopt(m_ptrCurlConn, CURLOPT_PROXY, m_strConnProxy.c_str());
		}

		//设置http头部
		if (!vecHttpHeader.empty()) {
			for (size_t idx = 0; idx < vecHttpHeader.size(); ++idx) {
				slistHeader = curl_slist_append(slistHeader, vecHttpHeader[idx].c_str());
			}
			curl_easy_setopt(m_ptrCurlConn, CURLOPT_HTTPHEADER, slistHeader);
		}

		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_CONNECTTIMEOUT, 2);
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_TIMEOUT, 10);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set TimeOut Faild: ");
			
			return resCode;
		}

		curl_easy_setopt(m_ptrCurlConn, CURLOPT_CUSTOMREQUEST, "PUT");
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_POSTFIELDS,szAgrv);
		//resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_PUT, true);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set CURLOPT_PUT Faild ");
			
			return resCode;
		}

		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_WRITEFUNCTION, _callbackWriteData);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set CURLOPT_WRITEFUNCTION Faild:  ");
			
			return resCode;
		}
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_WRITEDATA, &m_strResponce);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set CURLOPT_WRITEDATA Faild:  ");
			
			return resCode;
		}

		resCode = curl_easy_perform(m_ptrCurlConn);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL curl_easy_perform Faild: URL=[%s] ErrMsg=[%s] ", m_strRequest.c_str(), curl_easy_strerror(resCode));
			
			return resCode;
		}
		
	}

	if (slistHeader != NULL)
	{
		curl_slist_free_all(slistHeader); /* free the list again */
	}
	return m_ptrCurlConn == NULL?-1 : 0;
}

int32_t HttpServiceAdapter::callHttpMethodByPost(const std::vector<std::string>& vecHttpHeader, const char* szAgrv) {
	std::ostringstream ossUrl;
	
	ossUrl << "http://" << m_strURL;
	
	m_strRequest = ossUrl.str();
	
	struct curl_slist *slistHeader = NULL;
	
	if (m_ptrCurlConn) {
		CURLcode resCode;
		std::string strPostHeader("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
		
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_URL,m_strRequest.c_str());
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set Url Faild: URL=[%s]", m_strRequest.c_str());
			
			return resCode;
		}

		if (!m_strConnProxy.empty()) {
			curl_easy_setopt(m_ptrCurlConn, CURLOPT_PROXY, m_strConnProxy.c_str());
		}

		//设置http头部
		if (!vecHttpHeader.empty()) {
			for (size_t idx = 0; idx < vecHttpHeader.size(); ++idx) {
				slistHeader = curl_slist_append(slistHeader, vecHttpHeader[idx].c_str());
			}
			curl_easy_setopt(m_ptrCurlConn, CURLOPT_HTTPHEADER, slistHeader);
		}

		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_CONNECTTIMEOUT, 2);
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_TIMEOUT, 10);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set TimeOut Faild: ");
			
			return resCode;
		}

		//resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_HTTPHEADER, true);
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_POST, true);
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_POSTFIELDS,szAgrv);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set CURLOPT_POST Faild: ");
			
			return resCode;
		}

		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_WRITEFUNCTION, _callbackWriteData);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set CURLOPT_WRITEFUNCTION Faild: ");
			
			return resCode;
		}
		resCode = curl_easy_setopt(m_ptrCurlConn, CURLOPT_WRITEDATA, &m_strResponce);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL Set CURLOPT_WRITEDATA Faild: ");
			
			return resCode;
		}

		resCode = curl_easy_perform(m_ptrCurlConn);
		if (resCode != CURLE_OK) {
			TLOG_ERR("CURL curl_easy_perform Faild: URL=[%s] ErrMsg=[%s] ", m_strRequest.c_str(), curl_easy_strerror(resCode));
			
			return resCode;
		}
	}

	if (slistHeader != NULL)
	{
		curl_slist_free_all(slistHeader);
	}
	return m_ptrCurlConn == NULL?-1 : 0;
}

std::string HttpServiceAdapter::encodeUrl(const char * szSourc) {
	std::string strResult;
	
	if (m_ptrCurlConn) {
		char *encodedURL = curl_escape(szSourc, 0);
		if (encodedURL) {
			strResult = encodedURL;
		}
		curl_free(encodedURL);
	}
	return strResult;
}
;

std::string HttpServiceAdapter::decodeUrl(const char * szSourc) {
	std::string strResult;
	
	if (m_ptrCurlConn) {
		char *decodedURL = curl_unescape(szSourc, 0);
		if (decodedURL) {
			strResult = decodedURL;
		}
		curl_free(decodedURL);
	}
	return strResult;
}
;

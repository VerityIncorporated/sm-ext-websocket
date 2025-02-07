#include "extension.h"

HttpRequest::HttpRequest(const std::string &url, const std::string &method) : m_httpclient(true)
{
	this->m_request = m_httpclient.createRequest(url, method);
}

HttpRequest::~HttpRequest() 
{
	if (this->pResponseForward)
	{
		forwards->ReleaseForward(this->pResponseForward);
		this->pResponseForward = nullptr;
	}
}

void HttpRequest::SetBody(const std::string &body)
{
	this->m_request->body = body;
}

void HttpRequest::SetJsonBody(YYJsonWrapper* json)
{
	if (!json) return;
	
	char* jsonStr = yyjson_mut_write(json->m_pDocument_mut.get(), 0, nullptr);
	if (jsonStr)
	{
		this->m_request->body = jsonStr;
		this->m_request->extraHeaders["Content-Type"] = "application/json";
		free(jsonStr);
	}
}

void HttpRequest::AddHeader(const std::string &key, const std::string &value)
{
	this->m_request->extraHeaders[key] = value;
}

void HttpRequest::SetTimeout(int timeout)
{
	this->m_request->connectTimeout = timeout;
}

void HttpRequest::SetFollowRedirect(bool follow) 
{
	this->m_request->followRedirects = follow;
}

void HttpRequest::SetCompression(bool compress)
{
	this->m_request->compress = compress;
}

void HttpRequest::onResponse(const ix::HttpResponsePtr response) 
{
	if (!this->pResponseForward || !this->pResponseForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, response]()
	{
		HandleError err;
		HandleSecurity sec(nullptr, myself->GetIdentity());

		this->pResponseForward->PushCell(this->m_httpclient_handle);
		this->pResponseForward->PushString(response->body.c_str());
		this->pResponseForward->PushCell(response->statusCode);
		this->pResponseForward->Execute(nullptr);

		handlesys->FreeHandle(this->m_httpclient_handle, &sec);
	});
}

bool HttpRequest::Perform()
{
	return this->m_httpclient.performRequest(m_request, std::bind(&HttpRequest::onResponse, this, std::placeholders::_1));
}

const std::string& HttpRequest::GetHeader(const std::string& key) const 
{
    static const std::string empty;
    auto it = m_request->extraHeaders.find(key);
    return it != m_request->extraHeaders.end() ? it->second : empty;
}

bool HttpRequest::HasHeader(const std::string& key) const
{
    return m_request->extraHeaders.find(key) != m_request->extraHeaders.end();
}
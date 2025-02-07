#include "extension.h"

class HttpRequest
{
public:
	HttpRequest(const std::string &url, const std::string &method);
	~HttpRequest();
	
	bool Perform();
	
	void SetBody(const std::string &body);
	void SetJsonBody(YYJsonWrapper* json);
	void AddHeader(const std::string &key, const std::string &value);
	void SetTimeout(int timeout);
	void SetFollowRedirect(bool follow);
	void SetCompression(bool compress);

	const std::string& GetHeader(const std::string& key) const;
	bool HasHeader(const std::string& key) const;
	const ix::WebSocketHttpHeaders& GetHeaders() const { return m_request->extraHeaders; }

public:
	void onResponse(const ix::HttpResponsePtr response);
	
	ix::HttpClient m_httpclient;
	ix::HttpRequestArgsPtr m_request;

	Handle_t m_httpclient_handle = BAD_HANDLE;

	IChangeableForward *pResponseForward = nullptr;
};
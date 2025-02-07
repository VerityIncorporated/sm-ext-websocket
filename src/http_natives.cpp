#include "extension.h"

static HttpRequest *GetHttpPointer(IPluginContext *pContext, Handle_t Handle)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	HttpRequest *httpClient;
	if ((err = handlesys->ReadHandle(Handle, g_htHttp, &sec, (void **)&httpClient)) != HandleError_None)
	{
		pContext->ReportError("Invalid httpClient handle %x (error %d)", Handle, err);
		return nullptr;
	}

	return httpClient;
}

static cell_t http_CreateRequest(IPluginContext *pContext, const cell_t *params)
{
	char *url, *method;
	pContext->LocalToString(params[1], &url);
	pContext->LocalToString(params[2], &method);

	HttpRequest* pHttpRequest = new HttpRequest(url, method);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pHttpRequest->m_httpclient_handle = handlesys->CreateHandleEx(g_htHttp, pHttpRequest, &sec, nullptr, &err);

	if (pHttpRequest->m_httpclient_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create HttpRequest handle (error %d)", err);
		return BAD_HANDLE;
	}

	return pHttpRequest->m_httpclient_handle;
}

static cell_t http_SetResponseCallback(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);

	if (!pHttpRequest)
	{
		return 0;
	}

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);
	
	pHttpRequest->pResponseForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 3, nullptr, Param_Cell, Param_String, Param_Cell);
	if (!pHttpRequest->pResponseForward || !pHttpRequest->pResponseForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create response forward.");
		return 0;
	}

	return 1;
}

static cell_t http_Perform(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);

	if (!pHttpRequest)
	{
		return 0;
	}

	return pHttpRequest->Perform();
}

static cell_t http_SetBody(IPluginContext *pContext, const cell_t *params)
{
    HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
    if (!pHttpRequest) return 0;

    char *body;
    pContext->LocalToString(params[2], &body);
    pHttpRequest->SetBody(body);
    return 1;
}

static cell_t http_SetJsonBody(IPluginContext *pContext, const cell_t *params)
{
    HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
    YYJsonWrapper *json = g_WebsocketExt.GetJSONPointer(pContext, params[2]);
    
    if (!pHttpRequest || !json) return 0;
    
    pHttpRequest->SetJsonBody(json);
    return 1;
}

static cell_t http_AddHeader(IPluginContext *pContext, const cell_t *params)
{
    HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
    if (!pHttpRequest) return 0;

    char *key, *value;
    pContext->LocalToString(params[2], &key);
    pContext->LocalToString(params[3], &value);
    
    pHttpRequest->AddHeader(key, value);
    return 1;
}

static cell_t http_GetHeader(IPluginContext *pContext, const cell_t *params)
{
    HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
    if (!pHttpRequest) return 0;

    char *key;
    pContext->LocalToString(params[2], &key);
    
    const std::string& value = pHttpRequest->GetHeader(key);
    pContext->StringToLocalUTF8(params[3], params[4], value.c_str(), nullptr);
    
    return !value.empty();
}

static cell_t http_HasHeader(IPluginContext *pContext, const cell_t *params)
{
    HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
    if (!pHttpRequest) return 0;

    char *key;
    pContext->LocalToString(params[2], &key);
    
    return pHttpRequest->HasHeader(key);
}

const sp_nativeinfo_t http_natives[] =
{
	{"HttpRequest.HttpRequest", http_CreateRequest},
	{"HttpRequest.SetResponseCallback", http_SetResponseCallback},
	{"HttpRequest.Perform", http_Perform},
	{"HttpRequest.SetBody", http_SetBody},
	{"HttpRequest.SetJsonBody", http_SetJsonBody},
	{"HttpRequest.AddHeader", http_AddHeader},
	{"HttpRequest.GetHeader", http_GetHeader},
	{"HttpRequest.HasHeader", http_HasHeader},
	{nullptr, nullptr}
};
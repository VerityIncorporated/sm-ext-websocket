#include "extension.h"

WebSocketClient::WebSocketClient(ix::WebSocket* existingSocket) : m_webSocket(existingSocket) {}

WebSocketClient::WebSocketClient(const char* url, uint8_t type) 
{
	m_webSocket = new ix::WebSocket();
	m_webSocket->setUrl(url);
	m_webSocket->disableAutomaticReconnection();
	m_callback_type = type;

	m_webSocket->setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
		switch (msg->type)
		{
			case ix::WebSocketMessageType::Message:
			{
				OnMessage(msg->str);
				break;
			}
			case ix::WebSocketMessageType::Open:
			{
				m_headers = msg->openInfo.headers;
				OnOpen(msg->openInfo);
				break;
			}
			case ix::WebSocketMessageType::Close:
			{
				OnClose(msg->closeInfo);
				break;
			}
			case ix::WebSocketMessageType::Error:
			{
				OnError(msg->errorInfo);
				break;
			}
		}
	});
}

WebSocketClient::~WebSocketClient() 
{
	if (!m_keepConnecting)
		m_webSocket->stop();
}

bool WebSocketClient::IsConnected()
{
	return m_webSocket->getReadyState() == ix::ReadyState::Open;
}

void WebSocketClient::OnMessage(const std::string& message) 
{
	if (!pMessageForward || !pMessageForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, message]()
	{
		const size_t messageLength = message.length() + 1;

		switch (m_callback_type)
		{
			case Websocket_STRING:
			{
				pMessageForward->PushCell(m_websocket_handle);
				pMessageForward->PushString(message.c_str());
				pMessageForward->PushCell(messageLength);
				pMessageForward->Execute(nullptr);
				break;
			}
			case WebSocket_JSON:
			{
				auto pYYJsonWrapper = CreateWrapper();

				yyjson_read_err readError;
				yyjson_doc *idoc = yyjson_read_opts(const_cast<char*>(message.c_str()), message.length(), 0, nullptr, &readError);

				if (readError.code)
				{
					yyjson_doc_free(idoc);
					smutils->LogError(myself, "parse JSON message error (%u): %s at position: %d", readError.code, readError.msg, readError.pos);
					return;
				}

				pYYJsonWrapper->m_pDocument = WrapImmutableDocument(idoc);
				pYYJsonWrapper->m_pVal = yyjson_doc_get_root(idoc);

				HandleError err;
				HandleSecurity pSec(nullptr, myself->GetIdentity());
				m_json_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.release(), &pSec, nullptr, &err);

				if (!m_json_handle)
				{
					smutils->LogError(myself, "Could not create JSON handle (error %d)", err);
					return;
				}

				pMessageForward->PushCell(m_websocket_handle);
				pMessageForward->PushCell(m_json_handle);
				pMessageForward->PushCell(messageLength);
				pMessageForward->Execute(nullptr);

				handlesys->FreeHandle(m_json_handle, &pSec);
				break;
			}
		}
	});
}

void WebSocketClient::OnOpen(ix::WebSocketOpenInfo openInfo) 
{
	if (!pOpenForward || !pOpenForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, openInfo]()
	{
		pOpenForward->PushCell(m_websocket_handle);
		pOpenForward->Execute(nullptr);
	});
}

void WebSocketClient::OnClose(ix::WebSocketCloseInfo closeInfo) 
{
	if (!pCloseForward || !pCloseForward->GetFunctionCount())
	{
		return;
	}
	
	// TODO: Fixed crash when unload extension after connecting
	// 2024/06/30 - 23:09 - Fixed
	g_TaskQueue.Push([this, closeInfo]()
	{
		pCloseForward->PushCell(m_websocket_handle);
		pCloseForward->PushCell(closeInfo.code);
		pCloseForward->PushString(closeInfo.reason.c_str());
		pCloseForward->Execute(nullptr);
	});
}

void WebSocketClient::OnError(ix::WebSocketErrorInfo errorInfo) 
{
	if (!pErrorForward || !pErrorForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, errorInfo]()
	{
		pErrorForward->PushCell(m_websocket_handle);
		pErrorForward->PushString(errorInfo.reason.c_str());
		pErrorForward->Execute(nullptr);
	});
}
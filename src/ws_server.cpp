#include "extension.h"

WebSocketServer::WebSocketServer(const std::string& host, int port, int addressFamily, int pingInterval) : m_webSocketServer(
	port, 
	host,
	ix::SocketServer::kDefaultTcpBacklog,
	ix::SocketServer::kDefaultMaxConnections,
	ix::WebSocketServer::kDefaultHandShakeTimeoutSecs,
	addressFamily,
	pingInterval)
{
	m_webSocketServer.setOnClientMessageCallback([this](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg) {
		switch (msg->type)
		{
			case ix::WebSocketMessageType::Open:
			{
				OnOpen(msg->openInfo, connectionState);
				break;
			}
			case ix::WebSocketMessageType::Message:
			{
				OnMessage(msg->str, connectionState, &webSocket);
				break;
			}
			case ix::WebSocketMessageType::Close:
			{
				OnClose(msg->closeInfo, connectionState);
				break;
			}
			case ix::WebSocketMessageType::Error:
			{
				OnError(msg->errorInfo, connectionState);
				break;
			}
		}
	});
}

WebSocketServer::~WebSocketServer() 
{
	m_webSocketServer.stop();
}

void WebSocketServer::OnMessage(const std::string& message, std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket* client) 
{
	if (!pMessageForward || !pMessageForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, message, connectionState, client]()
	{
		HandleError err;
		HandleSecurity sec(nullptr, myself->GetIdentity());

		WebSocketClient* pWebSocketClient = new WebSocketClient(client);

		pWebSocketClient->m_websocket_handle = handlesys->CreateHandleEx(g_htWsClient, pWebSocketClient, &sec, nullptr, &err);
		if (!pWebSocketClient->m_websocket_handle) return;
		pWebSocketClient->m_keepConnecting = true;

		std::string remoteAddress = connectionState->getRemoteIp() + ":" + std::to_string(connectionState->getRemotePort());
		pMessageForward->PushCell(m_webSocketServer_handle);
		pMessageForward->PushCell(pWebSocketClient->m_websocket_handle);
		pMessageForward->PushString(message.c_str());
		pMessageForward->PushCell(message.length());
		pMessageForward->PushString(remoteAddress.c_str());
		pMessageForward->PushString(connectionState->getId().c_str());
		pMessageForward->Execute(nullptr);
		
		handlesys->FreeHandle(pWebSocketClient->m_websocket_handle, nullptr);
	});
}

void WebSocketServer::OnOpen(ix::WebSocketOpenInfo openInfo, std::shared_ptr<ix::ConnectionState> connectionState) 
{
	if (!pOpenForward || !pOpenForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, openInfo, connectionState]()
	{
		std::string remoteAddress = connectionState->getRemoteIp() + ":" + std::to_string(connectionState->getRemotePort());
		pOpenForward->PushCell(m_webSocketServer_handle);
		pOpenForward->PushString(remoteAddress.c_str());
		pOpenForward->PushString(connectionState->getId().c_str());
		pOpenForward->Execute(nullptr);
	});
}

void WebSocketServer::OnClose(ix::WebSocketCloseInfo closeInfo, std::shared_ptr<ix::ConnectionState> connectionState) 
{
	if (!pCloseForward || !pCloseForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, closeInfo, connectionState]()
	{
		std::string remoteAddress = connectionState->getRemoteIp() + ":" + std::to_string(connectionState->getRemotePort());
		pCloseForward->PushCell(m_webSocketServer_handle);
		pCloseForward->PushCell(closeInfo.code);
		pCloseForward->PushString(closeInfo.reason.c_str());
		pCloseForward->PushString(remoteAddress.c_str());
		pCloseForward->PushString(connectionState->getId().c_str());
		pCloseForward->Execute(nullptr);
	});
}

void WebSocketServer::OnError(ix::WebSocketErrorInfo errorInfo, std::shared_ptr<ix::ConnectionState> connectionState) 
{
	if (!pErrorForward || !pErrorForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, errorInfo, connectionState]()
	{
		std::string remoteAddress = connectionState->getRemoteIp() + ":" + std::to_string(connectionState->getRemotePort());
		pErrorForward->PushCell(m_webSocketServer_handle);
		pErrorForward->PushString(errorInfo.reason.c_str());
		pErrorForward->PushString(remoteAddress.c_str());
		pErrorForward->PushString(connectionState->getId().c_str());
		pErrorForward->Execute(nullptr);
	});
}

void WebSocketServer::broadcastMessage(const std::string& message) {
	auto clients = m_webSocketServer.getClients();

	for (const auto& client : clients)
	{
		client.first->send(message);
	} 
}

bool WebSocketServer::sendToClient(const std::string& clientId, const std::string& message) {
	auto clients = m_webSocketServer.getClients();
	
	for (const auto& client : clients)
	{
		if (client.second == clientId) {
			client.first->send(message);
			return true;
		}
	}
	return false;
}
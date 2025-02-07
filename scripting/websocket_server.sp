#include <sourcemod>
#include <websocket>

public Plugin myinfo = {
	name = "WebSocket Server Test",
	author = "ProjectSky",
	description = "Used for testing websocket server",
	version = "1.0.0",
	url = "https://github.com/ProjectSky/sm-ext-websocket"
}

WebSocketServer
g_hWsServer;

public void OnPluginStart()
{
  RegServerCmd("ws_server", ws_server);
  RegServerCmd("ws_server_stop", ws_server_stop);
  RegServerCmd("ws_server_broadcast", ws_server_broadcast);
  RegServerCmd("ws_server_clients", ws_server_clients);
  RegServerCmd("ws_server_sendtoclient", ws_server_sendtoclient);
}

Action ws_server(int args)
{
  g_hWsServer = new WebSocketServer("0.0.0.0", 9999);
  g_hWsServer.SetMessageCallback(onSrvMessage);
  g_hWsServer.SetOpenCallback(onSrvOpen);
  g_hWsServer.SetCloseCallback(onSrvClose);
  g_hWsServer.SetErrorCallback(onSrvError);
  g_hWsServer.Start();
  return Plugin_Handled;
}

Action ws_server_broadcast(int args)
{
  g_hWsServer.BroadcastMessage("Broadcast Message");
  return Plugin_Handled;
}

Action ws_server_sendtoclient(int args)
{
  char clientId[4], message[256];
  GetCmdArg(1, clientId, sizeof(clientId));
  GetCmdArg(2, message, sizeof(message));

  g_hWsServer.SendMessageToClient(clientId, message);
  return Plugin_Handled;
}

Action ws_server_clients(int args)
{
  PrintToServer("Connected clients: %d", g_hWsServer.ClientsCount)
  return Plugin_Handled;
}

Action ws_server_stop(int args)
{
  g_hWsServer.Stop();
  return Plugin_Handled;
}

void onSrvMessage(WebSocketServer ws, WebSocket client, const char[] message, int wireSize, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("message: %s, wireSize: %d, RemoteAddr: %s, RemoteId: %s", message, wireSize, RemoteAddr, RemoteId);
}

void onSrvError(WebSocketServer ws, const char[] errMsg, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("onError: %s, RemoteAddr: %s, RemoteId: %s", errMsg, RemoteAddr, RemoteId);
}

void onSrvOpen(WebSocketServer ws, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("onOpen: %x, RemoteAddr: %s, RemoteId: %s", ws, RemoteAddr, RemoteId);
}

void onSrvClose(WebSocketServer ws, int code, const char[] reason, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("onClose: %d, reason: %s, RemoteAddr: %s, RemoteId: %s", code, reason, RemoteAddr, RemoteId);
}
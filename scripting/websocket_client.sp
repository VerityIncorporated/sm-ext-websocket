#include <sourcemod>
#include <websocket>

public Plugin myinfo = {
	name = "WebSocket Client Test",
	author = "ProjectSky",
	description = "Used for testing websocket client",
	version = "1.0.0",
	url = "https://github.com/ProjectSky/sm-ext-websocket"
}

WebSocket
g_hWS;

public void OnPluginStart()
{
  RegServerCmd("ws_start", ws_start);
  RegServerCmd("ws_close", ws_close);
  RegServerCmd("ws_state", ws_state);
}

Action ws_start(int args)
{
  g_hWS = new WebSocket("ws://127.0.0.1:9999", WebSocket_JSON);
  g_hWS.SetMessageCallback(onMessage);
  g_hWS.SetOpenCallback(onOpen);
  g_hWS.SetCloseCallback(onClose);
  g_hWS.SetErrorCallback(onError)
  g_hWS.Connect();
}

Action ws_close(int args)
{
  g_hWS.Disconnect();
  return Plugin_Handled;
}

Action ws_state(int args)
{
  PrintToServer("ReadyState: %d", g_hWS.ReadyState);
  return Plugin_Handled;
}

void onOpen(WebSocket ws)
{
  PrintToServer("onOpen: %x", ws);
}

void onClose(WebSocket ws, int code, const char[] reason)
{
  PrintToServer("onClose: %d, %s", code, reason);
}

void onMessage(WebSocket ws, const YYJSON message, int wireSize)
{
  char[] buffer = new char[wireSize];
  message.ToString(buffer, wireSize);
  PrintToServer("msg: %s, size: %d", buffer, wireSize);
}

void onError(WebSocket ws, const char[] errMsg)
{
  PrintToServer("onError: %s", errMsg);
}
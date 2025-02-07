#include <sourcemod>
#include <websocket>

public Plugin myinfo = {
  name = "WebSocket Discord Test",
  author = "ProjectSky",
  description = "Used for testing connect discord websocket",
  version = "1.0.0",
  url = "https://github.com/ProjectSky/sm-ext-websocket"
}

WebSocket g_hWebSocket;
ConVar g_hBotToken;
char g_sDiscordToken[128];

public void OnPluginStart()
{
  RegServerCmd("discord_connect", discord_connect);
  RegServerCmd("discord_disconnect", discord_disconnect);

  g_hBotToken = CreateConVar("discord_bot_token", "", "Set Your Discord bot token", FCVAR_PROTECTED|FCVAR_HIDDEN);
}

Action discord_connect(int args)
{
  g_hBotToken.GetString(g_sDiscordToken, sizeof(g_sDiscordToken));

  if (g_sDiscordToken[0] == '\0') SetFailState("Discord bot token is empty!");

  g_hWebSocket = new WebSocket("wss://gateway.discord.gg/?v=6&encoding=json", WebSocket_JSON);
  g_hWebSocket.SetMessageCallback(onMessage);
  g_hWebSocket.SetOpenCallback(onOpen);
  g_hWebSocket.SetCloseCallback(onClose);
  g_hWebSocket.SetErrorCallback(onError);
  g_hWebSocket.Connect();
  return Plugin_Handled;
}

Action discord_disconnect(int args)
{
  g_hWebSocket.Disconnect();
  return Plugin_Handled;
}

/* Hello Event
{
  "t": null,
  "s": null,
  "op": 10,
  "d": {
    "heartbeat_interval": 41250,
    "_trace": [
      "[\"gateway-prd-us-east1-d-2gqr\",{\"micros\":0.0}]"
    ]
  }
}
*/

/* Gateway Events (GUILD_MESSAGES -> MESSAGE_CREATE)
{
  "t": "MESSAGE_CREATE",
  "s": 12,
  "op": 0,
  "d": {
    "webhook_id": "WEBHOOK_ID",
    "type": 0,
    "tts": false,
    "timestamp": "2024-09-23T05:06:54.730000+00:00",
    "pinned": false,
    "mentions": [],
    "mention_roles": [],
    "mention_everyone": false,
    "id": "MSG_ID",
    "flags": 0,
    "embeds": [],
    "edited_timestamp": null,
    "content": "MSG_CONTENT",
    "components": [],
    "channel_id": "CHANNEL_ID",
    "author": {
      "username": "USERNAME",
      "id": "WEBHOOK_ID",
      "global_name": null,
      "discriminator": "0000",
      "bot": true,
      "avatar": null
    },
    "attachments": [],
    "guild_id": "GUILD_ID"
  }
}
*/
void onMessage(WebSocket ws, const YYJSON message, int wireSize)
{
  int opcode = message.PtrGetInt("/op");

  switch (opcode)
  {
    case 10:
    {
      float heartbeat_interval = message.PtrGetInt("/d/heartbeat_interval") / 1000.0;
      CreateTimer(heartbeat_interval, Timer_SendHeartbeat, .flags = TIMER_REPEAT);
    }
    case 0:
    {
      char events[64];
      message.PtrGetString("/t", events, sizeof(events));

      if (strcmp(events, "MESSAGE_CREATE") == 0)
      {
        static char content[2048];
        message.PtrGetString("/d/content", content, sizeof(content));
        PrintToServer("content: %s", content);
      }
    }
  }
}

void onError(WebSocket ws, const char[] errMsg)
{
  PrintToServer("onError: %s", errMsg);
}

void onClose(WebSocket ws, int code, const char[] reason)
{
  PrintToServer("onClose: %d, %s", code, reason);
}

/* Identify Payload
{
  "op": 2,
  "d": {
    "token": "YOU_TOKEN",
    "properties": {
      "$os": "linux",
      "$browser": "chrome",
      "$device": "chrome"
    }
  }
}
*/
void onOpen(WebSocket ws)
{
  PrintToServer("onOpen: %x", ws);

  YYJSONObject payload = new YYJSONObject();
  payload.SetInt("op", 2);
  payload.PtrSetString("/d/token", g_sDiscordToken);
  payload.PtrSetString("/d/properties/$os", "linux");
  payload.PtrSetString("/d/properties/$browser", "chrome");
  payload.PtrSetString("/d/properties/$device", "chrome");
  g_hWebSocket.WriteJSON(payload);

  delete payload;
}

/* Heartbeat Requests
{
  "op": 1,
  "d": null
}
*/
Action Timer_SendHeartbeat(Handle timer)
{
  if (!g_hWebSocket.Connected) return Plugin_Stop;

  PrintToServer("SendHeartbeat");

  YYJSONObject heartbeat = new YYJSONObject();
  heartbeat.SetInt("op", 1);
  heartbeat.SetNull("d");
  g_hWebSocket.WriteJSON(heartbeat);

  delete heartbeat;

  return Plugin_Continue;
}
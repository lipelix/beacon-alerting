export default {
  async fetch(request, env) {
    const upgradeHeader = request.headers.get("Upgrade");

    if (!upgradeHeader || upgradeHeader !== "websocket") {
      return new Response("Expected Upgrade: websocket", { status: 426 });
    }

    const webSocketPair = new WebSocketPair();
    const [client, server] = Object.values(webSocketPair);

    server.accept();
    server.addEventListener("message", (event) => {
      console.log(event.data);
    });

    setInterval(async () => {
      const activeAlerts = await env.BEACON_ALERTS_KV.get("activeAlerts");

      server.send((new Set(JSON.parse(activeAlerts))).size > 0 ? "ON" : "OFF");
    }, 5000);

    return new Response(null, {
      status: 101,
      webSocket: client,
    });
  },
};

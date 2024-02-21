export default {
    async fetch(request) {
      const upgradeHeader = request.headers.get('Upgrade');
  
      console.log("delej", request.method);
      console.log("upgradeHeader", upgradeHeader);
  
      if (!upgradeHeader || upgradeHeader !== 'websocket') {
        return new Response('Expected Upgrade: websocket', { status: 426 });
      }
    
      const webSocketPair = new WebSocketPair();
      const [client, server] = Object.values(webSocketPair);
    
      server.accept();
      server.addEventListener('message', event => {
        console.log(event.data);
      });
  
      const actions = ["Acknowledge", "Close", "Create", "UnAcknowledge"]
  
      setInterval(() => server.send(actions[actions.length * Math.random() | 0]), 5000)
  
      return new Response(null, {
        status: 101,
        webSocket: client,
      });
    },
  };
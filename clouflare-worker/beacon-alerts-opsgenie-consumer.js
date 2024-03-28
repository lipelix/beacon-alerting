export default {
    async fetch(request, env) {
      if (request.method !== "POST") {
        return new Response("Method not allowed", { status: 405 });
      }
  
      const json = await request.json();
  
      const { alert, action } = json;
  
      const kvNamespace = "beacon-alerts-kv";
      const kvKey = alert.alertId;
      const kvValue = action;
  
      const kvTtl = 2 * 60 * 60; // 2 hours in seconds
  
      try {
  
        if (action === "Create" || action === "UnAcknowledge") {
          await env.BEACON_ALERTS_KV.put(kvKey, kvValue, {
            expirationTtl: kvTtl,
            namespace: kvNamespace,
          });
        }
  
        if (action === "Close" || action === "Acknowledge") {
          await env.BEACON_ALERTS_KV.delete(kvKey);
        }
  
        return new Response("Data saved to KV", { status: 200 });
      } catch (error) {
        return new Response("Error saving data to KV", { status: 500 });
      }
    },
  };
  
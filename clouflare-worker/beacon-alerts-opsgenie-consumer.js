export default {
    async fetch(request, env) {
      if (request.method !== "POST") {
        return new Response("Method not allowed", { status: 405 });
      }
  
      if (!request.headers.get("opsgenie-one-account-hash") || request.headers.get("opsgenie-one-account-hash") !== env.OPSGENIE_ONE_ACCOUNT_HASH) {
        return new Response(`'opsgenie-one-account-hash' is invalid or missing`, {status: 403});
      }
  
      let json;
      try {
        json = await request.json();
      } catch (error) {
        return new Response(`Malformated or missing event json in request body.`, {status: 400})
      }
  
      const { alert, action } = json;
  
      const kvNamespace = "beacon-alerts-kv";
      const kvKey = "activeAlerts";
  
      const kvTtl = 2 * 60 * 60; // 2 hours in seconds
  
      try {
        const activeAlertsValue = await env.BEACON_ALERTS_KV.get(kvKey)
  
        if (action === "Create" || action === "UnAcknowledge") {
  
          // Create KV activeAlerts if it doesnt exist (only on initilazation)
          if (!activeAlertsValue) {
            await env.BEACON_ALERTS_KV.put(kvKey, JSON.stringify([alert.alertId]), {
              expirationTtl: kvTtl,
              namespace: kvNamespace,
            });
          }
  
          // Update KV activeAlerts if it already exists
          if (activeAlertsValue) {
            const activeAlertsWithNewAlert = new Set(JSON.parse(activeAlertsValue))
            activeAlertsWithNewAlert.add(alert.alertId)
  
            await env.BEACON_ALERTS_KV.put(kvKey, JSON.stringify(Array.from(activeAlertsWithNewAlert.values())), {
              expirationTtl: kvTtl,
              namespace: kvNamespace,
            });
          }
        }
  
        if (action === "Close" || action === "Acknowledge") {
          const activeAlertsWithoutAlert = new Set(JSON.parse(activeAlertsValue))
          activeAlertsWithoutAlert.delete(alert.alertId)
  
          await env.BEACON_ALERTS_KV.put(kvKey, JSON.stringify(Array.from(activeAlertsWithoutAlert)), {
            expirationTtl: kvTtl,
            namespace: kvNamespace,
          });
        }
  
        return new Response("Data saved to KV", { status: 200 });
      } catch (error) {
        return new Response("Error saving data to KV", { status: 500 });
      }
    },
  };
  
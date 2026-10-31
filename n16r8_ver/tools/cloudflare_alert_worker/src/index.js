/**
 * MotoMap Emergency Alert Gateway - Cloudflare Worker
 *
 * Design Principles:
 * 1. Clean code & zero-dependency (native V8 fetch API).
 * 2. Security: No hardcoded secrets. Reads from Cloudflare Environment Secrets.
 * 3. CAD / eCall message format: Professional, uncluttered alert payload.
 * 4. Dynamic recipient dispatching via Chat IDs array.
 */

const ZALO_API_URL = "https://bot-api.zaloplatforms.com";

function formatTimestamp(isoString) {
  try {
    const d = isoString ? new Date(isoString) : new Date();
    // Convert UTC to ICT (Vietnam UTC+7)
    const options = {
      timeZone: "Asia/Ho_Chi_Minh",
      year: "numeric",
      month: "2-digit",
      day: "2-digit",
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
      hour12: false
    };
    return new Intl.DateTimeFormat("vi-VN", options).format(d);
  } catch {
    return isoString || "N/A";
  }
}

function formatEmergencyMessage(data) {
  const timeStr = formatTimestamp(data.timestamp);
  const lat = Number(data.latitude);
  const lon = Number(data.longitude);
  const latStr = !isNaN(lat) ? lat.toFixed(6) : "N/A";
  const lonStr = !isNaN(lon) ? lon.toFixed(6) : "N/A";
  const speedStr = data.speedKmh !== undefined && !isNaN(Number(data.speedKmh))
    ? `${Number(data.speedKmh).toFixed(1)} km/h`
    : "N/A";
  const course = Number(data.courseDeg);
  const headingStr = !isNaN(course) ? `${course.toFixed(0)} deg` : "N/A";
  const mapUrl = !isNaN(lat) && !isNaN(lon)
    ? `https://maps.google.com/?q=${latStr},${lonStr}`
    : "Unavailable";

  return [
    "[MOTOMAP - EMERGENCY ALERT / THONG BAO KHAN CAP]",
    `Event / Su kien: ${data.event || "Collision / Fall Detected"}`,
    `Time / Thoi gian: ${timeStr}`,
    `Coordinates / Toa do: ${latStr}, ${lonStr}`,
    `Map / Ban do: ${mapUrl}`,
    `Speed / Van toc: ${speedStr}`,
    `Heading / Huong: ${headingStr}`,
    `Device / Thiet bi: ${data.deviceId || "MotoMap-S3"}`,
    `Status / Trang thai: ${data.status || "Auto-Triggered"}`
  ].join("\n");
}

export default {
  async fetch(request, env) {
    // Support CORS preflight for testing and dashboard tools
    if (request.method === "OPTIONS") {
      return new Response(null, {
        headers: {
          "Access-Control-Allow-Origin": "*",
          "Access-Control-Allow-Methods": "POST, OPTIONS",
          "Access-Control-Allow-Headers": "Content-Type, Authorization"
        }
      });
    }

    if (request.method !== "POST") {
      return new Response(JSON.stringify({ error: "Method not allowed. Use POST." }), {
        status: 405,
        headers: { "Content-Type": "application/json" }
      });
    }

    // 1. Verify ESP32 request bearer token
    const authHeader = request.headers.get("Authorization");
    const expectedKey = env.GATEWAY_AUTH_KEY;
    if (expectedKey && (!authHeader || authHeader !== `Bearer ${expectedKey}`)) {
      return new Response(JSON.stringify({ error: "Unauthorized access" }), {
        status: 401,
        headers: { "Content-Type": "application/json" }
      });
    }

    // 2. Parse & Validate Payload JSON
    let payload;
    try {
      payload = await request.json();
    } catch {
      return new Response(JSON.stringify({ error: "Invalid JSON payload" }), {
        status: 400,
        headers: { "Content-Type": "application/json" }
      });
    }

    // 3. Resolve target chat IDs
    let targetChats = [];
    if (Array.isArray(payload.chatIds) && payload.chatIds.length > 0) {
      targetChats = payload.chatIds.map(id => String(id).trim()).filter(Boolean);
    } else if (payload.chatId) {
      targetChats = [String(payload.chatId).trim()];
    } else if (env.DEFAULT_ZALO_CHAT_IDS) {
      targetChats = env.DEFAULT_ZALO_CHAT_IDS.split(",").map(id => id.trim()).filter(Boolean);
    }

    if (targetChats.length === 0) {
      return new Response(JSON.stringify({ error: "No target Chat IDs provided" }), {
        status: 400,
        headers: { "Content-Type": "application/json" }
      });
    }

    const botToken = env.ZALO_BOT_TOKEN;
    if (!botToken) {
      return new Response(JSON.stringify({ error: "ZALO_BOT_TOKEN secret is not configured" }), {
        status: 500,
        headers: { "Content-Type": "application/json" }
      });
    }

    // 4. Format emergency message
    const messageText = formatEmergencyMessage(payload);

    // 5. Broadcast message to all target Chat IDs
    const results = [];
    for (const chatId of targetChats) {
      try {
        const zaloResponse = await fetch(`${ZALO_API_URL}/bot${botToken}/sendMessage`, {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({
            chat_id: chatId,
            text: messageText
          })
        });

        const zaloResult = await zaloResponse.json().catch(() => ({}));
        results.push({
          chatId,
          ok: zaloResponse.ok && (zaloResult.error === 0 || zaloResult.ok === true),
          status: zaloResponse.status,
          response: zaloResult
        });
      } catch (err) {
        results.push({
          chatId,
          ok: false,
          error: err.message
        });
      }
    }

    const allSuccessful = results.every(r => r.ok);
    return new Response(JSON.stringify({
      ok: allSuccessful,
      sentCount: results.filter(r => r.ok).length,
      totalTargets: targetChats.length,
      details: results
    }), {
      status: allSuccessful ? 200 : 207,
      headers: { "Content-Type": "application/json" }
    });
  }
};

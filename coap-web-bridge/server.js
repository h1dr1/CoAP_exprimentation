const express = require('express');
const coap = require('coap');
const app = express();

const ESP32_IP = '10.182.252.187'; // ⚠️ CHANGE THIS to your ESP32 IP
const COAP_PORT = 5683;
const COAP_TIMEOUT = 3000; // 3 seconds timeout

app.use(express.json());
app.use(express.static('public')); // serve the HTML page

// ===== MIDDLEWARE CORS =====
app.use((req, res, next) => {
  res.header('Access-Control-Allow-Origin', '*');
  res.header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
  res.header('Access-Control-Allow-Headers', 'Content-Type');
  if (req.method === 'OPTIONS') {
    return res.sendStatus(204);
  }
  next();
});

// ===== GET LED state =====
app.get('/led', (req, res) => {
  console.log('📥 GET /led request received');
  
  const reqCoap = coap.request({
    hostname: ESP32_IP,
    port: COAP_PORT,
    pathname: '/led',
    method: 'GET',
    confirmable: true
  });

  let timeoutOccurred = false;
  
  // Timeout handler
  const timeout = setTimeout(() => {
    timeoutOccurred = true;
    console.error('⏱️ CoAP request timeout - ESP32 not responding');
    console.error('Check:');
    console.error('  1. ESP32 is powered on');
    console.error('  2. ESP32 IP address is correct:', ESP32_IP);
    console.error('  3. ESP32 is on the same network');
    
    if (!res.headersSent) {
      res.status(504).send('ESP32 timeout');
    }
  }, COAP_TIMEOUT);

  reqCoap.on('response', (coapRes) => {
    if (timeoutOccurred) return;
    
    clearTimeout(timeout);
    const payload = coapRes.payload.toString();
    console.log('✅ CoAP response:', payload);
    
    try {
      // Try to parse JSON response
      const data = JSON.parse(payload);
      res.send(data.state);
    } catch (e) {
      // If not JSON, send as-is (should be "on" or "off")
      res.send(payload);
    }
  });

  reqCoap.on('error', (err) => {
    if (timeoutOccurred) return;
    
    clearTimeout(timeout);
    console.error('❌ CoAP error:', err.message);
    
    if (!res.headersSent) {
      res.status(500).send('CoAP Error: ' + err.message);
    }
  });

  reqCoap.end();
});

// ===== POST LED on/off =====
app.post('/led', (req, res) => {
  const { state } = req.body;
  console.log('📤 POST /led request:', state);

  if (!state || (state !== 'on' && state !== 'off')) {
    return res.status(400).send('Invalid state. Use "on" or "off"');
  }

  const reqCoap = coap.request({
    hostname: ESP32_IP,
    port: COAP_PORT,
    pathname: '/led',
    method: 'PUT',
    confirmable: true
  });

  let timeoutOccurred = false;

  // Timeout handler
  const timeout = setTimeout(() => {
    timeoutOccurred = true;
    console.error('⏱️ CoAP request timeout');
    
    if (!res.headersSent) {
      res.status(504).send('ESP32 timeout');
    }
  }, COAP_TIMEOUT);

  reqCoap.write(state);

  reqCoap.on('response', (coapRes) => {
    if (timeoutOccurred) return;
    
    clearTimeout(timeout);
    const payload = coapRes.payload.toString();
    console.log('✅ CoAP response:', payload);
    res.send(payload);
  });

  reqCoap.on('error', (err) => {
    if (timeoutOccurred) return;
    
    clearTimeout(timeout);
    console.error('❌ CoAP error:', err.message);
    
    if (!res.headersSent) {
      res.status(500).send('CoAP Error: ' + err.message);
    }
  });

  reqCoap.end();
});

// ===== Health check endpoint =====
app.get('/health', (req, res) => {
  res.json({ 
    status: 'ok', 
    esp32_ip: ESP32_IP,
    coap_port: COAP_PORT 
  });
});

// ===== Test ESP32 connectivity =====
app.get('/test-esp32', (req, res) => {
  console.log('🔍 Testing ESP32 connectivity...');
  
  const reqCoap = coap.request({
    hostname: ESP32_IP,
    port: COAP_PORT,
    pathname: '/led',
    method: 'GET',
    confirmable: true
  });

  let responded = false;

  const timeout = setTimeout(() => {
    if (!responded) {
      console.error('❌ ESP32 not responding');
      res.json({ 
        connected: false, 
        error: 'Timeout - ESP32 not responding',
        esp32_ip: ESP32_IP 
      });
    }
  }, 3000);

  reqCoap.on('response', (coapRes) => {
    responded = true;
    clearTimeout(timeout);
    console.log('✅ ESP32 is responding!');
    res.json({ 
      connected: true, 
      response: coapRes.payload.toString(),
      esp32_ip: ESP32_IP
    });
  });

  reqCoap.on('error', (err) => {
    responded = true;
    clearTimeout(timeout);
    console.error('❌ ESP32 error:', err.message);
    res.json({ 
      connected: false, 
      error: err.message,
      esp32_ip: ESP32_IP 
    });
  });

  reqCoap.end();
});

// ===== Start server =====
const PORT = 3000;
app.listen(PORT, () => {
  console.log('═══════════════════════════════════════');
  console.log('🚀 CoAP Bridge Server Started');
  console.log('═══════════════════════════════════════');
  console.log(`📡 Server: http://localhost:${PORT}`);
  console.log(`🔌 ESP32: coap://${ESP32_IP}:${COAP_PORT}`);
  console.log('═══════════════════════════════════════');
  console.log('Endpoints:');
  console.log('  GET  /led         - Get LED state');
  console.log('  POST /led         - Set LED state');
  console.log('  GET  /health      - Health check');
  console.log('  GET  /test-esp32  - Test ESP32 connection');
  console.log('═══════════════════════════════════════');
  console.log('');
  console.log('💡 Open http://localhost:3000/test-esp32 to test ESP32');
  console.log('');
});
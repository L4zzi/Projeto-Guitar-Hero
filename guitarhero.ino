#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>

// ✅ Configurações WiFi 
// rodear por iphone
//const char* ssid = "iPhone 8 de Lucca ";
//const char* password = "12345678";
 
// senha de casa teste
const char* ssid = "VIVOFIBRA-3161";
const char* password = "8MYLhYWz8T";

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

// ✅ Botões do Guitar Hero
const int buttonPins[] = {18, 19, 21};
const int numButtons = 3;

bool websocketConnected = false;

// ✅ Estado anterior dos botões para detecção de mudança
int lastButtonState[] = {HIGH, HIGH, HIGH, HIGH};

// ✅ Função de evento do WebSocket
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("❌ WebSocket desconectado!");
      websocketConnected = false;
      break;
      
    case WStype_CONNECTED:
      Serial.println("✅✅✅ WEBSOCKET CONECTADO COM SUCESSO!");
      websocketConnected = true;
      break;
      
    case WStype_TEXT:
      Serial.print("📨 Recebido: ");
      Serial.println((char*)payload);
      break;
      
    case WStype_ERROR:
      Serial.println("❌ Erro no WebSocket");
      break;
      
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("🎸 Iniciando Guitar Hero IoT...");

  // ✅ Configura botões com PULLUP
  for(int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    lastButtonState[i] = digitalRead(buttonPins[i]); // Lê estado inicial
    Serial.printf("✅ Botão %d no GPIO %d - Estado: %s\n", 
                  i, buttonPins[i], 
                  lastButtonState[i] == HIGH ? "SOLTO" : "PRESSIONADO");
  }

  // ✅ Conecta WiFi
  WiFiMulti.addAP(ssid, password);
  Serial.println("📡 Conectando WiFi...");
  
  while(WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println();
  Serial.println("✅ WiFi conectado!");
  Serial.print("📶 IP: ");
  Serial.println(WiFi.localIP());

  // ✅✅✅ CORRIGIDO: Aspas fechadas corretamente!
  // ALTERE ESTE IP PARA O DO SEU COMPUTADOR!
  webSocket.begin("192.168.15.7", 3000, "/"); // ← IP DO SEU PC AQUI!
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  Serial.println("🔄 Tentando conectar WebSocket...");
}

void loop() {
  webSocket.loop();
  checkButtons();
}

void checkButtons() {
  for(int i = 0; i < numButtons; i++) {
    int currentState = digitalRead(buttonPins[i]);
    
    // ✅ Só processa se o estado mudou
    if (currentState != lastButtonState[i]) {
      // Aguarda debounce
      delay(10);
      currentState = digitalRead(buttonPins[i]); // Lê novamente
      
      if (currentState != lastButtonState[i]) {
        lastButtonState[i] = currentState;
        
        // ✅ Só envia quando o botão é PRESSIONADO (LOW)
        if (currentState == LOW) {
          sendButtonPress(i);
        } else {
          Serial.printf("🔼 Botão %d SOLTO\n", i);
        }
      }
    }
  }
}

void sendButtonPress(int buttonIndex) {
  unsigned long timestamp = millis();
  
  String message = "{";
  message += "\"type\":\"button_press\",";
  message += "\"button\":" + String(buttonIndex) + ",";
  message += "\"timestamp\":" + String(timestamp) + ",";
  message += "\"device\":\"guitar_hero_esp32\"";
  message += "}";
  
  Serial.printf("🎮 Botão %d PRESSIONADO → %s\n", buttonIndex, message.c_str());
  
  if(websocketConnected) {
    webSocket.sendTXT(message);
    Serial.println("📤 Enviado via WebSocket!");
  } else {
    Serial.println("❌ WebSocket não conectado");
    
    // ✅ Tenta reconectar
    Serial.println("🔄 Tentando reconectar WebSocket...");
    webSocket.begin("192.168.15.7", 3000, "/"); // ← MESMO IP DE ANTES!
  }
}
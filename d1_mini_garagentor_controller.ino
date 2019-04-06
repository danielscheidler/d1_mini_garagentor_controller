#include <ESP8266WiFi.h>

/* ******************************** */
/*   CONFIGURATION START            */
/* ******************************** */

/* USED PINS */
#define RELAIS_PIN    4   // D2  Relais

/* WLAN SETTINGS */
const char* WIFI_SSID = "YOUR_WLAN_SSID";
const char* WIFI_PASSWORD = "YOUR_WLAN_PASSWORD";

IPAddress ip(192, 168, 1, 31);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

/* ******************************** */
/*   CONFIGURATION END              */
/* ******************************** */
 
String sGetstart = "GET ";
WiFiServer server(80);
WiFiClient client;

/**
 * default initialization
 */
void setup() {
  pinMode(RELAIS_PIN, OUTPUT);
  digitalWrite(RELAIS_PIN, HIGH);

  Serial.begin(9600);
  delay(1);

  WiFi.mode(WIFI_STA);
  WiFiStart();
}


/**
 * Main-Loop
 */
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi restart");
    WiFiStart();
  }

  if (waitForClient()) {
    String request = client.readStringUntil('\r');
    client.flush();
    
    if (request != "") {
      Serial.print("Request: ");
      Serial.println(request);

      if(validateUrl(getUrlPath(request))) {
        showWebsite(handleWebRequest(getUrlParams(request)));
      }
    }

    client.stop();
  }
  delay(1);
}


/**
 * Shift relais for 1 second
 */
void shiftRelais(){
  digitalWrite(RELAIS_PIN, LOW);
  delay(1000);
  digitalWrite(RELAIS_PIN, HIGH);
}

/**
 * handle single url-parameter
 */
boolean handleUrlParameter(String paramString){
  int splitIndex = paramString.indexOf("=");
  String pName = paramString.substring(0,splitIndex);
  String pValue = paramString.substring(splitIndex+1);
  
  Serial.print(pName);
  Serial.print(" = ");
  Serial.println(pValue);

  if(pName=="shift"){
    Serial.println("Schalte-Relais");
    shiftRelais();
    return true;
  }
  return false;
}

/**
 * split url and handle the parameter
 */
boolean handleWebRequest(String request){
  int startIndex = request.indexOf("?")>=0?request.indexOf("?")+1:(request.indexOf("/")>=0?request.indexOf("/")+1:0);
  boolean handled = false;
  
  int i = 0;
  while(startIndex>=0 && request.length()>0){ // && request.indexOf("=")>=0
    request = request.substring(startIndex);
    int endIndex = request.indexOf("&")>=0?request.indexOf("&"):(request.indexOf(" ")>=0?request.indexOf(" "):request.length());
    String splitString = request.substring(0, endIndex);
    Serial.println(splitString);
 
    if(handleUrlParameter(splitString)){
      handled = true;
    }
     
    startIndex = request.indexOf("&")>=0?request.indexOf("&")+1:-1;
    i++;
  }
  return handled;
}

/**
 * get Paramss from Request-URL
 */
String getUrlParams(String request){
  String sParam = "";
  
  int iStart, iEndSpace, iEndQuest;
  iStart = request.indexOf(sGetstart);
  if (iStart >= 0) {
    iStart += +sGetstart.length();
    iEndSpace = request.indexOf(" ", iStart);
    iEndQuest = request.indexOf("?", iStart);

    if (iEndSpace > 0) {
      if (iEndQuest > 0) {
        sParam = request.substring(iEndQuest, iEndSpace);
      }  
    }        
  }
  return sParam;
}

/**
 * get Path from Request-URL
 */
String getUrlPath(String request){
  String sPath = "";
  int iStart, iEndSpace, iEndQuest;
  iStart = request.indexOf(sGetstart);
  
  if (iStart >= 0) {
    iStart += +sGetstart.length();
    iEndSpace = request.indexOf(" ", iStart);
    iEndQuest = request.indexOf("?", iStart);

    if (iEndSpace > 0) {
      sPath = request.substring(iStart, iEndQuest>0?iEndQuest:iEndSpace);
    }        
  }
  return sPath;
}

 
/**
 * Wait for Webclient connection
 */
boolean waitForClient() {
  client = server.available();
  if (!client) {
    return false;
  }

  Serial.println("new client");
  unsigned long timeout = millis() + 250;
  while (!client.available() && (millis() < timeout) ) {
    delay(1);
  }

  if (millis() > timeout) {
    Serial.println("client timed out!");
    return false;
  }

  return true;
}

/**
 * Check for valid URL 
 * else show 404 error page
 */
boolean validateUrl(String urlPath){
  if (urlPath != "/" && urlPath != "/rawCmd") {
    String httpResponse, httpHeader;
    httpResponse = "<html><head><title>404 Not Found</title></head><body><h1>Requested URL  not found.</h1>";
    httpResponse += "Dieses Gerät unterstützt keine Unterseiten in der URL. <br/>";
    httpResponse += "Zum schalten des Relais muss der Parameter 'shift' in der URL angegeben sein.<br/><br/>";
    httpResponse += "<b>Beispiel:</b><br/>http://";  
    httpResponse += ip[0];
    httpResponse += ".";  
    httpResponse += ip[1];
    httpResponse += ".";  
    httpResponse += ip[2];
    httpResponse += ".";  
    httpResponse += ip[3];
    httpResponse += "/?shift";  

    httpResponse += "</body></html>";

    httpHeader  = "HTTP/1.1 404 Not found\r\n";
    httpHeader += "Content-Length: ";
    httpHeader += httpResponse.length();
    httpHeader += "\r\n";
    httpHeader += "Content-Type: text/html\r\n";
    httpHeader += "Connection: close\r\n";
    httpHeader += "\r\n";

    client.print(httpHeader);
    client.print(httpResponse);

    return false;
  }
  
  return true;
}

/**
 * Show default website
 */
void showWebsite(boolean handledParameter){
  String httpResponse, httpHeader;

  httpResponse  = "<html><head><title>Demo f&uumlr ESP8266 Steuerung</title></head><body>";
  httpResponse += "<font color=\"#ffffff\"><body bgcolor=\"#000000\">";
  httpResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
  httpResponse += "<h1>SmartHome yourself - Garagentor-Controller</h1><br/><br/>";
  if(handledParameter){
    httpResponse += "Der Schaltbefehl wurde ausgef&uuml;hrt.";
  } else {
    httpResponse += "Zum schalten des Relais muss der Parameter 'shift' in der URL angegeben sein.<br/><br/>";
    httpResponse += "<b>Beispiel:</b><br/>http://";  
    httpResponse += ip[0];
    httpResponse += ".";  
    httpResponse += ip[1];
    httpResponse += ".";  
    httpResponse += ip[2];
    httpResponse += ".";  
    httpResponse += ip[3];
    httpResponse += "/?shift";  
  }
  httpResponse += "</body></html>";

  httpHeader  = "HTTP/1.1 200 OK\r\n";
  httpHeader += "Content-Length: ";
  httpHeader += httpResponse.length();
  httpHeader += "\r\n";
  httpHeader += "Content-Type: text/html\r\n";
  httpHeader += "Connection: close\r\n";
  httpHeader += "\r\n";

  client.print(httpHeader);
  client.print(httpResponse);
}

/**
 * Initialize WiFi
 */
void WiFiStart() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  server.begin();

  Serial.println(WiFi.localIP());
}


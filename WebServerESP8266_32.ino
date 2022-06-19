// TWO --> answer Serial buffer

#include <ESP8266WiFi.h>
 
const char* ssid = "M bessa oi fibra 2.4";//DIGITE A REDE 
const char* password = "titan125";//ENTRE COM A SENHA

//const char* ssid = "AndroidAP";//type your ssid
//const char* password = "alan2580";//type your password
 
int ledPin = D5; // GPIO2 of ESP8266

String sBuffer = "";

int contador = 0;

int vLED=LOW;

WiFiServer server(80);
 
void setup() {
  Serial.begin(9600);
  delay(10);
 
 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
   
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
   
  WiFi.begin(ssid, password);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(500);
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
   
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  ESP.wdtEnable(2000);
}
 
void loop() {
  ESP.wdtFeed();
  // TWO
  
  if (Serial.available()) {
    while (Serial.available()) {
      int iRx = Serial.read();
      if (iRx > 31) {
        if (iRx < 120) {
          if (sBuffer.length() < 127) {
            sBuffer = sBuffer + (char)iRx; }
        }
      } 
    }
  }

  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  contador++;
  // Wait until the client sends some data
  Serial.println("new client");
  long startwaitting = millis();
  long passaramse = 0;
  while(!client.available() && (passaramse<2000)){
    delay(1);
    passaramse = millis() - startwaitting;
  }
  Serial.println("=");
  Serial.println(passaramse);

  if (passaramse < 2000) {
    // Read the first line of the request
    delay(100);
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();
   
    // Match the request
 
    if (request.indexOf("/LED=ON") != -1) {
      digitalWrite(ledPin, HIGH);
      vLED = HIGH;
    } 
    if (request.indexOf("/LED=OFF") != -1){
      digitalWrite(ledPin, LOW);
      vLED = LOW;
    }

    // Set ledPin according to the request
    //digitalWrite(ledPin, value);
  } 
 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<head>");
  client.println("  <title>PAGINA IOT</title>");
  client.println("</head>");
  client.println("<body>"); 
  client.print("Led pin is now: ");
   
  if(vLED == HIGH) {
    client.print("ON<br><br>");  
    client.print("Click <a href=\"/LED=OFF?par=");
    client.print(contador);
    client.println("\">here</a> turn the LED off pin 2 OFF<br><br>");
  } else {
    client.print("OFF<br><br>");
    client.print("Click <a href=\"/LED=ON?par=");
    client.print(contador);
    client.println("\">here</a> turn the LED on pin 2 ON<br><br>");
    //client.println("Click <a href=\"/LED=ON\">here</a> turn the LED on pin 2 ON<br>");
  }
  client.println("<br><br>");
  //client.println("Click <a href=\"/LED=ON\">here</a> turn the LED on pin 2 ON<br>");
  //client.println("Click <a href=\"/LED=OFF\">here</a> turn the LED on pin 2 OFF<br><br>");

  if (sBuffer.length() > 0) {
    client.println("--> ");   
    client.println(sBuffer);   
    client.println("<br>");   
  } else {
    client.println("NO Serial buffer!<br>");   
  }
  
  sBuffer = "";
  
  client.println("</body>"); 
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
 
}

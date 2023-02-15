#include <Arduino.h>


#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HardwareSerial.h>
#include <Wire.h>              
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RCSwitch.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define Czas_obrot 2000
#define Czas_wstecz 1000


// Przydział GPIO
// Zmienne to GPIO pins
// odbiornik 433 GPIO2
// LCD GPIO
// Koło prawe przód
const int PP = 26;
// Koło prawe tył
const int PT = 27;
// Koło lewe przód
const int LP = 25;
// Koło lewe tył
const int LT = 33;
// Silniki noża
const int NOZ = 14;
// Zderzak
const int ZDERZAK = 34;
// Podniesione kola
const int KOLA_UP = 35;


//czujnik odległości sonar LZ
const int trigPinLZ = 23; //out
const int echoPinLZ = 36; //input
//czujnik odległości sonar LW
const int trigPinLW = 19; //out
const int echoPinLW = 39; //input
//czujnik odległości sonar PZ
const int trigPinPZ = 12; //out
const int echoPinPZ = 13; //input
//czujnik odległości sonar PW
const int trigPinPW = 4; //out
const int echoPinPW = 15; //input


//Odległosc do przeszkody
const float minOdleglosc = 15;
//zmienne lokalizacyjne
float latitude , longitude;
String  latitude_string , longitiude_string;
// ekran
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Odbiornik 433


RCSwitch mySwitch = RCSwitch();


// Replace with your network credentials
const char* ssid = "nazwa sieci";
const char* password = "hasło sieci";
//zmienna do kierunku
int kierunek;
// Przeszkoda
boolean Emergency = true;
// stan detektora petli
int PetlaValue = 0;
// Set web server port number to 80
WiFiServer server(80);
//
  int wartosc_cyfrowa = 0;
  int wartosc_analogowa = 0;
// Variable to store the HTTP request
String header;


// Auxiliar variables to store the current output state


String prosto = "off";
String lewo = "off";
String prawo = "off";
String tyl = "off";


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


void setup() {
  Serial.begin(115200);
  // Connect To GPIO2
  mySwitch.enableReceive(2);  
 // display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
 
 // czujnik odleglosci
 pinMode(trigPinLZ, OUTPUT); // Sets the trigPin as an Output
 pinMode(echoPinLZ, INPUT); // Sets the echoPin as an Input
 pinMode(trigPinLW, OUTPUT); // Sets the trigPin as an Output
 pinMode(echoPinLW, INPUT); // Sets the echoPin as an Input
 pinMode(trigPinPW, OUTPUT); // Sets the trigPin as an Output
 pinMode(echoPinPW, INPUT); // Sets the echoPin as an Input
 pinMode(trigPinPZ, OUTPUT); // Sets the trigPin as an Output
 pinMode(echoPinPZ, INPUT); // Sets the echoPin as an Input


 // Wejscia bumper
  pinMode(ZDERZAK, INPUT);
  pinMode(KOLA_UP, INPUT);
 
  // Inicjalizacja wyjść


  pinMode(PP, OUTPUT);
  pinMode(PT, OUTPUT);
  pinMode(LP, OUTPUT);
  pinMode(LT, OUTPUT);
  pinMode(NOZ, OUTPUT);
  // ustawienie stanów to low
  digitalWrite(PP, LOW);
  digitalWrite(PT, LOW);
  digitalWrite(LP, LOW);
  digitalWrite(LT, LOW);
  digitalWrite(NOZ, HIGH);
  digitalWrite(ZDERZAK, LOW);




  // Połączenie z  Wi-Fi network  SSID i password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Wyświetlanie Adresu IP i komunikatów połączenia
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(WiFi.localIP());
  display.display();
}
// funkcie Kierunku jazdy
void przod(){
   Serial.println("Jedziemy do przodu");
  digitalWrite(PT, LOW);
  digitalWrite(LT, LOW);
  digitalWrite (PP, HIGH);
  digitalWrite (LP, HIGH);


  }


void w_tyl(){
  Serial.println("Jedziemy do tylu");
  digitalWrite(PP, LOW);
  digitalWrite(LP, LOW);
  digitalWrite (PT, HIGH);
  digitalWrite (LT, HIGH);
  delay(Czas_wstecz);
  digitalWrite(PT, LOW);
  digitalWrite(LT, LOW);
}


void stoj(){
  Serial.println("Stop");
  digitalWrite(PP, LOW);
  digitalWrite(LP, LOW);
  digitalWrite(PT, LOW);
  digitalWrite(LT, LOW);
}


void w_prawo(){
  Serial.println("Skret w prawo");
  digitalWrite(PP, LOW);
  digitalWrite(LT, LOW);
  digitalWrite (PT, HIGH);
  digitalWrite (LP, HIGH);
  delay(Czas_obrot);
  digitalWrite(PT, LOW);
  digitalWrite(LP, LOW);
}


void w_lewo(){
  Serial.println("Skret w lewo");
  digitalWrite(LP, LOW);
  digitalWrite(PT, LOW);
  digitalWrite (LT, HIGH);
  digitalWrite (PP, HIGH);
  delay(Czas_obrot);
  digitalWrite(LT, LOW);
  digitalWrite(PP, LOW);
}
// Część wykonawcza
void loop(){
//Inicjalizacje SONARÓW
  digitalWrite(trigPinLZ, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinLZ, LOW);
  int signalDelayLZ = pulseIn(echoPinLZ, HIGH);
  int distanceLZ = signalDelayLZ/58;


  digitalWrite(trigPinLW, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinLW, LOW);
  int signalDelayLW = pulseIn(echoPinLW, HIGH);
  int distanceLW = signalDelayLW/58;


  digitalWrite(trigPinPW, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinPW, LOW);
  int signalDelayPW = pulseIn(echoPinPW, HIGH);
  int distancePW = signalDelayPW/58;


  digitalWrite(trigPinPZ, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinPZ, LOW);
  int signalDelayPZ = pulseIn(echoPinPZ, HIGH);
  int distancePZ = signalDelayPZ/58;


  //odbiornik 433
  int value = mySwitch.getReceivedValue();
 
 if (Emergency == false){
  digitalWrite (NOZ, LOW);
//Reakcja na wykrycie pętli
  if (mySwitch.available()) {
      Serial.print("Adres Nadajnika ");
      Serial.println(mySwitch.getReceivedValue());
      display.clearDisplay();
      display.setCursor(0,0);             // Start at top-left corner
      display.println(F("Petla zawracamy"));
      display.display();
      stoj();
      w_tyl();
      w_prawo();
      przod();
      }
//Reakcja na wykrycie przeszkody ze zderzaka i sonarów
  if (digitalRead(ZDERZAK) == HIGH){
  Serial.println("Za blisko Zderzak ");
  display.clearDisplay();
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Zderzak STOP"));
  display.display();
 
  }
 
  if (distanceLZ < 15){
  Serial.println("Za blisko Sonar LZ ");
  display.clearDisplay();
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Sonar STOP LZ"));
  display.display();
  stoj();
  w_tyl();
  w_lewo();
  przod();
  }


  if (distanceLW < 15){
  Serial.println("Za blisko Sonar LW ");
  display.clearDisplay();
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Sonar STOP LW"));
  display.display();
  stoj();
  w_tyl();
  w_lewo();
  przod();
  }


  if (distancePW < 15){
  Serial.println("Za blisko Sonar PW ");
  display.clearDisplay();
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Sonar STOP PW"));
  display.display();
  stoj();
  w_tyl();
  w_prawo();
  przod();
  }


  if (distancePZ < 15){
  Serial.println("Za blisko Sonar PZ ");
  display.clearDisplay();
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Sonar STOP PZ"));
  display.display();
  stoj();
  w_tyl();
  w_prawo();
  przod();
  }
 }
 else {digitalWrite (NOZ, HIGH);}
  WiFiClient client = server.available();   // Listen for incoming clients


  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
           
            // turns the GPIOs on and off
            if (header.indexOf("GET /prosto/on") >= 0) {
              Serial.println("prosto on");
              prosto = "on";
              kierunek = 1;
                           
            } else if (header.indexOf("GET /prosto/off") >= 0) {
              Serial.println("prosto off");
              prosto = "off";
              kierunek = 0;
            } else if (header.indexOf("GET /prawo/on") >= 0) {
              Serial.println("prawo on");
              prawo = "on";
              kierunek = 2 ;
            } else if (header.indexOf("GET /prawo/off") >= 0) {
              Serial.println("prawo off");
              prawo = "off";
              kierunek = 0;
            } else if (header.indexOf("GET /lewo/on") >= 0) {
              Serial.println("lewo on");
              lewo = "on";
              kierunek =3;
            } else if (header.indexOf("GET /lewo/off") >= 0) {
              Serial.println("lewo off");
              lewo = "off";
               kierunek = 0;
            } else if (header.indexOf("GET /tyl/on") >= 0) {
              Serial.println("tyl on");
              tyl = "on";
              kierunek =4;
            } else if (header.indexOf("GET /tyl/off") >= 0) {
              Serial.println("tyl off");
              tyl = "off";
               kierunek = 0;
            }
              Serial.println (kierunek);
            // Wyświetla HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
           
            // Web Page Heading
            client.println("<body><h1>Kosiarka Serwer</h1>");
           
            // Display current state, and ON/OFF buttons for prosto
            client.println("<p>Jazda prosto - State " + prosto + "</p>");
            // If the prosto is off, it displays the ON button      
            if (prosto=="off") {
              Emergency= true;
              client.println("<p><a href=\"/prosto/on\"><button class=\"button button2\">OFF</button></a></p>");
            } else {
              Emergency= false;
              client.println("<p><a href=\"/prosto/off\"><button class=\"button\">ON</button></a></p>");
            }
               
            // Display current state, and ON/OFF buttons for prawo  
            client.println("<p>Jazda w prawo - State " + prawo + "</p>");
            // If the output27State is off, it displays the ON button      
            if (prawo=="off") {
              client.println("<p><a href=\"/prawo/on\"><button class=\"button button2\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/prawo/off\"><button class=\"button\">ON</button></a></p>");
            }


            // Display current state, and ON/OFF buttons for lewo
            client.println("<p>Jazda w lewo - State " + lewo + "</p>");
            // If the output27State is off, it displays the ON button      
            if (lewo=="off") {
              client.println("<p><a href=\"/lewo/on\"><button class=\"button button2\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/lewo/off\"><button class=\"button\">ON</button></a></p>");
            }


                        // Display current state, and ON/OFF buttons for tyl
            client.println("<p>Jazda w tyl - State " + tyl + "</p>");
            // If the output27State is off, it displays the ON button      
            if (tyl=="off") {
              client.println("<p><a href=\"/tyl/on\"><button class=\"button button2\">OFF</button></a></p>");
            }
            else {
              client.println("<p><a href=\"/tyl/off\"><button class=\"button\">ON</button></a></p>");
            }
            client.println("</body></html>");
           
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            Serial.println (kierunek);
           
              if (digitalRead(ZDERZAK) == LOW){
              Serial.println (ZDERZAK);
                switch (kierunek) {


                case 0:
                  stoj();
                  break;


                case 1:
                  przod();
                  break;


                case 2:
                  w_prawo();
                  break;


                case 3:
                  w_lewo();
                  break;


                case 4:
                  w_tyl();
                  break;
                }
              }
              else{
                stoj();
              }
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");  
  }
 mySwitch.resetAvailable();
}


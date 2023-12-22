

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <ESP32Servo.h>

// Set these to your desired credentials.
const char *ssid = "Smart Garage";
const char *password = "01113793255";

const int echoPin = 13;
const int trigPin = 14;
const int servoPin = 4;
//const int ledPin = 2;

int IR1 = 21;
int IR2 = 22;
int ir1_led = 18;
int ir2_led = 19;
int ultrasonic_led = 5;
int vibration = 23;
int g_led1 = 25;
int g_led2 = 27;
bool automatic = true;
bool empty1 = true;
bool empty2 = true;

Servo myServo;

WiFiServer server(80);


void setup() {
  Serial.begin(115200);

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(ir1_led, OUTPUT);
  pinMode(ir2_led, OUTPUT);
  pinMode(vibration, OUTPUT);
  pinMode(ultrasonic_led, OUTPUT);
  pinMode(g_led1, OUTPUT);
  pinMode(g_led2, OUTPUT);

  myServo.attach(servoPin, 500, 2500);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1)
      ;
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

String generateControlHTML(bool isAutomaticMode, bool isempty1, bool isempty2) {
  String htmlContent = "<!DOCTYPE html>"
                       "<html>"
                       "<head>"
                       "    <title>Smart Garage Control</title>"
                       "    <style>"
                       "        body {"
                       "            font-family: 'Arial', sans-serif;"
                       "            background: linear-gradient(to right, #3494e6, #ec6ead);"
                       "            box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1);"
                       "            text-align: center;"
                       "            padding: 40px; "
                       "        }"
                       "        h1 {"
                       "            color: #fff;"
                       "            font-size: 62px; "
                       "        }"
                       "        p {"
                       "            color: #fff;"
                       "            font-size: 50px; "
                       "        }"
                       "        a {"
                       "            display: inline-block;"
                       "            margin: 20px; "
                       "            padding: 40px 60px; "
                       "            text-decoration: none;"
                       "            font-size: 50px;"
                       "            color: #fff;"
                       "            background-color: #007bff;"
                       "            border-radius: 30px;"
                       "            transition: background-color 0.3s ease-in-out;"
                       "        }"
                       "        a:hover {"
                       "            background-color: #0056b3;"
                       "        }"
                       "    </style>"
                       "</head>"
                       "<body>"
                       "    <h1>Smart Garage Control</h1>";

  if (isAutomaticMode) {
    htmlContent += "    <p>Status: Automatic Mode</p>";
    htmlContent += "    <a href=\"/manual\">Manual</a><br>";

    htmlContent += "    <p>First Place is ";
    htmlContent += (isempty1 ? "Empty" : "Full");
    htmlContent += "</p>";
    htmlContent += "    <p>Second Place is ";
    htmlContent += (isempty2 ? "Empty" : "Full");
    htmlContent += "</p>";
  } else {
    htmlContent += "    <p>Status: Manual Mode</p>";
    htmlContent += "    <a href=\"/auto\">Automatic</a><br>"
                   "    <a href=\"/open\">Open Door</a><br>"
                   "    <a href=\"/close\">Close Door</a><br>"
                   "    <a href=\"/on1\">Open LED1</a><br>"
                   "    <a href=\"/off1\">Close LED1</a><br>"
                   "    <a href=\"/on2\">Open LED2</a><br>"
                   "    <a href=\"/off2\">Close LED2</a><br>";
  }

  htmlContent += "</body>"
                 "</html>";

  return htmlContent;
}

void loop() {
  long duration, cm;

  int IR1_State = digitalRead(IR1);
  int IR2_State = digitalRead(IR2);

  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
  cm = microsecondsToCentimeters(duration);
  //some prints
  // Serial.println("Distance:   | ir1:  | ir2: ");
  // Serial.print(cm);
  // Serial.print(" cm | ");
  // Serial.print(IR1_State);
  // Serial.print("  |  ");
  // Serial.print(IR2_State);
  // Serial.println();

  digitalWrite(g_led1, HIGH);
  digitalWrite(g_led2, HIGH);
  // digitalWrite(ultrasonic_led, HIGH);
  delay(100);
  // Serial.println(automatic);
  if (automatic) {
    if (IR1_State == 0) {
      // Serial.println("led1 on");
      empty1 = false;
      digitalWrite(ir1_led, HIGH);
    } else {
      // Serial.println("led1 off");
      empty1 = true;
      digitalWrite(ir1_led, LOW);
    }
    if (IR2_State == 0) {
      // Serial.println("led2 on");
      empty2 = false;
      digitalWrite(ir2_led, HIGH);
    } else {
      // Serial.println("led2 off");
      empty2 = true;
      digitalWrite(ir2_led, LOW);
    }
    // else {
    //   digitalWrite(ledPin, LOW);
    // }

    // Adjust servo angle based on distance
    if (cm < 20) {  //near
      digitalWrite(ultrasonic_led, HIGH);
      if (IR1_State == 1 || IR2_State == 1) {  //free
        for (float pos = 0; pos <= 100; pos += 0.5) {
          myServo.write(pos);
          delay(3);
        }
        delay(1500);
      } else {  //full
        myServo.write(0);
        digitalWrite(vibration, HIGH);
        for (int i = 0; i < 3; ++i) {
          digitalWrite(ir1_led, LOW);
          digitalWrite(ir2_led, LOW);
          delay(300);
          digitalWrite(ir1_led, HIGH);
          digitalWrite(ir2_led, HIGH);
          delay(300);
        }
        digitalWrite(vibration, LOW);
      }
    } else {  //far
      myServo.write(0);
    }
    digitalWrite(ultrasonic_led, LOW);
    delay(100);
  }

  //WIFI
  WiFiClient client = server.available();  // listen for incoming clients

  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        if (c == '\n') {            // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            String htmlContent = "";

            htmlContent = generateControlHTML(automatic, empty1, empty2);

            client.print(htmlContent);




            // client.print("Click <a href=\"/L\">here</a> to turn OFF the LED.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /open")) {
          for (float pos = 0; pos <= 100; pos += 0.5) {
            myServo.write(pos);
            delay(3);
          }
        }
        if (currentLine.endsWith("GET /close")) {
          myServo.write(0);
        }
        if (currentLine.endsWith("GET /manual")) {
          digitalWrite(ir1_led, LOW);
          digitalWrite(ir2_led, LOW);
          automatic = 0;
        } else if (currentLine.endsWith("GET /auto")) {
          automatic = 1;
        } else if (currentLine.endsWith("GET /on1")) {
          digitalWrite(ir1_led, HIGH);
        } else if (currentLine.endsWith("GET /off1")) {
          digitalWrite(ir1_led, LOW);
        } else if (currentLine.endsWith("GET /on2")) {
          digitalWrite(ir2_led, HIGH);
        } else if (currentLine.endsWith("GET /off2")) {
          digitalWrite(ir2_led, LOW);
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

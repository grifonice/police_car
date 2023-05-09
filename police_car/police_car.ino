#include "functions.hpp"
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include "secret.h" // ssid and password are defined in secret.h 

int motor1_1 = 5; // d1
int motor1_2 = 16; // d0
int motor2_1 = 0; // d3
int motor2_2 = 4; // d2
int speed_pin1 = 2; // d4
int speed_pin2 = 14; // d5
int speed = 255;
int pin_hand = 12;
int pin_arm = 13;
Servo hand_s;
Servo arm_s;
String output;
String current_command = "stop";
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void stop(bool command = false)
{
  digitalWrite(speed_pin1, 0);
  digitalWrite(speed_pin2, 0);
  digitalWrite(motor1_2, 0);
  digitalWrite(motor2_2, 0);
  digitalWrite(motor1_1, 0);
  digitalWrite(motor2_1, 0);
  if (command)
  {
    current_command = "stop";
  }
}

void forward(int speed)
{
  stop();
  digitalWrite(speed_pin1, 1);
  digitalWrite(speed_pin2, 1);
  digitalWrite(motor1_2, 1);
  digitalWrite(motor2_2, 1);
  delay((256 - speed) / 16);
  analogWrite(speed_pin1, speed);
  analogWrite(speed_pin2, speed);
  current_command = "forward";
}

void back(int speed)
{
  stop();
  digitalWrite(speed_pin1, 1);
  digitalWrite(speed_pin2, 1);
  digitalWrite(motor1_1, 1);
  digitalWrite(motor2_1, 1);
  delay((256 - speed) / 16);
  analogWrite(speed_pin1, speed);
  analogWrite(speed_pin2, speed);
  current_command = "back";
}

void left(int speed, int rotation_speed = 4)
{
  digitalWrite(speed_pin1, 1);
  digitalWrite(speed_pin2, 1);
  if (current_command == "forward" or current_command == "back" or current_command == "right")
  {
    analogWrite(speed_pin1, speed);
    analogWrite(speed_pin2, speed / rotation_speed);
  }
}

void right(int speed, int rotation_speed = 4)
{
  digitalWrite(speed_pin1, 1);
  digitalWrite(speed_pin2, 1);
  if (current_command == "forward" or current_command == "back" or current_command == "left")
  {
    analogWrite(speed_pin1, speed / rotation_speed);
    analogWrite(speed_pin2, speed);
  }
}

void linear()
{
  analogWrite(speed_pin1, speed);
  analogWrite(speed_pin2, speed);
}

void hand(int rotation)
{
  hand_s.write(rotation);
}

void arm(int rotation)
{
  arm_s.write(rotation);
}

void command_handler(String output)
{
  std::vector<String> command = split(output);

  if (command[0] == "forward")
  {
    Serial.println("forward");
    forward(speed);
  }
  if (command[0] == "back")
  {
    Serial.println("back");
    back(speed);
  }
  if (command[0] == "stop")
  {
    Serial.println("stop");
    stop(true);
  }
  if (command[0] == "speed")
  {
    Serial.println("speed");
    speed = atoi(command[1].c_str());
    analogWrite(speed_pin1, speed);
    analogWrite(speed_pin2, speed);
  }
  if (command[0] == "left")
  {
    Serial.println("left");
    left(speed);
  }
  if (command[0] == "right")
  {
    Serial.println("right");
    right(speed);
  }
  if (command[0] == "hand")
  {
    Serial.println("hand");
    hand(atoi(command[1].c_str()));
  }
  if (command[0] == "arm")
  {
    Serial.println("arm");
    arm(atoi(command[1].c_str()));
  }
  if (command[0] == "linear")
  {
    Serial.println("linear");
    linear();
  }
}


void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String coso = (char*)data;
    if (coso == "ping")
    {
      ws.textAll((String)"pong");
    }
    command_handler((char*)data);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      break;
    case WS_EVT_DISCONNECT:
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup()
{
  pinMode(motor1_1, OUTPUT);
  pinMode(motor1_2, OUTPUT);
  pinMode(motor2_1, OUTPUT);
  pinMode(motor2_2, OUTPUT);
  pinMode(speed_pin1, OUTPUT);
  pinMode(speed_pin2, OUTPUT);
  hand_s.attach(pin_hand);
  arm_s.attach(pin_arm);
  digitalWrite(motor1_1, 0);
  digitalWrite(motor2_1, 0);
  digitalWrite(motor1_2, 0);
  digitalWrite(motor2_2, 0);
  digitalWrite(speed_pin1, 0);
  digitalWrite(speed_pin2, 0);
  analogWriteFreq(100);
  hand_s.write(0);
  arm_s.write(0);
  Serial.begin(115200);
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  initWebSocket();
  server.begin();
  Serial.println(" WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  while (Serial.available() == 0)
  {
  }
  output = Serial.readString();
  output = output.substring(0, output.length() - 1);
  command_handler(output);
}
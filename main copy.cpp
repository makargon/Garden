#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> 
#include <ArduinoJson.h>
#include <DHT.h>
#include <Servo.h>

const char* ssid = "***";
const char* wifiPassword = "****";

const char* BOTtoken = "******";
const char* CHAT_ID = "****";

const uint8_t PIN_IN1 = 19;  //При подаче на нечетные, моторы крутятся ВЛЕВО, 
const uint8_t PIN_IN2 = 18;  //а на четные ВПРАВО
const uint8_t PIN_IN3 = 5;  //1, 2- ЛЕВЫЙ мотор
const uint8_t PIN_IN4 = 17;  //3, 4- ПРАВЫЙ мотор
const uint8_t PIN_ENA = 21; //Мощность моторов(HIGH-полная мощность)(Можно регулировать ШИМом)
const uint8_t PIN_ENB = 16;  // A-ЛЕВЫЙ мотор В-ПРАВЫЙ

const uint8_t PIN_pomp = 2; //Реле помпы(HIGH- Вкл)
const uint8_t DHTPIN = 15;   //Пин датчика температуры и влажности воздуха
const uint8_t Wlaga = 13;

const uint8_t PIN_plug = 25;// Пины сервомашинок
const uint8_t PIN_sagalkaLR = 26;
const uint8_t PIN_sagalkaC = 27;
const uint8_t PIN_ServoSensor = 14;

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

uint8_t i = 250;// Мощность двигателя

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

DHT dht(DHTPIN, DHT11);
Servo plug;
Servo sagalkaLR;
Servo sagalkaC;
Servo ServoSensor;

bool motorL_move(byte power, bool rev){//power-скорость моторов(0-255) rev- направление
  ledcWrite(0, 50);
  digitalWrite(PIN_IN1, rev);
  digitalWrite(PIN_IN2, !rev);
  return(true);
}
bool motorR_move(byte power, bool rev){//power-скорость моторов(0-255) rev- направление
  ledcWrite(1, 50);
  digitalWrite(PIN_IN3, !rev);
  digitalWrite(PIN_IN4, rev);
  return(true);
}
void motorStop(){//отключить все двигатели во всех направления
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}
void sensor(){
  ServoSensor.write(15);//опустить датчики
  delay(2000);
  float h_air = dht.readHumidity();//чтение всех данных и запись в соответствующие переменные
  float t_air = dht.readTemperature();
  int h_ground = analogRead(Wlaga);
  Serial.print("Humidity: "); 
  Serial.print(h_air);
  Serial.print(" %\t");
  Serial.print(h_ground);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(t_air);
  Serial.println(" *C ");
  ServoSensor.write(0);//поднять датчики
}
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    // Проверка личности
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      String welcome = "Добрый день, " + from_name + ".\n";
      welcome += "Это бот, способный управлять роботом огородником, использую следующие команды:\n\n";
      welcome += "/width <метр> - Ширина грядки\n";
      welcome += "/length <метр> - Длинна грядки, так же в метрах \n";
      welcome += "/culture <название или номер> - Выбор культуры из списка: \n";
      welcome += "1)Морковь \n";
      welcome += "2)Свекла \n";
      welcome += "3)Укроп \n";
      welcome += "Для добавления других культур обратитесь к разработчику\n";
      welcome += "/go - Эта команда начнет посадку с имеющимися настройками \n";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/help_manual") {
      String help = "Добрый день, " + from_name + ".\n"
      
      ;
      help += "Это список доступных команд для ручного управления:\n\n";
      help += "/left \n";
      help += "/right\n";
      help += "/forward\n";
      help += "/back\n";
      help += "/pomp\n";
      help += "/middle\n";
      help += "/side \n";
      bot.sendMessage(chat_id, help, "");
    }
    else if (text == "/width") {
      bot.sendMessage(chat_id, "Ширина:", "");
    }
    else if (text == "/left") {
      bot.sendMessage(chat_id, "Поворачиваем налево", "");
      //Разворот по ТС лево
      motorL_move(i, 0);
      motorR_move(i, 0);
      delay(300);
      motorStop();
    }
    else if (text == "/right") {
      bot.sendMessage(chat_id, "Поворачиваем направо", "");
      //Разворот по ТС право
      motorL_move(i, 1);
      motorR_move(i, 1);
      delay(300);
      motorStop();
    }
    else if (text == "/forward") {
      bot.sendMessage(chat_id, "Едем прямо", "");
      //Ехать вперед
      motorL_move(i, 1);
      motorR_move(i, 0);
      delay(3000);
      motorStop();
    }
    else if (text == "/back") { 
      bot.sendMessage(chat_id, "Едем назад", "");
      //Ехать вперед
      motorL_move(i, 0);
      motorR_move(i, 1);
      delay(3000);
      motorStop();
    }
    else if (text == "/pomp") {
      bot.sendMessage(chat_id, "Ширина:", "");
      digitalWrite(PIN_pomp, HIGH);
      delay(1000);
      digitalWrite(PIN_pomp, LOW);
      delay(1000);
    }
    else if (text == "/middle") {
      bot.sendMessage(chat_id, "Едем прямо", "");
      //Ехать вперед
      sagalkaC.write(42.5);
      delay(100);
      sagalkaC.write(255);
    }
    else if (text == "/side") {
      bot.sendMessage(chat_id, "Едем прямо", "");
      //Ехать вперед
      sagalkaLR.write(42.5);
      delay(500);
      sagalkaLR.write(0);
    }
    else if (text == "/culture") {
      bot.sendMessage(chat_id, "Выбор культуры", "");
    }
  }
}
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(PIN_pomp, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_ENB, OUTPUT);
  ledcSetup(0, 50, 8); ledcAttachPin(PIN_ENA, 0);
  ledcSetup(1, 50, 8); ledcAttachPin(PIN_ENB, 1);
  plug.attach(PIN_plug);
  sagalkaLR.attach(PIN_sagalkaLR);
  sagalkaC.attach(PIN_sagalkaC);
  ServoSensor.attach(PIN_ServoSensor);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifiPassword);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
}
void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
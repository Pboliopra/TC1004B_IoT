#include <DHT.h>
#include <WiFi.h>
#include <Wire.h>
#include <time.h>
#include <analogWrite.h>
#include <LiquidCrystal.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <Laika_FirebaseESP32.h>

// -------------- Constants -------------- //
// Firebase
#define FIREBASE_HOST " https://tc1004b-499ca-default-rtdb.firebaseio.com/" // 
#define FIREBASE_AUTH "jQdCBRGdOXCmgF6CiodZSgzIXFQs1prWEmARmKWW" // secreto de la base de datos 
// Redifined I2C protocol pins
// Acelerometer
#define Trigger 33
#define Echo 25
//DHT
#define dht_pin 13
#define dht_type DHT11
// LCD
#define contrast_pin 4
#define backlight 2
// Push Button
#define pot 34

// -------------- Objects -------------- //
// Wi-Fi
WiFiServer server(80);  // Internet port
WiFiClient client;       // Parameters
// DHT
DHT dht(dht_pin, dht_type);
//LCD
LiquidCrystal lcd(22, 23, 5, 18, 19, 21);
// Clock
struct tm timeinfo;

// -------------- Variables -------------- //
// Wi-Fi credentials
const char* WIFI_SSID = "Tec-IoT";
const char* WIFI_PASSWORD = "spotless.magnetic.bridge";
// Time server
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -21600;
const int   daylightOffset_sec = 3600;
// LCD 
int Contrast = 130;
// DHT
float humidity = 0, temperature = 0;
// Ultrasonic
long duracion = 0, distancia = 0;
// Push Button
int page = 0;
//Clock
String dispMonth;
char WeekDay[4];
char timeMonth[4];
char real_time[6];

// _______ Custom Chars _______ // 
// Temperature symbol
byte up_left_thermo [8] = {B00001, B00010, B00010,B00010, B00010, B00010, B00010, B00010};
byte up_right_thermo [8] = { B10000, B01000, B11000, B01000, B11000, B01000, B11000, B01000};
byte down_left_thermo [8] = { B00010, B00010, B00010, B00100, B01000, B01000, B00100, B00011};
byte down_right_thermo [8] = { B11000, B01000, B11000, B00100, B00010, B00010, B00100, B11000};

// Humidity symbol
byte up_left_water [8] = { B00001, B00001, B00011, B00010, B00110, B00100, B01100, B01000};
byte up_right_water [8] = { B10000, B10000, B11000, B01000, B01100, B00100, B00110, B00010};
byte down_left_water [8] = { B10000, B10000, B10000, B10000, B10000, B01000, B01100, B00011};
byte down_right_water [8] = { B00001, B00001, B00001, B00001, B00001, B00010, B00110, B11000};

// Clock Frame
byte clock_right [8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};
byte clock_left [8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};

byte clock_up [8] = { B00000, B00000, B00000, B11111, B11111, B11111, B11111, B11111};
byte clock_down [8] = { B11111, B11111, B11111, B11111, B11111, B00000, B00000, B00000};

// -------------- Funciones -------------- //
// Wi-Fi
void InitWiFi() {
  WiFi.mode(WIFI_STA); // Define el modo de coneccion como Estacion (ESP32 accede por medio de un Punto de Acceso)
  //WiFi.mode(WIFI_AP); // Define el modo de coneccion como Punto de Acceso (ESP32 genera una red WiFi y otros dispositivos se pueden conectar a el)
  //WiFi.mode(WIFI_MODE_APSTA); // Ambos

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Inicializamos el WiFi con nuestras credenciales.
  Serial.print("Conectando a ");
  Serial.print(WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED) // Espera a que se conecte
  {   //Espera a que esté conectado
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) // En este paso ya esta conectado
  {
    Serial.println();
    Serial.println();
    Serial.println("Connected to WIFI!!");
  }

  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Imprime la IP en donde se mandara la informacion de los sensores
}
// Ultrasonic
void ultrasonic(){
  digitalWrite(Trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trigger, LOW);
  
  duracion = pulseIn(Echo, HIGH);
  distancia = duracion/59;
}
// DHT
void dht_reads(){
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
}
// Firebase
void firebase_hum(){
  Firebase.setFloat("/smartwatch/humedad", humidity);
}
void firebase_temp(){
  Firebase.setFloat("/smartwatch/temperatura", temperature);
}
void firebase_time(){
  Firebase.setString("/smartwatch/tiempo", real_time);
}
// Clock
void clock_format(){
  strftime(timeMonth, 4, "%B", &timeinfo);
  strftime(WeekDay, 4, "%A", &timeinfo);
  String str_month(timeMonth);
  if (str_month == "Jan") {
      dispMonth = "01";
  }
  else if (str_month == "Feb")  {
      dispMonth = "02";
  }
  else if (str_month == "Mar")  {
      dispMonth = "03";
  }
  else if (str_month == "Apr")  {
      dispMonth = "04";
  }
  else if (str_month == "May")  {
      dispMonth = "05";
  }
  else if (str_month == "Jun")  {
      dispMonth = "06";
  }
  else if (str_month == "Jul")  {
      dispMonth = "07";
  }
  else if (str_month == "Aug")  {
      dispMonth = "08";
  }
  else if (str_month == "Sep")  {
      dispMonth = "09";
  }
  else if (str_month == "Oct")  {
      dispMonth = "10";
  }
  else if (str_month == "Nov")  {
      dispMonth = "11";
  }
  else if (str_month == "Dec"){
      dispMonth = "12";
  }
}
// LCD
void lcd_clock(){
  getLocalTime(&timeinfo);
  clock_format();

  lcd.createChar(0, clock_left);
  lcd.createChar(1, clock_right);
  lcd.createChar(2, clock_up);
  lcd.createChar(3, clock_down);

  lcd.setCursor(2, 0);
  for (short int i=0; i<16; ++i)
    lcd.write((byte)2);
  
  lcd.setCursor(2, 1);
  lcd.write((byte)0);
  lcd.print(WeekDay);
  lcd.print(" ");
  lcd.print(dispMonth);
  lcd.print(&timeinfo, "/%d/%Y");
  lcd.write((byte)1);
  

  lcd.setCursor(2,2);
  lcd.write((byte)0);
  lcd.print(&timeinfo, "    %H:%M     ");
  lcd.write((byte)1);

  lcd.setCursor(2,3);
  for (short int i=0; i<16; ++i)
    lcd.write((byte)3);
  strftime(real_time, 6, "%H:%M", &timeinfo);
  firebase_time();
}
void lcd_temp(){
  lcd.createChar(0, clock_left);
  lcd.createChar(1, clock_right);
  lcd.createChar(2, clock_up);
  lcd.createChar(3, clock_down);
  lcd.createChar(4, up_left_thermo);
  lcd.createChar(5, up_right_thermo);
  lcd.createChar(6, down_left_thermo);
  lcd.createChar(7, down_right_thermo);

  lcd.setCursor(2, 0);
  for (short int i=0; i<16; ++i)
    lcd.write((byte)2);
  
  lcd.setCursor(2, 1);
  lcd.write((byte)0);
  lcd.write((byte)4);
  lcd.write((byte)5);
  lcd.print(" Temperatura");
  lcd.write((byte)1);


  lcd.setCursor(2,2);

  lcd.write((byte)0);
  lcd.write((byte)6);
  lcd.write((byte)7);
  lcd.print("   ");
  lcd.print(temperature);
  lcd.print("\xDF" "C  ");
  lcd.write((byte)1);

  lcd.setCursor(2,3);
  for (short int i=0; i<16; ++i)
    lcd.write((byte)3);
  firebase_temp();
}
void lcd_hum(){
  lcd.createChar(0, clock_left);
  lcd.createChar(1, clock_right);

  lcd.createChar(2, clock_up);
  lcd.createChar(3, clock_down);

  lcd.createChar(4, up_left_water);
  lcd.createChar(5, up_right_water);
  lcd.createChar(6, down_left_water);
  lcd.createChar(7, down_right_water);

  lcd.setCursor(2, 0);
  for (short int i=0; i<16; ++i)
    lcd.write((byte)2);
  
  lcd.setCursor(2, 1);
  lcd.write((byte)0);
  lcd.write(4);
  lcd.write(5);
  lcd.print("   Humedad  ");
  lcd.write((byte)1);
  

  lcd.setCursor(2,2);
  lcd.write((byte)0);
  lcd.write(6);
  lcd.write(7);
  lcd.print("   ");
  lcd.print(humidity);
  lcd.print("%   ");
  lcd.write((byte)1);

  lcd.setCursor(2,3);
  for (short int i=0; i<16; ++i)
    lcd.write((byte)3);
  firebase_hum();
}
// Push_button 
void counter(){
  page = analogRead(pot);
  page = map(page, 0, 4095, 0, 2);
  Serial.println(page);
}
void interface(){
  counter();
  if (page == 0)
    lcd_clock();
  else if (page == 1){
    dht_reads();
    lcd_temp();
  }
  else if(page == 2){
    dht_reads();
    lcd_hum();
  }
}
void on_off(){
  ultrasonic();
  if (distancia >= 0 and distancia <= 40){
    digitalWrite(backlight, HIGH);
    interface();
  }
  else {
    digitalWrite(backlight, LOW);
    lcd.clear();
  }
}

void setup() {
  Serial.begin(115200);
  InitWiFi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  // Pins
  pinMode(pot, INPUT);
  // DHT
  dht.begin();
  // LCD
  pinMode(backlight, OUTPUT);
  analogWrite(contrast_pin, Contrast);
  lcd.begin(20, 4);
  lcd.clear();
  // Ultrasonic
  pinMode(Trigger, OUTPUT); //pin como salida
  pinMode(Echo, INPUT);  //pin como entrada
  digitalWrite(Trigger, LOW);//Inicializamos el pin con 0
}
void loop(){
  on_off();
  delay(1000);
}

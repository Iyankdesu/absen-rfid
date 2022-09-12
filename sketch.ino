//Library 
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

//Network SSID
const char* ssid = "ssid";
const char* password = "password";

//lcd
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

//pengenal host (server) = IP Address komputer server
const char* host = "192.168.1.16";

//LED dan BTN
#define LED_ 16 //Rx
#define LED_PIN 16 //D1
#define BTN_PIN 15 //D8

//sediakan variabel untuk RFID
#define SDA_PIN 2 //D4
#define RST_PIN 0 //D3
MFRC522 mfrc522(SDA_PIN, RST_PIN);

//jam internet
const long utcOffsetInSeconds = 288000; //(28800)WITA
char Time[ ] = "Jam: 00:00:00";
//char Date[ ] = "Tgl: 00/00/2000"; 
byte last_second, second_, minute_, hour_, day_, month_;
int year_;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", utcOffsetInSeconds);

void setup() {
  Serial.begin(9600);

  //setting koneksi wifi
  WiFi.hostname("NodeMCU");
  WiFi.begin(ssid, password);

  //cek koneksi wifi
  while(WiFi.status() != WL_CONNECTED)
  {
    //progress sedang mencari wifi
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi connected");
  Serial.println("IP Address : ");
  Serial.println(WiFi.localIP());

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, OUTPUT);
  pinMode(LED_, OUTPUT);

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Dekatkan Kartu RFID Anda ke Reader");
  Serial.println();

  
  // tampilan awal LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(2, 0);
  lcd.print("Lcd and Rfid");
  lcd.setCursor(5, 1);
  lcd.print("Testing");
  delay(3000);
  lcd.clear();
  Serial.begin(9600);
  
  lcd.clear();
   lcd.setCursor (0,0); //   column, row
   lcd.print("Created by");
   lcd.setCursor (0,1); //   column, row
   lcd.print("KELOMPOK (1)");
   delay(4000);
   lcd.clear();
   lcd.setCursor (0,0); //   column, row
   lcd.print("Searching WiFi..");
   delay(2000);
   lcd.setCursor (0,1); //   column, row
   lcd.print("Connected!");
   delay(1000);
   lcd.clear();

    //menampilkan char pada LCD
    lcd.setCursor(0, 1);
    lcd.print(Time); 
    //lcd.setCursor(0, 1);
    //lcd.print(Date);

     
}

void loop() {
 //code Jam 
 timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();    // Get Unix epoch time from the NTP server

  second_ = second(unix_epoch);
  if (last_second != second_) {
 

    minute_ = minute(unix_epoch);
    hour_   = hour(unix_epoch);
    day_    = day(unix_epoch);
    month_  = month(unix_epoch);
    year_   = year(unix_epoch);

 

    Time[12] = second_ % 10 + 48;
    Time[11] = second_ / 10 + 48;
    Time[9]  = minute_ % 10 + 48;
    Time[8]  = minute_ / 10 + 48;
    Time[6]  = hour_   % 10 + 48;
    Time[5]  = hour_   / 10 + 48;

    //Date[5]  = day_   / 10 + 48;
    //Date[6]  = day_   % 10 + 48;
    //Date[8]  = month_  / 10 + 48;
    //Date[9]  = month_  % 10 + 48;
    //Date[13] = (year_   / 10) % 10 + 48;
    //Date[14] = year_   % 10 % 10 + 48;

   

    //Serial.println(Time);
    //Serial.println(Date);
    

    lcd.setCursor(0, 1);
    lcd.print(Time);
    //lcd.setCursor(0, 1);
    // lcd.print(Date);
   lcd.setCursor(0, 0);
  lcd.print("Place Your Card");
    last_second = second_;
  
  }
  delay(25);
  

 //jika rfid terbaca LCD akan menampilkan 

 if (digitalRead(LED_)==1) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Please wait");
    lcd.setCursor(0, 1);
    lcd.print("  Absen Berhasi  ");
    
    delay(1300);

    lcd.clear();
  }
  
  WiFiClient client;
  //meruabh mode absen
  if(digitalRead(BTN_PIN)==1)   //ditekan
  {
    Serial.println("OK");
    //nyalakan lampu LED
    digitalWrite(LED_PIN, HIGH);
    while(digitalRead(BTN_PIN)==1);
    String getData, Link;
    HTTPClient http;
    //Get data
    Link = "http://192.168.1.16/absensi/ubahmode.php";
    http.begin(client, Link);

    int httpCode = http.GET();
    String payload = http.getString();

    Serial.println(payload);
    http.end();
    
  }


 //matikan lampu LED
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_, LOW);
  //code rfid
  if(! mfrc522.PICC_IsNewCardPresent())
  return ;

  if(! mfrc522.PICC_ReadCardSerial())
  return ;
  
  // untuk mendapatkan nomor kartu
  String IDTAG = "";
  for(byte i=0; i<mfrc522.uid.size; i++)
  {
    IDTAG += mfrc522.uid.uidByte[i];
  }
  //nyalakan lampu LED
  digitalWrite(LED_,HIGH);
  digitalWrite(LED_PIN,HIGH);
  
  
  //kirim nomor kartu RFID ke server

  const int httpPort = 80;
  if(!client.connect(host, httpPort))
  {
    Serial.println("Connection Failed");
    return;
  }
  
   String Link;
   HTTPClient http;
   Link = "http://192.168.1.16/absensi/kirimkartu.php?nokartu=" + IDTAG;
   http.begin(client, Link);
  
  int httpCode = http.GET();
  String payload = http.getString();
  Serial.println(payload);
  http.end();

  
  delay(500);

}

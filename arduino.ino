#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <SimpleSDAudio.h>
#include <SD.h>

SoftwareSerial mySerial(0, 1);  // RX, TX
int selenoidPin = 2;  // Selenoid kilidi kontrol pin
String valid_emotion = "mutlu,uzgun";
int RS = 8, E = 9, D4 = 3, D5 = 5, D6 = 6, D7 = 7;
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);  // LCD ekranının pin bağlantıları
void setup() {
  mySerial.begin(9600);
  pinMode(selenoidPin, OUTPUT);
  digitalWrite(selenoidPin,HIGH);
  lcd.begin(16, 2);  // LCD ekranını başlat
}
void playSound(String emotion,String username) {
  SdPlay.setSDCSPin(4); //SD Kart CS Pini
  username = username.substring(0,2);
  emotion = emotion.substring(0,3);//uzg_ta.wav
  String filename;
  filename = (valid_emotion.indexOf(emotion) == -1) ? "default.wav" : emotion + "_" + username + ".wav" ;
  mySerial.println(filename.c_str());
  if (!SD.begin(4)) {
    mySerial.println(F("initialization failed. Things to check:"));
    mySerial.println(F("* is a card is inserted?????????????????????????"));
    mySerial.println(F("* Is your wiring correct?"));
    mySerial.println(F("* maybe you need to change the chipSelect pin to match your shield or module?"));
    while (1);
  } else {
    mySerial.println(F("Wiring is correct and a card is present."));
  }
  if (!SdPlay.init(SSDA_MODE_FULLRATE | SSDA_MODE_STEREO )) {
    mySerial.println(F("initialization failed. Things to check:"));
    mySerial.println(F("* is a card is inserted?"));
    mySerial.println(F("* Is your wiring correct?"));
    mySerial.println(F("* maybe you need to change the chipSelect pin to match your shield or module?"));
    mySerial.print(F("Error code: "));
    mySerial.println(SdPlay.getLastError());
    while(1);
  } else {
   mySerial.println(F("Wiring is correct and a card is present.")); 
  }

  mySerial.print(F("Looking for EXAMPLE.AFM... "));
  if(!SdPlay.setFile(filename.c_str())) {
    mySerial.println(F(" not found on card! Error code: "));
    mySerial.println(SdPlay.getLastError());
    while(1);
  } else {
   mySerial.println(F("found.")); 
  }    

  mySerial.print(F("Playing... ")); 
  SdPlay.play();
  while(!SdPlay.isStopped()) {
    SdPlay.worker();
  }
  mySerial.println(F("done."));
  SdPlay.deInit();
}
void openDoor(String userName, String emotion) {
  digitalWrite(selenoidPin, LOW);  // Selenoid kilidi acik hale getir
  mySerial.println("Door opened");
  lcd.clear();
  lcd.setCursor(0, 0);  // İlk satırın başına git
  lcd.print("Hos geldiniz");
  lcd.setCursor(0, 1);  // İkinci satırın başına git
  lcd.print(userName.c_str());
  delay(5000);  //   saniye geçikme
  mySerial.println("isim bastirildi");
  playSound(emotion , userName);
  delay(3000);
  lcd.clear();
  digitalWrite(selenoidPin, HIGH);  // Selenoid kilidi kapali hale getir
}
void loop() {
  if (mySerial.available() > 0 ) { 
      if(digitalRead(selenoidPin) ==  LOW) digitalWrite(selenoidPin,HIGH); //kapi kapa
      String line = mySerial.readStringUntil('\n');
      int idx = line.indexOf(',');
      // virgul yoksa zaten bos mesaj gonderilmis
      if(idx != -1){
        // virgule kadar = recognisedPerson 
        // virgul sonrasi emotion
        openDoor(line.substring(0,idx),line.substring(idx+1));
      }
  }
}
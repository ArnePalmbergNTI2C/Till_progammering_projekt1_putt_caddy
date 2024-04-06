/*

  Fil: Mitt_f_rsta_projekt.ino
  Skapare: Arne Palmberg
  Datum: 2023-12-12
  Beskriving: Detta program beräknar lutning och avstånd till en punkt som sedan uppdaterar detta till en skärm som visar värdena.
  
 
 */


#define trigPin 8 // Definiera pin nummer för trig-signalen på ultraljudssensorn
#define echoPin 9// Definienra pin nummer för echo-signalen på ultraljudssensorn
#include "U8glib.h" // Inkludera biblioteket för OLED-skärmen
#include <Wire.h> // Inkludera Wire-biblioteket för I2C-kommunikation

U8GLIB_SSD1306_128X64 Oled(U8G_I2C_OPT_NO_ACK); // Skapa ett objekt för OLED-skärmen

// Variabler för avståndsmätning och hantering
const int numReadings = 10;
int readings[numReadings];
int readIndex = 0;
int total = 0;
long duration;
int distance;

//variabler från axelrometern
String right_or_left, up_or_down, long_or_short, right_or_left_op_sistasidan, cm;
int sida_formel;
float X_out, Y_out, Z_out;
float roll, pitch, rollF, pitchF = 0;
int rollFF, pitchFF;

//variabler får knappen
int buttonState = 0;
int lastButtonState = LOW;
bool displayAlternate = false;
const int buttonPin = 2;

// variaber för skärmen
int ADXL345 = 0x53;

void setup() {

  Serial.begin(9600); // Starta seriell kommunikation
  Oled.setFont(u8g_font_helvB10); // Ange en font för OLED-skärmen
  pinMode(trigPin, OUTPUT); // Ange trigPin som utgång
  pinMode(echoPin, INPUT); // Ange echoPin som ingång
  pinMode(buttonPin, INPUT); // Ange buttonPin som ingång

  Wire.begin(); // Starta I2C-kommunikationen - Initierar I2C-kommunikationen för att kunna kommunicera med enheter via I2C.

  Wire.beginTransmission(ADXL345); // Börja en I2C-sändning till enheten med adressen ADXL345.
  Wire.write(0x2D); // Skicka kommandot till registret 0x2D på ADXL345.
  Wire.write(8); // Skicka värdet 8 till registret 0x2D på ADXL345.
  Wire.endTransmission(); // Avsluta sändningen till ADXL345 och skicka kommandona. Vänta en kort stund (10 millisekunder) efter sändningen.

  //x axeln
  Wire.beginTransmission(ADXL345); // Börja en ny sändning till ADXL345.
  Wire.write(0x1E); // Skicka kommandot till registret 0x1E på ADXL345.
  Wire.write(-1); // Skicka värdet -1 till registret 0x1E på ADXL345.
  Wire.endTransmission(); // Avsluta sändningen till ADXL345 och vänta en stund.

  //y axeln
  Wire.beginTransmission(ADXL345); // Börja en ny sändning till ADXL345.
  Wire.write(0x1F); // Skicka kommandot till registret 0x1F på ADXL345.
  Wire.write(0); // Skicka värdet 0 till registret 0x1F på ADXL345.
  Wire.endTransmission(); // Avsluta sändningen till ADXL345.

  //z axeln
  Wire.beginTransmission(ADXL345); // Börja en ny sändning till ADXL345.
  Wire.write(0x20); // Skicka kommandot till registret 0x20 på ADXL345.
  Wire.write(3); // Skicka värdet 3 till registret 0x20 på ADXL345.
  Wire.endTransmission(); // Avsluta sändningen till ADXL345.

}

void loop() {

  buttonState = digitalRead(buttonPin); // Läs knappens aktuella tillstånd

  if (buttonState == HIGH && lastButtonState == LOW) { // Kolla om knappen trycks ned för att växla displayläge
    displayAlternate = !displayAlternate; // Växla displayläget
  }

  lastButtonState = buttonState; // Uppdatera knappens föregående tillstånd

  if ((displayAlternate) && (sida_formel >= 1)) { // Kontrollera displayläge och sida_formel för att uppdatera OLED 2
    updateOled2("Aim: " + String(sida_formel) + (" cm ") + String(right_or_left_op_sistasidan), String(long_or_short));
  } else if ((displayAlternate) && (sida_formel < 1)) { // Kontrollera displayläge och sida_formel för att uppdatera OLED 2
    updateOled2(String(right_or_left_op_sistasidan), String(long_or_short));
  } else if ((rollF < 1) && (rollF > -1) && (pitchFF < 1) && (pitchFF > -1)) { // Kontrollera rollF och pitchFF för att uppdatera OLED 1
    updateOled1("Lenght: " + String(distance) + " cm", String(up_or_down), String(right_or_left));
  } else if ((rollF < 1) && (rollF > -1)) { // Kontrollera rollF för att uppdatera OLED 1
    updateOled1("Lenght: " + String(distance) + " cm", String(up_or_down), String(right_or_left) + ": " + String(pitchFF) + " degrees");
  } else if ((pitchF < 1) && (pitchF > -1)) { // Kontrollera pitchF för att uppdatera OLED 1
    updateOled1("Lenght: " + String(distance) + " cm", String(up_or_down)  + ": " + String(rollFF) + " degrees", String(right_or_left));
  } else { // Uppdatera OLED 1 med avstånd, lutning och riktning
    updateOled1("Lenght: " + String(distance) + " cm", String(up_or_down) + ": " + String(rollFF) + " degrees", String(right_or_left) + ": " + String(pitchFF) + " degrees");
  }

  updateDistance(); // Uppdatera avståndet från ultraljudssensorn
  updateLut(); // Uppdatera data från accelerometer


}


void updateDistance() {

  digitalWrite(trigPin, LOW); // Sätt trigPin till låg nivå
  digitalWrite(trigPin, HIGH); // Sätt trigPin till hög nivå
  digitalWrite(trigPin, LOW); // Återgå trigPin till låg nivå
  duration = pulseIn(echoPin, HIGH); // Mät varaktigheten av ljudimpulsen
  distance = duration * 0.034 / 2; // Beräkna avståndet baserat på ljudets hastighet

  // Beräkna medelvärdet av de senaste 10 avståndsmätningarna
  total = total - readings[readIndex];
  readings[readIndex] = distance;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % numReadings;
  distance = total / numReadings;

}

void updateOled1(String text, String text2, String text3) {

  Oled.firstPage(); // Välj första sidan på OLED-skärmen
  do {
    Oled.drawStr(0, 11, text.c_str()); // Rita text på specificerade positioner på skärmen
    Oled.drawStr(0, 37.5, text2.c_str()); // Rita text på specificerade positioner på skärmen
    Oled.drawStr(0, 60, text3.c_str()); // Rita text på specificerade positioner på skärmen
  } while (Oled.nextPage()); // Fortsätt till nästa sida på skärmen om det finns mer att rita

}

void updateOled2(String text, String text2) {

  Oled.firstPage(); // Välj första sidan på OLED-skärmen
  do {
    Oled.drawStr(0, 11, text.c_str()); // Rita text på specificerade positioner på skärmen
    Oled.drawStr(0, 40, text2.c_str()); // Rita text på specificerade positioner på skärmen
  } while (Oled.nextPage()); // Fortsätt till nästa sida på skärmen om det finns mer att rita

}

void updateLut() {

  Wire.beginTransmission(ADXL345); // Starta I2C-sändning till ADXL345
  Wire.write(0x32); // Skicka kommando till ADXL345
  Wire.endTransmission(false); // Avsluta sändningen till ADXL345 (utan att släppa bussen)
  Wire.requestFrom(ADXL345, 6, true); // Begär data från ADXL345
  X_out = ( Wire.read() | Wire.read() << 8); // Läs X-axelvärdet från ADXL345
  X_out = X_out / 256; // Konvertera X-axelvärdet
  Y_out = ( Wire.read() | Wire.read() << 8); // Läs Y-axelvärdet från ADXL345
  Y_out = Y_out / 256; // Konvertera Y-axelvärdet
  Z_out = ( Wire.read() | Wire.read() << 8); // Läs Z-axelvärdet från ADXL345
  Z_out = Z_out / 256; // Konvertera Z-axelvärdet

  // Beräkna roll och pitch från accelerometerdata
  roll = atan(Y_out / sqrt(pow(X_out, 2) + pow(Z_out, 2))) * 180 / PI;
  pitch = atan(-1 * X_out / sqrt(pow(Y_out, 2) + pow(Z_out, 2))) * 180 / PI;

  // Filtrera och beräkna absoluta värden av roll och pitch
  rollF = 0.94 * rollF + 0.06 * roll;
  pitchF = 0.94 * pitchF + 0.06 * pitch;
  rollFF = abs(rollF);
  pitchFF = abs(pitchF);

  // Uppdatera riktningen beroende på pitch lutningsvärden
  if (pitchF > 1) { // Om pitch är större än 1
    right_or_left = "Right"; // Sätt variabel till höger
    right_or_left_op_sistasidan = "left"; // Sätt variabel till vänster
  } else if (pitchF < -1) { // Om pitch är mindre än -1
    right_or_left = "Left"; // Sätt variabel till vänster
    right_or_left_op_sistasidan = "right"; // Sätt variabel riktning till höger
  } else { // Annars, om pitch är mellan -1 och 1
    right_or_left = "Straight"; // Sätt variabel till rak
  }

  // Uppdatera lutning beroende på roll lutningsvärden
  if (rollF > 1) { // Om roll är större än 1
    up_or_down = "Up"; // Sätt variabel till uppåt
    long_or_short = "UpHill"; // Sätt variabeln till uppförsbacke
  } else if (rollF < -1) { // Om roll är mindre än -1
    up_or_down = "Down"; // Sätt variabeln till nedåt
    long_or_short = "DownHill"; // Sätt variabeln till nedförsbacke
  } else { // Annars, om roll är mellan -1 och 1
    up_or_down = "Flat"; // Sätt variabeln till platt
    long_or_short = "Flat"; // Sätt variablen till platt
  }

  // Beräkna formeln för sidans längd beroende på lutningsvärden och avstånd
  sida_formel = tan(pitchFF * PI / 180) * distance;

  // Kontrollera sidans längd och uppdatera motsatt riktning om den är negativ
  if (sida_formel < 0) { // Om sidans längd är mindre än 0
    sida_formel = sida_formel * -1; // Gör sidans längd positiv
  } else if (sida_formel < 1) { // Om sidans längd är mindre än 1
    right_or_left_op_sistasidan = "straight"; // Uppdatera varaibel till "rak"

  }
}

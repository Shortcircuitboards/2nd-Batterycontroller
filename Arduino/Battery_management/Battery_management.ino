/*##############################################################################################################################################################################  
Battery management module by Moe_Pfussel

To automatically manage the charging of my 2 Batteries over Solar,Generator and 230V Charger, to keep the Batteries all charged and use also my Starterbattery for the 
onboard system. It also keeps the Starterbattery alive with the Solar charger. On an OLED you can Show both Voltages and the Status of the Relais

Thanks to:
My Girlfriend for supporting me while I´m loosing my head over broken code...
https://github.com/MichaelUray/muTimer for it´s awesome delay free timer libary! If i had found it earlier it would helped me against a lot of headegg :-)
################################################################################################################################################################################*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 6  // not used 
//Adafruit_SSD1306 display(OLED_RESET); // I2C Display uses A5 for SCL and A4 for SDA
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


/*##############################################################################################################################################################################  
Setting the I/O and giving them Variablenames, Digital Inputs named on top
################################################################################################################################################################################*/

#define Generator 2  //D2 is Feedback from Generator (D+)
#define Ignition 3   //D3 is Feedback Ignition on
#define Starter 4    //D4 is Feedback Starter on (decuouple both batterys !!!)
#define Charger 5     //D5 is Feedback from 230V Charger

#define Q1 12          //D9 is Q1 Main Relais to connect both Batteries
#define Q2 11         //D10 is Q2 Relais to switch on the Booster
#define Q3 10         //D11 is Q3 Relais to cut off the Solarcharger 
#define Q4 9         //D12 is Q4 Relais to the 230V Charger

int Solar = A0;       //A0 is Voltagedivider from the Solar module
int Bat_1 = A1;       //A1 is Voltagedivider of the Starterbattery
int Bat_2 = A2;       //A2 is Voltagedivider of the Board/Backup battery
int Gen = LOW;        //Gen for the read input of Generator
int Ign = LOW;        //Ign for the read input of Ignition
int Sta = LOW;        //Sta for the read input of Starter
int Chg = LOW;        //Chg for the read input of Charger
int screen = 0;       //Generate a variable to switch between the different Screens
bool low_volt = 0;    //Generate a bool low_volt
bool overvoltage = 0; //Generate a bool overvoltage

bool Q1_stat = 0;
bool Q2_stat = 0;
bool Q3_stat = 0;
bool Q4_stat = 0;

int T0 = 0;    //Timer 0


void setup() {
pinMode(Generator, INPUT);
pinMode(Ignition, INPUT);
pinMode(Starter, INPUT);
pinMode(Charger, INPUT);


pinMode(Q1, OUTPUT);
pinMode(Q2, OUTPUT);
pinMode(Q3, OUTPUT);
pinMode(Q4, OUTPUT);

digitalWrite (Q1, LOW);
digitalWrite (Q2, LOW);
digitalWrite (Q3, LOW);
digitalWrite (Q4, LOW);

display.begin(SSD1306_SWITCHCAPVCC, 0x3C);   // initialize with the I2C addr 0x3C / mit I2C-Adresse 0x3c initialisieren
Serial.begin(9600);                          // Verwende die serielle Schnittstelle, um die Nummer auszugeben 
randomSeed(analogRead(0));                   // random start seed / zufälligen Startwert für Random-Funtionen initialisieren
}

#define DRAW_DELAY 118  //Deflault settings für das OLDED DO NOT TOUCH!
#define D_NUM 47        //Default settings für das OLED DO NOT TOUCH!

int i;


void loop() {
  
// Start with reading all the inputs
Gen = digitalRead(Generator);
Ign = digitalRead(Ignition);  
Sta = digitalRead(Starter) ;
Chg = digitalRead(Charger);

/* 
 * Bootscreen stuff 
 */
if (T0 <= 500){
T0 ++ ;
}

/*##############################################################################################################################################################################  
Read Values from the Input and scale them to the Real Values
################################################################################################################################################################################*/
 
  int temp_Bat_1 = analogRead(Bat_1);           //Read the Value of the Analog Input of Bat 1 and write it to the temp Variable
  int temp_Bat_2 = analogRead(Bat_2);           //Read the Value of the Analog Input of Bat 2 and write it to the temp Variable
  int temp_Solar = analogRead(Solar);           //Read the Value of the Analog Input of Solar and write it to the temp Variable
  float Volt_1 = temp_Bat_1 * (32.49 / 1023.0);    //Scale the Input to the displayed (Real) Value 36V on the Voltage divider are 3,3V on the Arduino
  float Volt_2 = temp_Bat_2 * (32.49 / 1023.0);    //Scale the Input to the displayed (Real) Value 36V on the Voltage divider are 3,3V on the Arduino
  float Volt_S = temp_Solar * (32.49 / 1023.0);    //Scale the Input to the displayed (Real) Value 36V on the Voltage divider are 3,3V on the Arduino


/*##############################################################################################################################################################################  
 Generating special cases  Low onboard Volzage
################################################################################################################################################################################*/
if (Volt_2 <= 11.2) { //If the Overall voltage (Both batteries connected) drops under 11.2V
  low_volt = 1 ;      //set the low_volt bit high
}
else if(Volt_2 >= 13){
  low_volt = 0 ;      //set the low_volt bit low
}


/*##############################################################################################################################################################################  
Cases to Switch Q1
################################################################################################################################################################################*/

if (low_volt == 0 || overvoltage == 0 || Gen == HIGH){ 
  Q1_stat = 1;
}

if ((low_volt == 1 &! Gen == HIGH) || Sta == HIGH || overvoltage == 1){
    Q1_stat = 0;
      }
      
/* ##############################################################################################################################################################################
 Cases to Switch Q2 Charging Booster
 ##############################################################################################################################################################################

/* ##############################################################################################################################################################################
 Cases to Switch Q3 Solar Charging Module    Gen == LOW 
 ##############################################################################################################################################################################*/
if (Ign == LOW || Gen == LOW || overvoltage == 0 || Chg == LOW) {
  Q3_stat = 0;
}
if (Ign == HIGH || Gen == HIGH || overvoltage == 1 || Chg == HIGH) {
  Q3_stat = 1;
}

/* ##############################################################################################################################################################################
 Overvoltage for disconnecting all Charging thingis
 ##############################################################################################################################################################################*/
if (Volt_1 >= 15.5 || Volt_2 >= 15.5){
   overvoltage = 1;   //set Overvoltage bit to hight
}
else if (Volt_1 <= 15 && Volt_2 <= 15){
  overvoltage = 0;    //reset the overvoltage bit
}
/* ##############################################################################################################################################################################
Calling the screens
 ##############################################################################################################################################################################

if (Q3_stat == 1 && low_volt == 1 && overvoltage == 0){
   screen = 300;
}
if (Q3_stat == 1 && low_volt == 0 && overvoltage == 0){
  screen = 500;
}
if (Chg == HIGH && low_volt == 1 && overvoltage == 0){
  screen = 600;
}
if (Chg == HIGH && low_volt == 0 && overvoltage == 0){
  screen = 700;
}
*/
if (Q1_stat == 0 && Q3_stat == 0){
  screen = 100;
}
if (Q1_stat == 1 && Q3_stat == 1){ //Both batteries connected --> Solar charging
  screen = 500;
}
if (Q1_stat == 1 && Q3_stat == 0){
  screen = 200;
}

if (overvoltage == 1){
  screen = 400;
}
/*##############################################################################################################################################################################  
Draw the values to de OLED Display
display.drawBitmap(0, 34, plug_icon16x16, 16, 16, 1); to draw an Icon (Test and Adjusting needed)
################################################################################################################################################################################*/
 unsigned char bat16x16[] = //Two batteries symbol
{
0x00, 0x00, 0x00, 0x00, 0x1E, 0x78, 0x3B, 0xDC, 0x73, 0xCE, 0x60, 0x86, 0x60, 0x86, 0x60, 0x86,
0x60, 0x86, 0x60, 0x86, 0x60, 0x86, 0x60, 0x86, 0x60, 0xC6, 0x3F, 0xFC, 0x00, 0x00, 0x00, 0x00
};

 unsigned char plug16x16[] =
{
0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x03, 0xE0, 0x07, 0xF0, 0x04, 0x3C, 0x0C, 0x20, 0x3C, 0x20,
0x24, 0x30, 0x24, 0x3C, 0x27, 0xE0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00
};

 unsigned char generator16x16[] =
{
0x00, 0x00, 0x00, 0x00, 0x10, 0x78, 0x10, 0xFC, 0x39, 0xCC, 0x1B, 0xCC, 0x12, 0xFC, 0x04, 0xF8,
0x08, 0x30, 0x18, 0x60, 0x18, 0xC0, 0x3D, 0x80, 0x27, 0x00, 0x3E, 0x00, 0x18, 0x00, 0x00, 0x00
};
 unsigned char herz82x64 [] = 
{
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xE0, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x0F, 0xF0,
0xFE, 0x00, 0x00, 0x01, 0xFF, 0xF8, 0x00, 0x00, 0x3F, 0x00, 0x1F, 0x80, 0x00, 0x07, 0xFF, 0xFE,
0x00, 0x00, 0x7C, 0x00, 0x07, 0xC0, 0x00, 0x1F, 0xC0, 0x3F, 0x80, 0x00, 0xF0, 0x00, 0x03, 0xE0,
0x00, 0x7E, 0x00, 0x07, 0xC0, 0x01, 0xE0, 0x00, 0x01, 0xF0, 0x00, 0xF8, 0x00, 0x01, 0xF0, 0x03,
0x80, 0x00, 0x00, 0xF0, 0x01, 0xF0, 0x00, 0x00, 0xF8, 0x07, 0x00, 0x00, 0x00, 0x78, 0x03, 0xC0,
0x00, 0x00, 0x7C, 0x0E, 0x00, 0x00, 0x00, 0x38, 0x03, 0x80, 0x00, 0x00, 0x3C, 0x1E, 0x00, 0x00,
0x00, 0x3C, 0x07, 0x80, 0x00, 0x00, 0x1E, 0x1C, 0x00, 0x00, 0x00, 0x1C, 0x0F, 0x00, 0x00, 0x00,
0x0F, 0x38, 0x00, 0x00, 0x00, 0x1E, 0x0E, 0x00, 0x00, 0x00, 0x0F, 0x30, 0x00, 0x00, 0x00, 0x0E,
0x1E, 0x00, 0x00, 0x00, 0x07, 0xF0, 0x00, 0x00, 0x00, 0x0E, 0x1C, 0x00, 0x00, 0x00, 0x03, 0xE0,
0x00, 0x00, 0x00, 0x0F, 0x1C, 0x00, 0x00, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x00, 0x0F, 0x1C, 0x00,
0x00, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x00, 0x07, 0x38, 0x00, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00,
0x00, 0x07, 0x38, 0x00, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x07, 0x38, 0x00, 0x00, 0x00,
0x01, 0xC0, 0x00, 0x00, 0x00, 0x07, 0x38, 0x00, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x07,
0x38, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x07, 0x38, 0x00, 0x00, 0x00, 0x00, 0x80,
0x00, 0x00, 0x00, 0x07, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x38, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x38, 0x00, 0x01, 0x80, 0xC0, 0x00, 0x60, 0x30,
0x00, 0x07, 0x1C, 0x00, 0x03, 0xC0, 0xE0, 0x00, 0x70, 0x38, 0x00, 0x07, 0x1C, 0x00, 0x03, 0xE3,
0xE0, 0x00, 0xF8, 0x78, 0x00, 0x07, 0x1C, 0x00, 0x07, 0xE7, 0xE0, 0x81, 0xF8, 0xF8, 0x00, 0x0F,
0x0E, 0x00, 0x07, 0xEF, 0xE1, 0x81, 0xFB, 0xF8, 0x00, 0x0E, 0x0E, 0x00, 0x0F, 0xFF, 0xE1, 0x81,
0xFF, 0xF8, 0x00, 0x0E, 0x07, 0x00, 0x0F, 0xFE, 0xC3, 0xC3, 0xFF, 0xB8, 0x00, 0x1E, 0x07, 0x80,
0x0E, 0xF9, 0xDF, 0xF3, 0xBE, 0x78, 0x00, 0x1C, 0x03, 0x80, 0x1E, 0x01, 0xCF, 0xE7, 0x80, 0x70,
0x00, 0x3C, 0x03, 0xC0, 0x1C, 0x01, 0xC3, 0x87, 0x00, 0x70, 0x00, 0x38, 0x01, 0xE0, 0x1C, 0x01,
0xC3, 0x07, 0x00, 0x70, 0x00, 0x78, 0x00, 0xF0, 0x18, 0x01, 0x83, 0x06, 0x00, 0x20, 0x00, 0xF0,
0x00, 0x78, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x03, 0xE0, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xC0, 0x00, 0x0F,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x07, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x1F, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFC, 0x00,
0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x03, 0xF0, 0x00,
0x00, 0x00, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00,
0x00, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x03, 0xF0, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00,
0x00, 0xFC, 0x00, 0x00, 0x01, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x07, 0xE0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,
0x80, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x01, 0xF8, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x01, 0xE0, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x1F,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x39, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0xF0, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
 unsigned char solar16x16[] =
{
0x00, 0x00, 0x01, 0x80, 0x0C, 0x30, 0x01, 0x80, 0x03, 0xC0, 0x1B, 0xD8, 0x00, 0x00, 0x1F, 0xF8,
0x30, 0x0C, 0x25, 0xA4, 0x20, 0x04, 0x2D, 0xB4, 0x20, 0x04, 0x3F, 0xFC, 0x00, 0x00, 0x00, 0x00};



if (screen == 100){                   //Default screen Just 2 Voltages, no charging
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
    display.drawBitmap(0, 8, solar16x16, 16, 16, 1);
  display.setCursor(44,5);
  display.println("Bat 1");
  display.setCursor(94,5);
  display.println("Bat 2");            
  display.setCursor(40,18);
  display.print(Volt_1,1);display.println(" V");
  display.setCursor(90,18);
  display.print(Volt_2,1);display.println(" V");            
  display.display();
}
if (screen == 200){            
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.drawBitmap(0, 8, solar16x16, 16, 16, 1);
  display.setCursor(66,5);
  display.println("Bat");         
  display.setCursor(58,18);
  display.print(Volt_2,1);;display.println(" V");            
  display.display();
}


if (screen == 300){                   //Default screen Just 2 Voltages, no charging
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.drawBitmap(0, 8, generator16x16, 16, 16, 1);
  display.setCursor(44,5);
  display.println("Bat 1");
  display.setCursor(94,5);
  display.println("Bat 2");            
  display.setCursor(40,18);
  display.print(Volt_1,1);display.println(" V");
  display.setCursor(90,18);
  display.print(Volt_2,1);display.println(" V");            
  display.display();
}



if (screen == 400){                   //Overvoltage screen
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20,5);
  display.println("Ueberspannung !!!");            
  display.setCursor(40,18);
  display.print(Volt_1,1);display.println(" V");
  display.setCursor(90,18);
  display.print(Volt_2,1);display.println(" V");            
  display.display();
}

if (screen == 500){            
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.drawBitmap(0, 8, generator16x16, 16, 16, 1);
  display.setCursor(66,5);
  display.println("Bat");         
  display.setCursor(58,18);
  display.print(Volt_2,1);;display.println(" V");            
  display.display();
}

if (screen == 600){                   //Default screen Just 2 Voltages, no charging
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.drawBitmap(0, 8, plug16x16, 16, 16, 1);
  display.setCursor(44,5);
  display.println("Bat 1");
  display.setCursor(94,5);
  display.println("Bat 2");            
  display.setCursor(40,18);
  display.print(Volt_1,1);display.println(" V");
  display.setCursor(90,18);
  display.print(Volt_2,1);display.println(" V");            
  display.display();
}

if (screen == 700){            
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.drawBitmap(0, 8, plug16x16, 16, 16, 1);
  display.setCursor(66,5);
  display.println("Bat");         
  display.setCursor(58,18);
  display.print(Volt_2,1);;display.println(" V");            
  display.display();
}
if (screen == 800){                   //Default screen Just 2 Voltages, no charging
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(44,5);
  display.println("Bat 1");
  display.setCursor(94,5);
  display.println("Bat 2");            
  display.setCursor(40,18);
  display.print(Volt_1,1);display.println(" V");
  display.setCursor(90,18);
  display.print(Volt_2,1);display.println(" V");            
  display.display();
}
if (screen == 900){            
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(66,5);
  display.println("Bat");         
  display.setCursor(58,18);
  display.print(Volt_2,1);;display.println(" V");            
  display.display();
}

if (T0 <= 495){
 screen = 999; 
}
/* if (screen == 999){
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.drawBitmap(0, 8, herz82x64, 82, 64, 1); 
  display.display(); 
}
*/

digitalWrite(Q1, Q1_stat);
digitalWrite(Q2, Q2_stat);
digitalWrite(Q3, Q3_stat);
digitalWrite(Q4, Q4_stat);

} // The End of Code!! Nothing else should come here!!

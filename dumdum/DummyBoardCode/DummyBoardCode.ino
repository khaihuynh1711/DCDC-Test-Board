
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);




//INPUTS
const int ON_DCDC0 = 14;
float DCDC0_on;

const int DCDC0_FAULT = 27; 
float DC0_f;

const int state_PRE = 26;
float pre_s;

const int CHRG_VOLT = 13; //fix
float CHRG_v;

const int CHRG_FAULT = 25;
float CHRG_f;

const int CHRG_ON = 33; //13
float CHRG_on;


const int volt_BATT = 39; //
float BATT_v;

const int fault_DCDC1 = 32;
float DC1_f;

const int DCDC1_OverCurrent = 35;
float DC1_oc;

const int DCDC1_ON = 34;
float DC1_on;




const int adcCurrent = 15; //never leave this pin loating --> read unwanted values
float iCurrent;

//OUTPUTS
const int oAUX = 4; //Analog output
const int oDCDC = 17;//digital output
const int oCHRG = 16;// digital output

void setup() {
  
  Serial.begin(115200);

  pinMode(ON_DCDC0, INPUT);
  pinMode(DCDC0_FAULT, INPUT);
  pinMode(CHRG_VOLT, INPUT);
  pinMode(CHRG_FAULT, INPUT);
  pinMode(CHRG_ON, INPUT);
  pinMode(volt_BATT, INPUT);
  pinMode(fault_DCDC1, INPUT);
  pinMode(DCDC1_OverCurrent, INPUT);
  pinMode(DCDC1_ON, INPUT);
  pinMode(state_PRE, INPUT);
  pinMode(adcCurrent, INPUT);

  // Set output pin modes
  pinMode(oAUX, OUTPUT);
  pinMode(oDCDC, OUTPUT);
  pinMode(oCHRG, OUTPUT);

  // Begin serial communication for debugging
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  delay(3000);
}



void loop() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  //Collecting inputs
  DCDC0_on = digitalRead(ON_DCDC0); //           AUX found: LOW       AUX not found: HIGH
  DC0_f = digitalRead(DCDC0_FAULT); //           Fault: HIGH          Good: LOW

  pre_s = digitalRead(state_PRE);//              Not done: HIGH       Done: LOW

  CHRG_v = digitalRead(CHRG_VOLT); //MOSFET//    CHARGE found: HIGH   CHARGE not found: LOW
  CHRG_f = digitalRead(CHRG_FAULT); //           Fault: HIGH          Good: LOW
  CHRG_on = digitalRead(CHRG_ON); //             CHARGE on: LOW       CHARGE off: HIGH

  BATT_v = digitalRead(volt_BATT); //MOSFET//    BATT found: HIGH     BATT not found: LOW
  DC1_f = digitalRead(fault_DCDC1); //           Fault: HIGH          Good: LOW
  DC1_oc = digitalRead(DCDC1_OverCurrent); //    Fault: HIGH          Good: LOW
  DC1_on = digitalRead(DCDC1_ON); //             DCDC1 on: LOW        DCDC1 off: HIGH
  
  iCurrent = analogRead(adcCurrent)/341.0;

  //Current display
  display.setCursor(0, 0);
  display.println("Current: " + String(iCurrent)+" A");
  Serial.print("Current: ");
  Serial.println(iCurrent);
  

  //Start up
  if (DCDC0_on < 1){

    if (DC0_f < 1) {

      if(iCurrent < 10) {

        if (pre_s < 1) {
          //Pre charge done
          Serial.println("Pre-Charge done");
          
          if ( CHRG_v >=1){
            //Charge ON
            Serial.println("Charge line detected");

            if (CHRG_on < 1){
            
              if (CHRG_f <1 && iCurrent < 4.75 ){
                //Charging
                display.setCursor(0, 18);
                display.println("__CHARGING__");
                Serial.println("AUX OFF - DCDC OFF - CHARGE ON");

                display.setCursor(0, 30);
                display.println("AUX->DCDC0    OFF");
                display.println("BATT->DCDC1   OFF");
                display.println("CHRG->LV      ON");

                analogWrite(oAUX, 0);
                digitalWrite(oDCDC, HIGH); // DCDC1 on: LOW  DCDC1 off: HIGH
                digitalWrite(oCHRG, LOW); // CHARGE on: LOW  CHARGE off: HIGH

              }
              else{
                if (BATT_v >= 1) {   //Batt on: high    Batt off: low 
                  //BATT ready
                  Serial.println("Battery detected");
                  

                
                  if (DC1_on < 1){

                    if (DC1_f <1 && DC1_oc <1 ){
                      //DCDC1 ready
                      display.setCursor(0, 18);
                      display.println("__CHARGE FAULT DCDC 1 ON__");
                      Serial.println("AUX OFF - DCDC ON - CHARGE OFF");

                      display.setCursor(0, 30);
                      display.println("AUX->DCDC0    OFF");
                      display.println("BATT->DCDC1   ON");
                      display.println("CHRG->LV      OFF");

                      analogWrite(oAUX, 0);
                      digitalWrite(oDCDC, LOW); // DCDC1 on: LOW  DCDC1 off: HIGH
                      digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH
                    }
                    else{
                      display.setCursor(0, 18);
                      display.println("__DCDC 1 FAULT__");
                      Serial.println("AUX ON - DCDC OFF - CHARGE OFF");

                      display.setCursor(0, 30);
                      display.println("AUX->DCDC0    ON");
                      display.println("BATT->DCDC1   OFF");
                      display.println("CHRG->LV      OFF");
                      analogWrite(oAUX, 255);
                      digitalWrite(oDCDC, HIGH); // DCDC1 on: LOW  DCDC1 off: HIGH
                      digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH
                    }
                  }
                  else{
                    display.setCursor(0, 18);
                    display.println("__DCDC 1 NOT READY__");
                    Serial.println("AUX ON - DCDC ON - CHARGE OFF");

                    display.setCursor(0, 30);
                    display.println("AUX->DCDC0    ON");
                    display.println("BATT->DCDC1   ON");
                    display.println("CHRG->LV      OFF");

                    analogWrite(oAUX, 255);
                    digitalWrite(oDCDC, LOW); // DCDC1 on: LOW  DCDC1 off: HIGH
                    digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH
                  }

                }
                else{
                  display.setCursor(0, 18);
                  display.println("__BATT NOT DETECTED__");
                  Serial.println("AUX ON - DCDC OFF - CHARGE OFF");
                  Serial.println("Batt not detected");

                  display.setCursor(0, 30);
                  display.println("AUX->DCDC0    ON");
                  display.println("BATT->DCDC1   OFF");
                  display.println("CHRG->LV      OFF");
                  display.println(BATT_v);
                  
                  analogWrite(oAUX, 255);
                  digitalWrite(oDCDC, HIGH); // DCDC1 on: LOW  DCDC1 off: HIGH
                  digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH
                }
                  }
//______
            }
            else {

              display.setCursor(0, 18);
              display.println("__SWITCHING TO CHARGER__");
              Serial.println("AUX ON - DCDC OFF - CHARGE ON");

              display.setCursor(0, 30);
              display.println("AUX->DCDC0    ON");
              display.println("BATT->DCDC1   OFF");
              display.println("CHRG->LV      ON");

              analogWrite(oAUX, 255);
              digitalWrite(oDCDC, HIGH); // DCDC1 on: LOW  DCDC1 off: HIGH
              digitalWrite(oCHRG, LOW); // CHARGE on: LOW  CHARGE off: HIGH
            }

          }
          else{
            if (BATT_v >= 1) {   //Batt on: high    Batt off: low 
              //BATT ready
              Serial.println("Battery detected");
              

        
              if (DC1_on < 1){

                if (DC1_f <1 && DC1_oc <1 ){
                  //DCDC1 ready
                  display.setCursor(0, 18);
                  display.println("__DCDC 1 ON__");
                  Serial.println("AUX OFF - DCDC ON - CHARGE OFF");

                  display.setCursor(0, 30);
                  display.println("AUX->DCDC0    OFF");
                  display.println("BATT->DCDC1   ON");
                  display.println("CHRG->LV      OFF");

                  analogWrite(oAUX, 0);
                  digitalWrite(oDCDC, LOW); // DCDC1 on: LOW  DCDC1 off: HIGH
                  digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH
                }
                else{
                  display.setCursor(0, 18);
                  display.println("__DCDC 1 FAULT__");
                  Serial.println("AUX ON - DCDC OFF - CHARGE OFF");

                  display.setCursor(0, 30);
                  display.println("AUX->DCDC0    ON");
                  display.println("BATT->DCDC1   OFF");
                  display.println("CHRG->LV      OFF");
                  analogWrite(oAUX, 255);
                  digitalWrite(oDCDC, HIGH); // DCDC1 on: LOW  DCDC1 off: HIGH
                  digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH
                }
              }
              else{
                display.setCursor(0, 18);
                display.println("__DCDC 1 NOT READY__");
                Serial.println("AUX ON - DCDC ON - CHARGE OFF");

                display.setCursor(0, 30);
                display.println("AUX->DCDC0    ON");
                display.println("BATT->DCDC1   ON");
                display.println("CHRG->LV      OFF");

                analogWrite(oAUX, 255);
                digitalWrite(oDCDC, LOW); // DCDC1 on: LOW  DCDC1 off: HIGH
                digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH
              }

            }
            else{
              display.setCursor(0, 18);
              display.println("__BATT NOT DETECTED__");
              Serial.println("AUX ON - DCDC OFF - CHARGE OFF");
              Serial.println("Batt not detected");

              display.setCursor(0, 30);
              display.println("AUX->DCDC0    ON");
              display.println("BATT->DCDC1   OFF");
              display.println("CHRG->LV      OFF");
              display.println(BATT_v);
              
              analogWrite(oAUX, 255);
              digitalWrite(oDCDC, HIGH); // DCDC1 on: LOW  DCDC1 off: HIGH
              digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH
            }
          }
        } 
        else {
          display.setCursor(0, 18);
          display.println("__PRE-CHARGING__");

          Serial.println("AUX ON - DCDC OFF - CHARGE OFF");

          display.setCursor(0, 30);
          display.println("AUX->DCDC0    ON");
          display.println("BATT->DCDC1   OFF");
          display.println("CHRG->LV      OFF");

          analogWrite(oAUX, 255);
          digitalWrite(oDCDC, HIGH); // DCDC1 on: LOW  DCDC1 off: HIGH
          digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH
        }
      }
      else{
        Serial.println("AUX OFF - DCDC OFF - CHARGE OFF");
        Serial.println("AUX Over Current");

        display.setCursor(0, 18);
        display.println("__OFF__");

        display.setCursor(0, 30);
        display.println("AUX->DCDC0    OFF");
        display.println("BATT->DCDC1   OFF");
        display.println("CHRG->LV      OFF");
        
        analogWrite(oAUX, 0);
        digitalWrite(oDCDC, HIGH); // DCDC1 on: LOW  DCDC1 off: HIGH
        digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH
      }

    }
    else {
      Serial.println("AUX OFF - DCDC OFF - CHARGE OFF");
      Serial.println("DCDC0 FAULT");

      display.setCursor(0, 18);
      display.println("__DCDC0 FAULT__");

      display.setCursor(0, 30);
      display.println("AUX->DCDC0    OFF");
      display.println("BATT->DCDC1   OFF");
      display.println("CHRG->LV      OFF");
      
      analogWrite(oAUX, 0);
      digitalWrite(oDCDC, HIGH); // DCDC1 on: LOW  DCDC1 off: HIGH
      digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH

    }
  }
  else{
    Serial.println("AUX not connected");
    analogWrite(oAUX, 0);
    digitalWrite(oDCDC, HIGH); // DCDC1 on: LOW  DCDC1 off: HIGH
    digitalWrite(oCHRG, HIGH); // CHARGE on: LOW  CHARGE off: HIGH


  }


  display.display(); 
  //delay(1000);


}



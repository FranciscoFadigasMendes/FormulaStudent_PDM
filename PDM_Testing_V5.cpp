/*

Description: Mock PDM (Power Distribution Module) to Place on the Test Board

Author: Francisco Fadigas Mendes

*/

#include <Arduino.h>
#include <ESP32_CAN.h>

#define ADCPIN_0 A0  //Cooling Board Current Sense
#define ADCPIN_4 A4 // Button
#define ADCPIN_5 A5 // Potentiometer
#define ADCPIN_13 A13 // Source Voltage

//Functions
void can_bus();
void cooling_board(); 
void terminal();

//CAN Bus Setup
TWAI_Interface CAN1(1000,4,5); // BaudRate/1000 , TX , RX

//Global Variables
float PotVoltValue,InputVoltValue;
int increment = 0,FAN;
unsigned long volts_print_time = 0, can_time = 0, cb_time=0;

//----------------------------------------------------------------
//----------------------------------------------------------------

void setup() {

  Serial.begin(115200);

  pinMode(23, OUTPUT); //Cooling Board Powertrain

  }

//----------------------------------------------------------------

void loop() {

    if( (millis() - can_time) > 333){  
      can_time = millis();
      cooling_board(); 
      can_bus();
      terminal();
    }

}

//----------------------------------------------------------------
//----------------------------------------------------------------

void can_bus(){

  //CAN Bus Periodic Test Message
  CAN1.TXpacketBegin(0x69,0);

  CAN1.TXpacketLoad(10); 
  CAN1.TXpacketLoad(FAN); 
  CAN1.TXpacketLoad(InputVoltValue);
           
  CAN1.TXpackettransmit();

  Serial.println("TEST MESSAGE SENT");

}

//----------------------------------------------------------------

void cooling_board(){

    int buttonValue,adc4Value,adc0Value;
    float cooling_power,cooling_p;

    //Power On Values
    if(PotVoltValue >= 0.5)     
      digitalWrite(23, HIGH); 
    if(PotVoltValue < 0.5)
      digitalWrite(23, LOW); 

    //Signals the Cooling Board to Turn on the Fan
    adc4Value = analogRead(ADCPIN_4);
    buttonValue = ((adc4Value * 3.3) / 4095);

    adc0Value = analogRead(ADCPIN_0);
    cooling_p = ((adc0Value * 3.3) / 4095);
    cooling_power = cooling_p / ( 0.0048 );

    if(buttonValue>1.5)
      increment++;

    if(increment % 2 == 0)
      FAN=20;   
    else
      FAN=30;  

    Serial.print("Current = ");
    Serial.print(cooling_power,4);
    Serial.println(" mA");

    Serial.print("Increment: ");
    Serial.println(increment);

    if(buttonValue > 3)
      Serial.print("Button ");

}

//----------------------------------------------------------------

void terminal(){

  int adc5Value,adc12Value;
  float InputVoltValue_ADC;

    adc5Value = analogRead(ADCPIN_5);
    PotVoltValue = ((adc5Value * 3.3) / 4095);

    adc12Value = analogRead(ADCPIN_13);
    InputVoltValue_ADC = ((adc12Value * 3.3) / 4095);
    
    Serial.print("POT = ");
    Serial.print(PotVoltValue, 3);
    Serial.println(" V ");
     
    InputVoltValue = InputVoltValue_ADC / 0.096;
    
    Serial.print("Volts_ADC = ");
    Serial.print(InputVoltValue_ADC, 3);
    Serial.println(" V ");

    Serial.print("Volts = ");
    Serial.print(InputVoltValue, 3);
    Serial.println(" V ");

}

//----------------------------------------------------------------

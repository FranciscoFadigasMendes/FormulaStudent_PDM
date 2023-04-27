/*

Description: Mock PDM (Power Distribution Module) to Place on the Test Board

Author: Francisco Fadigas Mendes

*/

#include <Arduino.h>
#include <ESP32_CAN.h>

#define ADCPIN_5 A5
#define ADCPIN_4 A4
#define ADCPIN_0 A0


//Functions
void can_bus();
void cooling_board(); 
void terminal();

//CAN Bus Setup
TWAI_Interface CAN1(1000,4,5); // BaudRate/1000 , TX , RX

//Global Variables
float voltValue;
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
           
  CAN1.TXpackettransmit();

  Serial.println("TEST MESSAGE SENT");

}

//----------------------------------------------------------------

void cooling_board(){

    int buttonValue,adc4Value,adc0Value;
    float cooling_power;

    //Power On Values
    if(voltValue >= 0.5)     
      digitalWrite(23, HIGH); 
    if(voltValue < 0.5)
      digitalWrite(23, LOW); 

    //Signals the Cooling Board to Turn on the Fan
    adc4Value = analogRead(ADCPIN_4);
    buttonValue = ((adc4Value * 3.3) / 4095);

    adc0Value = analogRead(ADCPIN_0);
    cooling_power = ((adc0Value * 3.3) / 4095);

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
}

//----------------------------------------------------------------

void terminal(){

  int adc5Value;

    adc5Value = analogRead(ADCPIN_5);
    voltValue = ((adc5Value * 3.3) / 4095);
    
    Serial.print("Volts = ");
    Serial.print(voltValue, 3);
    Serial.println(" V ");

}

//----------------------------------------------------------------

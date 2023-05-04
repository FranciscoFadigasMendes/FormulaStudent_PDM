/*
Description: Mock PDM (Power Distribution Module) to Place on the Test Board
Author: Francisco Fadigas Mendes
*/

#include <Arduino.h>
#include <ESP32_CAN.h>

#define ADCPIN_0 A0     //Cooling Board Current Sense
#define ADCPIN_4 A4     // Button
#define ADCPIN_5 A5     // Potentiometer
#define ADCPIN_13 A13   // Source Voltage

//Functions
void can_bus();
void cooling_board(); 
void datalogger();
void terminal();

//CAN Bus Setup
TWAI_Interface CAN1(1000,4,5); // BaudRate/1000 , TX , RX

//Global Variables
float PotVoltValue,InputVoltValue,cooling_power,cooling_p,InputVoltValue_ADC; 
int increment = 0,FAN,buttonValue,adc4Value,adc0Value,adc5Value,adc12Value;
unsigned long volts_print_time = 0, can_time = 0, cb_time=0;

//----------------------------------------------------------------
//----------------------------------------------------------------

void setup() {

  Serial.begin(115200);

  pinMode(23, OUTPUT); // Cooling Board Powertrain
  pinMode(22, OUTPUT); // DataLogger

  }

//----------------------------------------------------------------

void loop() {

    if( (millis() - can_time) > 333){  // 3 times a second
      can_time = millis();
      cooling_board(); 
      datalogger();
      can_bus();
      terminal();
    }

}

//----------------------------------------------------------------
//----------------------------------------------------------------

void can_bus(){

  //CAN Bus Periodic Test Message
  CAN1.TXpacketBegin(0x69,0);

  CAN1.TXpacketLoad(InputVoltValue*10);
  CAN1.TXpacketLoad(FAN); 

  CAN1.TXpackettransmit();

  Serial.println("CAN MESSAGE SENT");

}

//----------------------------------------------------------------

void cooling_board(){

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

    cooling_power = cooling_p / (0.031*50);

    if(buttonValue>1.5)
      increment++;

    if(increment % 2 == 0)
      FAN=20;   
    else
      FAN=30;  

    if(cooling_power > 20){
      Serial.print("Current = ");
      Serial.print(cooling_power,2);
      Serial.println(" mA");
    } 
    if(cooling_power < 20)
      Serial.println("Current Value Below 20mA");

    Serial.print("Increment: ");
    Serial.println(increment);

    if(buttonValue > 2)
      Serial.print("Button ");
}

//----------------------------------------------------------------

void datalogger(){

    //Power On Values
    if(PotVoltValue >= 1)     
      digitalWrite(22, HIGH); 
    if(PotVoltValue < 1)
      digitalWrite(22, LOW); 

}

//----------------------------------------------------------------

void terminal(){

    adc5Value = analogRead(ADCPIN_5);
    PotVoltValue = ((adc5Value * 3.3) / 4095);

    adc12Value = analogRead(ADCPIN_13);
    InputVoltValue_ADC = ((adc12Value * 3.3) / 4095);
    
    Serial.print("POT = ");
    Serial.print(PotVoltValue, 3);
    Serial.println(" V ");
     
    InputVoltValue = InputVoltValue_ADC * 11,8;

    Serial.print("Volts = ");
    Serial.print(InputVoltValue, 3);
    Serial.println(" V ");

}

//----------------------------------------------------------------

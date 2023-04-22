/*

Description: Mock PDM (Power Distribution Module) to Place on the Test Board

Author: Francisco Fadigas Mendes

*/

#include <Arduino.h>
#include <ESP32_CAN.h>

#define ADCPIN5 A5
#define ADCPIN4 A4

//Functions
void can_bus();
void cooling_board(); 
void datalogger();
void brake_light();
void terminal();

//CAN Bus Setup
TWAI_Interface CAN1(1000,4,5); // BaudRate/1000 , TX , RX

//Global Variables
float voltValue;
unsigned long volts_print_time = 0, can_time = 0;

//----------------------------------------------------------------
//----------------------------------------------------------------

void setup() {

  Serial.begin(115200);

  pinMode(23, OUTPUT); //Cooling Board Powertrain
  pinMode(18, OUTPUT); //DataLogger
  pinMode(5, OUTPUT);  //Devboard
  pinMode(25, OUTPUT); //Brake Light

  }

//----------------------------------------------------------------

void loop() {


    //CAN Bus Periodic Test Message
    if( (millis() - can_time) > 500){  
      can_time = millis();
      can_bus();
    }
    
    cooling_board(); 

    datalogger();

    brake_light();

    //Print Pot Value
    if( (millis() - volts_print_time) > 500){   
      volts_print_time = millis();  
      terminal();
    }
}

//----------------------------------------------------------------
//----------------------------------------------------------------

void can_bus(){

  //CAN Bus Periodic Test Message
  CAN1.TXpacketBegin(0x10,0);
  CAN1.TXpacketLoad(10);  
  CAN1.TXpacketLoad(20);  
  CAN1.TXpacketLoad(30);  
  CAN1.TXpackettransmit();

  Serial.println("TEST MESSAGE SENT");

}

//----------------------------------------------------------------

void cooling_board(){

    int buttonValue,adc4Value;

    //Power On Values
    if(voltValue >= 0.5)     
      digitalWrite(23, HIGH); 
    if(voltValue < 0.5)
      digitalWrite(23, LOW); 

    //Signals the Cooling Board to Turn on the Fan
    adc4Value = analogRead(ADCPIN4);
    buttonValue = ((adc4Value * 3.3) / 4095);

    if(buttonValue>1.5){
      CAN1.TXpacketBegin(0x20,0);      
      CAN1.TXpacketLoad(20);  
      CAN1.TXpackettransmit();
      Serial.println("TURN ON FAN"); 
  }

}

//----------------------------------------------------------------

void datalogger(){

    //Power On Values
    if(voltValue >= 1)       
      digitalWrite(18, HIGH); 
    if(voltValue < 1)
      digitalWrite(18, LOW); 
  
}

//----------------------------------------------------------------

void brake_light(){

    //Devboard Power On Values
    if(voltValue >= 1.5)    
      digitalWrite(5, HIGH); 
    if(voltValue < 1.5)
      digitalWrite(5, LOW); 
    //Brake Light Power On Values
    if(voltValue >= 2)       
      digitalWrite(25, HIGH); 
    if(voltValue < 2)
      digitalWrite(25, LOW); 
  
}

//----------------------------------------------------------------

void terminal(){

  int adc5Value;

    adc5Value = analogRead(ADCPIN5);
    voltValue = ((adc5Value * 3.3) / 4095);
    Serial.print("Volts = ");
    Serial.print(voltValue, 3);
    Serial.println(" V ");
    
}

//----------------------------------------------------------------

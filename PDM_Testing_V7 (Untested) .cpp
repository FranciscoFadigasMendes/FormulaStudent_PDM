/*
Description: Mock PDM (Power Distribution Module) to Place on the Test Board
Author: Francisco Fadigas Mendes
*/

#include <Arduino.h>
#include <ESP32_CAN.h>

#define ADCPIN_6 A6     // Potentiometer -> ADC1_CH6
#define ADCPIN_7 A7     // Button -> ADC1_CH7
#define ADCPIN_18 A18   // Source Voltage -> ADC2_CH8
#define ADCPIN_19 A19     //Cooling Board Current Sense -> ADC2_CH9

//Functions
void can_bus();
void inputs_status();
void outputs_status();
void turn_on_sequence();
void cooling_board(); 
void brakelight();
void terminal();

//CAN Bus Setup
TWAI_Interface CAN1(1000,4,5); // BaudRate/1000 , 4 TX , 5 RX

//Global Variables
int adc6Value,adc7Value,adc18Value,adc19Value; //ADC RAW VALUES
unsigned long volts_print_time = 0, can_time = 0, cb_time=0; // VARIABLES FOR MILLIS FUCNTION

int increment = 0,FAN,terminal_cycles_counter=0;
float PotVoltValue,InputVoltValue,buttonValue,cooling_power,cooling_p,InputVoltValue_ADC; 


//----------------------------------------------------------------
//----------------------------------------------------------------

void setup() {

  Serial.begin(115200);

  pinMode(23, OUTPUT); // Cooling Board 
  pinMode(22, OUTPUT); // DataLogger
  pinMode(19, OUTPUT); // Brake Light

  }

//----------------------------------------------------------------

void loop() {

  terminal_cycles_counter++;

  Serial.println("----------<= terminal_cycles_counter =>---------");

    if( (millis() - can_time) > 333){  // 3 times a second
      can_time = millis();
      turn_on_sequence();
      cooling_board(); 
      can_bus();
      terminal();
    }

  Serial.println("--------------------------------");
}

//----------------------------------------------------------------
//----------------------------------------------------------------

void can_bus(){

  int InputVoltValue_Offset = InputVoltValue * 100;
  Serial.println(InputVoltValue_Offset);

  inputs_status();
  outputs_status();

  //---------------------CAN ID40 MESSAGE BEGIN---------------------
  CAN1.TXpacketBegin(0x40,0);    
      
  //BYTE 0/1/2 -- OUTPUTS (8*3)
  CAN1.TXpacketLoad(0); 
  CAN1.TXpacketLoad(0); 
  CAN1.TXpacketLoad(0); 

  //BYTE 3 -- INPUTS (8*1)
  CAN1.TXpacketLoad(0); 

  //BYTE 4/5 -- LV VOLTAGE
  CAN1.TXpacketLoad(InputVoltValue_Offset);      //LOW BYTE
  CAN1.TXpacketLoad(InputVoltValue_Offset >> 8); //HIGH BYTE

  //BYTE 6/7 -- TOTAL PWR
  CAN1.TXpacketLoad(0); 
  CAN1.TXpacketLoad(0); 

  CAN1.TXpackettransmit();
  Serial.println("CAN ID40 SENT");
  //---------------------CAN ID40 MESSAGE END---------------------


  //---------------------CAN ID41 MESSAGE BEGIN---------------------
  CAN1.TXpacketBegin(0x41,0);    
      
  //BYTE 0/1 -- COOLING BOARD POWERTRAIN POWER
  CAN1.TXpacketLoad(0); 
  CAN1.TXpacketLoad(0); 

  //BYTE 2/3 -- COOLING BOARD BATTERY PACK POWER
  CAN1.TXpacketLoad(0); 
  CAN1.TXpacketLoad(0); 

  //BYTE 4/5 -- VCU POWER
  CAN1.TXpacketLoad(0); 
  CAN1.TXpacketLoad(0); 

  //BYTE 6/7 -- INVERTER POWER
  CAN1.TXpacketLoad(0); 
  CAN1.TXpacketLoad(0); 

  CAN1.TXpackettransmit();
  Serial.println("CAN ID41 SENT");
  //---------------------CAN ID41 MESSAGE END---------------------


  //---------------------CAN ID42 MESSAGE BEGIN---------------------
  CAN1.TXpacketBegin(0x42,0);    
      
  //BYTE 0 -- SHUTDOWN CIRCUIT STATE
  CAN1.TXpacketLoad(0); 

  //BYTE 1 -- COOLING BOARD POWERTRAIN ERROR
  CAN1.TXpacketLoad(0); 

  //BYTE 2 -- COOLING BOARD BATTERY PACK ERROR
  CAN1.TXpacketLoad(0); 

  //BYTE 3 -- VCU ERROR
  CAN1.TXpacketLoad(0); 

  CAN1.TXpackettransmit();
  Serial.println("CAN ID42 SENT");
  //---------------------CAN ID41 MESSAGE END---------------------
  
}

//----------------------------------------------------------------
void outputs_status(){



}

void outputs_status(){



}
//----------------------------------------------------------------

void turn_on_sequence(){

    //Reads Pot Value
    adc6Value = analogRead(ADCPIN_6);
    PotVoltValue = ((adc6Value * 3.3) / 4095);

    //Cooling Board Power On Values
    if(PotVoltValue >= 0.5)     
      digitalWrite(23, HIGH); 
    if(PotVoltValue < 0.5)
      digitalWrite(23, LOW); 

    //DataLogger Power On Values
    if(PotVoltValue >= 1)     
      digitalWrite(22, HIGH); 
    if(PotVoltValue < 1)
      digitalWrite(22, LOW); 

    //Brake Light Power On Values
    if(PotVoltValue >= 1.5)     
      digitalWrite(19, HIGH); 
    if(PotVoltValue < 1.5)
      digitalWrite(19, LOW); 


}

//----------------------------------------------------------------

void cooling_board(){

    //Signals the Cooling Board to Turn on the Fan
    adc7Value = analogRead(ADCPIN_7);
    buttonValue = ((adc7Value * 3.3) / 4095);

    //Reads the Cooling Board's Power Consumption
    adc19Value = analogRead(ADCPIN_19);
    cooling_p = ((adc19Value * 3.3) / 4095);
    cooling_power = ( (cooling_p / (0.002*50)) + 0.08 );

    if(buttonValue>1.5)
      increment++;

    if(increment % 2 == 0)
      FAN=20;   
    else
      FAN=30; 

}

//----------------------------------------------------------------

void terminal(){

    adc18Value = analogRead(ADCPIN_18);
    InputVoltValue_ADC = ((adc18Value * 3.3) / 4095);
     
    InputVoltValue = InputVoltValue_ADC * 11.7;

    Serial.print("Source Voltage = ");
    Serial.print(InputVoltValue, 3);
    Serial.println(" V ");

    Serial.print("POT = ");
    Serial.print(PotVoltValue, 3);
    Serial.println(" V ");

    Serial.print("Increment =  ");
    Serial.println(increment);

}

//----------------------------------------------------------------

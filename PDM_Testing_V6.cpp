/*
Description: Mock PDM (Power Distribution Module) to Place on the Test Board
Author: Francisco Fadigas Mendes
*/

#include <Arduino.h>
#include <ESP32_CAN.h>

#define ADCPIN_0 A0     //Cooling Board Current Sense -> ADC1_CH0
#define ADCPIN_4 A4     // Button -> ADC1_CH4
#define ADCPIN_5 A5     // Potentiometer -> ADC1_CH5
#define ADCPIN_13 A13   // Source Voltage -> ADC2_CH4

//Functions
void can_bus();
void cooling_board(); 
void datalogger();
void brakelight();
void terminal();

//CAN Bus Setup
TWAI_Interface CAN1(1000,4,5); // BaudRate/1000 , TX , RX

//Global Variables
float PotVoltValue,InputVoltValue,cooling_power,cooling_p,InputVoltValue_ADC; 
int buttonValue,adc4Value,adc0Value,adc5Value,adc13Value;
int increment = 0,FAN;
unsigned long volts_print_time = 0, can_time = 0, cb_time=0;

//----------------------------------------------------------------
//----------------------------------------------------------------

void setup() {

  Serial.begin(115200);

  pinMode(23, OUTPUT); // Cooling Board 
  pinMode(22, OUTPUT); // DataLogger
  //pinMode(1, OUTPUT); // Brake Light

  }

//----------------------------------------------------------------

void loop() {

    if( (millis() - can_time) > 333){  // 3 times a second
      can_time = millis();
      cooling_board(); 
      datalogger();
     // brakelight();
      can_bus();
      terminal();
    }
}

//----------------------------------------------------------------
//----------------------------------------------------------------

void can_bus(){

  int InputVoltValue_Offset = InputVoltValue * 100;
  Serial.println(InputVoltValue_Offset);

  //---------------------CAN ID40 MESSAGE BEGIN---------------------
  CAN1.TXpacketBegin(0x40,0);    
      
  //BYTE 0/1/2 -- OUTPUTS (8*3)
  CAN1.TXpacketLoad(0); 
  CAN1.TXpacketLoad(0); 
  CAN1.TXpacketLoad(0); 

  //BYTE 3 -- INPUTS (8*1)
  CAN1.TXpacketLoad(0); 

  //BYTE 4/5 -- LV VOLTAGE
  CAN1.TXpacketLoad(InputVoltValue_Offset);      //LOW
  CAN1.TXpacketLoad(InputVoltValue_Offset >> 8); //HIGH

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

void cooling_board(){

    //Power On Values
    if(PotVoltValue >= 0.5)     
      digitalWrite(23, HIGH); 
    if(PotVoltValue < 0.5)
      digitalWrite(23, LOW); 

    //Signals the Cooling Board to Turn on the Fan
    adc4Value = analogRead(ADCPIN_4);
    buttonValue = ((adc4Value * 3.3) / 4095);

    //Reads the Cooling Board's Power Consumption
    adc0Value = analogRead(ADCPIN_0);
    cooling_p = ((adc0Value * 3.3) / 4095);
    cooling_power = ( (cooling_p / (0.002*50)) + 0.08 );

    if(buttonValue>1.5)
      increment++;

    if(increment % 2 == 0)
      FAN=20;   
    else
      FAN=30; 

    /*
    if(cooling_power > 20){
      Serial.print("Current = ");
      Serial.print(cooling_power,2);
      Serial.println(" mA");
    } 
    if(cooling_power < 20)
      Serial.println("Current Value Below 20mA");
    */
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
/*
void brakelight(){

    //Power On Values
    if(PotVoltValue >= 1)     
      digitalWrite(1, HIGH); 
    if(PotVoltValue < 1)
      digitalWrite(1, LOW); 

}
*/
//----------------------------------------------------------------

void terminal(){

    adc5Value = analogRead(ADCPIN_5);
    PotVoltValue = ((adc5Value * 3.3) / 4095);

    adc13Value = analogRead(ADCPIN_13);
    InputVoltValue_ADC = ((adc13Value * 3.3) / 4095);
    
    Serial.print("POT = ");
    Serial.print(PotVoltValue, 3);
    Serial.println(" V ");
     
    InputVoltValue = InputVoltValue_ADC * 11.7;

    Serial.print("Volts = ");
    Serial.print(InputVoltValue, 3);
    Serial.println(" V ");

    Serial.print("Increment: ");
    Serial.println(increment);

}

//----------------------------------------------------------------

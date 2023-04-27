/*

Description: Mock PDM (Power Distribution Module) to Place on the Test Board

Author: Francisco Fadigas Mendes

*/

#include <Arduino.h>
#include <ESP32_CAN.h>

#define ADCPIN1_5 A5
#define ADCPIN1_4 A4
#define ADCPIN2_2 B1


//Functions
void can_bus();
void cooling_board(); 
void terminal();

//CAN Bus Setup
TWAI_Interface CAN1(1000,4,5); // BaudRate/1000 , TX , RX

//Global Variables
float voltValue;
int increment = 0;
unsigned long volts_print_time = 0, can_time = 0, cb_time=0;

//----------------------------------------------------------------
//----------------------------------------------------------------

void setup() {

  Serial.begin(115200);

  pinMode(23, OUTPUT); //Cooling Board Powertrain

  }

//----------------------------------------------------------------

void loop() {


    //CAN Bus Periodic Test Message
    if( (millis() - can_time) > 500){  
      can_time = millis();
      can_bus();
    }
    
        //CAN Bus Periodic Test Message
    if( (millis() - cb_time) > 200){  
      cb_time = millis();
      cooling_board(); 
    }

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
  CAN1.TXpackettransmit();

  Serial.println("TEST MESSAGE SENT");

}

//----------------------------------------------------------------

void cooling_board(){

    int buttonValue,adc4Value,adc2Value;
    float cooling_power;

    //Power On Values
    if(voltValue >= 0.5)     
      digitalWrite(23, HIGH); 
    if(voltValue < 0.5)
      digitalWrite(23, LOW); 

    //Signals the Cooling Board to Turn on the Fan
    adc4Value = analogRead(ADCPIN1_4);
    buttonValue = ((adc4Value * 3.3) / 4095);

    adc2Value = analogRead(ADCPIN2_2);
    cooling_power = ((adc4Value * 3.3) / 4095);

    if(buttonValue>1.5){

      increment++;

      if(increment % 2 == 0){
        CAN1.TXpacketBegin(0x20,0);      
        CAN1.TXpacketLoad(20);  
        CAN1.TXpackettransmit();
        Serial.println("TURN ON FAN"); 
      }else{
        CAN1.TXpacketBegin(0x20,1);      
        CAN1.TXpacketLoad(30);  
        CAN1.TXpackettransmit();
        Serial.println("TURN OFF FAN"); 
      }
      
  }

}

//----------------------------------------------------------------

void terminal(){

  int adc5Value;

    adc5Value = analogRead(ADCPIN1_5);
    voltValue = ((adc5Value * 3.3) / 4095);
    
    Serial.print("Volts = ");
    Serial.print(voltValue, 3);
    Serial.println(" V ");
    Serial.print("Increment: ");
    Serial.println(increment);
    
}

//----------------------------------------------------------------

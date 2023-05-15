/*
Description: Mock PDM (Power Distribution Module) to Place on the Test Board
Author: Francisco Fadigas Mendes
*/

#include <Arduino.h>
#include <ESP32_CAN.h>

#define ADCPIN_6 A6   // Potentiometer -> ADC1_CH6 (34)
#define ADCPIN_7 A7   // Button -> ADC1_CH7 (35)
#define ADCPIN_18 A18 // Source Voltage -> ADC2_CH8 (25)
#define ADCPIN_19 A19 // Cooling Board Current Sense -> ADC2_CH9 (26)

// Functions
void can_bus();
void input_status();
void output_status();
void average();
void turn_on_sequence();
void cooling_board();
void brakelight();
void terminal();

// CAN Bus Setup
TWAI_Interface CAN1(1000, 21, 22); // BaudRate/1000 , TX , RX

// Global Variables
int adc6Value, adc7Value, adc18Value, adc19Value, counter_avg; // ADC RAW VALUES
unsigned long volts_print_time = 0, can_time = 0, cb_time = 0, terminal_cycles_counter = 0,terminal_time;// VARIABLES THAT MEASURE TIME

int increment=0, source_avg,FAN;
float PotVoltValue,avgPotVoltValue,AveragePotVoltValue,buttonValue,cooling_power,cooling_p,InputVoltValue,InputVoltValue_ADC,AverageInputVoltValue,avgInputVoltValue;

//Arrays
int output_status_byte1[9];
int output_status_byte2[9];
int output_status_byte3[9];

int input_status_byte1[9];

//----------------------------------------------------------------
//----------------------------------------------------------------

void setup()
{

  Serial.begin(115200);

  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
  pinMode(A18, INPUT);
  pinMode(A19, INPUT);

  pinMode(23, OUTPUT); // Cooling Board
  pinMode(18, OUTPUT); // DataLogger
  pinMode(19, OUTPUT); // Brake Light
}

//----------------------------------------------------------------

void loop()
{

  // 200hz
  if ((millis() - can_time) > 5)
  { 
    can_bus();

    can_time = millis();
  }

  //on a loop
  input_status();
  output_status();
  turn_on_sequence();
  average();

  // 3 times a second
  if ((millis() - terminal_time) > 333)
  { 
    terminal_cycles_counter++;

    Serial.print("----------<= ");
    Serial.print(terminal_cycles_counter);
    Serial.println(" =>---------");

    cooling_board();
    terminal();

    Serial.println("--------------------------------");
    Serial.println("");

    terminal_time = millis();
  }

}

//----------------------------------------------------------------
//----------------------------------------------------------------

void can_bus()
{

  // Source Voltage
  int InputVoltValue_Offset = InputVoltValue * 100;

  //---------------------CAN ID40 MESSAGE BEGIN---------------------
  CAN1.TXpacketBegin(0x40, 0);

  // BYTE 0/1/2 -- OUTPUTS (8*3)
  CAN1.TXpacketLoad(0);
  CAN1.TXpacketLoad(0);
  CAN1.TXpacketLoad(0);

  // BYTE 3 -- INPUTS (8*1)
  CAN1.TXpacketLoad(0);

  // BYTE 4/5 -- LV VOLTAGE
  CAN1.TXpacketLoad(InputVoltValue_Offset);      // LOW BYTE
  CAN1.TXpacketLoad(InputVoltValue_Offset >> 8); // HIGH BYTE

  // BYTE 6/7 -- TOTAL PWR
  CAN1.TXpacketLoad(0);
  CAN1.TXpacketLoad(0);

  CAN1.TXpackettransmit();
  //---------------------CAN ID40 MESSAGE END---------------------

  //---------------------CAN ID41 MESSAGE BEGIN---------------------
  CAN1.TXpacketBegin(0x41, 0);

  // BYTE 0/1 -- COOLING BOARD POWERTRAIN POWER
  CAN1.TXpacketLoad(0);
  CAN1.TXpacketLoad(0);

  // BYTE 2/3 -- COOLING BOARD BATTERY PACK POWER
  CAN1.TXpacketLoad(0);
  CAN1.TXpacketLoad(0);

  // BYTE 4/5 -- VCU POWER
  CAN1.TXpacketLoad(0);
  CAN1.TXpacketLoad(0);

  // BYTE 6/7 -- INVERTER POWER
  CAN1.TXpacketLoad(0);
  CAN1.TXpacketLoad(0);

  CAN1.TXpackettransmit();
  //---------------------CAN ID41 MESSAGE END---------------------

  //---------------------CAN ID42 MESSAGE BEGIN---------------------
  CAN1.TXpacketBegin(0x42, 0);

  // BYTE 0 -- SHUTDOWN CIRCUIT STATE
  CAN1.TXpacketLoad(0);

  // BYTE 1 -- COOLING BOARD POWERTRAIN ERROR
  CAN1.TXpacketLoad(0);

  // BYTE 2 -- COOLING BOARD BATTERY PACK ERROR
  CAN1.TXpacketLoad(0);

  // BYTE 3 -- VCU ERROR
  CAN1.TXpacketLoad(0);

  CAN1.TXpackettransmit();
  //---------------------CAN ID41 MESSAGE END---------------------
}

//----------------------------------------------------------------

void output_status(){

  output_status_byte1[0] = digitalRead(18); //GPIO Datalogger
  output_status_byte1[1] = digitalRead(19); //GPIO Brake Light
  output_status_byte1[2] = digitalRead(23); //GPIO Cooling Board
  output_status_byte1[3] = 0;
  output_status_byte1[4] = 0;
  output_status_byte1[5] = 0;
  output_status_byte1[6] = 0;
  output_status_byte1[7] = 0;

  output_status_byte2[0] = 0;
  output_status_byte2[1] = 0;
  output_status_byte2[2] = 0;
  output_status_byte2[3] = 0;
  output_status_byte2[4] = 0;
  output_status_byte2[5] = 0;
  output_status_byte2[6] = 0;
  output_status_byte2[7] = 0;

  output_status_byte3[0] = 0;
  output_status_byte3[1] = 0;
  output_status_byte3[2] = 0;
  output_status_byte3[3] = 0;
  output_status_byte3[4] = 0;
  output_status_byte3[5] = 0;
  output_status_byte3[6] = 0;
  output_status_byte3[7] = 0;

  int i;
  for(i=0;i++;i<8){
    output_status_byte1[7] += output_status_byte1[i];
    output_status_byte2[7] += output_status_byte2[i];
    output_status_byte3[7] += output_status_byte3[i];
  }

  output_status_byte1[8] = 0;
  output_status_byte2[8] = 0;
  output_status_byte3[8] = 0;

}

void input_status(){

 input_status_byte1[0] = 0;
 input_status_byte1[1] = 0;
 input_status_byte1[2] = 0;
 input_status_byte1[3] = 0;
 input_status_byte1[4] = 0;
 input_status_byte1[5] = 0;
 input_status_byte1[6] = 0;
 input_status_byte1[7] = 0;

  int i;
  for(i=0;i++;i<8){
    output_status_byte1[7] += output_status_byte1[i];
    output_status_byte2[7] += output_status_byte2[i];
    output_status_byte3[7] += output_status_byte3[i];
  }

}

//----------------------------------------------------------------

void average(){

  //Average Pot Value
  counter_avg++;
  avgPotVoltValue += PotVoltValue;
  if(counter_avg == 100){
    counter_avg = 0;
    AveragePotVoltValue =  avgPotVoltValue/100;
    avgPotVoltValue = 0;
  }

  //Average Source Value
  source_avg++;
  avgInputVoltValue += InputVoltValue;
  if(source_avg == 100){
    source_avg = 0;
    AverageInputVoltValue =  avgInputVoltValue/100;
    avgInputVoltValue = 0;
  }
  
}

//----------------------------------------------------------------

void turn_on_sequence()
{
  // Reads Pot Value
  adc6Value = analogRead(ADCPIN_6);
  PotVoltValue = ((adc6Value * 3.3) / 4095);

  // Cooling Board Power On Values
  if (PotVoltValue >= 0.5)
    digitalWrite(23, HIGH);
  if (PotVoltValue < 0.5)
    digitalWrite(23, LOW);

  // DataLogger Power On Values
  if (PotVoltValue >= 1)
    digitalWrite(18, HIGH);
  if (PotVoltValue < 1)
    digitalWrite(18, LOW);

  // Brake Light Power On Values
  if (PotVoltValue >= 1.5)
    digitalWrite(19, HIGH);
  if (PotVoltValue < 1.5)
    digitalWrite(19, LOW);
}

//----------------------------------------------------------------

void cooling_board()
{

  // Signals the Cooling Board to Turn on the Fan
  adc7Value = analogRead(ADCPIN_7);
  buttonValue = ((adc7Value * 3.3) / 4095);

  // Reads the Cooling Board's Power Consumption
  adc19Value = analogRead(ADCPIN_19);
  cooling_p = ((adc19Value * 3.3) / 4095);
  cooling_power = ((cooling_p / (0.002 * 50)) + 0.08);

  // Increments Every the Button is Pressed
  if (buttonValue > 1.5)
    increment++;

  // Even Number-FAN = 20 ; Odd Number-FAN = 30
  if (increment % 2 == 0)
    FAN = 20;
  else
    FAN = 30;
}

//----------------------------------------------------------------

void terminal()
{

  // Reads Source Voltage
  adc18Value = analogRead(ADCPIN_18);
  InputVoltValue_ADC = ((adc18Value * 3.3) / 4095);
  InputVoltValue = InputVoltValue_ADC * 11.6;

  // Prints Source Voltage
  Serial.print("Source Voltage = ");
  Serial.print(AverageInputVoltValue);
  Serial.println(" V ");

  // Prints POT Voltage
  Serial.print("POT = ");
  Serial.print(AveragePotVoltValue, 3);
  Serial.println(" V ");

  // Prints Button Presses
  Serial.print("Button Presses = ");
  Serial.println(increment);
}

//----------------------------------------------------------------

/*
Description: Mock PDM (Power Distribution Module) to Place on the Test Board
Author: Francisco Fadigas Mendes
*/

#include <Arduino.h>
#include <ESP32_CAN.h>

#define ADCPIN_6 A6   // Potentiometer -> ADC1_CH6 (34)
#define ADCPIN_18 A18 // Source Voltage -> ADC2_CH8 (25)
#define ADCPIN_19 A19 // Cooling Board Current Sense -> ADC2_CH9 (26)
#define ADCPIN_16 A16 // Power Module 1 -> ADC2_CH6 (14)
#define ADCPIN_15 A15 // Power Module 1 -> ADC2_CH5 (12)
#define ADCPIN_14 A14 // Power Module 1 -> ADC2_CH4 (13)

// Functions
void can_bus();
void input_status();
void output_status();
void source_voltage_monitoring();
void turn_on_sequence();
void power_monitoring();
void cooling_board();
void brake_light();
void terminal();

// CAN Bus Setup
TWAI_Interface CAN1(1000, 21, 22); // BaudRate/1000 , TX , RX

// Global Variables
int adc6Value, adc7Value, adc18Value,adc19Value,adc16Value,adc15Value,adc14Value; // ADC RAW VALUES
unsigned long volts_print_time = 0, can_time = 0, cb_time = 0, terminal_cycles_counter = 0,terminal_time; // MILLIS VARIABLES 
uint8_t byte_1_out,byte_2_out,byte_3_out,byte_1_in; // MONITORING IN/OUT 

int increment=0, source_avg,BAR,counter_avg;
float PotVoltValue,avgPotVoltValue,AveragePotVoltValue,buttonValue,cooling_power,cooling_p,InputVoltValue,InputVoltValue_ADC,AverageInputVoltValue,avgInputVoltValue;
float m1_power,module_1_power,m2_power,module_2_power,m3_power,module_3_power,TotalPWR;

// Arrays
int output_status_byte1[9];
int output_status_byte2[9];
int output_status_byte3[9];
int input_status_byte1[9];

//----------------------------------------------------------------
//----------------------------------------------------------------

void setup()
{

  Serial.begin(115200);

  pinMode(34, INPUT); // ADC
  pinMode(25, INPUT); // ADC
  pinMode(26, INPUT); // ADC
  pinMode(14, INPUT); // ADC
  pinMode(12, INPUT); // ADC
  pinMode(13, INPUT); // ADC

  pinMode(35, INPUT); // Button

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

  // constant loop
  turn_on_sequence();
  power_monitoring();
  source_voltage_monitoring();
  

  // 3 times a second
  if ((millis() - terminal_time) > 333)
  { 
    terminal_cycles_counter++;

    Serial.print("----------<= ");
    Serial.print(terminal_cycles_counter);
    Serial.println(" =>---------");

    cooling_board();
    brake_light();
    terminal();

    Serial.println("----------------------------");
    Serial.println("");

    terminal_time = millis();
  }

}

//----------------------------------------------------------------
//----------------------------------------------------------------

void can_bus(){

  // Source Voltage OFFSET
  int InputVoltValue_Offset = InputVoltValue * 100;
  int TotalPWR_Offset = TotalPWR * 100;
  int module_1_power_offset = module_1_power;
  int module_2_power_offset = module_2_power;
  int module_3_power_offset = module_3_power;

  //.....................CAN ID20 MESSAGE BEGIN.................
  CAN1.TXpacketBegin(0x20, 0);

    CAN1.TXpacketLoad(0);
    CAN1.TXpacketLoad(BAR); 
    CAN1.TXpacketLoad(0);
    CAN1.TXpacketLoad(0);
    CAN1.TXpacketLoad(0);
    CAN1.TXpacketLoad(0);
    CAN1.TXpacketLoad(0);
    CAN1.TXpacketLoad(0);

  CAN1.TXpackettransmit();
  //.....................CAN ID20 MESSAGE END......................

  //.....................CAN ID40 MESSAGE BEGIN.................
  CAN1.TXpacketBegin(0x40, 0);

  // BYTE 0/1/2 -- OUTPUTS (8*3)
  CAN1.TXpacketLoad(byte_1_out); 
  CAN1.TXpacketLoad(byte_2_out); 
  CAN1.TXpacketLoad(byte_3_out); 

  // BYTE 3 -- INPUTS (8*1)
  CAN1.TXpacketLoad(byte_1_in); 

  // BYTE 4/5 -- LV VOLTAGE
  CAN1.TXpacketLoad(InputVoltValue_Offset);      // LOW BYTE
  CAN1.TXpacketLoad(InputVoltValue_Offset >> 8); // HIGH BYTE

  // BYTE 6/7 -- TOTAL PWR
  CAN1.TXpacketLoad(TotalPWR_Offset);
  CAN1.TXpacketLoad(TotalPWR_Offset >> 8);

  CAN1.TXpackettransmit();
  //.....................CAN ID40 MESSAGE END......................

  //.....................CAN ID41 MESSAGE BEGIN....................
  CAN1.TXpacketBegin(0x41, 0);

  // BYTE 0/1 -- COOLING BOARD POWERTRAIN POWER
  CAN1.TXpacketLoad(module_1_power_offset);
  CAN1.TXpacketLoad(module_1_power_offset >> 8);

  // BYTE 2/3 -- COOLING BOARD BATTERY PACK POWER
  CAN1.TXpacketLoad(module_2_power_offset);
  CAN1.TXpacketLoad(module_2_power_offset >> 8);

  // BYTE 4/5 -- VCU POWER
  CAN1.TXpacketLoad(module_3_power_offset);
  CAN1.TXpacketLoad(module_3_power_offset >> 8);

  // BYTE 6/7 -- INVERTER POWER
  CAN1.TXpacketLoad(0);
  CAN1.TXpacketLoad(0);

  CAN1.TXpackettransmit();
  //......................CAN ID41 MESSAGE END.........................

  //.....................CAN ID42 MESSAGE BEGIN........................
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
  //.....................CAN ID41 MESSAGE END..........................
}

//--------------------------------------------------------------------

void output_status(){

  output_status_byte1[0] = digitalRead(23); //GPIO Cooling Board
  output_status_byte1[1] = digitalRead(18); //GPIO Datalogger
  output_status_byte1[2] = digitalRead(19); //GPIO Brake Light
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


Serial.print("Byte Output 1: ");
for(int i=0;i<8;i++){
  byte_1_out = output_status_byte1[7-i];
  byte_1_out << 1;
  Serial.print(byte_1_out,BIN);
}
  Serial.println("");

Serial.print("Byte Output 2: ");
for(int i=0;i<8;i++){
  byte_2_out = output_status_byte2[7-i];
  byte_2_out << 1;
  Serial.print(byte_2_out,BIN);

}
  Serial.println("");

Serial.print("Byte Output 3: ");
for(int i=0;i<8;i++){
  byte_3_out = output_status_byte3[7-i];
  byte_3_out << 1;
  Serial.print(byte_3_out,BIN);
}
  Serial.println("");

}

//----------------------------------------------------------------

void input_status(){

 input_status_byte1[0] = digitalRead(35); //Button State;
 input_status_byte1[1] = 0;
 input_status_byte1[2] = 0;
 input_status_byte1[3] = 0;
 input_status_byte1[4] = 0;
 input_status_byte1[5] = 0;
 input_status_byte1[6] = 0;
 input_status_byte1[7] = 0;

Serial.print("Byte Input 1: ");
for(int i=0;i<8;i++){
  byte_1_in = input_status_byte1[7-i];
  byte_1_in << 1;
  Serial.print(byte_1_in,BIN);
}
  Serial.println("");
 
}

//----------------------------------------------------------------

void source_voltage_monitoring(){
  
  // Reads Source Voltage
  adc18Value = analogRead(ADCPIN_18);
  InputVoltValue_ADC = ((adc18Value * 3.3) / 4095);
  InputVoltValue = InputVoltValue_ADC * 11.6;

  // Average Source Value
  source_avg++;
  avgInputVoltValue += InputVoltValue;
  if(source_avg == 100){
    source_avg = 0;
    AverageInputVoltValue =  avgInputVoltValue/100;
    avgInputVoltValue = 0;
  }
  
}

//----------------------------------------------------------------

void turn_on_sequence(){

  // Reads Pot Value
  adc6Value = analogRead(ADCPIN_6);
  PotVoltValue = ((adc6Value * 3.3) / 4095);

  // Average Pot Value
  counter_avg++;
  avgPotVoltValue += PotVoltValue;
  if(counter_avg == 100){
    counter_avg = 0;
    AveragePotVoltValue =  avgPotVoltValue/100;
    avgPotVoltValue = 0;
  }

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

void power_monitoring(){

  // Reads Module 1 Power Consumption
  adc16Value = analogRead(ADCPIN_19);
  m1_power = ((adc16Value * 3.3) / 4095);
  module_1_power = ((m1_power / (0.002 * 50)) + 0.08);

  // Reads Module 2 Power Consumption
  adc15Value = analogRead(ADCPIN_19);
  m2_power = ((adc15Value * 3.3) / 4095);
  module_2_power = ((m2_power / (0.002 * 50)) + 0.08);

  // Reads Module 3 Power Consumption
  adc14Value = analogRead(ADCPIN_19);
  m3_power = ((adc14Value * 3.3) / 4095);
  module_3_power = ((m3_power / (0.002 * 50)) + 0.08);

  // Calculates Full Power Consumption
  TotalPWR = module_1_power + module_2_power + module_3_power;


}

//----------------------------------------------------------------

void cooling_board(){

  // Reads the Cooling Board's Power Consumption
  adc19Value = analogRead(ADCPIN_19);
  cooling_p = ((adc19Value * 3.3) / 4095);
  cooling_power = ((cooling_p / (0.002 * 50)) + 0.08);

}

//----------------------------------------------------------------

void brake_light(){

  // Signals the Cooling Board to Turn on the Fan
  buttonValue = digitalRead(35);

  // Increments Every the Button is Pressed
  if (buttonValue)
    increment++;

  // Even Number -> FAN = 6 ; Odd Number -> FAN = 0
  if (increment % 2 == 0)
    BAR = 80;
  else
    BAR = 6;

}

//----------------------------------------------------------------


void terminal(){

  input_status();
  output_status();

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

  // Prints Brake Pressure
  Serial.print("BRAKE PRESSURE =  ");
  Serial.print(BAR);
  Serial.println(" Bar");

// Prints Module 1 Power
 Serial.print("Module 1 Power:");
 Serial.println(module_1_power);

// Prints Module 2 Power
 Serial.print("Module 2 Power:");
 Serial.println(module_2_power);

// Prints Module 3 Power
 Serial.print("Module 3 Power:");
 Serial.println(module_3_power);
  
}

//----------------------------------------------------------------

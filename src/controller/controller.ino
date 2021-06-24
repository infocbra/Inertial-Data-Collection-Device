/*
AUTHOR: Wanghley Soares Martins (@wanghley)
DATE: April 11, 2019
CONTACT: wanghleys@gmail.com
REV1: Fabio Henrique, Lissa Ximenes
DATE: Sep 19, 2019
REV2: Wanghley Soares Martins
DATE: Nov 19, 2020
*/
#include <TimerOne.h>       // Library for timer interrupt
#include <MPU6050.h>        // Standard library for manipulation on the MPU6050
#include <Wire.h>           // Control library of the I²C protocol, used by the MPU6050
#include <SoftwareSerial.h> // Bluetooth control library

#define BTNMODE 9 //Button for mode selection
#define BTNCOLLECTION 12 //Button for start collection
#define POWERLED 7 //LED power indicator
#define COLLECTLED 8 //LED collection happening indicator

// LED for indication of operation mode: Standalone, Serial, MotionSense
#define OPERATIONLED_RED 5 //Standalone
#define OPERATIONLED_BLUE 3 //Serial
#define OPERATIONLED_GREEN 6 //MotionSense

SoftwareSerial BTSerial(10, 11); // RX | TX

const int MPU = 0x68; // MPU6050 address on the I²C bus (available in datasheet)
volatile bool allowSensorReading = false; // responsible for allowing the data reading from teh I2C sensor
bool handshaked = false; // Variable of flow control - handshake
bool collectionStarted = false; // Variable of control of collection procedure
double AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; // Variables used to store data
char a, c='1';
volatile bool flag = false;
bool authSendData;
long Micros;
const int STANDALONE[3] = {255,0,0}; //1
const int SERIALDATA[3] = {0,0,255}; //2
const int MOTIONSENSE[3] = {0,255,0}; //0
int operation = 0;

void setup()
{
    /*Bluetooth comunication: 
      Module - HC-05
      Password - 1234
    */
    Wire.begin();                // I²C communication starts
    Wire.beginTransmission(MPU); // Indicates the address of the MPU to start communication
    Wire.write(0x6B);            // Indicates the MPU that the communication will start
    Wire.write(0);               // Indicates the MPU that wants to leave the communication on standby
    Wire.endTransmission(true);  // Disables communication

    Serial.begin(57600);
    Serial.println("==================================================");
    Serial.println("|                   MOTIONSENSE                  |");
    Serial.println("|       Open Parkinson's Diagnostic Device       |");
    Serial.println("==================================================");
    BTSerial.begin(57600); // HC-05 default speed in AT command more

    // starts the sample rate by time (clock)
    Timer1.initialize(20000); // Adjusted for 50hz = 10⁶ / 50 = 20,000ms
    Timer1.attachInterrupt(changeFlag); // Indicates the function that will be called every 20,000ms

    //start LEDS default
    pinMode(BTNCOLLECTION,INPUT_PULLUP);
    pinMode(BTNMODE,INPUT_PULLUP);
    pinMode(COLLECTLED,OUTPUT);
    digitalWrite(COLLECTLED,LOW);
    pinMode(POWERLED,OUTPUT);
    digitalWrite(POWERLED,HIGH);

    //RGB
    pinMode(OPERATIONLED_RED,OUTPUT);
    pinMode(OPERATIONLED_BLUE,OUTPUT);
    pinMode(OPERATIONLED_GREEN,OUTPUT);

    RGB_color(MOTIONSENSE);
}

void stopProcedure(){
  noInterrupts();
  allowSensorReading = false;
  collectionStarted = false;
  flag = false;
  handshaked = false;
  digitalWrite(COLLECTLED,LOW);
  interrupts();
  
}

//Function that controls the RGB mode indicator LED
void RGB_color(int color[])
 {
  analogWrite(OPERATIONLED_RED, color[0]);
  analogWrite(OPERATIONLED_GREEN, color[1]);
  analogWrite(OPERATIONLED_BLUE, color[2]);
}

//Function that get the micros of the devide and indicates the flag authorization to send data
void changeFlag(){
  if (allowSensorReading){
    flag = true; // active data display
    Micros = micros();
  }    
}

void loop(){
  //COLLECTION BUTTON ON/OFF CONTROLL
  if(digitalRead(BTNCOLLECTION)==LOW){
   if(collectionStarted==true){ //Close the collection procedure by hardware interaction BTN
     Serial.print("stopping... ");
     // TODO write the function to clean buffer and send everything that was collected
     collectionStarted = false;
     BTSerial.write("stop#");
     digitalWrite(COLLECTLED,LOW);
     Serial.println("STOPPED");
   }else{
     Serial.print("starting... ");
     // TODO write the function to clean buffer and start collection from beginning
     collectionStarted=true;
     digitalWrite(COLLECTLED,HIGH);
     Serial.println("STARTED");
   }
   delay(350); //manual delay adjustment when button is pressed
  }
  if(digitalRead(BTNMODE)==LOW){
    if(!collectionStarted){
      if(operation==0){
        operation+=1;
        RGB_color(STANDALONE);
      } else if(operation==1){
        operation+=1;
        RGB_color(SERIALDATA);
      } else {
        operation=0;
        RGB_color(MOTIONSENSE);
      }
    }
    delay(350);
  }
  //==========================================================================
  
  //SOFTWARE CLOSURES COMMANDS - BLE & SERIAL
  if (collectionStarted){ 
    if(BTSerial.available()){ //Close connection by software attempt - through BLE command f
     a = (char)BTSerial.read(); //Wait for the
     if(a=='f'){
      BTSerial.flush();//Descarrega o buffer
      BTSerial.write("stop#");
      stopProcedure();
      Serial.println("FINISHED!\nReady for next...");
     }
    }
    if (Serial.available()){//Close connection by software attempt - through Serial console by cable
      a = Serial.read();
      if (a == 'f'){
        BTSerial.write("stop#");
        BTSerial.flush(); //Descarrega o buffer
        stopProcedure();
        Serial.println("FINISHED!\nReady for next...");
        Serial.println("==================================================");
      }
    }
    //==========================================================================

    noInterrupts();// From here interruptions are not allowed
    authSendData = flag; // authorization for send data - change at each time interruption time - default 50Hz
    flag = false; // set the flag back to original state
    interrupts(); // From here interruptions are allowed
    
    if (authSendData){ // control of data sending - based on the time interruption
      sendData(); //send function called
    }
    //==========================================================================

    //THREE WAY HANDSHAKE
    int val = BTSerial.read(); // Read data comming from the bluetooth
    c='1'; // Variable 
    if (val != -1){
        c = val;
    }
    if (c == 's'){ //First 3-way call - command received
      Serial.println("> Handshake started!\n>> Waiting final handshake");
      BTSerial.write("ok#"); //2nd 3-way call - acknowledge sent
      BTSerial.flush(); //clean buffer
      while (!handshaked){
        val = BTSerial.read();
        if (val != -1){
          c = val;
          if(c=='c'){
            handshaked = true;
            Serial.println("HANDSHAKE SUCCESSED!");
            Serial.println("==================================================");
            allowSensorReading = true;
          }
        }
      }
    }
    //==========================================================================

    //GETTING DATA FROM SENSOR
    if (allowSensorReading){
        Wire.beginTransmission(MPU); //Indicates the resumption of i²c communication
        Wire.write(0x3B);            /*initiate the register of the MPU6050, indicating that it will receive data*/
        Wire.endTransmission(false); //initiate stable communication again
        //Request sensor data
        Wire.requestFrom(MPU, 14, true);
        getData(); // Keep reading from HC-05
    }
    //==========================================================================
  }
}

//Função que recebe os dados do MPU6050 e
//faz as conversões do valor bruto para acc = g e gy = º/seg
void getData(){
  //Armazena o valor dos sensores nas variaveis correspondentes
  AcX = Wire.read() << 8 | Wire.read(); //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); //0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  
  
  //Dados recebidos em 2g
  AcX = AcX / 16384.0;        //Conversão para G
  AcY = AcY / 16384.0;        //Conversão para G
  AcZ = AcZ / 16384.0;        //Conversão para G
  Tmp = Tmp / 340.00 + 36.53; //Conversão para ºC
  GyX = GyX / 250.0;          //Conversão para º/seg
  GyY = GyY / 250.0;          //Conversão para º/seg
  GyZ = GyZ / 250.0;          //Conversão para º/seg
}

void sendData(){
  BTSerial.print((String)Micros+","+(String)AcX+","+(String)AcY+","+(String)AcZ+","+(String)GyX+","+(String)GyY+","+(String)GyZ+","+(String)Tmp+"#");
  BTSerial.flush();
//  Serial.println((String)Micros+","+(String)AcX+","+(String)AcY+","+(String)AcZ+","+(String)GyX+","+(String)GyY+","+(String)GyZ+","+(String)Tmp+"#");
}

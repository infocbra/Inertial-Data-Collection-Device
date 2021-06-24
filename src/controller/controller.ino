/*
AUTHOR: Wanghley Soares Martins (@wanghley)
DATE: April 11, 2019
CONTACT: wanghleys@gmail.com
REV1: Fabio Henrique, Lissa Ximenes
DATE: Sep 19, 2019
REV2: Wanghley Martins
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

// LED for indication of operation mode: Standalone, MotionSense, simple 3-way-handshake
#define OPERATIONLED_RED 5 //Standalone
#define OPERATIONLED_BLUE 3 //Simple 3way handshake
#define OPERATIONLED_GREEN 6 //MotionSense

SoftwareSerial BTSerial(10, 11); // RX | TX

const int MPU = 0x68; // MPU6050 address on the I²C bus (available in datasheet)
volatile bool visualization = false; // Variable is loaded into the "RAM memory" of the microcontroller
boolean controle = true; // Variable of flow control
bool collectionStarted = false;
int ledstate = 0; // LED state indicator
double AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; // Variables used to store data
char a, c='-1', c1='-1';
unsigned long current;
unsigned long last;
volatile bool flag = false;
bool authSendData;
//volatile unsigned long interval = 0;
//unsigned long intervalCopy;
long Micros;

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
    Serial.println("MotionSense - Open Parkinson's Diagnostic Device");
    BTSerial.begin(57600); // HC-05 default speed in AT command more

    // starts the sample rate by time (clock)
    Timer1.initialize(20000); // Adjusted for 50hz = 10⁶ / 50 = 20,000µs
    Timer1.attachInterrupt(changeFlag); // Indicates the function that will be called every 20,000ms

    //start LEDS default
    pinMode(BTNCOLLECTION,INPUT_PULLUP);
    pinMode(COLLECTLED,OUTPUT);
    digitalWrite(COLLECTLED,LOW);
    pinMode(POWERLED,OUTPUT);
    digitalWrite(POWERLED,HIGH);

    //RGB
    pinMode(OPERATIONLED_RED,OUTPUT);
    pinMode(OPERATIONLED_BLUE,OUTPUT);
    pinMode(OPERATIONLED_GREEN,OUTPUT);

    RGB_color(0,255,0);
}

//Function that controls the RGB mode indicator LED
void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(OPERATIONLED_RED, red_light_value);
  analogWrite(OPERATIONLED_GREEN, green_light_value);
  analogWrite(OPERATIONLED_BLUE, blue_light_value);
}

//Function that get the micros of the devide and indicates the flag authorization to send data
void changeFlag(){
  if (visualization){
    flag = true; // active data display
    Micros = micros();
  }    
}

void loop(){
  if(digitalRead(BTNCOLLECTION)==LOW){
   if(collectionStarted==true){ //Close the collection procedure by hardware interaction BTN
     Serial.println("stopping...");
     // TODO write the function to clean buffer and send everything that was collected
     collectionStarted = false;
     BTSerial.write("stop#");
     digitalWrite(COLLECTLED,LOW);
     Serial.println("STOPPED");
   }else{
     Serial.println("starting...");
     // TODO write the function to clean buffer and start collection from beginning
     collectionStarted=true;
     digitalWrite(COLLECTLED,HIGH);
     Serial.println("STARTED");
   }
   delay(350);
  }
    
  if (collectionStarted){ 
    if(BTSerial.available()){ //Close connection by software attempt - through BLE command f
     a = (char)BTSerial.read(); //Wait for the
     if(a=='f'){
      BTSerial.flush();//Descarrega o buffer
      BTSerial.write("stop#");
      Serial.println("Comunication Closed!");
      visualization = false;
     }
    }
    if (Serial.available()){//Close connection by hardware attempt
      a = Serial.read();
      if (a == 'f'){
        BTSerial.write("stop#");
        BTSerial.flush(); //Descarrega o buffer
        Serial.println("Canal fechado..");
        visualization = false;
      }
    }

    noInterrupts();// From here interruptions are not allowed
    authSendData = flag; // authorization for send data - change at each time interruption time - default 50Hz
    flag = false; // set the flag back to original state
//    intervalCopy = interval;
//    interval = 0;
    interrupts(); // From here interruptions are allowed
    
    if (authSendData){
      printData();
    }
    int val = BTSerial.read();
    
    c='-1';
    c1='-1';
    if (val != -1){
        c = val;
    }
    
    if (c == 's'){
      Serial.println("Comunication started!\nWaiting final handshake");
      BTSerial.write("ok#");
      BTSerial.flush(); //Limpar buffer
      while (controle){
        val = BTSerial.read();
        if (val != -1){
          c1 = val;
          if(c1=='c'){
            controle = false;
            Serial.println("Data collection started!");
            visualization = true;
          }
        }
      }
    }
    
    if (visualization){
        Wire.beginTransmission(MPU); //Indica a retomada na comunicação i²c
        Wire.write(0x3B);            /*initiate o registrador do MPU6050, indicando que irá receber dados*/
        Wire.endTransmission(false); //initiate a comunicação estável novamente
        //Solicita dados do sensor
        Wire.requestFrom(MPU, 14, true);
        getData(); // Keep reading from HC-05
    }
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

void printData(){
  BTSerial.print((String)Micros+","+(String)AcX+","+(String)AcY+","+(String)AcZ+","+(String)GyX+","+(String)GyY+","+(String)GyZ+","+(String)Tmp+"#");
  BTSerial.flush();
//  Serial.println((String)Micros+","+(String)AcX+","+(String)AcY+","+(String)AcZ+","+(String)GyX+","+(String)GyY+","+(String)GyZ+","+(String)Tmp+"#");
}

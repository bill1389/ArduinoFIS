#include <avr/eeprom.h>
#include "SoftwareSerial.h"
#include "SPI.h"
#include "EncButton.h"

#define pinButton 2
#define pinKLineTX 3
#define pinKLineRX 4

//WRITE TO CLUSTER
#define FIS_WRITE_ENA 5 
#define FIS_WRITE_CLK 6 
#define FIS_WRITE_DATA 7 
#define FIS_WRITE_PULSEW 50
#define FIS_WRITE_STARTPULSEW 100
#define FIS_WRITE_START 15 //something like address, first byte is always 15
//END WRITE TO CLUSTER

//WRITE TO CLUSTER
String FIS_WRITE_line1="AUDI A6 C5";
String FIS_WRITE_line2="BILL1389";
long FIS_WRITE_rotary_position_line1=-8;
long FIS_WRITE_rotary_position_line2=-8;
char FIS_WRITE_CHAR_FROM_SERIAL;
int FIS_WRITE_line=1;
long FIS_WRITE_last_refresh=0;
int FIS_WRITE_nl=0;
int FIS_WRITE_ENA_STATUS=0;
uint8_t FIS_WRITE_CRC=0;
//END WRITE TO CLUSTER

//WRITE TO CLUSTER
void FIS_WRITE_sendTEXT(String FIS_WRITE_line1,String FIS_WRITE_line2);
void FIS_WRITE_sendByte(int Bit);
void FIS_WRITE_startENA();
void FIS_WRITE_stopENA();
//END WRITE TO CLUSTER

EncButton<EB_TICK, pinButton> btnUp(INPUT);


#define ADR_Engine 0x01
#define ADR_Gears  0x02
#define ADR_ABS_Brakes 0x03
#define ADR_Airbag 0x15
#define ADR_Dashboard 0x17
#define ADR_Immobilizer 0x25
#define ADR_Central_locking 0x35

int ADR_Engine_Speed = 10000;
int ADR_Dashboard_Speed  = 10400;

SoftwareSerial obd(pinKLineRX, pinKLineTX, false); // RX, TX, inverse logic


uint8_t currAddr = 0;
uint8_t blockCounter = 0;
uint8_t errorTimeout = 0;
uint8_t errorData = 0;
bool connected = false;
int sensorCounter = 0;
int pageUpdateCounter = 0;
int alarmCounter = 0;

uint8_t currPage = 1;
uint8_t currPageOld;


String readString;

int param0;
int param1;
int param2;
int param3;

String turboZapros;
String turboPress;
String turboPressMax;

int8_t ambientTemperature = 0;
int8_t coolantTemp = 0;
int8_t oilTemp = 0;
int8_t intakeAirTemp = 0;
int8_t oilPressure = 0;
float engineLoad = 0;
int   engineSpeed = 0;
float throttleValve = 0;
float supplyVoltage = 0;
uint8_t vehicleSpeed = 0;
uint8_t fuelConsumption = 0;
uint8_t fuelLevel = 0;
unsigned long odometer = 0;

float injektTime = 0;
float MAF = 0;
 float Lhour = 0;
 float LhourAVGtmp = 0;
 float LhourAVG = 0;
 float L100Current = 0;
 float L100;

 float L100tmp;
 int ix = 1;

  int iy = 1; 
 float L100Move = 0;
 float L100Movetmp = 0;

 float vehicleSpeedAVGtmp;
 float vehicleSpeedAVG;
 float L100AVG;
//  float L100tmp =0;
//  int ix = 1;
bool SaveL100Flag = false;


//WRITE TO CLUSTER 
void FIS_WRITE_sendTEXT(String FIS_WRITE_line1,String FIS_WRITE_line2) {
  Serial.println(FIS_WRITE_line1);
  Serial.println(FIS_WRITE_line2);
  int FIS_WRITE_line1_length=FIS_WRITE_line1.length();
  int FIS_WRITE_line2_length=FIS_WRITE_line2.length();
    if (FIS_WRITE_line1_length<=8){
        for (int i=0;i<(8-FIS_WRITE_line1_length);i++){
          FIS_WRITE_line1+=" ";
        } 
    }
    if (FIS_WRITE_line2_length<=8){
      for (int i=0;i<(8-FIS_WRITE_line2_length);i++){
        FIS_WRITE_line2+=" ";
      }
    }

FIS_WRITE_CRC=(0xFF^FIS_WRITE_START);

FIS_WRITE_startENA();
FIS_WRITE_sendByte(FIS_WRITE_START);
  for (int i = 0; i <= 7; i++)
  { 
    FIS_WRITE_sendByte(0xFF^FIS_WRITE_line1[i]);
    FIS_WRITE_CRC+=FIS_WRITE_line1[i];
  }
    for (int i = 0; i <= 7; i++)
  { 
    FIS_WRITE_sendByte(0xFF^FIS_WRITE_line2[i]);
    FIS_WRITE_CRC+=FIS_WRITE_line2[i];
  }
  
FIS_WRITE_sendByte(FIS_WRITE_CRC%0x100);

FIS_WRITE_stopENA();
}

void FIS_WRITE_sendByte(int Byte){
  static int iResult[8];
  for (int i = 0; i <= 7; i++)
  {    
    iResult[i] = Byte % 2;
    Byte = Byte / 2;
  }
  for(int i=7;i>=0;i--){
  switch (iResult[i]) {
    case 1: digitalWrite(FIS_WRITE_DATA,HIGH);
            break;
    case 0:digitalWrite(FIS_WRITE_DATA,LOW);
           break;
    }
    digitalWrite(FIS_WRITE_CLK,LOW);
    delayMicroseconds(FIS_WRITE_PULSEW);
    digitalWrite(FIS_WRITE_CLK,HIGH);
    delayMicroseconds(FIS_WRITE_PULSEW);
}
}

void FIS_WRITE_startENA(){
 if (!digitalRead(FIS_WRITE_ENA)) {
  digitalWrite(FIS_WRITE_ENA,HIGH);
//  delayMicroseconds(FIS_WRITE_STARTPULSEW);
//  digitalWrite(FIS_WRITE_ENA,LOW);
//  delayMicroseconds(FIS_WRITE_STARTPULSEW);
//  digitalWrite(FIS_WRITE_ENA,HIGH);
//  delayMicroseconds(FIS_WRITE_STARTPULSEW);
 // FIS_WRITE_ENA_STATUS=1;
  }
}

void FIS_WRITE_stopENA(){
 digitalWrite(FIS_WRITE_ENA,LOW);
// FIS_WRITE_ENA_STATUS=0;
}
//END WRITE TO CLUSTER




String floatToString(float v){
  String res; 
  char buf[16];      
  dtostrf(v,4, 2, buf); 
  res=String(buf);
  return res;
}

void disconnect(){
  connected = false;
  currAddr = 0;
}

void obdWrite(uint8_t data){
#ifdef DEBUG
//  Serial.print("uC:");
//  Serial.println(data, HEX);
#endif
  obd.write(data);
}

uint8_t obdRead(){
  unsigned long timeout = millis() + 1000;  
  while (!obd.available()){
    if (millis() >= timeout) {
 //     Serial.println(F("ERROR: obdRead timeout"));
      disconnect();      
      errorTimeout++;
      return 0;
    }
  }
  uint8_t data = obd.read();
#ifdef DEBUG  
//  Serial.print("ECU:");
//  Serial.println(data, HEX);
#endif  
  return data;
}



// 5Bd, 7O1
void send5baud(uint8_t data){
  // // 1 start bit, 7 data bits, 1 parity, 1 stop bit
  #define bitcount 10
  byte bits[bitcount];
  byte even=1;
  byte bit;
  for (int i=0; i < bitcount; i++){
    bit=0;
    if (i == 0)  bit = 0;
      else if (i == 8) bit = even; // computes parity bit
      else if (i == 9) bit = 1;
      else {
        bit = (byte) ((data & (1 << (i-1))) != 0);
        even = even ^ bit;
      }
 //   Serial.print(F("bit"));      
 //   Serial.print(i);          
 //   Serial.print(F("="));              
 //   Serial.print(bit);
  //  if (i == 0) Serial.print(F(" startbit"));
 //     else if (i == 8) Serial.print(F(" parity"));    
  //    else if (i == 9) Serial.print(F(" stopbit"));              
 //   Serial.println();      
    bits[i]=bit;
  }
  // now send bit stream    
  for (int i=0; i < bitcount+1; i++){
    if (i != 0){
      // wait 200 ms (=5 baud), adjusted by latency correction
      delay(200);
      if (i == bitcount) break;
    }
    if (bits[i] == 1){ 
      // high
      digitalWrite(pinKLineTX, HIGH);
    } else {
      // low
      digitalWrite(pinKLineTX, LOW);
    }
  }
  obd.flush();
}


bool KWP5BaudInit(uint8_t addr){
  Serial.println(F("---KWP 5 baud init"));
  //delay(3000);
 // digitalWrite(pinKLineTX, LOW);
  //delay(2400);
  send5baud(addr);
  return true;
}


bool KWPSendBlock(char *s, int size){
  Serial.print(F("---KWPSend sz="));
  Serial.print(size);
  Serial.print(F(" blockCounter="));
  Serial.println(blockCounter);    
  // show data
  Serial.print(F("OUT:"));
  for (int i=0; i < size; i++){    
    uint8_t data = s[i];
    Serial.print(data, HEX);
    Serial.print(" ");    
  }  
  Serial.println();
  for (int i=0; i < size; i++){
    uint8_t data = s[i];    
    obdWrite(data);
    /*uint8_t echo = obdRead();  
    if (data != echo){
      Serial.println(F("ERROR: invalid echo"));
      disconnect();
      errorData++;
      return false;
    }*/
    if (i < size-1){
      uint8_t complement = obdRead();        
      if (complement != (data ^ 0xFF)){
        Serial.println(F("ERROR: invalid complement"));
        disconnect();
        errorData++;
        return false;
      }
    }
  }
  blockCounter++;
  return true;
}

// count: if zero given, first received byte contains block length
// 4800, 9600 oder 10400 Baud, 8N1
bool KWPReceiveBlock(char s[], int maxsize, int &size){  
  bool ackeachbyte = false;
  uint8_t data = 0;
  int recvcount = 0;
  if (size == 0) ackeachbyte = true;
  Serial.print(F("---KWPReceive sz="));
  Serial.print(size);
  Serial.print(F(" blockCounter="));
  Serial.println(blockCounter);
  if (size > maxsize) {
    Serial.println("ERROR: invalid maxsize");
    return false;
  }  
  unsigned long timeout = millis() + 2000;  
  while ((recvcount == 0) || (recvcount != size)) {
    while (obd.available()){      
      data = obdRead();
      delay(5); 

           s[recvcount] = data;    
      recvcount++; 
          
          
      if ((size == 0) && (recvcount == 1)) {
        size = data + 1;
        if (size > maxsize) {
          Serial.println("ERROR: invalid maxsize");
          return false;
        }  
      }
      if ((ackeachbyte) && (recvcount == 2)) {
        if (data != blockCounter){
          Serial.println(F("ERROR: invalid blockCounter"));
          disconnect();
          errorData++;
          return false;
        }
      }
      if ( ((!ackeachbyte) && (recvcount == size)) ||  ((ackeachbyte) && (recvcount < size)) ){
        obdWrite(data ^ 0xFF);  // send complement ack        
        /*uint8_t echo = obdRead();        
        if (echo != (data ^ 0xFF)){
          Serial.print(F("ERROR: invalid echo "));
          Serial.println(echo, HEX);
          disconnect();
          errorData++;
          return false;
        }*/
      }
      timeout = millis() + 2000;        
    } 
    if (millis() >= timeout){
      Serial.println(F("ERROR: timeout"));
      disconnect();
      errorTimeout++;
      return false;
    }
  }
  // show data
  Serial.print(F("IN: sz="));  
  Serial.print(size);  
  Serial.print(F(" data="));  
  for (int i=0; i < size; i++){
    uint8_t data = s[i];
    Serial.print(data, HEX);
    Serial.print(F(" "));    
  }  
  Serial.println();
  blockCounter++;
  return true;
}

bool KWPSendAckBlock(){
  Serial.print(F("---KWPSendAckBlock blockCounter="));
  Serial.println(blockCounter);  
  char buf[32];  
  sprintf(buf, "\x03%c\x09\x03", blockCounter);  
  return (KWPSendBlock(buf, 4));
}
 
bool readConnectBlocks(){  
  // read connect blocks
  Serial.println(F("------readconnectblocks"));
//  lcdPrint(0,0, F("KW1281 label"), 20);
  String info;  
  while (true){
    int size = 0;
    char s[64];
    if (!(KWPReceiveBlock(s, 64, size))) return false;
    if (size == 0) return false;
    if (s[2] == '\x09') break; 
    if (s[2] != '\xF6') {
      Serial.println(F("ERROR: unexpected answer"));
      disconnect();
      errorData++;
      return false;
    }
    String text = String(s);
    info += text.substring(3, size-2);
    if (!KWPSendAckBlock()) return false;
  }
  Serial.print("label=");
  Serial.println(info);
  //lcd.setCursor(0, 1);
  //lcd.print(info);      
  return true;
}

bool connect(uint8_t addr, int baudrate){  
  Serial.print(F("------connect addr="));
  Serial.print(addr);
  Serial.print(F(" baud="));  
  Serial.println(baudrate);  
 
  blockCounter = 0;  
  currAddr = 0;
  obd.begin(baudrate);       
  KWP5BaudInit(addr);
  // answer: 0x55, 0x01, 0x8A          
  char s[3];
 
  
  if(addr == ADR_Engine){
     int size = 5;
    if (!KWPReceiveBlock(s, 5, size)) return false;
      if (    (((uint8_t)s[0]) != 0x00)
     ||   (((uint8_t)s[1]) != 0x00)
     ||   (((uint8_t)s[2]) != 0x55)
     ||   (((uint8_t)s[3]) != 0x01) 
     ||   (((uint8_t)s[4]) != 0x8A)){
    Serial.println(F("ERROR: invalid magic"));
    disconnect();
    errorData++;
    return false;
     }
   }
     if(addr == ADR_Dashboard){
       int size = 6;
      if (!KWPReceiveBlock(s, 6, size)) return false;
      if ((((uint8_t)s[0]) != 0x00)
     ||   (((uint8_t)s[1]) != 0x00)
     ||   (((uint8_t)s[2]) != 0x00)
     ||   (((uint8_t)s[3]) != 0x55)
     ||   (((uint8_t)s[4]) != 0x01) 
     ||   (((uint8_t)s[5]) != 0x8A)){
    Serial.println(F("ERROR: invalid magic"));
    disconnect();
    errorData++;
    return false;
     }
   }

  currAddr = addr;
  connected = true;  
  if (!readConnectBlocks()) return false;
  return true;
}
 
bool readSensors(int group){
  Serial.print(F("------readSensors "));
  Serial.println(group);
//  lcdPrint(0,0, F("KW1281 sensor"), 20);  
  char s[64];
  sprintf(s, "\x04%c\x29%c\x03", blockCounter, group);
  if (!KWPSendBlock(s, 5)) return false;
  int size = 0;
  KWPReceiveBlock(s, 64, size);
  if (s[2] != '\xe7') {
    Serial.println(F("ERROR: invalid answer"));
    disconnect();
    errorData++;
    return false;
  }
  int count = (size-4) / 3;
  Serial.print(F("count="));
  Serial.println(count);
  for (int idx=0; idx < count; idx++){
    byte k=s[3 + idx*3];
    byte a=s[3 + idx*3+1];
    byte b=s[3 + idx*3+2];
    String n;
    float v = 0;
    Serial.print(F("type="));
    Serial.print(k);
    Serial.print(F("  a="));
    Serial.print(a);
    Serial.print(F("  b="));
    Serial.print(b);
    Serial.print(F("  text="));
    String t = "";
    String units = "";
    char buf[32];    
    switch (k){
      case 1:  v=0.2*a*b;             units=F("rpm"); break;
      case 2:  v=a*0.002*b;           units=F("%%"); break;
      case 3:  v=0.002*a*b;           units=F("Deg"); break;
      case 4:  v=abs(b-127)*0.01*a;   units=F("ATDC"); break;
      case 5:  v=a*(b-100)*0.1;       units=F("°C");break;
      case 6:  v=0.001*a*b;           units=F("V");break;
      case 7:  v=0.01*a*b;            units=F("km/h");break;
      case 8:  v=0.1*a*b;             units=F(" ");break;
      case 9:  v=(b-127)*0.02*a;      units=F("Deg");break;
      case 10: if (b == 0) t=F("COLD"); else t=F("WARM");break;
      case 11: v=0.0001*a*(b-128)+1;  units = F(" ");break;
      case 12: v=0.001*a*b;           units =F("Ohm");break;
      case 13: v=(b-127)*0.001*a;     units =F("mm");break;
      case 14: v=0.005*a*b;           units=F("bar");break;
      case 15: v=0.01*a*b;            units=F("ms");break;
      case 18: v=0.04*a*b/1000;            units=F("mbar");break;
      case 19: v=a*b*0.01;            units=F("l");break;
      case 20: v=a*(b-128)/128;       units=F("%%");break;
      case 21: v=0.001*a*b;           units=F("V");break;
      case 22: v=0.001*a*b;           units=F("ms");break;
      case 23: v=b/256*a;             units=F("%%");break;
      case 24: v=0.001*a*b;           units=F("A");break;
      case 25: v=(b*1.421)+(a/182);   units=F("g/s");break;
      case 26: v=float(b-a);          units=F("C");break;
      case 27: v=abs(b-128)*0.01*a;   units=F("°");break;
      case 28: v=float(b-a);          units=F(" ");break;
      case 30: v=b/12*a;              units=F("Deg k/w");break;
      case 31: v=b/2560*a;            units=F("°C");break;
      case 33: v=100*b/a;             units=F("%%");break;
      case 34: v=(b-128)*0.01*a;      units=F("kW");break;
      case 35: v=0.01*a*b;            units=F("l/h");break;
      case 36: v=((unsigned long)a)*2560+((unsigned long)b)*10;  units=F("km");break;
      case 37: v=b; break; // oil pressure ?!
      // ADP: FIXME!
      /*case 37: switch(b){
             case 0: sprintf(buf, F("ADP OK (%d,%d)"), a,b); t=String(buf); break;
             case 1: sprintf(buf, F("ADP RUN (%d,%d)"), a,b); t=String(buf); break;
             case 0x10: sprintf(buf, F("ADP ERR (%d,%d)"), a,b); t=String(buf); break;
             default: sprintf(buf, F("ADP (%d,%d)"), a,b); t=String(buf); break;
          }*/
      case 38: v=(b-128)*0.001*a;        units=F("Deg k/w"); break;
      case 39: v=b/256*a;                units=F("mg/h"); break;
      case 40: v=b*0.1+(25.5*a)-400;     units=F("A"); break;
      case 41: v=b+a*255;                units=F("Ah"); break;
      case 42: v=b*0.1+(25.5*a)-400;     units=F("Kw"); break;
      case 43: v=b*0.1+(25.5*a);         units=F("V"); break;
      case 44: sprintf(buf, "%2d:%2d", a,b); t=String(buf); break;
      case 45: v=0.1*a*b/100;            units=F(" "); break;
      case 46: v=(a*b-3200)*0.0027;      units=F("Deg k/w"); break;
      case 47: v=(b-128)*a;              units=F("ms"); break;
      case 48: v=b+a*255;                units=F(" "); break;
      case 49: v=(b/4)*a*0.1;            units=F("mg/h"); break;
      case 50: v=(b-128)/(0.01*a);       units=F("mbar"); break;
      case 51: v=((b-128)/255)*a;        units=F("mg/h"); break;
      case 52: v=b*0.02*a-a;             units=F("Nm"); break;
      case 53: v=(b-128)*1.4222+0.006*a;  units=F("g/s"); break;
      case 54: v=a*256+b;                units=F("count"); break;
      case 55: v=a*b/200;                units=F("s"); break;
      case 56: v=a*256+b;                units=F("WSC"); break;
      case 57: v=a*256+b+65536;          units=F("WSC"); break;
      case 59: v=(a*256+b)/32768;        units=F("g/s"); break;
      case 60: v=(a*256+b)*0.01;         units=F("sec"); break;
      case 62: v=0.256*a*b;              units=F("S"); break;
      case 64: v=float(a+b);             units=F("Ohm"); break;
      case 65: v=0.01*a*(b-127);         units=F("mm"); break;
      case 66: v=(a*b)/511.12;          units=F("V"); break;
      case 67: v=(640*a)+b*2.5;         units=F("Deg"); break;
      case 68: v=(256*a+b)/7.365;       units=F("deg/s");break;
      case 69: v=(256*a +b)*0.3254;     units=F("Bar");break;
      case 70: v=(256*a +b)*0.192;      units=F("m/s^2");break;
      default: sprintf(buf, "%2x, %2x      ", a, b); break;
    }
      if (units.length() != 0){
      dtostrf(v,4, 2, buf); 
      t=String(buf);
    } 

    switch (currAddr){
      case ADR_Engine: 
        switch(group){
          case 4: 
            switch (idx){
              case 0: engineSpeed = v; break;
              case 1: supplyVoltage=v; break;
              case 2: coolantTemp =v; break;
              case 3: intakeAirTemp=v; break;
            }              
            break;
          case 11: 
            switch (idx){
              case 1: engineLoad=v; break;
              case 2: vehicleSpeed =v; break;
              case 3: fuelConsumption=v; break;
            }              
            break;
          case 2: 
            switch (idx){
              case 0: engineSpeed = v; break;
              case 2: injektTime =v; break;
              case 3: MAF=v; break;
            }              
            break;
          case 5: 
            switch (idx){
              case 0: engineSpeed = v; break;
              case 2: vehicleSpeed =v; break;
            }              
            break;
          case 115:
            switch (idx){
              case 0: engineSpeed = v; break;
              case 1: engineLoad=v; break;
              case 2: turboZapros =v; break;
              case 3: turboPress=v-1; break;

            }
            break;
          default:  
            switch (idx){
              case 0: param0 = v; break;
              case 1: param1 = v; break;
              case 2: param2 = v; break;
              case 3: param3 = v; break;    
            }
          break;

        }
        break;
      case ADR_Dashboard: 
        switch (group){ 
          case 1:  
            switch (idx){
              case 0: vehicleSpeed = v; break;
              case 1: engineSpeed = v; break;
              case 2: oilPressure = v; break;
            }
            break;
          case 2:
            switch (idx){
              case 0: odometer = v; break;
              case 1: fuelLevel = v; break; 
              case 3: ambientTemperature = v; break;          
            }
            break;
          case 50:
            switch (idx){
             // case 1: engineSpeed = v; break;
              case 2: oilTemp = v; break;
              case 3: coolantTemp = v; break;
            }
            break;
         
        }
        break;
    }
    

     if (units.length() != 0){
      dtostrf(v,4, 2, buf); 
      t=String(buf) + " " + units;
    }     
    Serial.println(t);
    
    
  }
  sensorCounter++;
  return true;
}

void alarm()
{
  if (alarmCounter > 10)
    return;
  // tone(pinBuzzer, 1200);
  // delay(100);
  // noTone(pinBuzzer);
  alarmCounter++;
}

void updateDisplay()
{
  if (!connected)
  {
    if ((errorTimeout != 0) || (errorData != 0))
    {
    FIS_WRITE_line1="AUDI";
    FIS_WRITE_line2="QUATTRO";
    }
  }
  else
  {
    switch (currPage)
    {
    case 1:
      FIS_WRITE_line1="AUDI";
      FIS_WRITE_line2="QUATTRO";

      // FIS_WRITE_line1="FUEL:";
      // FIS_WRITE_line2=String(fuelLevel);
    break;

    case 2:
      FIS_WRITE_line1="COOL:"+ String(coolantTemp);
      FIS_WRITE_line2="AIR:"+ String(intakeAirTemp);
    break;

    case 3:
      FIS_WRITE_line1="BOOST:";
      FIS_WRITE_line2= String(turboPress);

    break;
    
    case 4:
      FIS_WRITE_line1="INJ:"+ String(injektTime);
      FIS_WRITE_line2="MAF:"+ String(MAF);
    break;
    case 5:
      FIS_WRITE_line1="RPM:"+ String(engineSpeed);
      FIS_WRITE_line2="SPD:"+ String(vehicleSpeed);
    break;

    }
  }

  pageUpdateCounter++;
}

void btnInterrupt(){
  btnUp.tickISR();
  //  if (btnUp.click()){    
  //   currPage++;
  //   if (currPage > 6) currPage = 1;
  //   eeprom_update_byte(0, currPage);       
  // }

  // if (btnUp.held()){
  //   currPage = 1;
  //   eeprom_update_byte(0, currPage);
  //       }
        //Serial.println("click");
}
void setup()
{
  currPage = eeprom_read_byte(0);
  pinMode(pinKLineTX, OUTPUT);
  digitalWrite(pinKLineTX, HIGH);
  pinMode(pinButton, INPUT_PULLUP);

  attachInterrupt(0, btnInterrupt, FALLING);
  


  //WRITE TO CLUSTER
  pinMode(FIS_WRITE_ENA, OUTPUT);
  digitalWrite(FIS_WRITE_ENA,LOW);
  pinMode(FIS_WRITE_CLK, OUTPUT); 
  digitalWrite(FIS_WRITE_CLK, HIGH);
  pinMode(FIS_WRITE_DATA, OUTPUT); 
  digitalWrite(FIS_WRITE_DATA, HIGH);
  //END WRITE TO CLUSTER

 // pinMode(pinBuzzer, OUTPUT);
  /*tone(pinBuzzer, 1200);    
  delay(100);
  noTone(pinBuzzer);*/

  Serial.begin(9600);
  Serial.println(F("SETUP"));
  Serial.println(F("START"));
}

void loop()
{
  //eeprom_write_float(1, 1);
  btnUp.tick(); 
        // eeprom_write_dword(1,1);
        // eeprom_write_float(5,1);
  if (btnUp.hasClicks(1)){    
    currPage++;
    if (currPage > 9) currPage = 7;
    eeprom_update_byte(0, currPage);       
  }

  if (btnUp.hasClicks(2)){    
    currPage--;
    if (currPage < 7) currPage = 9;
    eeprom_update_byte(0, currPage);       
  }

  if (btnUp.held()){

        eeprom_write_dword(1,1);
        eeprom_write_float(5,0);

           LhourAVGtmp = 0;
           L100tmp = 0;
           ix = 1;
           iy = 1;
           vehicleSpeedAVGtmp=0;
           L100Movetmp = 0;

    
    //currPage = 1;
    //eeprom_update_byte(0, currPage);
        }

//  if (digitalRead(pinButton) == LOW){    
//     currPage++;
//     if (currPage > 5) currPage = 1;
//     eeprom_update_byte(0, currPage);
//     errorTimeout = 0;
//     errorData = 0;            
//     while (digitalRead(pinButton) == LOW);        
//   }

//рассчет расхода топлива
  if (currAddr != ADR_Engine)
    {
      if(SaveL100Flag){
        eeprom_update_dword(1, ix);
        eeprom_update_float(5,L100tmp);
        SaveL100Flag = false;
      }
      connect(ADR_Engine, ADR_Engine_Speed);
    }
    else
    {
      readSensors(2);
      readSensors(5);

      if(engineSpeed>0){
        Lhour = (float)engineSpeed * (float)injektTime * float(0.0004478);
       if (vehicleSpeed == 0){
        L100Current = (float)Lhour * (float)100.0 / 2.0; //пусть при стоящем авто скорость будет 1 км/ч
       } else {
        L100Current = (float)Lhour * (float)100.0 / (float)vehicleSpeed;
        L100Movetmp = L100Movetmp + L100Current;
        L100Move = L100Movetmp/iy;
        iy++;
       }

        L100tmp = L100tmp+L100Current;
        L100 = L100tmp / ix;

// посчитать л.ч среднее и среднюю скорость, а потом вычислить средний расход
        LhourAVGtmp = LhourAVGtmp+Lhour;
        LhourAVG = LhourAVGtmp/ix;
        vehicleSpeedAVGtmp = vehicleSpeedAVGtmp+vehicleSpeed;
        vehicleSpeedAVG = vehicleSpeedAVGtmp / ix;
        L100AVG= (float)LhourAVG * (float)108.0 / (float)vehicleSpeedAVG;

        ix++;
      }
      


        SaveL100Flag = true;
        // if(ix%10){

        // } else {
        // eeprom_update_dword(1, ix);
        // eeprom_update_float(5,L100tmp);
        // }


    }

  Serial.println(ix);
   Serial.println(iy);
  
  switch (currPage)
  {
  case 1:
      FIS_WRITE_line1="AUDI";
      FIS_WRITE_line2="QUATTRO";    
    break;

  case 2:
    if (currAddr != ADR_Engine)
    {
      connect(ADR_Engine, ADR_Engine_Speed);
    }
    else
    {
      
      readSensors(4);
    }

    FIS_WRITE_line1="COOL:"+ String(coolantTemp);
    FIS_WRITE_line2="AIR:"+ String(intakeAirTemp);

    break;

  case 3:
    if (currAddr != ADR_Engine)
    {
      connect(ADR_Engine, ADR_Engine_Speed);
    }
    else
    {
      readSensors(115);
    }
    FIS_WRITE_line1="BOOST:";
    FIS_WRITE_line2= String(turboPress);

    break;
     case 4:
    // if (currAddr != ADR_Engine)
    // {
    //   connect(ADR_Engine, ADR_Engine_Speed);
    // }
    // else
    // {
    //   readSensors(2);
    // }
    FIS_WRITE_line1="INJ:"+ String(injektTime);
    FIS_WRITE_line2="MAF:"+ String(MAF);
    break;
     case 5:
        // if (currAddr != ADR_Engine)
        // {
        //   connect(ADR_Engine, ADR_Engine_Speed);
        // }
        // else
        // {
        //   readSensors(5);

        // }
        FIS_WRITE_line1="RPM:"+ String(engineSpeed);
        FIS_WRITE_line2="SPD:"+ String(vehicleSpeed);
      break;
      case 6:
        if (currAddr != ADR_Engine)
        {
          connect(ADR_Engine, ADR_Engine_Speed);
        }
        else
        {
          readSensors(20);

        }
        FIS_WRITE_line1="RETARDS:";
        FIS_WRITE_line2=String(param0)+":"+String(param1)+":"+String(param2)+":"+String(param3);
      break;
      case 7:
        FIS_WRITE_line1=String(Lhour)+"L:H";
        FIS_WRITE_line2=String(L100)+"L";
      break;
      case 8:
        FIS_WRITE_line1=String(vehicleSpeedAVG)+"K";
        FIS_WRITE_line2=String(L100AVG)+"L";
      break;
      case 9:
        FIS_WRITE_line1=String(Lhour)+"K";
        FIS_WRITE_line2=String(L100Move)+"L";
      break;


    case 17:
     if (currAddr != ADR_Dashboard)
     {
    //   disconnect();
      connect(ADR_Dashboard, ADR_Dashboard_Speed);
     }
     else
     {
       readSensors(2);
       //readSensors(50);
     }
     FIS_WRITE_line1="FUEL:";
     FIS_WRITE_line2=String(fuelLevel);
    break;
        case 18:
     if (currAddr != ADR_Dashboard)
     {
    //   disconnect();
      connect(ADR_Dashboard, ADR_Dashboard_Speed);
     }
     else
     {
       //readSensors(2);
       readSensors(50);
     }
     FIS_WRITE_line1="COOL:";
     FIS_WRITE_line2=String(coolantTemp);
    break;
  }
 
 // updateDisplay();





  //WRITE TO CLUSTER
if (Serial.available()) {
        FIS_WRITE_CHAR_FROM_SERIAL=(char)Serial.read();
        Serial.print(FIS_WRITE_CHAR_FROM_SERIAL);
        if (FIS_WRITE_CHAR_FROM_SERIAL == '\n') {
          FIS_WRITE_nl=1;
          if (FIS_WRITE_line==1){
            FIS_WRITE_line=2;
          } else {
            FIS_WRITE_line=1;
           }
        } else {
          if (FIS_WRITE_line==1){
              if (FIS_WRITE_nl){
                FIS_WRITE_nl=0;
                FIS_WRITE_line1="";
                FIS_WRITE_rotary_position_line1=-8;
                }
              FIS_WRITE_line1+=FIS_WRITE_CHAR_FROM_SERIAL;
          } else {
              if (FIS_WRITE_nl){
                FIS_WRITE_nl=0;
                FIS_WRITE_line2="";
                FIS_WRITE_rotary_position_line2=-8;
                }
               FIS_WRITE_line2+=FIS_WRITE_CHAR_FROM_SERIAL;
          }
        }

 }
   



   
  int FIS_WRITE_line1_length=FIS_WRITE_line1.length();
  int FIS_WRITE_line2_length=FIS_WRITE_line2.length();
  String FIS_WRITE_sendline1="        ";
  String FIS_WRITE_sendline2="        ";

  
   
    //do rotary and refresh each 0.5second
    //refresh cluster each 5s 
    if(millis()-FIS_WRITE_last_refresh>500 && (FIS_WRITE_line1_length>0 || FIS_WRITE_line2_length>0)){
      if (FIS_WRITE_line1_length>8){
      for (int i=0;i<8;i++){
        if (FIS_WRITE_rotary_position_line1+i>=0 && (FIS_WRITE_rotary_position_line1+i)<FIS_WRITE_line1_length) {
          FIS_WRITE_sendline1[i]=FIS_WRITE_line1[FIS_WRITE_rotary_position_line1+i];
        }
      }
       if (FIS_WRITE_rotary_position_line1<FIS_WRITE_line1_length){
              FIS_WRITE_rotary_position_line1++;
       } else {
              FIS_WRITE_rotary_position_line1=-8;
              FIS_WRITE_sendline1="        ";
       }
     } else {
       FIS_WRITE_sendline1=FIS_WRITE_line1;
     }
     if (FIS_WRITE_line2_length>8){
      for (int i=0;i<8;i++){
        if (FIS_WRITE_rotary_position_line2+i>=0 && (FIS_WRITE_rotary_position_line2+i)<FIS_WRITE_line2_length) {
          FIS_WRITE_sendline2[i]=FIS_WRITE_line2[FIS_WRITE_rotary_position_line2+i];
        }
      }
       if (FIS_WRITE_rotary_position_line2<FIS_WRITE_line2_length){
              FIS_WRITE_rotary_position_line2++;
       } else {
             
              FIS_WRITE_rotary_position_line2=-8;
       }
     } else {
       FIS_WRITE_sendline2=FIS_WRITE_line2;
     }
      // Serial.println("refresh");
      FIS_WRITE_sendTEXT(FIS_WRITE_sendline1,FIS_WRITE_sendline2);
      FIS_WRITE_last_refresh=millis();
    //end refresh
  }
//END WRITE TO CLUSTER 




}
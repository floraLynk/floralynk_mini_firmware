// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPLxlXQ5Pi1"
#define BLYNK_DEVICE_NAME "floraLynk mini"
#define BLYNK_FIRMWARE_VERSION        "1.0.2"
#define BLYNK_PRINT Serial
#include <ESPmDNS.h> 
#include <EEPROM.h>
#include <ESP32Ping.h>
#include <driver/adc.h>
#include "DHT.h"
#include <Wire.h>
#include <SFE_BMP180.h>
#include <math.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
//#define BLYNK_DEBUG
#define APP_DEBUG


// ====================== TEMP & HUMIDITY =============================

#define DHT_A_PIN 18       // DHT-22 Output Pin connection
#define DHT_A_TYPE DHT11  // DHT Type is DHT 22 (AM2302)*/
DHT dht_a(DHT_A_PIN, DHT_A_TYPE);
float RH;    // Stores humidity value in percent
float TC;   // Stores temperature value in Celcius
float TF;  // Stores temperature value in Farenheit
float logRM2, RM2, TM, TMc, TMf;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

// VPD, DP & DPF
float aa = 6.1121;
float bb = 18.678;
float cc = 257.14;
float dd = 234.5;
float Ps;
float VPD;
float Gamma;
float DP,DPf;

// ======================= PRESSURE SENSOR =============================

SFE_BMP180 bmp180;
double AP;

//========================= CO2 SENSOR =================================

// Parameters to model temperature and humidity dependence
float Correction_factor;
#define CORA 0.00035
#define CORB 0.02718
#define CORC 1.39538
#define CORD 0.0018

#define ATMOCO2 400.0f // Atmospheric CO2 level for calibration purposes
#define RLOAD 22000.0f // Calibration resistance at atmospheric CO2 level

// Average CO2 value from last 3 readings

#define CO2_READINGS 3
int INDEXPPM = 1;
unsigned int SUMPPM = 0;
unsigned int READINGS[CO2_READINGS];
float PPM_AVERAGE;

// Average CO2 zero from last 10 readings

#define CO2_ZERO 10
int INDEXZERO = 1;
unsigned int SUMZERO = 0;
unsigned int READINGS_ZERO[CO2_ZERO];
int CO2_ZERO_AVERAGE;
float MP = 0;

float const a=116.6020682, b=2.769034857;
//float const a=110.47, b=2.862;
float RS_co2;
float R0_co2;
float RS_co2_corrected;
float PPM, PPMA, PPMB;
int ppm_count = 0;

float CO2volt = 0;
unsigned long CO2rc;
float cVR;
int RepeatCO2;
  extern "C"{ int rom_phy_get_vdd33();
  float voltage;
  }

//============== Ground Temp =================

// Average Ground Temp from last 10 readings

#define TM_READINGS 10
int INDEXTM = 1;
float SUMTM = 0;
float READINGSTM[TM_READINGS];
float TM_AVERAGE;

float GroundT_sensor;
int StartTM = 0;
unsigned long RM1 = 10000;

//=============== Moisture Sensor =================

#define resistorValue 5000.0F
int const ChargePin = 16;
int const DischargePin = 17;
int ms1 = 0;
float microFarads, SoilMoisture;
float VOLTAGE_CONDENSATOR, VOLTAGE_CONDENSATOR2;
float EC;
float RMS;
float SM;
float ECconst1 = -2100445.98303406e-06;
float ECconst2 = -1159516.15776e-08;
float ECconst3 = 2405472.61697951e-05;
float ECconst4 = -1722213.5530653e-08;
bool ChargePinON = false;
bool pin1 = true;
unsigned long startTime;
unsigned long elapsedTime;
unsigned long endTime;
unsigned long SMTimeout = 20000;
bool RTest = false;
float VOLTAGE_CONDENSATOR_TEST,VOLTAGE_MOISTURE;
const int PWMFreq1 = 160000; /* 5 KHz */
const int PWMChannel1 = 4;//12;
const int PWMResolution1 = 8;//4;

//=============== LIGHT SENSOR =================

float SunL, HPS, MetH, FluF;
float Lux1;
float RL1;
float VL1;
int RL = 5000;


//================ BLYNK ============================

#define blynkRH V0
#define blynkTC V1
#define blynkTF V2
#define blynkLight V3
#define blynkCO2 V4
#define blynkMT V5
#define blynkMTf V6
#define blynkAtmP V7
#define blynkNanoF V8
#define blynkEC V9
#define blynkVPD V10
#define blynkDP V11
#define blynkDPf V12
#define blynkSunL V13
#define blynkHPS V14
#define blynkMetH V15
#define blynkFluF V16
#define CO2R V17
#define blynkVM V18
#define blynkVC V19

bool reconnectFlag = true;
bool Blynk_write_flag = false;
bool BlynkRunFlag;
bool Flag_local = true;
bool StD = false;
int StartingDelay = 5000;
int CountPing = 0;
unsigned long WT;
unsigned long reconnectWifi;
unsigned long lastdata;
unsigned long lastdata_local;


String port_and_UDP_str;
unsigned int localPort = 8000;
IPAddress DestinationIP(255,255,255,255 ); 
WiFiUDP udp;

String port_and_UDP_str2;
unsigned int localPort2 = 8080;
WiFiUDP udp8266;

//================= HTML ======================

String content1;
String content2;
String config_form;
String Month;
String ipStr;

// LOGO
const char floraLynkLogo[10000] PROGMEM = R"=====(

<HTML>
  <HEAD>
     
  </HEAD>
<BODY>
  <CENTER>
      
<img src='data:image/png;base64,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
SUVORK5CYII=' alt='Logo' width='422' height='140'>
</CENTER>
  
</BODY>
</HTML>
)====="; 

String html_WiFi = R"=====(

<HTML>
  <HEAD>
     
  </HEAD>
<BODY>
  <CENTER>
      

</CENTER>
  
</BODY>
</HTML>
)====="; 

bool WiFiConnected = true;
bool WiFiHotSpot = true;
int NetworksTotal;
bool AccP = true;
//================== EEPROM READ/WRITE ============================

// CO2 - callibration

String CO2_CF = "";
int CO2_CFint;
//String CO2_calibration = "47230";//"559251";

// Device name 

const char* mDNSname;
String DN = "";
//String DevicemDNSname = "floralynk-15";

// AUTH TOKEN
const char* DeviceToken;
String DT = "";
//String DeviceTokenWrite =  "9Fn3f0gmUu3Mxbw3oke2n9a-mmSTm6He";

String LCS_EEPROM = "";
float LCS;
//String DeviceLCS = "167779360";

String LCE_EEPROM = "";
float LCE;
//String DeviceLCE = "-1.38";

String VoltageROM = "";
float VoltR;
//String DeviceVoltageROM = "6.55";
uint32_t brown_reg_temp;

#include "BlynkEdgent.h"

void setup()
{
  Serial.begin(115200);
  delay(100);
brown_reg_temp = READ_PERI_REG(RTC_CNTL_BROWN_OUT_REG);
WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  dht_a.begin();
    EEPROM.begin(512);
    
// ===================== CALIBRATION FACTOR WRITING =============================

/*for (int i = 0; i < 10; i++){
      EEPROM.put(0 + i,CO2_calibration[i]);
      EEPROM.commit();
   }

   for (int i = 0; i < 41; i++){
      EEPROM.put(10 + i,DevicemDNSname[i]);
      EEPROM.commit();
     }

   for (int i = 0; i < 33; i++){
      EEPROM.put(51 + i,DeviceTokenWrite[i]);
      EEPROM.commit();
   }
 for (int i = 0; i < 12; i++){
      EEPROM.put(84 + i,DeviceLCS[i]);
      EEPROM.commit();
   }
   for (int i = 0; i < 12; i++){
      EEPROM.put(96 + i,DeviceLCE[i]);
      EEPROM.commit();
   }
  for (int i = 0; i < 4; i++){
      EEPROM.put(108 + i,DeviceVoltageROM[i]);
      EEPROM.commit();
   }*/
// ====================== READ CALLIBRATION FACTORS =============================

// Read CO2 ZERO
  for (int i = 0; i < 10; i++)
  {
    CO2_CF += char(EEPROM.read(i));
  }

// Read DEVICE NAME
  for (int i = 10; i < 51; i++)
  {
    DN += char(EEPROM.read(i));
  }

// READ DEVICE TOKEN
   for (int i = 51; i < 84; i++)
  {
    DT += char(EEPROM.read(i));
  }

// READ LCS FOR LIGHT SENSOR
     for (int i = 84; i < 96; i++)
  {
    LCS_EEPROM += char(EEPROM.read(i));
  }

// READ LCE FOR LIGHT SENSOR
       for (int i = 96; i < 108; i++)
  {
    LCE_EEPROM += char(EEPROM.read(i));
  }

      for (int i = 108; i < 112; i++)
  {
    VoltageROM += char(EEPROM.read(i));
  }
  
  EEPROM.end();
  
  mDNSname = DN.c_str();
  DeviceToken = DT.c_str();
  CO2_CFint = CO2_CF.toInt();
  LCS = LCS_EEPROM.toFloat();
  LCE = LCE_EEPROM.toFloat();
  VoltR = VoltageROM.toFloat();
  NetworksTotal = 0;
  
  bool success = bmp180.begin();
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11); // CO2 - PIN 34
  adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11); // MOISTURE - PIN 35
  adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_6); // Ground T -
  adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_11); // LIGHT SENSOR - 
  ledcSetup(PWMChannel1, PWMFreq1, PWMResolution1);
  ledcAttachPin(ChargePin, PWMChannel1);
  ledcAttachPin(DischargePin, PWMChannel1); 
  Serial.begin(115200);
  BlynkEdgent.begin();
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, brown_reg_temp);  
}

//****************************************************************************************************************************************************************************************************************************************

void loop() {
  if (WT < millis() - 900){
    BlynkEdgent.run();
    //Serial.println(voltage);
    ReadingFromSensors ();
    if (BlynkState::is(MODE_RUNNING) ){
      BlynkWrite ();
    }
    yield();
  //Serial.println (configStore.cloudHost);
    //Serial.println(ESP.getFreeHeap()); 
    WT = millis();
  }
  serverWiFi.handleClient();
  //if (lastdata <= millis()){
  if (WiFiConnected == true & RTest == false){
     wifi();
     WiFiConnected = false;
     //lastdata = millis() + 30000; 
  //Serial.println(millis()); 

}}

//****************************************************************************************************************************************************************************************************************************************

void ReadingFromSensors (){
  while (StD == false){
    delay(StartingDelay); 
    StD = true;
  }

//=========================================================================== PRESSURE ===========================================================================
  
  for (int i = 0; i < 1; i++){
    char status;
    double T, P;
    bool success = false;
    status = bmp180.startTemperature();

    if (status != 0) {
    delay(10);
    status = bmp180.getTemperature(T);

      if (status != 0) {
        status = bmp180.startPressure(3);
  
        if (status != 0) {
          delay(status);
          status = bmp180.getPressure(P, T);
  
          if (status != 0) {
            Serial.print("Pressure: ");
            Serial.print(P);
            Serial.print(" hPa, ");
  
            Serial.print("Temperature: ");
            Serial.print(T);
            Serial.print(" C, ");
            
            AP = P;
          }
        }
      }
    }
  }  

//=========================================================================== TEMP & HUMIDITY ===========================================================================

    for (int i = 0; i < 1; i++){
    RH = dht_a.readHumidity(); 
    TC= dht_a.readTemperature();  // Get Temperature value  */
    TF = (TC * 9.0)/ 5.0 + 32.0;
    Ps = aa*exp((bb-TC/dd)*(TC/(cc+TC)));
    VPD = Ps*(1 - RH*0.01);
    Gamma = log((RH*0.01)*(exp((bb-TC/dd)*(TC/(cc+TC)))));
    DP = cc*Gamma/(bb-Gamma);
    DPf = (DP * 9.0)/ 5.0 + 32.0;


//=========================================================================== GROUND TEMPERATURE ===========================================================================

       for (int i = 0; i < 1; i++){
      adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_6);
      for (int i=0;i<100;i++)
      GroundT_sensor += (adc1_get_raw(ADC1_CHANNEL_0)*2200)/4095.0;
      GroundT_sensor /= 100;
      if (GroundT_sensor  < 1100){
        adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);
        for (int i=0;i<100;i++)
        GroundT_sensor += (adc1_get_raw(ADC1_CHANNEL_0)*1100)/4095.0;
        GroundT_sensor /= 100;
      }
      if (GroundT_sensor  >= 2200){
        adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11);
        for (int i=0;i<100;i++)
        GroundT_sensor += (adc1_get_raw(ADC1_CHANNEL_0)*3300)/4095.0;
        GroundT_sensor /= 100;
      }
      /*Serial.print("GroundT_sensor: ");
      Serial.println(GroundT_sensor);*/
        RM2 = RM1 * (4095.0 / ((float)GroundT_sensor*(4095/3300)) - 1.0);
        logRM2 = log(RM2);
        TM = (1.0 / (c1 + c2*logRM2 + c3*logRM2*logRM2*logRM2));
        if (TM > 263.15){
          SUMTM = SUMTM - READINGSTM[INDEXTM];
          READINGSTM[INDEXTM] = TM;           // Add the newest reading to the window
          SUMTM = SUMTM + TM;                 // Add the newest reading to the sum
          TM_AVERAGE = SUMTM/TM_READINGS;
          INDEXTM = (INDEXTM + 1) % TM_READINGS;
          StartTM = StartTM + 1;
        if (StartTM < TM_READINGS){
          TMc = TM - 273.15;
          TMf = (TMc * 9.0)/ 5.0 + 32.0;
        } else {
          TMc = TM_AVERAGE - 273.15;
          TMf = (TMc * 9.0)/ 5.0 + 32.0;
      }
     } else {
       StartTM = 0;
       TMc = NULL;
       TMf = NULL;
    }
       }

//=========================================================================== LIGHT SENSOR ===========================================================================       

    for (int i = 0; i < 1; i++){
      adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_11);
      for (int i=0;i<1000;i++)
      VL1 += (adc1_get_raw(ADC1_CHANNEL_3)*3300)/4095.0;
      VL1 /= 1000;
      //Serial.println(VL1);

      
      if (VL1 < 2200){
        if (VL1 < 1100){
          adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_0);
          for (int i=0;i<1000;i++)
          VL1 += (adc1_get_raw(ADC1_CHANNEL_3)*1100)/4095.0;
          VL1 /= 1000;
        } else {
          adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_6);
          for (int i=0;i<1000;i++)
          VL1 += (adc1_get_raw(ADC1_CHANNEL_3)*2200)/4095.0;
          VL1 /= 1000;
      }
      }
      if (VL1 < 0.01){
        VL1 = 0.01;
        }
        RL1 = (3.3 - ((float)VL1/1000))/((float)VL1/1000) * RL;
        Lux1 = LCS*pow(RL1,LCE);
        SunL = Lux1*0.0185;
        HPS = Lux1*0.0122;
        MetH = Lux1*0.0141;
        FluF = Lux1*0.0135;
    }
    

//=========================================================================== CO2 SENSOR ===========================================================================

    if (CO2rc < millis() - 1000){
    for (int i = 0; i < 1; i++){
      Correction_factor = CORA * TC * TC - CORB * TC + CORC - (RH-33.)*CORD;
      voltage = 0;
      cVR = (float)rom_phy_get_vdd33() / 1000;
      voltage = (5 - (VoltR - cVR));
      int voltCount = 0;
      while (voltage < 4 || voltCount <= 5){
        cVR = (float)rom_phy_get_vdd33() / 1000;
         voltage = (5 - (VoltR - cVR));  
         voltCount = voltCount + 1;
      }
      adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11);
      for (int i=0;i<100;i++)
      CO2volt += (adc1_get_raw(ADC1_CHANNEL_6)*3300)/4095.0;
      CO2volt /= 100;
      if (CO2volt < 1100){
        adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);
        for (int i=0;i<100;i++)
        CO2volt += (adc1_get_raw(ADC1_CHANNEL_6)*1100)/4095.0;
        CO2volt /= 100;
      }
      if (4 <= voltage <= 5.5){
        RS_co2 = ((/*(float)voltage*/5 - (CO2volt/1000))/(CO2volt/1000)) * RLOAD;
        if (Correction_factor > 0){
        RS_co2_corrected = RS_co2 / Correction_factor;
        }
        R0_co2 = RS_co2 * pow((ATMOCO2/a),(1.0/b));
        float R0_co2_corrected = RS_co2_corrected * pow((ATMOCO2/a),(1.0/b));
        PPM = a * pow((RS_co2_corrected/CO2_CFint) , (-b));
        PPMA = PPM;
        for (RepeatCO2 = 0; RepeatCO2 < 5; RepeatCO2++){
          if ((PPM_AVERAGE / PPM > 1.2 || PPM / PPM_AVERAGE > 1.2) && ppm_count == 10 && RepeatCO2 < 5 && ((PPM_AVERAGE - PPM) > 100 || (PPM - PPM_AVERAGE) > 100)){
          voltage = 0;
          cVR = (float)rom_phy_get_vdd33() / 1000;
          voltage = (5 - (VoltR - cVR));
          voltCount = 0;
          while (voltage < 4 || voltCount <= 5){
            cVR = (float)rom_phy_get_vdd33() / 1000;
            voltage = (5 - (VoltR - cVR));  
            voltCount = voltCount + 1;
          }
      adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11);
      
      for (int i=0;i<100;i++)
      CO2volt += (adc1_get_raw(ADC1_CHANNEL_6)*3300)/4095.0;
      CO2volt /= 100;
      
      if (CO2volt < 1100){
        adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);
        for (int i=0;i<100;i++)
        CO2volt += (adc1_get_raw(ADC1_CHANNEL_6)*1100)/4095.0;
        CO2volt /= 100;
      }
              RS_co2 = ((/*(float)voltage*/5 - (CO2volt/1000))/(CO2volt/1000)) * RLOAD;
              if (Correction_factor > 0){
                RS_co2_corrected = RS_co2 / Correction_factor;
              }
              R0_co2 = RS_co2 * pow((ATMOCO2/a),(1.0/b));
              float R0_co2_corrected = RS_co2_corrected * pow((ATMOCO2/a),(1.0/b));
              PPMB = a * pow((RS_co2_corrected/CO2_CFint) , (-b));
              if ((0.80 < PPM_AVERAGE / PPMB < 1.20) && (0.80 < PPMB / PPM_AVERAGE < 1.20) && ((0 < (PPM_AVERAGE - PPMB) < 100) || (0 < (PPM_AVERAGE - PPMB) < 100))){
                PPM = PPMB;
                RepeatCO2 = 5;
              } else {
              if ((0.80 < PPMB / PPM < 1.20) && (0.80 < PPM / PPMB  < 1.20) && ((0 < PPM - PPMB < 100) || (0 < PPMB - PPM < 100))){
                PPM = PPMB;
                RepeatCO2 = 5;
              } else {
              if ( (0.80 < PPMB / PPMA < 1.20) && (0.80 < PPMA / PPMB  < 1.20) && ((0 < PPMA - PPMB < 100) || (0 < PPMB - PPMA < 100))){
                PPM = PPMB;
                RepeatCO2 = 5;
              }
          }
        }
        PPMA = PPMB;
         }
       }
        if (0 < PPM < 500000){

       // CO2 ZERO AVERAGE
       
            SUMZERO = SUMZERO - READINGS_ZERO[INDEXZERO];
            READINGS_ZERO[INDEXZERO] = R0_co2_corrected;           // Add the newest reading to the window
            SUMZERO = SUMZERO + R0_co2_corrected;                 // Add the newest reading to the sum
            CO2_ZERO_AVERAGE = float(SUMZERO)/CO2_ZERO;
            INDEXZERO = (INDEXZERO + 1) % CO2_ZERO; 

       //PPM AVERAGE
            
            SUMPPM = SUMPPM - READINGS[INDEXPPM];
            READINGS[INDEXPPM] = PPM;           // Add the newest reading to the window
            SUMPPM = SUMPPM + PPM;                 // Add the newest reading to the sum
            PPM_AVERAGE = float(SUMPPM)/CO2_READINGS;
            INDEXPPM = (INDEXPPM + 1) % CO2_READINGS;   // Increment the index, and wrap to 0 if it exceeds the window size
            CO2rc = millis ();
          if (ppm_count < 10){
            ppm_count = ppm_count + 1;
          }
      //PPM = a * pow((RS_co2/R0_co2), -b);
      }
    }
   }
 }
  
//=========================================================================== MOISTURE SENSOR ===========================================================================

if (SMTimeout < millis() && RTest == true){
       //while (analogRead(MoisturePin1) == 0){}
       for (int i = 0; i < 1; i++){
       /*adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11);*/
       VOLTAGE_MOISTURE = float(analogRead(35))/4095*3300;
       //VOLTAGE_MOISTURE = (adc1_get_raw(ADC1_CHANNEL_7)*3300)/4095.0;
       /*Serial.print("VOLTAGE_MOISTURE: ");
       Serial.println(VOLTAGE_MOISTURE);*/
       SoilMoisture = float(map(VOLTAGE_MOISTURE, 2050, 3300, 100, -1));
       EC =ECconst1*exp(ECconst2*SoilMoisture)*log(RMS)+ECconst3*exp(ECconst4*SoilMoisture);
      /* Serial.print("EC: ");
       Serial.println(EC);*/
       }
        if (pin1 == true){
          ledcWrite(PWMChannel1, 0);
          delay(50);
          ledcDetachPin(ChargePin);
          ledcDetachPin(DischargePin);
          pinMode(ChargePin, OUTPUT);
          digitalWrite(ChargePin, LOW);             // set charge pin to  LOW
          pinMode(DischargePin, OUTPUT);            // set discharge pin to output
          digitalWrite(DischargePin, LOW);
          RTest = false;
       } else { 

          ledcWrite(PWMChannel1, 0);
          delay(50);
          ledcDetachPin(ChargePin);
          ledcDetachPin(DischargePin);
          pinMode(ChargePin, OUTPUT);
          digitalWrite(ChargePin, LOW);             // set charge pin to  LOW
          pinMode(DischargePin, OUTPUT);            // set discharge pin to output
          digitalWrite(DischargePin, LOW);
          RTest = false;
       }
       SMTimeout = millis() + 30000;
     }  

if (SMTimeout < millis() && RTest == false){
    for (int i=0;i<100;i++)
    MP += (adc1_get_raw(ADC1_CHANNEL_7)*3.30)/4095.0;
    MP /= 100;
    //Serial.println(MP);
     if (ChargePinON == false && float(MP) <= 0.01){
          if (pin1 == true){
          ChargePinON = true;
          pinMode(DischargePin, INPUT);
          pinMode(ChargePin, OUTPUT);
          digitalWrite(ChargePin, HIGH);
          //Serial.println("true");
        } else {
          ChargePinON = true;
          pinMode(ChargePin, INPUT);
          pinMode(DischargePin, OUTPUT);
          digitalWrite(DischargePin, HIGH); 
          //Serial.println("false"); 
          }
       //while (analogRead(MoisturePin1) == 0){}
       adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_2_5);
       VOLTAGE_CONDENSATOR = (adc1_get_raw(ADC1_CHANNEL_7)*1500)/4095.0;
       adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11);
       VOLTAGE_CONDENSATOR2 = (adc1_get_raw(ADC1_CHANNEL_7)*3300)/4095.0;
       /*Serial.println(VOLTAGE_CONDENSATOR);    
       Serial.println(VOLTAGE_CONDENSATOR2);     */
     }
  }
  
   if (ChargePinON == true){
        if (pin1 == true){
          pinMode(DischargePin, OUTPUT);
          digitalWrite(DischargePin, LOW); 
          digitalWrite(ChargePin, LOW); 
          pin1 = false;
        } else {
          pinMode(ChargePin, OUTPUT);
          digitalWrite(ChargePin, LOW); 
          digitalWrite(DischargePin, LOW); 
          pin1 = true;
          }
      ChargePinON = false;
      //RMS = ((3.3 - VOLTAGE_CONDENSATOR*(pwmEC/4095)*CO2_CFint)/(pwmEC-(VOLTAGE_CONDENSATOR*(pwmEC/4095))));
      //RMS = ((3.3 - VOLTAGE_CONDENSATOR*(pwmEC/4095))/(VOLTAGE_CONDENSATOR*(pwmEC/4095)))*11000;
      //RMS = ((4095./(float)VOLTAGE_CONDENSATOR) * 3.3 - 1.)*11000;
      if (VOLTAGE_CONDENSATOR >= 1500){
        VOLTAGE_CONDENSATOR = VOLTAGE_CONDENSATOR2;
      }
      RMS = (((float)VOLTAGE_CONDENSATOR*11)/(3.3 - VOLTAGE_CONDENSATOR/1000));
      //RMS = map(VOLTAGE_MOISTURE, 1900, 3300, 100, 0);
      //EC = -2.10044598303406*pow(-0.01159516157706,20)*log10(15555)+24.0547261697951*pow(-0.017222135530653,20);
      RTest = true;
      ledcAttachPin(ChargePin, PWMChannel1);
      ledcAttachPin(DischargePin, PWMChannel1);
      ledcWrite(PWMChannel1, 255);    
      SMTimeout = millis() + 500;
   }
    
    port_and_UDP_str=(String)PPM_AVERAGE+","+(String)RH+","+(String)VPD+","+(String)TC+","+(String)AP+","+(String)DP+","+(String)Lux1+","+(String)TMc+","+(String)SM+","+(String)EC;

    udp.beginPacket(DestinationIP, localPort);
    //udp.write(port_and_UDP_str.c_str());
    udp.print(port_and_UDP_str);
    udp.endPacket();

    String port_and_UDP_str2 = String(mDNSname) + " | " + String(PPM_AVERAGE) + " | " + String(TC) + " | " + String(TF) + " | " + String(RH) + " | " + String(Lux1) + " | " + String(TMc) + " | " + String(TMf) + " | " + String(SoilMoisture) + " | " + String(RMS);

    udp8266.beginPacket(DestinationIP, localPort2);
    //udp.write(port_and_UDP_str.c_str());
    udp8266.print(port_and_UDP_str2);
    udp8266.endPacket();

   /*Serial.print(" RH: ");
   Serial.print(RH);
   Serial.print(", Tc: ");
   Serial.print(TC);
   Serial.print(", MT: "); Serial.print(TMc); 
   Serial.print(", LIGHT: "); Serial.print(Lux1); 
   Serial.print(", CO2: "); Serial.print(PPM_AVERAGE); 
   Serial.print(", MS1: "); Serial.print(ms1);  
   Serial.print(", RMS: "); Serial.print(RMS);
   Serial.print(", EC: "); Serial.println(EC);*/
   
   }
}

//****************************************************************************************************************************************************************************************************************************************
   
void BlynkWrite (){
  Ping.ping(configStore.cloudHost , 1);
  bool success = Ping.ping(configStore.cloudHost , 1);
  if(!success){
    Serial.println("Ping failed");
    return;
} else {
        //Serial.println("Ping succesful.");
        Blynk.virtualWrite(blynkRH, RH); 
        Blynk.virtualWrite(blynkTC, TC);
        Blynk.virtualWrite(blynkTF, TF);
        Blynk.virtualWrite(blynkAtmP, AP);
        Blynk.virtualWrite(blynkMT, TMc);
        Blynk.virtualWrite(blynkMTf, TMf);
        Blynk.virtualWrite(blynkVPD, VPD);
        Blynk.virtualWrite(blynkDP, DP);
        Blynk.virtualWrite(blynkDPf, DPf);
        Blynk.virtualWrite(blynkLight, Lux1);
        Blynk.virtualWrite(blynkSunL, SunL);
        Blynk.virtualWrite(blynkHPS, HPS);
        Blynk.virtualWrite(blynkMetH, MetH);
        Blynk.virtualWrite(blynkFluF, FluF);
        Blynk.virtualWrite(blynkCO2, PPM_AVERAGE);
        Blynk.virtualWrite(blynkNanoF, SoilMoisture);
        Blynk.virtualWrite(blynkEC, EC);
        Blynk.virtualWrite(CO2R, RS_co2);
        Blynk.virtualWrite(blynkVM, VOLTAGE_MOISTURE);
        Blynk.virtualWrite(blynkVC, VOLTAGE_CONDENSATOR);
  }
  }

//****************************************************************************************************************************************************************************************************************************************

void wifi(){

//==========================================================================================================================================================================================================================

  serverWiFi.on("/", []() {

    content2 = String(html_WiFi);
    content2 += String(floraLynkLogo);
    content2 += "<h1 style='color : #01686d; font-family: Helvetica'><center><!DOCTYPE HTML>\r\n<html>floraLynk Mini v1.0</h1><center><head><style>";//<meta http-equiv='refresh' content='1'>";
    content2 +=".table_pad {font-family: Helvetica;font-size: 18px; color: #FFFFFF;text-align: center; margin-top:5px; border-collapse: collapse; border-radius: 10px; overflow: hidden; margin-left: auto; margin-right: auto;  }"; //width: 100%;
    content2 +=".table_pad th {border: solid 0px #000000; background-color:#FFFFFF; color: #01686d;line-height: 30px; width:100px; height: 50px;text-align: left; padding-left: 10px;}";
    content2 +=".table_pad td {padding: 0px;border: solid 0px #000000;background-color: #FFFFFF;color: #000000; width:100px; height: 50px;  }"; //line-height: 100px; 
    content2 +=".air {border: 5px solid #01686d; background-color:#76D7EA; margin-top:20px; text-align: center; width: 600px;   padding-top: 20px; padding-right: 20px; padding-bottom: 20px;  padding-left: 20px; border-radius:20px; }"; //width: 670px; height: 400px;
    content2 +=".light {border: 5px solid #01686d; background-color: #FFF700; margin-top:20px; text-align: center; width: 400px;  padding-top: 20px; padding-right: 20px; padding-bottom: 20px;  padding-left: 20px; border-radius:20px;}";
    content2 +=".soil {border: 5px solid #01686d; background-color: #9b7653; margin-top:20px; text-align: center; width: 400px;  padding-top: 20px; padding-right: 20px; padding-bottom: 20px;  padding-left: 20px; border-radius:20px;}";
    content2 +=".lbl { background-color: #01686d; color: #FFFFFF;  font-family: Helvetica; font-size: 25px; font-weight: bold; border: solid 2px #FFFFFF; padding-top: 3px; padding-right: 10px; padding-bottom: 3px;  padding-left: 10px; border-radius:10px; }";
    content2 +=".ipaddres { background-color: #01686d; color: #FFFFFF;  font-family: Helvetica; font-size: 14px; font-weight: bold; border: solid 2px #000000; padding-top: 3px; padding-right: 10px; padding-bottom: 3px;  padding-left: 10px; border-radius:10px; }";
    content2 +=".tbl  {margin-left: auto; margin-right: auto;}";
    content2 +=".lux {margin-left: auto; margin-right: auto; font-family: Helvetica;font-size: 20px;color: #FFFFFF;text-align: center; margin-top:10px; border-collapse: collapse; border-radius: 10px; overflow: hidden; }";
    content2 +=".lux th {border: solid 0px #000000; background-color:#FFFFFF; color: #01686d;line-height: 30px; width:200px; height: 50px;}";
    content2 +=".lux td {padding: 0px;border: solid 0px #000000;background-color: #FFFFFF;color: #000000; width:200px; height: 50px;}";
    content2 +=".lgnd {font-family: Helvetica;font-size: 15px;color: #000000;text-align: left; font-weight: bold;}";
    content2 +="#soilTemp {margin-top:10px; margin-left: auto; margin-right: auto;}"; //margin-bottom:30px;
    content2 +=".clock {border: 1px solid #01686d; margin-top:20px; text-align: center; width: 250px;   padding-top: 7px; padding-right: 7px; padding-bottom: 7px;  padding-left: 7px; border-radius:50px 20px;}";
    content2 +="#disconnect {font-family: Helvetica; font-size:16px; font-weight:bold; background-color:red; color:white; green border: 2px solid #000000; border-radius:10px;}";
    content2 +="#ip {font-family: Helvetica; font-size:14px; font-weight:bold; background-color:#01686d; color:white; border: 2px solid #01686d; padding-top: 2px; padding-right: 20px; padding-bottom: 2px;  padding-left: 20px; border-radius:10px;}";
    content2 +="</style></head>";

    content2 += "<script>setInterval(function() {  getData();}, 500);";
    content2 += "function getData() {  var xhttp = new XMLHttpRequest();  xhttp.onreadystatechange = function()  {  if (this.readyState == 4 && this.status == 200)";
    content2 += "{     var web_values = this.responseText.split('|');";
    content2 += "      document.getElementById('CO2_val').innerHTML = web_values[0];";
    content2 += "      document.getElementById('TC_val').innerHTML = web_values[1];";
    content2 += "      document.getElementById('TF_val').innerHTML = web_values[2];";
    content2 += "      document.getElementById('RH_val').innerHTML = web_values[3];";
    content2 += "      document.getElementById('VPD_val').innerHTML = web_values[4];";
    content2 += "      document.getElementById('DP_val').innerHTML = web_values[5];";
    content2 += "      document.getElementById('DPF_val').innerHTML = web_values[6];";
    content2 += "      document.getElementById('LIGHT_val').innerHTML = web_values[7];";
    content2 += "      document.getElementById('SL_val').innerHTML = web_values[8];";
    content2 += "      document.getElementById('MH_val').innerHTML = web_values[9];";
    content2 += "      document.getElementById('FLU_val').innerHTML = web_values[10];";
    content2 += "      document.getElementById('HPS_val').innerHTML = web_values[11];";
    content2 += "      document.getElementById('TMC_val').innerHTML = web_values[12];";
    content2 += "      document.getElementById('TMF_val').innerHTML = web_values[13];";
    content2 += "      document.getElementById('SM_val').innerHTML = web_values[14];";
    content2 += "      document.getElementById('EC_val').innerHTML = web_values[15];";
    content2 += "      document.getElementById('currentDate').innerHTML = web_values[16];";
    content2 += "      document.getElementById('currentTime').innerHTML = web_values[17];";
    content2 += "      document.getElementById('IP_val').innerHTML = web_values[18];";
    content2 += "      document.getElementById('AP_val').innerHTML = web_values[19];}  };";
    content2 += "xhttp.open('GET', '/web_values', true);  xhttp.send();}</script>"; 

    content2 +="<body>";
    content2+="<form>";
    content2+="<table class='tbl'>";
    content2 += "<th>";
    content2+="<label class='ipaddres'>"; content2 += "LOCAL IP ADDRESS: <span id='IP_val'>"; content2 += "---.---.---.--- "; content2 += "</span>";   content2 += "| DEVICE NAME: "; content2 += DN; content2+="</label>";
    content2 += "</th>";
    content2+="</table>";
    //content2 += "<form  method='post' action='/'><input id='ip' type='submit' value='LOCAL IP ADDRESS: '";content2 += "<span id='IP_val'>"; content2 += ipStr; content2 += "</span></h5>";
    content2+="</form>";
    content2+="<form  method='post' action='disconnect'>  <input id='disconnect'  type='submit' value='DISCONNECT'>    </form>"; 
    content2+="<div class='clock'>";
    content2+="<table class='table_pad'>";
    content2 += "<tr>";
    content2 += "<th>";
    content2+="<label>"; content2 += "<div><span id='currentDate'>"; /*content2+= "---/--/----"/*currentDate/;*/ content2 += "</span>";  content2+="</label>";
    content2 += "</th>";
    content2 += "<th>";
    content2 +="<label>"; content2 += "<div><span id='currentTime'>";/* content2+= "--:--:--"/*currentTime;*/ content2 += "</span>";   content2+="</label>";
    content2 += "</th>";
    content2+="</tr>";  
    content2+="</table>";
    content2+="</div>";
 
//=========================================  AIR LABEL ===================================

    content2+="<div class='air'>";
    content2+="<label class='lbl'>AIR</label>";

// CO2 

    content2 += "<table class='tbl'>";
    content2 += "<tr>";
    content2 += "<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr><th>CO<sub>2</sub></th><td><font size='4'>";content2 += "<div><span id='CO2_val'>"; content2 += PPM_AVERAGE; content2 += "</span><br>";  content2 += " ppm </font> </td> </tr>"; //<th>AtmP</th>
    content2 +="</table>";
    content2 += "</th>";

// RH

    content2 += "<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th>RH</th> <td> <font size='4'>";content2 += "<div><span id='RH_val'>"; content2+=RH; content2 += "</span><br>"; content2 += " % </font> </td> </tr>";
    content2 +="</table>";
    content2 += "</th>";

// VPD

    content2 += "<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr>  <th>VPD</th> <td> <font size='4'>"; content2 += "<div><span id='VPD_val'>"; content2+=VPD; content2 += "</span><br>";   content2 += " hPa </font> </td> </tr>";
    content2 +="</table>";
    content2 += "</th>";
    content2 += "</tr>";
    content2 += "</table>";


// Air Temperature (AT)

    content2 += "<table class='tbl'>";
    content2 += "<tr>";
    content2 += "<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th colspan='2'> T</th> <td> <font size='4'>"; content2 += "<div><span id='TC_val'>"; content2 += TC; content2 += "</span><br>"; content2 += " &#8451;</font></td><td><font size='4'>"; content2 += "<div><span id='TF_val'>"; content2 += TF; content2 += "</span><br>";  content2 += " &#8457;</font></td> </tr>";
    content2 +="</table>";
    content2 += "</th>";

// Atmospheric pressure (AP)

if (AP > 0){
    content2 += "<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr><th colspan='2'>P</th> <td> <font size='4'>";content2 += "<div><span id='AP_val'>"; content2+=AP; content2 += "</span><br> hPa</font> </td> </tr>";
    content2 +="</table>";
    content2 += "</th>";
}

// Dew point (DP & DPf)

    content2 += "<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th colspan='2'>DP</th><td> <font size='4'>"; content2 += "<div><span id='DP_val'>"; content2+= DP; content2 += "</span><br>";  content2 += " &#8451; </font></td> <td> <font size='4'>"; content2 += "<div><span id='DPF_val'>"; content2 +=DPf; content2 += "</span><br>";  content2 += " &#8457;</font> </td> </tr>";
    content2 +="</table>";
    content2 += "</th>";
    content2 += "</tr>";
    content2 += "</table>";

// Air legend

    content2+="<p class='lgnd'>LEGEND</p>";
    content2+="<p class='lgnd'>RH - Relative Humidity, VPD - Vapour-pressure deficit,</p><p class='lgnd'>DP - Dew Point, AP - Atmospheric pressure</p>";
    content2+="</div>";

//=========================================  LIGHT LABEL ===================================

    content2+="<div class='light'>";
    content2+="<label class='lbl'>LIGHT</label>";
    

    content2 +="<table class='lux'>";
    content2 += "<tr> <th>Illuminance</th> <td colspan='2'> <font size='4'>"; content2 += "<span id='LIGHT_val'>"; content2 += Lux1; content2 += "</span><br>";    content2 += " lux </font></td> </tr>";
    content2 +="</table>";

    content2 += "<table class='tbl'>";

    content2 +="<tr>";

// SUNLIGHT

    content2 +="<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th>Sunlight</th> <td> <font size='4'>";  content2 += "<span id='SL_val'>"; content2 += SunL; content2 += "</span><br>";   content2 += " &#956;mol/(m<sup>2</sup>s) </font></td> </tr>";
    //content2 += "<tr><td>"; content2 += Lux1;   content2 += " lux</td></tr>";
    content2 +="</table>";
    content2 +="</th>";

//Metal Halide (MH)

    content2 +="<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th>MH</th> <td> <font size='4'>"; content2 += "<span id='MH_val'>"; content2 += MetH; content2 += "</span><br>";    content2 += " &#956;mol/(m<sup>2</sup>s) </font></td> </tr>";
    content2 +="</table>";
    content2 +="</th>";
    content2 +="</tr>";

// FLUORESCENT

    content2 +="<tr>";
    content2 +="<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th>Fluorescent</th> <td> <font size='4'>"; content2 += "<span id='FLU_val'>";  content2 += FluF; content2 += "</span><br>";   content2 += " &#956;mol/(m<sup>2</sup>s) </font></td> </tr>";
    content2 +="</table>";
    content2 +="</th>";

// High Pressure Sodium (HPS)

    content2 +="<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th>HPS</th> <td> <font size='4'>"; content2 += "<span id='HPS_val'>";  content2 += HPS; content2 += "</span><br>";    content2 += " &#956;mol/(m<sup>2</sup>s) </font></td> </tr>";
    content2 +="</table>";
    content2 +="</th>";
    content2 +="</tr>";

// LEGEND FOR LIGHT LABEL

    content2 +="</table>";
    content2+="<p class='lgnd'>LEGEND</p>";
    content2+="<p class='lgnd'>MH - Metal Halide, HPS - High Pressure Sodium</p>";
    content2+="</div>";


//=========================================  MOISTURE LABEL ===================================

    content2+="<div class='soil'>";
    content2+="<label class='lbl'>SOIL</label>";
    content2 +="<table id='soilTemp' class='table_pad'>";
    content2 += "<tr> <th>T</th> <td> <font size='4'>"; content2 += "<span id='TMC_val'>";  content2+=TMc; content2 += "</span><br>";  content2 += " &#8451; </font></td> <td> <font size='4'>"; content2 += "<div><span id='TMF_val'>";  content2 += TMf; content2 += "</span><br>";  content2 += " &#8457; </font></td> </tr>";
    content2 +="</table>";
    content2 += "<table class='tbl'>";
    content2 +="<tr>";

// SOIL MOISTURE 
   
    content2 +="<th>";
    content2 +="<table  class='table_pad'>";
    content2 += "<tr> <th>SM</th> <td> <font size='4'>"; content2 += "<span id='SM_val'>";  content2+=RMS; content2 += "</span><br>";  content2 += " %</font></td> </tr>";
    //content2 += "<tr> <td>"; content2+=Tc; content2 += " &#8451;</td> <td>"; content2 += Tf; content2 += " &#8457;</td> </tr>";
    content2 +="</table>";
    content2 +="</th>";

// EC 

    content2 +="<th>";
    content2 +="<table  class='table_pad'>";
    content2 += "<tr> <th>EC</th> <td> <font size='4'>"; content2 += "<span id='EC_val'>";  content2+=EC; content2 += "</span><br>";  content2 += " dS/m</font></td> </tr>";
    
    content2 +="</table>";
    content2 +="</th>";
    content2 +="</tr>";
    content2 +="</table>";
 
// LEGEND FOR MOISTURE 

    content2 +="<p class='lgnd'>LEGEND</p>";
    content2 +="<p class='lgnd'>SM - Soil Moisture, EC - Electrical Conductivity</p>";    
    content2 +="</div>";
    content2 += "<br><img class='centrated' src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAARIAAACICAMAAAAcY5SAAAAABGdBTUEAALGPC/xhBQAAAAFzUkdCAK7OHOkAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAA/UExURQATfzAwME9ZkXh4eBAQEPP0+fz8/f//";
    content2 += "/xEPEA8ODhopfhgYGLCwsCkoKDA8hZmZmWJiYkdHRzxHi4qKin9+freUQDcAAAb7SURBVHja7ZzbYtsgDIbphq1fSdv0sPd/1l34EGzAIAFO1sFVYnMQH0ICQWKop10yHUFH0pF0JE+OZDRLijW+pOFJkfxaU2bZJfuQ6HAESur9w5EYqYSiHof0QNCiMUYF8Ncua5Y+Gm2jl032MdkN8fsDInlMPNHe8wrH";
    content2 += "kLyLlCTUTD0kRoEklDd3AGLNPg8SI0cSzpvbnlY3T0NixEhieTPrMNr5ehKSUUwknnn/YvgnkQzlRIzUShutWT8FyUVKZDQiJCaxVCsajSZIjJTIcf7Qy1+HSHzCl8ciqUPESOsz2hVReySViJjs96Ftn0SA5khqETH5GUI7YYEIrZEEhX9XEEkjGWRIxgchyZE9YwGzrThPTYx2g9UWSVDykQoMSX6udJlH";
    content2 += "IClexB+txFI598W+crW1JZI6RGJKNSTyZpi105HIiVCxbzqMver0ryKSKkRyo8Kh9rX1N0OiICKP9tC/hOQcIocxBK0atkEyaojootUiJFlMhNBS7wdleEQfvx9jxUwk1p/0xGkxhpRD9MpfVERIGklPkDRaC648WZEvQJPFBq0oIiQZTOojURLRSzKGSxpSMqmOREukQJInR6ImQnpBRhGSJJPKSIx871tB";
    content2 += "kKAPkRwpNUSSGfBKF72UNJtCMhx7Ytm6ZNARkVvXoqFIIUmoSVL0sTDYkTvqZyIhSdCuZI6bo/RcSA6Z1ENiTBGTH4jEmCImQ+rwri4SwZmzFsmYJJLwxEOR45MjOWBSB0kGkUQ/389GEvfEVZC8G1PK5HQtiatJDSSRiSLyxIMRrH+qIIkyqR1jcjKW3Oo4A0lMwFZIxAH3n4RkiJsN/RWGU5BQ1kjWQXKJ";
    content2 += "Njk+FZIwkxZIBs1ZlSmwr0aLJCh7AySj6vyuXrzkko+EMtxCBST5R5TVZo4sqpZiUh2J6szxzNhr+d6sEEnFmy4/Bkk2k0ubc5xRdXfnbCRjVTX5OkSSsXc6H4n29o/RKckOSUaVw/lI2jJJ3BxQX3triyTXE7e4X6Iai/ZIctVkbHALqTwC1gYJadVEHQaSIaEHIGnDZMy40ahbJz41EuGZgBYJnY+khEnR";
    content2 += "7ehcJJfzkRQx8RZ3Q4b0MiT0ACRDxsW5TMliWYYSJHQ+kjI1MRm/xwn++CQfyXg+klIm0uGUIqEHIClkIhVdjISeF0nVX4CKln7nIyljIhZcjuRyPpIiJkcvQ5cvFEjoAUiGrM4k1uq5JkkTlzofiT46fYSE6iGh85HU+AlzLtN/BYkkTjRkIDk4UdUFdJ8bSSS8nuvelDFu8VWJ8mM6ZZTo4leQKKg8QxyT";
    content2 += "RdJR/7Jh+Mo7zB53xxTpa/lGJ500BP6VyiKukZol/+rZO/3nqf+TZ0fSkXQkHUlH0pF0JB1JR9KRdCQdSUfSkXQkPXUkHUlH0pF0JB1JR9KRdCQdSUfyPyKxAAMMgGGJ6OWeXtdMzGzXL29vu08vL+s75+NvMK6HLTOz9wj4XL+BaSvQUjnADHArJAzAMjOzBYiIwQwGg+2M5IbpO9YSW5kJ966tn3iSGthm";
    content2 += "3bbL7D5+ZQZg9w2BGRbMqwSWMdWOlhMH1hHVUyN+m4YGy4O1GNZB25W2wJ+ZjNcCEdF1LuKQtjx9/LRrGQ6UXFslmZ6IkSBEh4iI/qwyX+dcvpbchVsgrXk+1jw7Hdnr1Tecaq9Omb0y3EEs+VojYRuT4D5Pdt1huz5bkThmCL7yud30QK/1gkNqey97s9wOiWPn9hI4r173T+69mTs+P7HwmB4gsSE1ujfk";
    content2 += "aYm9kiKJtSSKxP06eub1PsCb+bGZ5AH1Z9z2TYRs5VSN9W0JTkDidJz595KCnmIyHcyT317H+eYO6UahLW4+kq39DrdDiwX+cAUiugIMz+hWR2KdobGz20fQAU1I7Oyq73rAfHV64WR/sy+e4rCnDIyYhQODYQG4Zn3y4CdpyW64olPcKeZMjQDHKz6OkbCLhMHrEm6qlzniEWRMCmzJDklAINcLOqJ/Mq+9";
    content2 += "8M3rBq0NaIn1xyFsXl3d5kdoyYYJdorD2PhifIZ6MeuQ2+XbxuPik4gsXrw2OeiE6SOCtqF5ja/ipj0L/NUr3Z3xvDTf1701hy53b7XjMA6sHT/d4UPDiWOjWmIdAjasJUvpK2AXSC9Be0O7hTAR2aVWf23ECKottr7qhIkDz3RMT74tf4QX9Ksd4LVfi3/+njdI+2rXLYC7MZw/vWJrS2xsWcIST1xkXhmW";
    content2 += "4Tg9TGsQ4G0PbSP6dm8z7a4t8Huzn78zYzDAbnxgcuvMu23ftAsGs2W8LlvF6+2DGS09zkZpAYCtu3GfQioBj4PdtLNuRZadYeQ55OBukLz9PcCw+0iDnRZBAM9IJtzuOqVH1YiIvl97oLHHXjuSjqQj6Ug6ko6kI/kp6S+KwRkMoSN6VwAAAABJRU5ErkJggg==' alt='Logo' width='100' height='50' >";
    content2 += "<p class='stl'>NIKO Technologies. We deliver technology.</p>";  
    content2 +="</body></html>";
  serverWiFi.send(200, "text/html", content2);
  });

//============================================================================================================================================================================================================================

  serverWiFi.on("/disconnect", []() {
   String contentReset;
   contentReset="<html>"; 
   contentReset+="<head>";
   contentReset+="<style>";
   contentReset+="#dialog{font-family: Helvetica;font-weihgt:bold; border: 5px solid #01686d;   text-align: center; width: 555px;   padding-top: 10px; padding-right: 5px;   padding-left: 5px; border-radius: 20px;}"; //padding-bottom: 5px;
   contentReset+=".dialog_button{border-radius:10px; width:80px; cursor:pointer;}";
   contentReset+=".lbl { background-color: #01686d; color: #FFFFFF;  font-family: Helvetica; font-size: 35px; font-weight: bold; border-radius:10px; padding-top: 2px; padding-right: 2px; padding-bottom: 2px;  padding-left: 2px;}";
   contentReset+=".centered { position: fixed; top: 50%; left: 50%; transform: translate(-50%, -50%); }";
   contentReset+="#warn{color:red;}";
   contentReset+="</style>";
   contentReset+="<script> function goBack() {window.location.href='/';} function closeTab() {window.close;}</script>"; 
   contentReset+="</head>";
   contentReset+="<body>";
   contentReset+="<div id='dialog' class='centered'>  <label class='lbl'>floraLynk Mini v1.0</label> <h3><p id='warn'>Do you really want to disconnect your floraLynk device from current wifi network?!</p></h3>";
   contentReset+="<form method='post' action='clear_reset'>  <button class='dialog_button' type='button' onClick='goBack()'>CANCEL</button> "; 
   contentReset+="<input id='disconnect' class='dialog_button'  type='submit' onClick='close_window();return false;'  value='OK'>  </form></div>";
   contentReset+="</body>";  
   contentReset+="</html>";
   serverWiFi.send(200, "text/html", contentReset);
   });

//============================================================================================================================================================================================================================
   
  serverWiFi.on("/clear_reset", []() {
   String contentInfo;
   contentInfo="<html>";
   contentInfo+="<head>";
   contentInfo+="<style>";
   contentInfo+="#information{font-family: Helvetica;font-weihgt:bold; border: 5px solid #01686d;   text-align: center; width: 555px;   padding-top: 10px; padding-right: 5px;   padding-left: 5px; border-radius: 20px;}"; //padding-bottom: 5px;
   contentInfo+=".dialog_button{border-radius:10px; width:80px; cursor:pointer;}";
   contentInfo+=".lbl { background-color: #01686d; color: #FFFFFF;  font-family: Helvetica; font-size: 35px; font-weight: bold; border-radius:10px; padding-top: 2px; padding-right: 2px; padding-bottom: 2px;  padding-left: 2px;}";
   contentInfo+=".centered { position: fixed; top: 50%; left: 50%; transform: translate(-50%, -50%); }";
   contentInfo+="#warn{color:01686d;}";
   contentInfo+="</style>";
   contentInfo+="<script> function goBack() {window.location.href='/';} function closeTab() {window.close;}</script>"; 
   contentInfo+="</head>";
   contentInfo+="<body>";
   contentInfo+="<div id='information' class='centered'>  <label class='lbl'>floraLynk Mini v1.0</label> <h3><p id='warn'>Your floraLynk device is disconnected from current WiFi network. Starting hotspot. When you are connected to device hotspot, you can access it on 'floralynk.setup' or '192.168.4.1' in your internet browser!</p></h3>";
   contentInfo+="</body>";
   contentInfo+="</html>";
   serverWiFi.send(200, "text/html", contentInfo);
   delay (1000);
    eraseMcuConfig();
    restartMCU();    
    });
  
//==========================================================================================================================

 serverWiFi.on("/calibration", []() {
 content2 = "<meta http-equiv='refresh' content='100'><h1 style='color : #000000;'><center><!DOCTYPE HTML>\r\n<html>floraLynk</h1><center><head><style>";
    content2 += "#connect {font-family: Helvetica; font-size:16px; font-weight:bold; background-color:01696E; color:white; border: 2px solid #000000; border-radius:10px;}";
    content2 +=".table_pad {width: 20%;font-family: cursive;font-size: 15px;color: #FFF;text-align: center;}";
    content2 +=".table_pad th {border: solid 1px #FFFFFF;background: linear-gradient(to right, #01696E,  #01696E,  #01696E);color: #F0FFF0;line-height: 20px;}";
    content2 +=".table_pad td {padding: 15px;border: solid 2px #000000;background: #FFFFFF;color: #000;line-height: 4px;}</style></head><body><table class='table_pad'>";
    content2 += "<script>setInterval(function() {  getData();}, 500);";
    content2 += "function getData() {  var xhttp = new XMLHttpRequest();  xhttp.onreadystatechange = function()  {  if (this.readyState == 4 && this.status == 200)";
    content2 += "{     var calibration_values = this.responseText.split('|');";
    content2 += "      document.getElementById('L_R_val').innerHTML = calibration_values[0];";
    content2 += "      document.getElementById('CO2_R_val').innerHTML = calibration_values[1];";
    content2 += "      document.getElementById('CO2_zero_val').innerHTML = calibration_values[2];";
    content2 += "      document.getElementById('VOLT_val').innerHTML = calibration_values[3];";
    content2 += "      document.getElementById('VOLT_ROM_val').innerHTML = calibration_values[4];";
    content2 += "      document.getElementById('VOLT_CO2_val').innerHTML = calibration_values[5];}  };";
    content2 += "xhttp.open('GET', '/calibration_values', true);  xhttp.send();}</script>"; content2 += "<tr><th>CO2-value</th><th>LCS</th><th>LCE</th></tr>";
    content2 += "<tr><td>";content2 += CO2_CFint; content2 += "</td><td>"; content2 += LCS; content2 += "</td><td>"; content2 += LCE; content2 += "</td></tr>";
    content2 += "<tr><th>LIGHT-RESISTANCE</th><th>CO2-RESISTANCE</th><th>CO2-ZERO 400.00</th><th>Voltage</th><th>VoltageROM</th><th>VoltageCO2</th></tr>"; 
    content2 += "<tr><td><span id='L_R_val'>";content2 += RL1; content2 += "</span></td><td><span id='CO2_R_val'>"; content2 += RS_co2; content2 += "</span></td><td><span id='CO2_zero_val'>"; content2 += CO2_ZERO_AVERAGE; content2 += "</span></td><td><span id='VOLT_val'>"; content2 += voltage; content2 += "</span></td><td><span id='VOLT_ROM_val'>"; content2 += cVR; content2 += "</span></td><td><span id='VOLT_CO2_val'>"; content2 += CO2volt; content2 += "</span></td></tr>";
    content2 +="<form  method='post' action='save_new_CO2_zero'>  <input id='connect'  type='submit' value='SET CO2-R0 value'> ";   content2 += "</div></form>";
    content2 +="</table></body></html>";
  serverWiFi.send(200, "text/html", content2);
  });

//============================================================================================================================================================================================================================

serverWiFi.on("/calibration_values", []() {
    String calibration_values = String(RL1) + " | " + String(RS_co2) + " | " + String(CO2_ZERO_AVERAGE) + " | " + String(voltage) + " | " + String(cVR) + " | " + String(CO2volt);
    serverWiFi.send(200, "text/plane", calibration_values);
});

//============================================================================================================================================================================================================================

serverWiFi.on("/save_new_CO2_zero", []() {
   CO2_CF = CO2_ZERO_AVERAGE;
   CO2_CFint = CO2_ZERO_AVERAGE;
   EEPROM.begin(512);
   for (int i = 0; i < 10; i++){
      EEPROM.put(0 + i,CO2_CF[i]);
      EEPROM.commit();
   }
   EEPROM.end();  
   
   content2 +="<HEAD>";
   content2 += "<meta http-equiv=\"refresh\" content=\"0;url=/\">" ; //<meta http-equiv=\"refresh\" content=\"1;
   content2 +="</head>";
  serverWiFi.send(200, "text/html", content2);
  });

//============================================================================================================================================================================================================================

  serverWiFi.on("/web_values",[](){ 

    if (month() == 1){
      Month = "Jan";
    }
    
    if (month() == 2){
       Month = "Feb";
    }
    
    if (month() == 3){
        Month = "Mar";
    }
    
    if (month() == 4){
        Month = "Apr";
    }
    
    if (month() == 5){
      Month = "May";
    }
    
    if (month() == 6){
      Month = "Jun";
    }
    
    if (month() == 7){
      Month = "Jul";
    }
    
    if (month() == 8){
      Month = "Aug";
    }
    
    if (month() == 9){
      Month = "Sep";
    }
    
    if (month() == 10){
      Month = "Oct";
    }
    
    if (month() == 11){
      Month = "Nov";
    }
    
    if (month() == 12){
            Month = "Dec";
    }

    char Day1[2] = {0};
    sprintf(Day1, "%02d/%04d",day(),year());
    String currentDate = Month + "/" + String(Day1)/* + "/" + String(Year)*/;
    
    uint16_t currentTime1[] = {hour(),minute(),second()};
    char currentTime[10] = {0};
    sprintf(currentTime, "%02d:%02d:%02d",currentTime1[0],currentTime1[1],currentTime1[2]);
    // current date/time based on current system
    time_t now = time(0);
    
    //char *ltm = ctime(&now);
    tm *ltm = gmtime(&now);

    // Declaring argument for time()
    time_t tt;
  
    // Declaring variable to store return value of
    // localtime()
    struct tm * ti;
  
    // Applying time()
    time (&tt);
  
    // Using localtime()
    ti = localtime(&tt);
    IPAddress ip = WiFi.localIP();
    ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    
    String web_values = String(PPM_AVERAGE) + " | " + String(TC) + " | " + String(TF) + " | " + String(RH) + " | " + String(VPD) + " | " + String(DP) + " | " + String(DPf) + " | " + String(Lux1) + " | " + String(SunL) + " | " + String(MetH) + " | " + String(FluF) + " | " + String(HPS) + " | " + String(TMc) + " | " + String(TMf) + " | " + String(SoilMoisture) + " | " + String(EC) + " | "  + currentDate + " | " + currentTime + " | " + String(ipStr) + " | " + String(AP);
    serverWiFi.send(200, "text/plane", web_values);
  });
}

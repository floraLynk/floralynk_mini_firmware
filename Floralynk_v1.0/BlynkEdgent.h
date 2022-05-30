void offlineSensors (){
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
      for (int i=0;i<100;i++)
      VL1 += (adc1_get_raw(ADC1_CHANNEL_3)*3300)/4095.0;
      VL1 /= 100;
      
      if (VL1  < 2200){
        if (VL1  < 1100){
          adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_0);
          for (int i=0;i<100;i++)
          VL1 += (adc1_get_raw(ADC1_CHANNEL_3)*1100)/4095.0;
          VL1 /= 100;
        } else {
          adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_6);
          for (int i=0;i<100;i++)
          VL1 += (adc1_get_raw(ADC1_CHANNEL_3)*2200)/4095.0;
          VL1 /= 100;
      }
      }
      if (VL1 < 0.01){
        VL1 = 0.01;
      }
        RL1 = (3.3 - (VL1/1000))/(VL1/1000) * RL;
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
        RS_co2 = (((float)voltage - (CO2volt/1000))/(CO2volt/1000)) * RLOAD;
        RS_co2_corrected = RS_co2 / Correction_factor;
        R0_co2 = RS_co2 * pow((ATMOCO2/a),(1.0/b));
        float R0_co2_corrected = RS_co2_corrected * pow((ATMOCO2/a),(1.0/b));
        PPM = a * pow((RS_co2/CO2_CFint) , (-b));
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
      CO2volt += (adc1_get_raw(ADC1_CHANNEL_6)*3900)/4095.0;
      CO2volt /= 100;
      
      if (CO2volt < 1100){
        adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);
        for (int i=0;i<100;i++)
        CO2volt += (adc1_get_raw(ADC1_CHANNEL_6)*1100)/4095.0;
        CO2volt /= 100;
      }
              RS_co2 = (((float)voltage - (CO2volt/1000))/(CO2volt/1000)) * RLOAD;
              RS_co2_corrected = RS_co2 / Correction_factor;
              R0_co2 = RS_co2 * pow((ATMOCO2/a),(1.0/b));
              float R0_co2_corrected = RS_co2_corrected * pow((ATMOCO2/a),(1.0/b));
              PPMB = a * pow((RS_co2/CO2_CFint) , (-b));
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
            READINGS_ZERO[INDEXZERO] = R0_co2;           // Add the newest reading to the window
            SUMZERO = SUMZERO + R0_co2;                 // Add the newest reading to the sum
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
       Serial.print("VOLTAGE_MOISTURE: ");
       Serial.println(VOLTAGE_MOISTURE);
       SoilMoisture = float(map(VOLTAGE_MOISTURE, 2050, 3300, 100, -1));
       EC = ECconst1*exp(ECconst2*SoilMoisture)*log(RMS)+ECconst3*exp(ECconst4*SoilMoisture);
       Serial.print("EC: ");
       Serial.println(EC);
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
        } else {
          ChargePinON = true;
          pinMode(ChargePin, INPUT);
          pinMode(DischargePin, OUTPUT);
          digitalWrite(DischargePin, HIGH);  
          }
       //while (analogRead(MoisturePin1) == 0){}
       adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_2_5);
       VOLTAGE_CONDENSATOR = (adc1_get_raw(ADC1_CHANNEL_7)*1500)/4095.0;
       adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11);
       VOLTAGE_CONDENSATOR2 = (adc1_get_raw(ADC1_CHANNEL_7)*3300)/4095.0;
       Serial.println(VOLTAGE_CONDENSATOR);    
       Serial.println(VOLTAGE_CONDENSATOR2);     
     }
  }
  
   if (ChargePinON == true){
        if (pin1 == true){
          pinMode(DischargePin, OUTPUT);
          digitalWrite(DischargePin, LOW); 
          digitalWrite(ChargePin, LOW); 
        } else {
          pinMode(ChargePin, OUTPUT);
          digitalWrite(ChargePin, LOW); 
          digitalWrite(DischargePin, LOW); 
          }
      ChargePinON = false;
      //RMS = ((3.3 - VOLTAGE_CONDENSATOR*(pwmEC/4095)*CO2_CFint)/(pwmEC-(VOLTAGE_CONDENSATOR*(pwmEC/4095))));
      //RMS = ((3.3 - VOLTAGE_CONDENSATOR*(pwmEC/4095))/(VOLTAGE_CONDENSATOR*(pwmEC/4095)))*11000;
      //RMS = ((4095./(float)VOLTAGE_CONDENSATOR) * 3.3 - 1.)*11000;
      if (VOLTAGE_CONDENSATOR == 1500){
        VOLTAGE_CONDENSATOR = VOLTAGE_CONDENSATOR2;
      }
      RMS = (((float)VOLTAGE_CONDENSATOR*11)/(3.3 - VOLTAGE_CONDENSATOR/1000));
      //RMS = map(VOLTAGE_MOISTURE, 1900, 3300, 100, 0);
      //EC = -2.10044598303406*pow(-0.01159516157706,20)*log10(15555)+24.0547261697951*pow(-0.017222135530653,20);
      RTest = true;
      ledcAttachPin(ChargePin, PWMChannel1);
      ledcAttachPin(DischargePin, PWMChannel1);
      ledcWrite(PWMChannel1, 255);    
      SMTimeout = millis() + 1000;
   }
   
   Serial.print(" RH: ");
   Serial.print(RH);
   Serial.print(", Tc: ");
   Serial.print(TC);
   Serial.print(", MT: "); Serial.print(TMc); 
   Serial.print(", LIGHT: "); Serial.print(Lux1); 
   Serial.print(", CO2: "); Serial.print(PPM_AVERAGE); 
   Serial.print(", MS1: "); Serial.print(ms1);  
   Serial.print(", RMS: "); Serial.print(RMS);
   Serial.print(", EC: "); Serial.println(EC);
   
   }
}

//****************************************************************************************************************************************************************************************************************************************

extern "C" {
  void app_loop();
  void eraseMcuConfig();
  void restartMCU();
}

#include "Settings.h"
#include <BlynkSimpleEsp32_SSL.h>

#ifndef BLYNK_NEW_LIBRARY
#error "Old version of Blynk library is in use. Please replace it with the new one."
#endif

#if !defined(BLYNK_TEMPLATE_ID) || !defined(BLYNK_DEVICE_NAME)
#error "Please specify your BLYNK_TEMPLATE_ID and BLYNK_DEVICE_NAME"
#endif

#include "BlynkState.h"
#include "ConfigStore.h"
#include "ResetButton.h"
#include "ConfigMode.h"
#include "Indicator.h"
#include "OTA.h"
#include <WidgetRTC.h>
#include <TimeLib.h>

WidgetRTC rtc;
BlynkTimer timer;

String currentTime, currentDate;

// Esp8266 pins.
#define ESP8266_GPIO2    2 // Blue LED.
#define ESP8266_GPIO4    4 // Relay control.
#define LED_PIN          ESP8266_GPIO2
#define RELAY_PIN        V17
#define INICIO           V18
#define FIN              V19

// para detectar horarios de inicio y fin de relay
long startTimeInSecs;
long endTimeInSecs;
//start time
long shs;
long smins;
long ssecs;
//end time
long ehs;
long emins;
long esecs;

// Flag for sync on re-connection.
bool isFirstConnect = true;
volatile int relayState = LOW;  // Blynk app pushbutton status.

//****************************************************************************************************************************************************************************************************************************************

void clockDisplay()
{
  //time_t t = now();
  currentTime = String(hour()) + ":" + minute() + ":" + second();
  currentDate = String(day()) + "." + month() + "." + year();
  //Blynk.virtualWrite(V1, currentTime);
  //Blynk.virtualWrite(V2, currentDate);
  //Serial.println("HORA ACTUAL: "+currentTime);
  //content2+="<p><label>"; content2+=currentDate;  content2+="</label></p>";
  //content2+="<p><label>"; content2+=currentTime;  content2+="</label></p>";

}

BLYNK_CONNECTED() {
  if ( isFirstConnect ) {
    Blynk.syncAll();
    rtc.begin();
    isFirstConnect = false;
  }
}
 
// Blynk app relay command.
BLYNK_WRITE( RELAY_PIN ) {
  if ( param.asInt() != relayState ) {
    relayState = !relayState;                  // Toggle state.
    digitalWrite( ESP8266_GPIO4, relayState ); // Relay control pin.
  }
}

// Para leer hora de inicio
BLYNK_WRITE(INICIO) {
  startTimeInSecs = param[0].asLong();
  shs = startTimeInSecs/3600;
  smins = (startTimeInSecs/60) - (60*shs);
  ssecs = (startTimeInSecs) - ((60*smins) + (3600*shs));
  //Serial.println("Inicio: "+String(shs)+":"+String(smins)+":"+String(ssecs));
}

// Para leer hora de fin
BLYNK_WRITE(FIN) {
  endTimeInSecs = param[0].asLong();
  ehs = endTimeInSecs/3600;
  emins = (endTimeInSecs/60) - (60*ehs);
  esecs = (endTimeInSecs) - ((60*emins) + (3600*ehs));
  //Serial.println("Fin: "+String(ehs)+":"+String(emins)+":"+String(esecs));
}

inline

//****************************************************************************************************************************************************************************************************************************************

void BlynkState::set(State m) {
  if (state != m && m < MODE_MAX_VALUE) {
    DEBUG_PRINT(String(StateStr[state]) + " => " + StateStr[m]);
    state = m;

    // You can put your state handling here,
    // i.e. implement custom indication
  }
}

//****************************************************************************************************************************************************************************************************************************************

void printDeviceBanner()
{
  Blynk.printBanner();
  DEBUG_PRINT("--------------------------");
  DEBUG_PRINT(String("Product:  ") + BLYNK_DEVICE_NAME);
  DEBUG_PRINT(String("Hardware: ") + BOARD_HARDWARE_VERSION);
  DEBUG_PRINT(String("Firmware: ") + BLYNK_FIRMWARE_VERSION " (build " __DATE__ " " __TIME__ ")");
  if (configStore.getFlag(CONFIG_FLAG_VALID)) {
    DEBUG_PRINT(String("Token:    ...") + (configStore.cloudToken+28));
  }
  DEBUG_PRINT(String("Device:   ") + BLYNK_INFO_DEVICE + " @ " + ESP.getCpuFreqMHz() + "MHz");
  DEBUG_PRINT(String("MAC:      ") + WiFi.macAddress());
  DEBUG_PRINT(String("Flash:    ") + ESP.getFlashChipSize() / 1024 + "K");
  DEBUG_PRINT(String("ESP sdk:  ") + ESP.getSdkVersion());
  DEBUG_PRINT(String("Chip rev: ") + ESP.getChipRevision());
  DEBUG_PRINT(String("Free mem: ") + ESP.getFreeHeap());
  DEBUG_PRINT("--------------------------");
}

//****************************************************************************************************************************************************************************************************************************************

void runBlynkWithChecks() {
  /*unsigned long timer2 = millis()  + 5000;
  BlynkRunFlag = true;
  while ((timer2 > millis()) && (BlynkRunFlag == true)){*/
  
  if (WiFi.status() == WL_CONNECTED) {
  for (int i = 0; i < 3; i++){
  boolean checkBlynk = Ping.ping(configStore.cloudHost , 1);
  if(checkBlynk){ 
  CountPing = 0;
  Blynk.run();

  if (BlynkState::get() == MODE_RUNNING) {
    if (!Blynk.connected()) {
      if (WiFi.status() == WL_CONNECTED) {
        BlynkState::set(MODE_CONNECTING_CLOUD);
      } else {
        BlynkState::set(MODE_CONNECTING_NET);
      }
    }/* else {
  BlynkRunFlag = false;
    }*/
  }
} else {
  CountPing = CountPing + 1;
  if (CountPing > 25){
    if (WiFi.status() == WL_CONNECTED) {
      CountPing = 0;
      BlynkState::set(MODE_LOCAL_NETWORK);
    } else {
      CountPing = 0;
      BlynkState::set(MODE_CONNECTING_NET);
    }
  }
}
  /*if (timer2 < millis() -10000){
    Blynk_write_flag = false;
    BlynkState::set(MODE_CONNECTING_CLOUD);
  }*/
  }
} else { 
  BlynkState::set(MODE_CONNECTING_NET);
}

}

//****************************************************************************************************************************************************************************************************************************************

class Edgent {

public:
  void begin()
  {
    indicator_init();
    button_init();
    config_init();

    WiFi.persistent(false);
    WiFi.enableSTA(true);   // Needed to get MAC

    printDeviceBanner();

BlynkState::set(MODE_WAIT_CONFIG);
if (configStore.getFlag(CONFIG_FLAG_VALID)) {
      BlynkState::set(MODE_CONNECTING_NET);
    } else if (config_load_blnkopt()) {
      DEBUG_PRINT("Firmware is preprovisioned");
      BlynkState::set(MODE_CONNECTING_NET);
    } else {
      BlynkState::set(MODE_WAIT_CONFIG);
    }
  }

//****************************************************************************************************************************************************************************************************************************************

  void run() {
    app_loop();
    switch (BlynkState::get()) {
    case MODE_WAIT_CONFIG:       
    case MODE_CONFIGURING:       enterConfigMode();    break;
    case MODE_CONNECTING_NET:    enterConnectNet();    break;
    case MODE_CONNECTING_CLOUD:  enterConnectCloud();  break;
    case MODE_RUNNING:           runBlynkWithChecks(); break;
    case MODE_OTA_UPGRADE:       enterOTA();           break;
    case MODE_SWITCH_TO_STA:     enterSwitchToSTA();   break;
    case MODE_RESET_CONFIG:      enterResetConfig();   break;
    case MODE_LOCAL_NETWORK:     localNetwork();       break; 
    default:                     enterError();         break;
    }
  }

};

Edgent BlynkEdgent;

//****************************************************************************************************************************************************************************************************************************************

void app_loop() {
    timer.run();
         boolean isconnected = Blynk.connected();
     if (!isconnected) {
        if ((hour() == shs) and (minute() == smins) and (second() == ssecs)) {
        digitalWrite( ESP8266_GPIO4, HIGH );
        digitalWrite( LED_PIN, HIGH ); // Turn off LED.
        }
    if ((hour() == ehs) and (minute() == emins) and (second() == esecs)) {
        digitalWrite( ESP8266_GPIO4, LOW );
        digitalWrite( LED_PIN, LOW );          // Turn off LED.
       }
       //Serial.println("Not connected to Blynk!");
    }
    else
    {
      //Serial.println("Connected to Blynk!"); 
    }
    
}

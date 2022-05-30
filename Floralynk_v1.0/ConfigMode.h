
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Update.h>
WebServer serverWiFi(80);
MDNSResponder mdns;
   
#include <nvs.h>
#include <nvs_flash.h>

WebServer server(WIFI_AP_CONFIG_PORT);
DNSServer dnsServer;
const byte DNS_PORT = 53;
long int Local = 0;
long int OfflineReadings;

static const char serverUpdateForm[] PROGMEM =
  R"(<html><body>
      <form method='POST' action='' enctype='multipart/form-data'>
        <input type='file' name='update'>
        <input type='submit' value='Update'>
      </form>
    </body></html>)";

//****************************************************************************************************************************************************************************************************************************************

void restartMCU() {
  ESP.restart();
  while(1) {};
}

//****************************************************************************************************************************************************************************************************************************************

void eraseMcuConfig() {
  // Erase ESP32 NVS
  int err;
  //err=nvs_flash_init();
  //BLYNK_LOG2("nvs_flash_init: ", err ? String(err) : "Success");
  err=nvs_flash_erase();
  Serial.setTimeout(5000);
  nvs_flash_init();
  EEPROM.begin(512);
  for (int i = 0; i < 10; i++){
      EEPROM.put(0 + i,CO2_CF[i]);
      EEPROM.commit();
   }

    for (int i = 0; i < 41; i++){
      EEPROM.put(10 + i,DN[i]);
      EEPROM.commit();
     }

   for (int i = 0; i < 33; i++){
      EEPROM.put(51 + i,DT[i]);
      EEPROM.commit();
   }
   for (int i = 0; i < 12; i++){
      EEPROM.put(84 + i,LCS_EEPROM[i]);
      EEPROM.commit();
   }
   for (int i = 0; i < 12; i++){
      EEPROM.put(96 + i,LCE_EEPROM[i]);
      EEPROM.commit();
   }

   for (int i = 0; i < 4; i++){
      EEPROM.put(108 + i,VoltageROM[i]);
      EEPROM.commit();
   }
  BLYNK_LOG2("nvs_flash_erase: ", err ? String(err) : "Success");
}

//****************************************************************************************************************************************************************************************************************************************

void getWiFiName(char* buff, size_t len, bool withPrefix = true) {
  const uint64_t chipId = ESP.getEfuseMac();
  uint32_t unique = 0;
  for (int i=0; i<4; i++) {
    unique = BlynkCRC32(&chipId, sizeof(chipId), unique);
  }
  if (withPrefix) {
    snprintf(buff, len, "%s-%05X", BLYNK_DEVICE_NAME, unique & 0xFFFFF);
  } else {
    snprintf(buff, len, "%s-%05X", BLYNK_DEVICE_NAME, unique & 0xFFFFF);      
  }
}

//****************************************************************************************************************************************************************************************************************************************

void enterConfigMode()
{
if (WiFiHotSpot == true){

  char ssidBuff[64];
  getWiFiName(ssidBuff, sizeof(ssidBuff));
  if (reconnectFlag == true){
    reconnectWifi = millis() + 120000;
    serverWiFi.stop();
    mdns.end();
    reconnectFlag = false;
  }
  WiFi.mode(WIFI_OFF);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  delay(100);
  WiFi.mode(WIFI_AP);
  esp_err_t esp_wifi_set_max_tx_power(20);
  delay(2000);
  WiFi.softAPConfig(WIFI_AP_IP, WIFI_AP_IP, WIFI_AP_Subnet);
  //WiFi.softAP(ssidBuff);
  WiFi.softAP(DN.c_str());
  delay(500);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, brown_reg_temp);
  IPAddress myIP = WiFi.softAPIP();
  //DEBUG_PRINT(String("AP SSID: ") + ssidBuff);
  DEBUG_PRINT(String("AP SSID: ") + DN);
  DEBUG_PRINT(String("AP IP:   ") + myIP[0] + "." + myIP[1] + "." + myIP[2] + "." + myIP[3]);

  // Set up DNS Server
  dnsServer.setTTL(300); // Time-to-live 300s
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure); // Return code for non-accessible domains
#ifdef WIFI_CAPTIVE_PORTAL_ENABLE
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP()); // Point all to our IP
  server.onNotFound(handleRoot);
#else
  dnsServer.start(DNS_PORT, CONFIG_AP_URL, WiFi.softAPIP());
  DEBUG_PRINT(String("AP URL:  ") + CONFIG_AP_URL);

#endif


#ifndef BLYNK_USE_SPIFFS

  server.on("/", []() {
    for (int i = 0; i < 1; i++){
    content2 = String(html_WiFi);
    content2 += String(floraLynkLogo);
    content2 += "<h1 style='color : #01686d; font-family: Helvetica'><center><!DOCTYPE HTML>\r\n<html>floraLynk Mini v1.0</h1><center><head><style>";/*<meta http-equiv='refresh' content='1'>*/
    content2 +=".table_pad {font-family: Helvetica;font-size: 18px; color: #FFFFFF;text-align: center; margin-top:5px; border-collapse: collapse; border-radius: 10px; overflow: hidden; margin-left: auto; margin-right: auto;  }"; //width: 100%;
    content2 +=".table_pad th {border: solid 0px #000000; background-color:#FFFFFF; color: #01686d;line-height: 30px; width:100px; height: 50px;text-align: left; padding-left: 10px;}";
    content2 +=".table_pad td {padding: 0px;border: solid 0px #000000;background-color: #FFFFFF;color: #000000; width:100px; height: 50px;  }"; //line-height: 100px; 
    content2 +=".air {border: 5px solid #01686d; background-color:#76D7EA; margin-top:20px; text-align: center; width: 600px;   padding-top: 20px; padding-right: 20px; padding-bottom: 20px;  padding-left: 20px; border-radius:20px; }"; //width: 670px; height: 400px;
    content2 +=".light {border: 5px solid #01686d; background-color: #FFF700; margin-top:20px; text-align: center; width: 400px;  padding-top: 20px; padding-right: 20px; padding-bottom: 20px;  padding-left: 20px; border-radius:20px;}";
    content2 +=".soil {border: 5px solid #01686d; background-color: #9b7653; margin-top:20px; text-align: center; width: 400px;  padding-top: 20px; padding-right: 20px; padding-bottom: 20px;  padding-left: 20px; border-radius:20px;}";
    content2 +=".lbl { background-color: #01686d; color: #FFFFFF;  font-family: Helvetica; font-size: 25px; font-weight: bold; border: solid 2px #FFFFFF; padding-top: 3px; padding-right: 10px; padding-bottom: 3px;  padding-left: 10px; border-radius:10px; }";
    content2 +=".tbl  {margin-left: auto; margin-right: auto;}";
    content2 +=".lux {margin-left: auto; margin-right: auto; font-family: Helvetica;font-size: 20px;color: #FFFFFF;text-align: center; margin-top:10px; border-collapse: collapse; border-radius: 10px; overflow: hidden; }";
    content2 +=".lux th {border: solid 0px #000000; background-color:#FFFFFF; color: #01686d;line-height: 30px; width:200px; height: 50px;}";
    content2 +=".lux td {padding: 0px;border: solid 0px #000000;background-color: #FFFFFF;color: #000000; width:200px; height: 50px;}";
    content2+=".lgnd {font-family: Helvetica;font-size: 15px;color: #000000;text-align: left; font-weight: bold;}";
    content2+="#soilTemp {margin-top:10px; margin-left: auto; margin-right: auto;}"; //margin-bottom:30px;
    content2+=".clock {border: 1px solid #01686d; margin-top:20px; text-align: center; width: 250px;   padding-top: 7px; padding-right: 7px; padding-bottom: 7px;  padding-left: 7px; border-radius:50px 20px;}";
    content2+="#connect {font-family: Helvetica; font-size:16px; font-weight:bold; background-color:green; color:white;green border: 2px solid #000000; border-radius:10px;}";
    content2 +="</style></head>";   
    content2 +="<body>";
    
    content2 += "<script>setInterval(function() {  getData();}, 3000);";
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
    content2 += "      document.getElementById('AP_val').innerHTML = web_values[16];;}  };";
    content2 += "xhttp.open('GET', '/web_values', true);  xhttp.send();}</script>"; 

    content2+="<form  method='post' action='wireless_setup'>  <input id='connect'  type='submit' value='CONNECT TO WIRELESS NETWORK'>    </form>"; 
    content2+="<div class='clock'>";
    content2+="<table class='table_pad'>";
    
    content2 += "<tr>";
    
    content2 += "<th>";
    content2+="<label>"; /*content2+=currentDate;*/  content2+="</label>";
    content2 += "</th>";

    content2 += "<th>";
    content2 +="<label>"; /*content2+=currentTime;*/  content2+="</label>";
    content2 += "</th>";
    
    content2+="</tr>";
    
    
    content2+="</table>";
    content2+="</div>";
    

    
    //clockDisplay();
    //content2+="<p><label>"; content2+=asctime(ti); content2+="</label></p>";

//=========================================  AIR LABEL ===================================

    content2+="<div class='air'>";
    content2+="<label class='lbl'>AIR</label>";

// CO2 

    content2 += "<table class='tbl'>";
    content2 += "<tr>";
    content2 += "<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr><th>CO<sub>2</sub></th><td><font size='4'>";content2 += "<div><span id='CO2_val'>"; content2 += PPM_AVERAGE; content2 += "</span><br>";  content2 += " ppm </font> </td> </tr>"; //<th>AtmP</th>
    //content2 += "<tr><td>"; content2 += PPM;   content2 += " ppm</td></tr>";
    content2 +="</table>";
    content2 += "</th>";

// RH

    content2 += "<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th>RH</th> <td> <font size='4'>";content2 += "<div><span id='RH_val'>"; content2+=RH; content2 += "</span><br>"; content2 += " % </font> </td> </tr>";
    //content2 += "<tr> <td>"; content2+=RH; content2 += " %</td>  </tr>";
    content2 +="</table>";
    content2 += "</th>";

// VPD

    content2 += "<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr>  <th>VPD</th> <td> <font size='4'>"; content2 += "<div><span id='VPD_val'>"; content2+=VPD; content2 += "</span><br>";   content2 += " hPa </font> </td> </tr>";
    //content2 += "<tr> <td>";  content2+=VPD; content2 += " hPa;</td> </tr>";
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
    content2 += "<tr> <th>Illuminance</th> <td colspan='2'> <font size='4'>"; content2 += "</td><td><font size='4'>"; content2 += "<div><span id='LIGHT_val'>"; content2 += Lux1; content2 += "</span><br>";    content2 += " lux </font></td> </tr>";
    content2 +="</table>";

    content2 += "<table class='tbl'>";

    content2 +="<tr>";

// SUNLIGHT

    content2 +="<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th>Sunlight</th> <td> <font size='4'>";  content2 += "<div><span id='SL_val'>"; content2 += SunL; content2 += "</span><br>";   content2 += " &#956;mol/(m<sup>2</sup>s) </font></td> </tr>";
    //content2 += "<tr><td>"; content2 += Lux1;   content2 += " lux</td></tr>";
    content2 +="</table>";
    content2 +="</th>";

//Metal Halide (MH)

    content2 +="<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th>MH</th> <td> <font size='4'>"; content2 += "<div><span id='MH_val'>"; content2 += MetH; content2 += "</span><br>";    content2 += " &#956;mol/(m<sup>2</sup>s) </font></td> </tr>";
    content2 +="</table>";
    content2 +="</th>";
    content2 +="</tr>";

// FLUORESCENT

    content2 +="<tr>";
    content2 +="<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th>Fluorescent</th> <td> <font size='4'>"; content2 += "<div><span id='FLU_val'>";  content2 += FluF; content2 += "</span><br>";   content2 += " &#956;mol/(m<sup>2</sup>s) </font></td> </tr>";
    content2 +="</table>";
    content2 +="</th>";

// High Pressure Sodium (HPS)

    content2 +="<th>";
    content2 +="<table class='table_pad'>";
    content2 += "<tr> <th>HPS</th> <td> <font size='4'>"; content2 += "<div><span id='HPS_val'>";  content2 += HPS; content2 += "</span><br>";    content2 += " &#956;mol/(m<sup>2</sup>s) </font></td> </tr>";
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
    content2 += "<tr> <th>T</th> <td> <font size='4'>"; content2 += "<div><span id='TMC_val'>";  content2+=TMc; content2 += "</span><br>";  content2 += " &#8451; </font></td> <td> <font size='4'>"; content2 += "<div><span id='TMF_val'>";  content2 += TMf; content2 += "</span><br>";  content2 += " &#8457; </font></td> </tr>";
    content2 +="</table>";
    content2 += "<table class='tbl'>";
    content2 +="<tr>";

// SOIL MOISTURE 
   
    content2 +="<th>";
    content2 +="<table  class='table_pad'>";
    content2 += "<tr> <th>SM</th> <td> <font size='4'>"; content2 += "<div><span id='SM_val'>";  content2+=SoilMoisture; content2 += "</span><br>";  content2 += "% </font></td> </tr>";
    //content2 += "<tr> <td>"; content2+=Tc; content2 += " &#8451;</td> <td>"; content2 += Tf; content2 += " &#8457;</td> </tr>";
    content2 +="</table>";
    content2 +="</th>";

// EC 

    content2 +="<th>";
    content2 +="<table  class='table_pad'>";
    content2 += "<tr> <th>EC</th> <td> <font size='4'>"; content2 += "<div><span id='EC_val'>";  content2+=EC; content2 += "</span><br>";  content2 += " dS/m </font></td> </tr>";
    
    content2 +="</table>";
    content2 +="</th>";
    content2 +="</tr>";
    content2 +="</table>";
 
// LEGEND FOR MOISTURE 
    
    content2+="<p class='lgnd'>LEGEND</p>";
    content2+="<p class='lgnd'>SM - Soil Moisture, EC - Electrical Conductivity</p>";    
    content2+="</div>";        
    content2 +="</body></html>";
    server.send(200, "text/html", content2);
  }});
 

#endif

//=========================================================================================================================

      server.on("/wireless_setup", []() {
        config_form = "<!DOCTYPE HTML><html><head>";
            config_form += "<title>'FloraLynk'</title><style>";
            config_form += ".centrated{    display: block;    margin-left: auto;     margin-right: auto;    }";
            config_form += ".tbl{    margin-left: auto;     margin-right: auto;    }  ";
            config_form += "p{text-align: center;}";
            config_form += ".stl {    font-family: Helvetica;    }";
            config_form += "#flr {color: #01686d;}";
            config_form += "#sbm {    background-color: #01686d;     color:#ffffff;     border-radius: 8px;     padding: 8px 15px;     font-size: 20px;      cursor: pointer;     font-weight: bold;     margin-bottom:60px;     } ";
            config_form += "#Readings {    font-family: Helvetica;    padding: 5px 10px;     font-size:20px;    font-weight:bold;    background-color:#01686d;    color:white; green border:    2px solid #000000;    border-radius:30px;    margin-bottom:60px; }";
            config_form += "#ip1,#ip2,#ip3,#ip4,#ssid,#pass,#blynk,#host,#port_ssl {font-size: 20px;} ";
            config_form += ".config { width: 20em; }";
            config_form += ".nextScan { background-color: #ffffff; color: #01686d;  font-family: Helvetica; font-size: 14px; font-weight: bold; border: solid 2px #000000; padding-top: 3px; padding-right: 10px; padding-bottom: 3px;  padding-left: 10px; border-radius:10px; border-color:#01686d; }"; 
            
            // Dropdown menu css
            
            config_form +=".dropbtn {  background-color: #01686d;  color: white;  padding: 10px; font-family: Helvetica  font-size: 14px;  Green   border: none;  border-radius: 8px;}";
            config_form +=".dropdown {  position: relative;  display: inline-block;}";
            config_form +=".dropdown-content {  display: none;  position: absolute;  background-color:  #01686d;  min-width: 150px;  box-shadow: 0px 8px 16px 20px rgba(0,0,0,0.7);  z-index: 1;  border: 20px;  border-radius: 10px; opacity: 0.85;}";
            config_form +=".dropdown-content a {  color: white;  padding: 10px 16px;  text-decoration: none;  display: block; border-radius: 80px;}";
            config_form +=".dropdown-content a:hover {background-color: green;}";
            config_form +=".dropdown:hover .dropdown-content {display: block;}";
            config_form +=".dropdown:hover .dropbtn {background-color: green;}";
  
            //Scan button css
            
            config_form += ".ScanButton {  background-color: #01686d;  Green   border: none;  color: white;  padding: 10px 10px; font-family: Helvetica;  text-align: center;  text-decoration: none;  display: inline-block;  font-size: 14px; border-radius: 8px;}";
            config_form += "</style><script>";

            // takes value from dropdown menu and put it into SSID field
            
            config_form += "function changeSSID(option) {document.getElementById('ssid').value = document.getElementById('WiFiNetworks' + option).innerHTML; disableDropdown();}";

            // disable dropdown menu on network choose
            
            config_form += "function disableDropdown() {  document.getElementById('Dropdown_networks').style.display = 'none';  }";

            // enable dropdown menu when mouse is over the button
            
            config_form += "function enableDropdown()  {  document.getElementById('Dropdown_networks').style.display = '';}";
            
            // scan WiFi function and get 25 Wi-fi networks
            
            config_form += "function scan() {  var xhttp = new XMLHttpRequest();  xhttp.onreadystatechange = function() {    if (this.readyState == 4 && this.status == 200) {";
            config_form += "var x = this.responseText.split(','); ";
            config_form += "for (i=0; i<=25; i++) {document.getElementById('WiFiNetworks' + i).innerHTML = x[i]; }";
            config_form +="}  };  xhttp.open('GET', 'avaible_networks', true);  xhttp.send();}";

            // Remove empty field from scan
          
            config_form +="function FilterSSID() {  var input, filter, a, i, w;";
            config_form +="input = document.getElementById('myInput');  filter = input.value.toUpperCase();  div = document.getElementById('Dropdown_networks');  a = div.getElementsByTagName('a');";
            config_form +="for (i = 0; i < a.length; i++) {w = i + 1;  txtValue = document.getElementById('WiFiNetworks' + w).textContent;    if (txtValue.toUpperCase().indexOf(filter) > -1) {      a[i].style.display = 'none';";
            config_form +="} else {      a[i].style.display = '';    }  }}";
            //config_form += "setInterval(function() {  scan();}, 30000);";
            config_form += "var c = 65;var timer;var timer_is_on = 0;";

            // Set autoscan WiFi timer
            
            config_form += "function timedCount() {  document.getElementById('ScanTimer').value = c; document.getElementById('timeR').innerHTML= 'Next scan after: ' + c + ' seconds';   c = c - 1;  timer = setTimeout(timedCount, 1000);";
            config_form += "  if (document.getElementById('ScanTimer').value == 65){ document.getElementById('timeR').innerHTML = 'START NEW SCAN'; scan();}";
            config_form += "  if (document.getElementById('ScanTimer').value == 61 || document.getElementById('ScanTimer').value == 62 || document.getElementById('ScanTimer').value == 63 || document.getElementById('ScanTimer').value == 64){ document.getElementById('timeR').innerHTML = 'SCANNING';}";
            config_form += "  if (document.getElementById('ScanTimer').value == 0){   c = 65;}}";
            config_form += "function startCount() {  if (!timer_is_on) {    timer_is_on = 1;    timedCount();  }}";
            config_form += "function stopCount() {  clearTimeout(t);  timer_is_on = 0;}";

            // Set interval for filter function
            
            config_form += "setInterval(function() {  FilterSSID();}, 50);";
            config_form += "</script></head> <body>";
            
            config_form += "<img class='centrated' src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAk8AAADECAMAAACx8TVuAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAbFBMVEVHcEwEKSqBk5RCXV7u9fUBaW7////v8fHj5+ft9PXBysqpzM7e5uYSNTagrq5heHnP1tcQcnczh4vk7/Dz+PgifYHL4eIiQ0NYnKBDkJRuqayYwsSQoaGwu7y619gyUFFRa2xxhoYzUVKEt7nx";
            config_form += "JtEGAAAACnRSTlMA////bf//SSKS6GGHCwAAEv1JREFUeNrtndmCo6wSgFVPtA/BNTEmRrO+/zv+CahBZXObaHfVxYxtFBE+i6KAwjAYuWRxEm0WLVESH7f/M0AWLzhbOEqMnC5QXwun6bhZlcRA1KJxSjZrky3U2mLlEG3WJ2eoN8BpSnlCzS1Tks065QBVt0Q5rRSnTYSh8pYn581q5QS1tzyJ1svTBrwGi5NsxThtYqg/UE9gkv9mX8GqcdpkUIPQ3E0oCdQgOAumFKjBZUm8cp52UIVgjoNBDjzBNAMQDdkA";
            config_form += "TyDAE/AEPAFPwBPwBAI8gQBPwBPwBDyBAE8gwBPwBDwBT8AT8AQCPIEAT8AT8AQ8gQBPIMAT8AQ8AU8gY3mKDifgCWQynraOc46AJ5BpeMqcl+AYeAKZgqfYofIEnkDG8xTtSp6cQwI8gYzlaevUsj8CTyDjeMocVr5klgNPv4Wn2GnKd9o84Om38IRbPH2nzQOefglPT6crT+AJeJqmtaNyiYAn4GmUq6ApuwR4Ap76y9kRyL82ooCn38DTyRFLBjwBT9O0dl+wyoGnX8DT2ZHKFngCniZq7f51N683T/nNM4mk";
            config_form += "d3rCrAVoaMqQkhlShzsVT86/2wGtH08oMDuFtESe3L/E09NRyz/zG/TiyTLNdfBkmEvI1b/hKXZ0ZBcvjifXNoGn5fF00OLJ2cdL4+lmAk/L4ylzdOW0LJ4sE3haHk/RXpun/WlRPJnA0wJ5Ojv68i+aPG2eAuBpgTwljrMsoLZD1JNn/IwrNeBpIp4uTj+gkqXw5AtrCHj6Hk8np6fsooXwFAgLB3j6Hk+4L0+ze8p1efLqskmBp6XwdHT6y2UZPInLBnj6Gk94AE8zzzb4Vzy5eLp6QgjhxfDkhl/jSaie5C7z";
            config_form += "bN08uf7dfpCG0g4sbaqQ9R7dsTpp2dWgjxcUtKNgVeKP4cnVONN5VhGk9Bm2hQby5I7gSaiezt9zlGvwdLdtCgSVh01Es9TCwGs4rWxfB9e8vKnJE26l5VmuYXwGFe2+PLHjkb60R2u3b3idcK1GblJfk6eQve02Qj+Jrafj97wG237GCKeIZDVXPLo3EgqkRe+n1R8sT9jmpIXG8ISYqx6dX9msF12efK+dmUeow5PLpnt1R/CExbwoTCgcrZIn9869s1vwzUQY14Ql+KwZvTKCJ+PKXNbOE2Z1T0eh2bxX80IN";
            config_form += "ntivwnNH2E9iJbTb7L9mk8/J08MUSS7hiR3YsbitT7MSR/DEJhpIhr8tDgvcvGAlT3cJf/14ukh4Us5hOa6PJ9+TlPzNFT2iAaElHj3sSn+ejFSsK5ifTKzJUyML3JJpfBbFmP6dZBod1hglTlbHk7zkb1oXVzwV5jw8WUKLPOTlVclTM5FuHhqNtj/KX3CW8aSeFHVYG0+eouStHjy53kw8sQk/hNMpCn2eHlKe3FTSwPbjSTbv6aAzBzhbFU/uQ1n0hT5PN3MmnhrYhILPITVEPHl2YLdQD2U82UqctHnKpLon";
            config_form += "+tpUg5l4ato71xyFP6gIxL0hKU/N1i61yI3Yv03AExbUsM/VpE2eriHHj3GX8BRIPQX9eMLytuzwrXEXTf943tXdMp5Qo55r3/GPpVH/lfv7xSCnn+gxVge6juaJVX2sRc6y6vJ5smSeT17JsJel7rjxllhhG+lM24y/yFPP8ZZA8MUayON0m7o8pb7L1yFXLGyuhvGEuAayy1dbtsD+C/iPa54MFZ6CXjw9FTzprFI4rIcnsasJe9yaauIUuIJOmPcjVi/DeGJ9mg+eOjYRl6dU9FKikmn0KdDY8eCdAhStRXnH";
            config_form += "tfBkSQaoCm4DI+n6sZ6gTj38pGN58nnG9IMPji36TGw1Tw+Vp6APTyel4tHhCa+FJ0+sUBqNQ87lqYVTKHIXtnEYxhOLa8BpYXM+Ns0G667kKRC/4ACezkqeDl9SULPwJDCeOpXFm6TQgQbJHcreWJ7yrsK8c61xkRpqKmSfWzK52lPQh6edkqfzlxTUzDxh6TSRB6/+C7EPW95BG8gTa9j4QpUl58ngKh/mJNulGD+fLlZjojcV+LgKnizhXHMhIKb4FkvW3LHqZSBPrDayO32+kM+T3Y8nDcdTH56eap70FuYd";
            config_form += "VsHTXU6Az2lMJK3BTWzbNyt/KE+47cIIRMpkCp7CCeb7KoyjnbpJnM0HNQdPttxUQJz+msRYteV27HieWH7urbEWf3Ke/PE8qUZTCE9bLZ7Ov54nibkVzMQTYlc+N/RnaxLLFDwpFNQUy6T2+nFX9tFvb+8k2sOeiSfWzvYbPtLAmJ6n1B3LkzIgnYbNPpdFPq89/uhvj2tdPqk93nJiYbEumYQn/mfRh6eDDk8bPZ62K+AJ6foLrj15CufwF7QcBDgXd+2n4YnnlOvFkzLkU6QfKmPqBm9mnqze/kyZdZN3U/vx";
            config_form += "puCJYej+ENvOE/E0crxF3ZIlWq3iLIvxZh5vSaXunlyHJyyc7z/NeEvLp+k1jfNZeBJPLzAmMMdLnjRjrzxXwNNdotsRdyxDVv1Xyejy+PHg7giLOO+jeGLn+j7cETw99bxKkfMNl+YsPGHx8MmPar6KtDGStEDjeMI8nvCUPPnydcE9eDprtmGagX9XNp/OayAQpqr5dJzGSGh6/NjmRDzxFmTZxoQ85a21UtZwni6aPGkaUPEKeGrM9w0+3OSe3tRY8aQ5NjU/NSfjKdSxmofzFHQa1WIwT1jTqXT6hgdqpvgq";
            config_form += "zbUCQYEMA6NmAAkv1OUJtfRGjhBCfpDqrUeQiKjl5FvjI3iyu8/wBvOkjhCdbXQNqP05WUN7Z7jXsUsflday7nopTZ4K+QLBcTxVpaK2yTWqTLvPpjSgcDZ5YIy54j+FqiWYd0OfJw06R/NkpCprfDxPzVIJ5uZJNa1Fz/UUJUkcn4hE3+NJtUTcNnrwxLNuJufJly6In4an5kPyQTxF2rMGpAbUTmI3RfExO18OB7zb9/V9zhefThoPw3Z78SQOrzIdT61F7cUsPDWbbjSEp0R/UE5iaT0FuiZ+ni+7Eab7jPEO";
            config_form += "Q3Erlesnr2o/val4am5Qkxrz8DQ+/pOGfrqoPAsXnhUeHc+H0V3BOeNnuoIoO2lo9ObJwHw61fHEtHnCSv/QFDw1bfJ5eMKKIAdPHkyX/RSuhXnjsRYcCLzANQbw1I5XSasVG9Px1PBp4rl4Utjkk9jjO2nTyNln6rjdT+Sqmju+b9jSUVff7Zc8S5TfCliADGNKnmzVeMgkPDVtQX8O/5MjW1fV2akzOevueTYdTy73UO/WPLBTGt3mZoUjk0dWmdj15rta2dUXrLUgXP95g8psEv+4E4mH+nDLdIq3jrZMqJ/G";
            config_form += "Cg6NpctdYY3/E5lk/K6OZnhU4RT32qBqQTwtX1h/Qb5knjSW/p5EtnsziliydXrJCXjSF1+wyHxxPGU9FMlFtqFitneAp9kk1Yww8HWeNOYNZAL22KCZUc+9GIGnXoKGW+P/mCf9Aby2x4DdqyzZ9cZJZ6oU8NR1PqXGsnm66A7gteYYsMZTvHeApzn7n/pLwr/O01N7wKV5LdPanYbgpBMEH3jqDN557sJ5Ui+YOvCuxSO108etBTwpxVuENa4ZX0Vp+ux51368R8kwnHSWLgBPHWdBuHienj00ybk9qKe/8kU0";
            config_form += "LAg8KcXWjR+3CJ7iHpbOsWs9PYfhpBUeEXh6S7gQa1w3fuahh6do31ZZ8UCctLZGB55azoKvWuO6PB21HZp1g3cZ29rpxWIBnt5ys2uxjBXwpLTInx13eqYf/mBMMDvgaVmiyVPWQ5XsmxYVHsxTBjz9Vp5UVFza8xF249WTVmgf4GmdPJ30u/ZxQ2FdhvOUAE+/lidV/N62sZXpr7YaFYkFeFopT9FeW5c8Gee4TtjfPR7szgSe1sqTwhA6tRq8WL+52z5HhG4FntbKk3zeb9Zyf0a6i2Pw6TwmMiLwtFqepLrm";
            config_form += "3PIuaPrG95kY0xPw9Lt5ig6aYyNRzZPCfLokm/OY2SrA05p52kQ7zbkA571OG7k/yoDbbYCnX86TbBJ4Q5vEe40mchtJjfwt8PTreZIA1ZzrjZVjwbuTwkt6BJ5+P0+bBGtVfxXYcCdTTvJpwBHw9Ad4Ehrlzd59pOApU84q14x7DzytnCfRusxtj2FkGnJFviLvCTz9DZ4E83d3+jxdiPKK8BRh74Gn9fO0Oe107Z2DWPNcRs8d1+cJI1QHbMNFUR7TmbEu+SlERMjfqHF1dYzrk+wREbdMALcfWiZZPokcY0TW";
            config_form += "n4TkJsRZixIWqDVl1xX+8UrOlV/xyZKLirA6+uQVVWvTw+qIvh0mmXDLy1z6KiG57nMbrl7+87LDon5HZ02FchBGq8um2YZKhycaB/NOS55ESbqRY0RCsiMSks2uA7EhJry4e/vcadWh29ijOl4AScBjZ9tan9huN7JIgBxjsrMApvO8aZw4m9knhmwQ49F4O7ZPfyWJlcsMCoumVIeS477D59XTCiiyVciDEIWYEHbl5eGjzgJ9O4u8FCrfFH2i2tHs0Nssk7mLPntoHPmuisq0ZrkcEr0gG8mEPD1M289TWoA3";
            config_form += "8+r7tvmgpfSuDFpmvhWYtmWRs6//r3SZyPtO/2rexDy9rrUsTMo5sKyUDXNvvc+QJF8/+nUlBO/jgCZR8fS60COAhZ4ZFLlnliQFH57oqdAreXqnTHQEiQ77eYeU5obJQ4nTi/XiRilG76tuNEgUzRN+P/ZOH8vjCb9fzSJMN3l6Z5xEB6pedvjOBM+9xmzvM9d0Us5+0VraosuTT2rNvYekgK6f+kVkE5bPN8j8X9AKoiRdSdnyeaoVkv2+6Cc1MVOX6PPjG7Sy7szUKNVTzZPxji6HCGw5oYb8bJO1vhVPXmgY";
            config_form += "L9wsRqvQQ+YdjFZgzBcnNEOuR/Zb8TF9Q9txEL20Ypw8lqDI46nmqM0Tcpx7WTpouP3EjQ520DDdt7prXo4T8hQwsd0telyQz/YFl+mFXJ5y8kdAC6koS5jHU2rbtl/z9KKCfdbj8+P19cSSgsD0S/XE8mSTdb1edQltQj3z4VY8vbLqPsyKp/dSFpqEZ3q+iKfAtKmCQi+e0VuB+BVPOf1WaJ48k1Vp/XiyS57oy47aPCU+qKZTHvlGkSrk3W4zIU83Jh5S+RkhUpjItHzTDDo81Vuz3tQ81XZMVaxFy34qf3Sv";
            config_form += "5qPkCZtptUqOsZ+o4iy3bbqXPP0E5uNRVt3VTB+vfxj7qUwCefU7tHjCZuqkJRh3mh/7Yw2hDxjmUJ7qjFcvO3I7ntNFbpDHXAurz3K+8Tzl9BMt3E8LRvX7u7Te0bTbPKV22Urk1By6MXaFvL0zHmbIb+/omstKazSr3zavqXl1P+z/UJvkfdfdrKrOeh0GqNve2cw7tHh6m4Q2OYVpUCir/I48u9qQo+wvkMe6xeedbDFPYc299cn4BO1dm6hMHovsE4ZcZTw5u2hKntzUvIfhjX5HV/MWhnczrXtFYWp227uA";
            config_form += "Iuh6plXfaZlB2f3vHNHKzxG6sfVpmXnlLygtJLNSG9Ui3rq9e+WxoA/3/LB4UI7JXX7NkxFc3Zqnusv/TsK9cnkKmXB1d/NRGL5nUkrsd0+C4al6rEVzh4zcTOn5oPEK5Bt7YLcsnxdEbtlzqV52M17i805sQx84YciVc4B7bLmo07+jG7HQ76g8Dj8EudcuT1Up0e1dqy+wamQ6R5+49Ow+QYy/gFaGX2mVwDJaPL1NcJInugtRwKiEwqt4cl0DmW1/Ae1s2Dye7Joeo1ySTvv25Izd0HTsY8l+xx5i/AQsT/RF";
            config_form += "P0qpzPhYfwFHSe1lHbxdor94/bKZlqd3T9qq7ZrCqnw5mPa43ZD9i/4fWnn7TmLOUo9C+wjTC6284Z78XPb6kT664YSsvUr+O4GifI6bW1XUfN9iE0NMJmnKxD1avkzIJljq5fIdcot+Sa/84fIN/fev5LRVPoN5bOPK9iuUBVg6aD8Zr152M5EkzwPXgDp2Qh8qW7t9MjlPIIsebxE4zXlWT9KJG72dsLUDnn4xT+JIGixOyujT2w3wBDzJVlmxG0xFqkgtOAKegCdp4IOj7iK+vsYT8PT3eNo0Ng5Wxqo7bYAn";
            config_form += "4EnqntJdEtrbFgee/iJPfaL+xBvgCXjSF/kU30OyAZ6AJ33JRofLBJ6Ap6YvStjWHTfAE/A0el1M6SZ4DkwTePqzPIkH7i7HwYkCT39XP8XZhYPUIYtGpAk8/eX+3Zup53l7Oez2zn53uJyzUzQuPeDpb/M0tQBPwBPwBDwBTyDAEwjwBDwBT8ATCPAEAjwBT8AT8AQ8AU8gwBMI8AQ8AU/AEwjwBAI8AU/AE/AEPAFPIMAT8LQ0iVbO0wWqcFESr5ynHVThouS0cp6gBpcl2bpxSqAGlyWHdfOUQQ0uTJJV83SA";
            config_form += "CoQGbzqJoP6WJj9r9hiA92l58lwvTjHUHrigJmztwHpaouzWapKDb3yZgtdpQp2h5kBDTdfYgS2+YDmuDacEbKdFy3ZdbV72f6iypTs210PUCZTTGuSSxcnCoYqS+LjFUFVLlf8AuGavJssTOmwAAAAASUVORK5CYII=' alt='Logo' width='422' height='140'>";
            
            config_form += "<h1 id='flr' class='stl'><p>floraLynk Mini v1.0</p></h1>";
            config_form += "<h3  class='stl'> <p><label class='stl'>Private IP of your FloraLynk device: </label> <label id='ipStr' name='ipStr'  class='stl'> </label></p> </h3>";
            //config_form += "<center><button class='ScanButton' onclick='scan()'>SCAN</button>  ";
            config_form += "<center><label class='nextScan'><span id = 'timeR'>Point at <q>Wi-Fi Networks</q> to start autoscan</span></center><br></label><input type='text' id='ScanTimer' disabled hidden>";
            config_form +="<center><div class='dropdown'>  <button class='dropbtn' onkeyup='FilterSSID()' onmouseover='enableDropdown(),startCount()'>Wi-Fi Networks : <span id='WiFiNetworks0'>-</span></button>  <div class='dropdown-content' id='Dropdown_networks'> <input type='text' id='myInput' onkeyup='FilterSSID()' value = 'undefined' hidden><input type='text' id='myInput' onkeyup='FilterSSID()' value = 'undefined' hidden>";
            config_form +="<a href='#'onclick= 'changeSSID(1)'><div id='WiFiNetworks1'>   Wait for a scan  </div></a>";
            config_form +="<a href='#'onclick= 'changeSSID(2)'><div id='WiFiNetworks2'>undefined</div></a>";
            config_form +="<a href='#'onclick= 'changeSSID(3)'><span id='WiFiNetworks3'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(4)'><span id='WiFiNetworks4'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(5)'><span id='WiFiNetworks5'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(6)'><span id='WiFiNetworks6'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(7)'><span id='WiFiNetworks7'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(8)'><span id='WiFiNetworks8'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(9)'><span id='WiFiNetworks9'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(10)'><span id='WiFiNetworks10'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(11)'><span id='WiFiNetworks11'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(12)'><span id='WiFiNetworks12'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(13)'><span id='WiFiNetworks13'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(14)'><span id='WiFiNetworks14'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(15)'><span id='WiFiNetworks15'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(16)'><span id='WiFiNetworks16'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(17)'><span id='WiFiNetworks17'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(18)'><span id='WiFiNetworks18'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(19)'><span id='WiFiNetworks19'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(20)'><span id='WiFiNetworks20'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(21)'><span id='WiFiNetworks21'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(22)'><span id='WiFiNetworks22'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(23)'><span id='WiFiNetworks23'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(24)'><span id='WiFiNetworks24'>undefined</span></a>";
            config_form +="<a href='#'onclick= 'changeSSID(25)'><span id='WiFiNetworks25'>undefined</span></a> </div></div>";
            //config_form += "dropdown() </div></div>";
            config_form += "<h3 class='stl'><form method='post' action='config'>";
            config_form += "<table class='tbl'>";
            config_form += "<tr><th align='right'><label class='stl' for='ssid'>WiFi SSID:</label></th>  <th><input id='ssid' class='stl config' type='text' name='ssid' length=64 placeholder='Name of your network' required='required'></th></tr>";
            config_form += "<tr><th align='right'><label class='stl' for='pass'>Password:</label></th>   <th><input id='pass' class='stl config' type='password' name='pass' length=64 placeholder='Password for your network'></th>   <th><input type='checkbox' onclick='showPass()'>Show Password</th></tr>";
            config_form += "<tr><th align='right'><label class='stl' for='blynk'>Auth token:</label></th><th><input id='blynk' class='stl config' type='text' name='blynk' value="; config_form += DeviceToken; config_form +=" pattern='[-_a-zA-Z0-9]{32}' maxlength='32' required='required'></th></tr>";
            config_form += "<tr><th align='right'><label class='stl' for='host'>Host:</label></th>       <th><input id='host' class='stl config' type='text' name='host' length=64></th></tr>";
            config_form += "<tr><th align='right'><label class='stl' for='port_ssl'>Port:</label></th>   <th><input id='port_ssl' class='stl config' type='number' name='port_ssl' value='443' min='1' max='65535'></th></tr>";
            config_form += "</table><br/>";
            config_form += "<input id='sbm' class='stl centrated' type='submit' value='CONNECT'>  ";
            config_form += "</form></h3> ";
            config_form += "<form  method='post' action='/'>  <input id='Readings' class='stl centrated'  type='submit' value='SENSORS VALUES'>  ";
            config_form += "<tr></tr><tr></tr>";
            
            config_form += "<img class='centrated' src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAARIAAACICAMAAAAcY5SAAAAABGdBTUEAALGPC/xhBQAAAAFzUkdCAK7OHOkAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAA/UExURQATfzAwME9ZkXh4eBAQEPP0+fz8/f//";
            config_form += "/xEPEA8ODhopfhgYGLCwsCkoKDA8hZmZmWJiYkdHRzxHi4qKin9+freUQDcAAAb7SURBVHja7ZzbYtsgDIbphq1fSdv0sPd/1l34EGzAIAFO1sFVYnMQH0ICQWKop10yHUFH0pF0JE+OZDRLijW+pOFJkfxaU2bZJfuQ6HAESur9w5EYqYSiHof0QNCiMUYF8Ncua5Y+Gm2jl032MdkN8fsDInlMPNHe8wrH";
            config_form += "kLyLlCTUTD0kRoEklDd3AGLNPg8SI0cSzpvbnlY3T0NixEhieTPrMNr5ehKSUUwknnn/YvgnkQzlRIzUShutWT8FyUVKZDQiJCaxVCsajSZIjJTIcf7Qy1+HSHzCl8ciqUPESOsz2hVReySViJjs96Ftn0SA5khqETH5GUI7YYEIrZEEhX9XEEkjGWRIxgchyZE9YwGzrThPTYx2g9UWSVDykQoMSX6udJlH";
            config_form += "IClexB+txFI598W+crW1JZI6RGJKNSTyZpi105HIiVCxbzqMver0ryKSKkRyo8Kh9rX1N0OiICKP9tC/hOQcIocxBK0atkEyaojootUiJFlMhNBS7wdleEQfvx9jxUwk1p/0xGkxhpRD9MpfVERIGklPkDRaC648WZEvQJPFBq0oIiQZTOojURLRSzKGSxpSMqmOREukQJInR6ImQnpBRhGSJJPKSIx871tB";
            config_form += "kKAPkRwpNUSSGfBKF72UNJtCMhx7Ytm6ZNARkVvXoqFIIUmoSVL0sTDYkTvqZyIhSdCuZI6bo/RcSA6Z1ENiTBGTH4jEmCImQ+rwri4SwZmzFsmYJJLwxEOR45MjOWBSB0kGkUQ/389GEvfEVZC8G1PK5HQtiatJDSSRiSLyxIMRrH+qIIkyqR1jcjKW3Oo4A0lMwFZIxAH3n4RkiJsN/RWGU5BQ1kjWQXKJ";
            config_form += "Njk+FZIwkxZIBs1ZlSmwr0aLJCh7AySj6vyuXrzkko+EMtxCBST5R5TVZo4sqpZiUh2J6szxzNhr+d6sEEnFmy4/Bkk2k0ubc5xRdXfnbCRjVTX5OkSSsXc6H4n29o/RKckOSUaVw/lI2jJJ3BxQX3triyTXE7e4X6Iai/ZIctVkbHALqTwC1gYJadVEHQaSIaEHIGnDZMy40ahbJz41EuGZgBYJnY+khEnR";
            config_form += "7ehcJJfzkRQx8RZ3Q4b0MiT0ACRDxsW5TMliWYYSJHQ+kjI1MRm/xwn++CQfyXg+klIm0uGUIqEHIClkIhVdjISeF0nVX4CKln7nIyljIhZcjuRyPpIiJkcvQ5cvFEjoAUiGrM4k1uq5JkkTlzofiT46fYSE6iGh85HU+AlzLtN/BYkkTjRkIDk4UdUFdJ8bSSS8nuvelDFu8VWJ8mM6ZZTo4leQKKg8QxyT";
            config_form += "RdJR/7Jh+Mo7zB53xxTpa/lGJ500BP6VyiKukZol/+rZO/3nqf+TZ0fSkXQkHUlH0pF0JB1JR9KRdCQdSUfSkXQkPXUkHUlH0pF0JB1JR9KRdCQdSUfyPyKxAAMMgGGJ6OWeXtdMzGzXL29vu08vL+s75+NvMK6HLTOz9wj4XL+BaSvQUjnADHArJAzAMjOzBYiIwQwGg+2M5IbpO9YSW5kJ966tn3iSGthm";
            config_form += "3bbL7D5+ZQZg9w2BGRbMqwSWMdWOlhMH1hHVUyN+m4YGy4O1GNZB25W2wJ+ZjNcCEdF1LuKQtjx9/LRrGQ6UXFslmZ6IkSBEh4iI/qwyX+dcvpbchVsgrXk+1jw7Hdnr1Tecaq9Omb0y3EEs+VojYRuT4D5Pdt1huz5bkThmCL7yud30QK/1gkNqey97s9wOiWPn9hI4r173T+69mTs+P7HwmB4gsSE1ujfk";
            config_form += "aYm9kiKJtSSKxP06eub1PsCb+bGZ5AH1Z9z2TYRs5VSN9W0JTkDidJz595KCnmIyHcyT317H+eYO6UahLW4+kq39DrdDiwX+cAUiugIMz+hWR2KdobGz20fQAU1I7Oyq73rAfHV64WR/sy+e4rCnDIyYhQODYQG4Zn3y4CdpyW64olPcKeZMjQDHKz6OkbCLhMHrEm6qlzniEWRMCmzJDklAINcLOqJ/Mq+9";
            config_form += "8M3rBq0NaIn1xyFsXl3d5kdoyYYJdorD2PhifIZ6MeuQ2+XbxuPik4gsXrw2OeiE6SOCtqF5ja/ipj0L/NUr3Z3xvDTf1701hy53b7XjMA6sHT/d4UPDiWOjWmIdAjasJUvpK2AXSC9Be0O7hTAR2aVWf23ECKottr7qhIkDz3RMT74tf4QX9Ksd4LVfi3/+njdI+2rXLYC7MZw/vWJrS2xsWcIST1xkXhmW";
            config_form += "4Tg9TGsQ4G0PbSP6dm8z7a4t8Huzn78zYzDAbnxgcuvMu23ftAsGs2W8LlvF6+2DGS09zkZpAYCtu3GfQioBj4PdtLNuRZadYeQ55OBukLz9PcCw+0iDnRZBAM9IJtzuOqVH1YiIvl97oLHHXjuSjqQj6Ug6ko6kI/kp6S+KwRkMoSN6VwAAAABJRU5ErkJggg==' alt='Logo' width='100' height='50' >";

            config_form += "<p class='stl'>NIKO Technologies. We deliver technology.</p>";
            config_form += "<script>function showPass() {  var x = document.getElementById('pass');  if (x.type === 'password') {x.type = 'text'; }  else {x.type = 'password'; }}</script></body></html>";


      server.send(200, "text/html", config_form);
    });  

//=========================================================================================================================

  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverUpdateForm);
  });

//=========================================================================================================================

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    if (!Update.hasError()) {
      server.send(200, "text/plain", "OK");
    } else {
      server.send(500, "text/plain", "FAIL");
    }
    delay(1000);
    restartMCU();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      DEBUG_PRINT(String("Update: ") + upload.filename);
      //WiFiUDP::stop();

      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(BLYNK_PRINT);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(BLYNK_PRINT);
      }
      BLYNK_PRINT.print(".");
    } else if (upload.status == UPLOAD_FILE_END) {
      BLYNK_PRINT.println();
      DEBUG_PRINT("Finishing...");
      if (Update.end(true)) { //true to set the size to the current progress
        DEBUG_PRINT("Update Success. Rebooting");
      } else {
        Update.printError(BLYNK_PRINT);
      }
    }
  });

//=========================================================================================================================

  server.on("/config", []() {
    DEBUG_PRINT("Applying configuration...");
    String ssid = server.arg("ssid");
    String ssidManual = server.arg("ssidManual");
    String pass = server.arg("pass");
    if (ssidManual != "") {
      ssid = ssidManual;
    }
    String token = server.arg("blynk");
    String host  = server.arg("host");
    String port  = server.arg("port_ssl");

    String ip   = server.arg("ip");
    String mask = server.arg("mask");
    String gw   = server.arg("gw");
    String dns  = server.arg("dns");
    String dns2 = server.arg("dns2");

    bool save  = server.arg("save").toInt();

    String content;

    DEBUG_PRINT(String("WiFi SSID: ") + ssid + " Pass: " + pass);
    DEBUG_PRINT(String("Blynk cloud: ") + token + " @ " + host + ":" + port);

    if (token.length() == 32 && ssid.length() > 0) {
      configStore.setFlag(CONFIG_FLAG_VALID, false);
      CopyString(ssid, configStore.wifiSSID);
      CopyString(pass, configStore.wifiPass);
      CopyString(token, configStore.cloudToken);
      if (host.length()) {
        CopyString(host,  configStore.cloudHost);
      }
      if (port.length()) {
        configStore.cloudPort = port.toInt();
      }

      IPAddress addr;
      
      if (ip.length() && addr.fromString(ip)) {
        configStore.staticIP = addr;
        configStore.setFlag(CONFIG_FLAG_STATIC_IP, true);
      } else {
        configStore.setFlag(CONFIG_FLAG_STATIC_IP, false);
      }
      if (mask.length() && addr.fromString(mask)) {
        configStore.staticMask = addr;
      }
      if (gw.length() && addr.fromString(gw)) {
        configStore.staticGW = addr;
      }
      if (dns.length() && addr.fromString(dns)) {
        configStore.staticDNS = addr;
      }
      if (dns2.length() && addr.fromString(dns2)) {
        configStore.staticDNS2 = addr;
      }

      if (save) {
        configStore.setFlag(CONFIG_FLAG_VALID, true);
        config_save();

        /*/content = R"json({"status":"ok","msg":"Configuration saved"})json";*/
      } else {
        /*content = R"json({"status":"ok","msg":"Trying to connect..."})json";*/
      }
       content +="<html> <head>";
       content +="<style>";
       content +="#data_saved{ font-family: Helvetica; font-weihgt:bold;  text-align:center; border-radius:10px; width:500px; background-color:green; color:white; border: 5px solid #000000;}";
       content +="#auth_wrong{ font-family: Helvetica; font-weihgt:bold;  text-align:center; border-radius:10px; width:500px; background-color:red; color:white; border: 5px solid #000000; margin-top:20px; }";
       content +=".centered { position: fixed; top: 50%; left: 50%; transform: translate(-50%, -50%); }";
       content +="</style>";
       content +="</head>";
       content +="<body><div class='centered'> <h3><div id='data_saved' > <p>CONGRATULATIONS</p> <p>Your data are saved!<br>Now you can connect to your WiFi network <br>&<br>and access your FloraLynk device interface on <a href='http://"; content += mDNSname; content +=".local/' style='color:white'>http://"; content += mDNSname; content +=".local/</a></p></div>";
       content +="<div id='auth_wrong' ><p>ATTENTION</p><p>In case of wrong authentication,<br> you will be reconnected to this hotspot!</p><p>In case of successful authentication,<br>this hotspot will be destroyed!</p></div></h3> </div> </body> </html>";
       server.send(200,"text/html",content);

       BlynkState::set(MODE_SWITCH_TO_STA);
    } else {
      DEBUG_PRINT("Configuration invalid");
      content = R"json({"status":"error","msg":"Configuration invalid"})json";
      server.send(500, "application/json", content);
    }
  });

//=========================================================================================================================

  server.on("/board_info.json", []() {
    DEBUG_PRINT("Sending board info...");
    const char* tmpl = BLYNK_TEMPLATE_ID;
    char ssidBuff[64];
    getWiFiName(ssidBuff, sizeof(ssidBuff));
    char buff[512];
    snprintf(buff, sizeof(buff),
      R"json({"board":"%s","tmpl_id":"%s","fw_type":"%s","fw_ver":"%s","hw_ver":"%s","ssid":"%s","bssid":"%s","last_error":%d,"wifi_scan":true,"static_ip":true})json",
      BLYNK_DEVICE_NAME,
      tmpl ? tmpl : "Unknown",
      BLYNK_FIRMWARE_TYPE,
      BLYNK_FIRMWARE_VERSION,
      BOARD_HARDWARE_VERSION,
      ssidBuff,
      WiFi.softAPmacAddress().c_str(),
      configStore.last_error
    );
    server.send(200, "application/json", buff);
  });
  server.on("/wifi_scan.json", []() {
    DEBUG_PRINT("Scanning networks...");
    int wifi_nets = WiFi.scanNetworks(true, true);
    while (wifi_nets == -1) {
      delay(20);
      wifi_nets = WiFi.scanComplete();
    }
    DEBUG_PRINT(String("Found networks: ") + wifi_nets);

    String result = "[\n";
    if (wifi_nets) {
      
      // Sort networks
      int indices[wifi_nets];
      for (int i = 0; i < wifi_nets; i++) {
        indices[i] = i;
      }
      for (int i = 0; i < wifi_nets; i++) {
        for (int j = i + 1; j < wifi_nets; j++) {
          if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            std::swap(indices[i], indices[j]);
          }
        }
      }

      wifi_nets = BlynkMin(15, wifi_nets); // Show top 15 networks

      // TODO: skip empty names

      char buff[256];
      for (int i = 0; i < wifi_nets; i++){
        int id = indices[i];

        const char* sec;
        switch (WiFi.encryptionType(id)) {
        case WIFI_AUTH_WEP:          sec = "WEP"; break;
        case WIFI_AUTH_WPA_PSK:      sec = "WPA/PSK"; break;
        case WIFI_AUTH_WPA2_PSK:     sec = "WPA2/PSK"; break;
        case WIFI_AUTH_WPA_WPA2_PSK: sec = "WPA/WPA2/PSK"; break;
        case WIFI_AUTH_OPEN:         sec = "OPEN"; break;
        default:                     sec = "unknown"; break;
        }

        snprintf(buff, sizeof(buff),
          R"json(  {"ssid":"%s","bssid":"%s","rssi":%i,"sec":"%s","ch":%i})json",
          WiFi.SSID(id).c_str(),
          WiFi.BSSIDstr(id).c_str(),
          WiFi.RSSI(id),
          sec,
          WiFi.channel(id)
        );

        result += buff;
        if (i != wifi_nets-1) result += ",\n";
      }
      server.send(200, "application/json", result + "\n]");
    } else {
      server.send(200, "application/json", "[]");
    }
  });

  server.on("/board_info.json", []() {
    DEBUG_PRINT("Sending board info...");
    const char* tmpl = BLYNK_TEMPLATE_ID;
    char ssidBuff[64];
    getWiFiName(ssidBuff, sizeof(ssidBuff));
    char buff[512];
    snprintf(buff, sizeof(buff),
      R"json({"board":"%s","tmpl_id":"%s","fw_type":"%s","fw_ver":"%s","hw_ver":"%s","ssid":"%s","bssid":"%s","last_error":%d,"wifi_scan":true,"static_ip":true})json",
      BLYNK_DEVICE_NAME,
      tmpl ? tmpl : "Unknown",
      BLYNK_FIRMWARE_TYPE,
      BLYNK_FIRMWARE_VERSION,
      BOARD_HARDWARE_VERSION,
      ssidBuff,
      WiFi.softAPmacAddress().c_str(),
      configStore.last_error
    );
    server.send(200, "application/json", buff);
  });
  
  server.on("/avaible_networks", []() {
    Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    NetworksTotal = WiFi.scanNetworks();
    Serial.println("scan done");
    String AvaibleNetworks = "";
    AvaibleNetworks += NetworksTotal;
    if (NetworksTotal == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(NetworksTotal);
        Serial.println(" networks found");
        for (int i = 0; i < NetworksTotal; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            AvaibleNetworks += ",";
            AvaibleNetworks += (WiFi.SSID(i));
          }
    }
    Serial.println("");
    server.send(200, "text/html", AvaibleNetworks);
  });

//==========================================================================================================================

 server.on("/calibration", []() {
 for (int i = 0; i < 1; i++){
 content2 = "<meta http-equiv='refresh' content='5000'><h1 style='color : #000000;'><center><!DOCTYPE HTML>\r\n<html>floraLynk</h1><center><head><style>";
    content2+="#connect {font-family: Helvetica; font-size:16px; font-weight:bold; background-color:01696E; color:white; border: 2px solid #000000; border-radius:10px;}";
    content2 +=".table_pad {width: 20%;font-family: cursive;font-size: 15px;color: #FFF;text-align: center;}";
    content2 +=".table_pad th {border: solid 1px #FFFFFF;background: linear-gradient(to right, #01696E,  #01696E,  #01696E);color: #F0FFF0;line-height: 20px;}";
    content2 +=".table_pad td {padding: 15px;border: solid 2px #000000;background: #FFFFFF;color: #000;line-height: 4px;}</style></head><body><table class='table_pad'>";
    content2 += "<script>setInterval(function() {  getData();}, 3000);";
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
  server.send(200, "text/html", content2);
  }});

//=========================================================================================================================

server.on("/calibration_values", []() {
    String calibration_values = String(RL1) + " | " + String(RS_co2) + " | " + String(CO2_ZERO_AVERAGE) + " | " + String(voltage) + " | " + String(cVR) + " | " + String(CO2volt);
    server.send(200, "text/plane", calibration_values);
});

//=========================================================================================================================

server.on("/save_new_CO2_zero", []() {
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
  server.send(200, "text/html", content2);
  });

//=========================================================================================================================

  server.on("/reset", []() {
    BlynkState::set(MODE_RESET_CONFIG);
    server.send(200, "application/json", R"json({"status":"ok","msg":"Configuration reset"})json");
  });

//=========================================================================================================================

  server.on("/reboot", []() {
    restartMCU();
  });

  server.on("/web_values",[](){   
    String web_values = String(PPM_AVERAGE) + " | " + String(TC) + " | " + String(TF) + " | " + String(RH) + " | " + String(VPD) + " | " + String(DP) + " | " + String(DPf) + " | " + String(Lux1) + " | " + String(SunL) + " | " + String(MetH) + " | " + String(FluF) + " | " + String(HPS) + " | " + String(TMc) + " | " + String(TMf) + " | " + String(SoilMoisture) + " | " + String(EC) + " | " + String(AP);
    server.send(200, "text/plane", web_values);
  });


#ifdef BLYNK_USE_SPIFFS
  if (SPIFFS.begin()) {
    server.serveStatic("/img/favicon.png", SPIFFS, "/img/favicon.png");
    server.serveStatic("/img/logo.png", SPIFFS, "/img/logo.png");
    server.serveStatic("/", SPIFFS, "/index.html");
  } else {
    DEBUG_PRINT("Webpage: No SPIFFS");
  }
#endif

  server.begin();

while (BlynkState::is(MODE_WAIT_CONFIG) || BlynkState::is(MODE_CONFIGURING)) {
    delay(10);
    dnsServer.processNextRequest();
    server.handleClient();
    /*if (AccP == true){
      reconnectWifi = 10000;
      AccP = false;
    }*/
    if (OfflineReadings < millis() - 2000){
      offlineSensors();
      OfflineReadings = millis();
    }
   /* if (reconnectWifi < millis() && (configStore.getFlag(CONFIG_FLAG_VALID))){
    
        server.stop();
        BlynkState::set(MODE_SWITCH_TO_STA);
     }*/

    if (reconnectWifi < millis() && (configStore.getFlag(CONFIG_FLAG_VALID))){
       NetworksTotal = WiFi.scanNetworks();
       Serial.println("scan done");
       if (NetworksTotal == 0) {
        Serial.println("no networks found");
       } else {
        Serial.print(NetworksTotal);
        Serial.println(" networks found");
        for (int i = 0; i < NetworksTotal; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
          }
    }
    for (int i = 0; i < NetworksTotal; ++i) {
    if (WiFi.SSID(i) == configStore.wifiSSID){
        server.stop();
        BlynkState::set(MODE_SWITCH_TO_STA);
    } else {
      reconnectWifi = millis() + 60000;
    }
    }
   }
     
    if (BlynkState::is(MODE_WAIT_CONFIG) && WiFi.softAPgetStationNum() > 0) {
      BlynkState::set(MODE_CONFIGURING);
    } else if (BlynkState::is(MODE_CONFIGURING) && WiFi.softAPgetStationNum() == 0) {
      BlynkState::set(MODE_WAIT_CONFIG);
    }
  }

#ifdef BLYNK_USE_SPIFFS
  SPIFFS.end();
#endif

}

bool WiFiHotSpot = false;
}

//****************************************************************************************************************************************************************************************************************************************

void enterConnectNet() {
  BlynkState::set(MODE_CONNECTING_NET);
  DEBUG_PRINT(String("Connecting to WiFi: ") + configStore.wifiSSID);

  char ssidBuff[64];
  getWiFiName(ssidBuff, sizeof(ssidBuff));
  String hostname(ssidBuff);
  hostname.replace(" ", "-");

  WiFi.setHostname(hostname.c_str());

  if (configStore.getFlag(CONFIG_FLAG_STATIC_IP)) {
    if (!WiFi.config(configStore.staticIP,
                    configStore.staticGW,
                    configStore.staticMask,
                    configStore.staticDNS,
                    configStore.staticDNS2)
    ) {
      DEBUG_PRINT("Failed to configure Static IP");
      config_set_last_error(BLYNK_PROV_ERR_CONFIG);
      BlynkState::set(MODE_ERROR);
      return;
    }
  }
  
  WiFi.begin(configStore.wifiSSID, configStore.wifiPass);

  unsigned long timeoutMs = millis() + WIFI_NET_CONNECT_TIMEOUT;
  while ((timeoutMs > millis()) && (WiFi.status() != WL_CONNECTED))
  {
    delay(10);
    app_loop();
    if (!BlynkState::is(MODE_CONNECTING_NET)) {
      WiFi.disconnect();
      return;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, brown_reg_temp);
    config_save();
    IPAddress localip = WiFi.localIP();
    mDNSname = DN.c_str();
    if (mdns.begin(mDNSname),localip) {
      Serial.println("mDNS responder started");
    }
      serverWiFi.begin();
      mdns.addService("http", "tcp", 80);
    if (configStore.getFlag(CONFIG_FLAG_STATIC_IP)) {
      BLYNK_LOG_IP("Using Static IP: ", localip);
    } else {
      BLYNK_LOG_IP("Using Dynamic IP: ", localip);
    }
    BlynkState::set(MODE_CONNECTING_CLOUD);
  } else {
    config_set_last_error(BLYNK_PROV_ERR_NETWORK);
    reconnectFlag = true;
    BlynkState::set(MODE_WAIT_CONFIG);
  }
}

//****************************************************************************************************************************************************************************************************************************************

void enterConnectCloud() {
  BlynkState::set(MODE_CONNECTING_CLOUD);

  Blynk.config(configStore.cloudToken, configStore.cloudHost, configStore.cloudPort);
  Blynk.connect();
  unsigned long timeoutMs = millis();
  while ((timeoutMs < millis() - 10000) &&
         (!Blynk.isTokenInvalid()) &&
         (Blynk.connected() == false))
    { 
    if (Blynk.connected() == true){
    Serial.println("CONNECTED");
    Blynk.run();
    app_loop();
   } else {  
    
    Serial.println("NOT CONNECTED");
    app_loop();
    }
    if (!BlynkState::is(MODE_CONNECTING_CLOUD)) {
      Blynk.disconnect();
      return;
    }
  }

  if ((millis() - 15000) > timeoutMs) {
    DEBUG_PRINT("Timeout");
  }

  if (Blynk.isTokenInvalid()) {
    config_set_last_error(BLYNK_PROV_ERR_TOKEN);
    BlynkState::set(MODE_WAIT_CONFIG);
  } else if (Blynk.connected()) {
    BlynkState::set(MODE_RUNNING);

    if (!configStore.getFlag(CONFIG_FLAG_VALID)) {
      configStore.last_error = BLYNK_PROV_ERR_NONE;
      configStore.setFlag(CONFIG_FLAG_VALID, true);
      config_save();
    }
  } else {
    config_set_last_error(BLYNK_PROV_ERR_CLOUD);
    BlynkState::set(MODE_LOCAL_NETWORK);
  }
}

//****************************************************************************************************************************************************************************************************************************************

void enterSwitchToSTA() {
  BlynkState::set(MODE_SWITCH_TO_STA);

  DEBUG_PRINT("Switching to STA...");
  server.stop();
  delay(1000);
  WiFi.mode(WIFI_OFF);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  delay(100);
  WiFi.mode(WIFI_STA);
  esp_err_t esp_wifi_set_max_tx_power(20);
  
  
  BlynkState::set(MODE_CONNECTING_NET);
}

//****************************************************************************************************************************************************************************************************************************************

void enterError() {
  BlynkState::set(MODE_ERROR);
  
  unsigned long timeoutMs = millis() + 10000;
  while (timeoutMs > millis() || g_buttonPressed)
  {
    delay(10);
    app_loop();
    if (!BlynkState::is(MODE_ERROR)) {
      return;
    }
  }
  DEBUG_PRINT("Restarting after error.");
  delay(10);

  restartMCU();
}

//****************************************************************************************************************************************************************************************************************************************

void localNetwork() {
  BlynkState::set(MODE_LOCAL_NETWORK);
    delay (500);
      if (WiFi.status() == WL_CONNECTED) {
          bool CheckServer = Ping.ping(configStore.cloudHost , 1);
          if (CheckServer){
          Flag_local = true;
          BlynkState::set(MODE_CONNECTING_CLOUD);
          }  
      if (lastdata_local < millis() - 1000){
        lastdata_local = millis();}
      } else {
        BlynkState::set(MODE_CONNECTING_NET);
      } 
    }
   

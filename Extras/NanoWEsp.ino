//Processor atmega328P(OLD BootLoader)
/***************************************************************************

  ----> http://www.adafruit.com/products/2650

 ***************************************************************************/

#define DEBUG false
#define USESENSOR true
#define USEBME false
#define USEDHT false
#define USERAW true
#define USEDHTONRX false  //When using the ESP01 adapter module, we need to re-use RX (GPIO 3 after changing mode) pin since the board does not expose GPIOs
const unsigned long DELAYTIME = 60000;
const unsigned long DELAYMULTIPLIER = 15;
const int MAX_WAIT_COUNT = 100;

#if USEDHT
#include <DHT.h>
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#endif


#include <Wire.h>

#include <EEPROM.h>  //needed to persist to EEPROM
#include <Adafruit_Sensor.h>

#if USEBME
#include <Adafruit_BME280.h>
Adafruit_BME280 bme; // I2C
#endif


#if USERAW
unsigned long int hum_raw, temp_raw, pres_raw;
signed long int t_fine;

uint16_t dig_T1;
int16_t dig_T2;
int16_t dig_T3;
uint16_t dig_P1;
int16_t dig_P2;
int16_t dig_P3;
int16_t dig_P4;
int16_t dig_P5;
int16_t dig_P6;
int16_t dig_P7;
int16_t dig_P8;
int16_t dig_P9;
int8_t  dig_H1;
int16_t dig_H2;
int8_t  dig_H3;
int16_t dig_H4;
int16_t dig_H5;
int8_t  dig_H6;
#define BME280_ADDRESS 0x76
#endif
//int connectionId;

// esp8266 using AT commands
//https://room-15.github.io/blog/2015/03/26/esp8266-at-command-reference/
#include <SoftwareSerial.h>
SoftwareSerial wifi(9, 10); // RX, TX


//Network settings
//#include <ESP8266WiFi.h>  //http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
#define SSID "cedar142"
#define SSIDPWD  "Cedar,70427."
String server = "192.168.1.102";
String path = "/api/values";
String port = "51081";

float temperature, ltemperature;
float pressure, lpressure;
float humidity, lhumidity;
unsigned long lastTime, lastPost;

//WiFiClient client; //https://arduino-esp8266.readthedocs.io/en/2.4.0/esp8266wifi/client-class.html
void setup() {
#if DEBUG
  Serial.begin(9600);
  Serial.println(F("Starting"));
#endif
  lastTime = millis() - DELAYTIME;

  pinMode(LED_BUILTIN, OUTPUT);
#if USEBME
  if (!bme.begin()) {
    if (DEBUG) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
    }
    while (1) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
    }
  }
#endif

#if USEDHT
#if USEDHTONRX
  pinMode(3, FUNCTION_3);
#endif
  dht.begin();

#endif

#if USERAW
  rawSetup();
#endif

  // use this to reset the baud rate of of the esp266, 115200 is to fast for the nano
  wifi.begin(115200);
  sendData("AT+UART_DEF=9600,8,1,0,0\r\n", 2000);

  wifi.begin(9600);
  setupWiFi();
}
void loop() {
  getCommandsFromSerialPort();

  if (millis() > lastTime + DELAYTIME) {
    unsigned long processStart = millis();
    UpdateTPH();

    if (!USESENSOR || bigChange(temperature, ltemperature) || bigChange(pressure, lpressure) || bigChange(humidity, lhumidity) || millis() > (lastPost + (DELAYTIME * DELAYMULTIPLIER)) ) {
      postData(server, path, port, temperature, pressure, humidity);
      ltemperature = temperature;
      lpressure = pressure;
      lhumidity = humidity;
      //WiFiSleep();
      lastPost = millis()  + (millis() - processStart);
    }

    lastTime = millis() + (millis() - processStart);
  }
}
void UpdateTPH() {

#if USEBME
  temperature = bme.readTemperature() * 1.8 + 32;
  pressure = bme.readPressure() * .000295300F;
  humidity = bme.readHumidity();
#endif

#if USEDHT
  float sensorTemperature = dht.readTemperature(true);
  pressure  = 12;
  float sensorHumidity = dht.readHumidity();
  if (isnan(sensorTemperature) || isnan(sensorHumidity)  ) {
#if DEBUG
    Serial.println("Read T:" + String(sensorTemperature) + "  H:" + String(sensorHumidity) + " from the DHT sensor.");
#endif
    temperature = -555.55;
    humidity = -2.22;
    dht.begin();
  } else {
    temperature = sensorTemperature;
    humidity = sensorHumidity;
  }
#endif
#if USERAW
  rawRead();
#endif
#if DEBUG && USESENSOR
  Serial.println("Temperature = " + String(temperature) + " *F");
  Serial.println("Pressure = " + String(pressure) + " inHg");
  Serial.println("Humidity = " + String(humidity) + " %");
  Serial.println();
#endif
}
String sendData(String command, const int timeout) {
  return sendData(command, timeout, "OK\r\n");
}
String sendData(String command, const int timeout, String terminator) {
  String response = "";
  wifi.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (wifi.available())
    {
      char c = wifi.read(); // read the next character.
      response += String(c);
    }
    if (response.indexOf(terminator) > -1) {
      time = millis() - timeout - 100;
    }
  }
#if DEBUG
  Serial.println(response); //displays the esp response messages in arduino Serial monitor
#endif
  return response;
}
void setupWiFi() {
  sendData("AT+RST\r\n", 2000, "ready\r\n"); // reset module
  sendData("AT+CWMODE=1\r\n", 1000); // configure as Station
  //sendData("AT+CWMODE=2\r\n",1000); // configure as access point
  //sendData("AT+CWMODE=3\r\n",1000); // configure as access point + Station mode
  sendData("AT+CWJAP=\"cedar142\",\"Cedar,70427.\"\r\n", 12000); // connect to local access point
  //sendData("AT+CWSAP=\"tph\",\"t,p,h,0.\",1,3\r\n",1000);  // add security to the access point

  sendData("AT+CIFSR\r\n", 2000); // get ip address
  //sendData("AT+CIPMUX=1\r\n", 1000); // configure for multiple connections
  //sendData("AT+CIPSERVER=1,80\r\n", 1000); // turn on server on port 80

  //sendData("AT+CWSAP?\r\n", 1000); // read access point configuration
  sendData("AT+CWJAP?\r\n", 1000); //read ssid configuration
  sendData("AT+CIPSTA?\r\n", 1000); // read ip address

}
void GetRequest(String &host, String &path, String &port) {
#if DEBUG
  Serial.println("Request Status");
#endif
  if (startConnection(host, port)) {

    String request = "GET " + path + F(" HTTP/1.1\r\n") +
                     F("Host: ") + host + F("\r\n") +
                     F("Accept: */*\r\n\r\n\r\n");
    String result = sendCommand(request);
    parseBody(result);
    stopConnection();
  }
}
String sendCommand(String &command) {
  sendData("AT+CIPSEND=" + String(command.length()) + "\r\n" , 1000, ">");
  sendData(command, 3000, "SEND OK\r\n");

  String result = wifi.readStringUntil(':');
  unsigned int readSize = result.substring(result.lastIndexOf(",") + 1).toInt();
  result = wifi.readString();

  unsigned int eob = result.indexOf(F("\r\n\r\n"));
  result.remove(0, eob + 4); //result.substring(eob + 4, readSize - 1) ;
  return result;
}
void postData(String &host, String &path, String &port, float temperature, float pressure, float humidity) {
  if (startConnection(host, port)) {
    String data = "T=" + String(temperature) + "&P=" + String(pressure) + "&H=" + String(humidity) + "&I=" + getMACAddress() + "&O=0";
    int dataLength = data.length();
    String request = "POST " + path + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Content-Type: application/x-www-form-urlencoded; charset=UTF-8 \r\n" +
                     "Content-Length: " + String(dataLength) + "\r\n\r\n" +
                     data +  "\r\n";
#if DEBUG
    Serial.println(request);
#endif
    String result = sendCommand(request);
    parseBody(result);
    delay(1000);
    stopConnection();
  }
}
bool startConnection(String &host, String &port) {
  sendData("AT+CIPSTART=\"TCP\",\"" +
           host + "\"," + port + "\r\n" , 3000);
  sendData("AT+CIPSTATUS\r\n", 10000);
  return true;
}
void stopConnection() {
  String result =     sendData("AT+CIPSTATUS\r\n", 10000);
  if (result.indexOf(":4") == -1) {
    sendData("AT+CIPCLOSE\r\n" , 3000);
  }
}
String getMACAddress() {
  String rawResult = sendData("AT+CIPSTAMAC?\r\n", 1000);

  unsigned int start = rawResult.indexOf(F("+CIPSTAMAC:\""));

  rawResult.remove(0 , start + 12);
  rawResult.remove(17);
#if DEBUG
  Serial.print (F("Mac Address =>"));
  Serial.println(rawResult);
#endif
  return rawResult;
}
void getCommandsFromSerialPort() {
  while (wifi.available() > 0  )
  {
    char a = wifi.read();
    if (a == '\0')
      continue;
    if (a != '\r' && a != '\n' && (a < 32))
      continue;
    Serial.print(a);
  }

  while (Serial.available() > 0)
  {
    char a = Serial.read();
    wifi.write(a);
  }
}
bool bigChange(float x, float y) {
  return (abs(x - y) > 1);
}
void parseBody(String &body) {
#if DEBUG
  Serial.println(body);
#endif
}

#if USERAW
void rawSetup() {
  uint8_t osrs_t = 1;             //Temperature oversampling x 1
  uint8_t osrs_p = 1;             //Pressure oversampling x 1
  uint8_t osrs_h = 1;             //Humidity oversampling x 1
  uint8_t mode = 3;               //Normal mode
  uint8_t t_sb = 5;               //Tstandby 1000ms
  uint8_t filter = 0;             //Filter off
  uint8_t spi3w_en = 0;           //3-wire SPI Disable

  uint8_t ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | mode;
  uint8_t config_reg    = (t_sb << 5) | (filter << 2) | spi3w_en;
  uint8_t ctrl_hum_reg  = osrs_h;

  Wire.begin();

  writeReg(0xF2, ctrl_hum_reg);
  writeReg(0xF4, ctrl_meas_reg);
  writeReg(0xF5, config_reg);
  readTrim();                    //
}
void readTrim()
{
  uint8_t data[32], i = 0;
  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(0x88);
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDRESS, 24);
  while (Wire.available()) {
    data[i] = Wire.read();
    i++;
  }

  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(0xA1);
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDRESS, 1);
  data[i] = Wire.read();
  i++;

  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(0xE1);
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDRESS, 7);
  while (Wire.available()) {
    data[i] = Wire.read();
    i++;
  }
  dig_T1 = (data[1] << 8) | data[0];
  dig_T2 = (data[3] << 8) | data[2];
  dig_T3 = (data[5] << 8) | data[4];
  dig_P1 = (data[7] << 8) | data[6];
  dig_P2 = (data[9] << 8) | data[8];
  dig_P3 = (data[11] << 8) | data[10];
  dig_P4 = (data[13] << 8) | data[12];
  dig_P5 = (data[15] << 8) | data[14];
  dig_P6 = (data[17] << 8) | data[16];
  dig_P7 = (data[19] << 8) | data[18];
  dig_P8 = (data[21] << 8) | data[20];
  dig_P9 = (data[23] << 8) | data[22];
  dig_H1 = data[24];
  dig_H2 = (data[26] << 8) | data[25];
  dig_H3 = data[27];
  dig_H4 = (data[28] << 4) | (0x0F & data[29]);
  dig_H5 = (data[30] << 4) | ((data[29] >> 4) & 0x0F);
  dig_H6 = data[31];

}
void writeReg(uint8_t reg_address, uint8_t data)
{
  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(reg_address);
  Wire.write(data);
  Wire.endTransmission();
}
void readData()
{
  int i = 0;
  uint32_t data[8];
  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(0xF7);
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDRESS, 8);
  while (Wire.available()) {
    data[i] = Wire.read();
    i++;
  }
  pres_raw = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
  temp_raw = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
  hum_raw  = (data[6] << 8) | data[7];
}


signed long int calibration_T(signed long int adc_T)
{

  signed long int var1, var2, T;
  var1 = ((((adc_T >> 3) - ((signed long int)dig_T1 << 1))) * ((signed long int)dig_T2)) >> 11;
  var2 = (((((adc_T >> 4) - ((signed long int)dig_T1)) * ((adc_T >> 4) - ((signed long int)dig_T1))) >> 12) * ((signed long int)dig_T3)) >> 14;

  t_fine = var1 + var2;
  T = (t_fine * 5 + 128) >> 8;
  return T;
}

unsigned long int calibration_P(signed long int adc_P)
{
  signed long int var1, var2;
  unsigned long int P;
  var1 = (((signed long int)t_fine) >> 1) - (signed long int)64000;
  var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((signed long int)dig_P6);
  var2 = var2 + ((var1 * ((signed long int)dig_P5)) << 1);
  var2 = (var2 >> 2) + (((signed long int)dig_P4) << 16);
  var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((signed long int)dig_P2) * var1) >> 1)) >> 18;
  var1 = ((((32768 + var1)) * ((signed long int)dig_P1)) >> 15);
  if (var1 == 0)
  {
    return 0;
  }
  P = (((unsigned long int)(((signed long int)1048576) - adc_P) - (var2 >> 12))) * 3125;
  if (P < 0x80000000)
  {
    P = (P << 1) / ((unsigned long int) var1);
  }
  else
  {
    P = (P / (unsigned long int)var1) * 2;
  }
  var1 = (((signed long int)dig_P9) * ((signed long int)(((P >> 3) * (P >> 3)) >> 13))) >> 12;
  var2 = (((signed long int)(P >> 2)) * ((signed long int)dig_P8)) >> 13;
  P = (unsigned long int)((signed long int)P + ((var1 + var2 + dig_P7) >> 4));
  return P;
}

unsigned long int calibration_H(signed long int adc_H)
{
  signed long int v_x1;

  v_x1 = (t_fine - ((signed long int)76800));
  v_x1 = (((((adc_H << 14) - (((signed long int)dig_H4) << 20) - (((signed long int)dig_H5) * v_x1)) +
            ((signed long int)16384)) >> 15) * (((((((v_x1 * ((signed long int)dig_H6)) >> 10) *
                (((v_x1 * ((signed long int)dig_H3)) >> 11) + ((signed long int) 32768))) >> 10) + (( signed long int)2097152)) *
                ((signed long int) dig_H2) + 8192) >> 14));
  v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * ((signed long int)dig_H1)) >> 4));
  v_x1 = (v_x1 < 0 ? 0 : v_x1);
  v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);
  return (unsigned long int)(v_x1 >> 12);
}
void rawRead() {
  double temp_act = 0.0, press_act = 0.0, hum_act = 0.0;
  signed long int temp_cal;
  unsigned long int press_cal, hum_cal;

  readData();

  temp_cal = calibration_T(temp_raw);
  press_cal = calibration_P(pres_raw);
  hum_cal = calibration_H(hum_raw);
  temp_act = (double)temp_cal / 100.0;
  press_act = (double)press_cal / 100.0;
  hum_act = (double)hum_cal / 1024.0;

  temperature = temp_act * 1.8 + 32;
  pressure = press_act * .0295300F;
  humidity = hum_act;
}
#endif

//AT+GMR
//AT+CWMODE=1
//AT+CWJAP="cedar142","Cedar,70427."
//AT+CIFSR
//AT+CIUPDATE
//AT+GMR


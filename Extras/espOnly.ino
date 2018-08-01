//Board Generic ESPModule, QIO, 1M (NO SIPFFS),  Programmer: Arduino as ISP
#define DEBUG  true
#define USESENSOR true
#define USEBME false
#define USEDHT false
#define USERAW true
#define USE_TX_RX false  //When using the ESP01 adapter module, we need to re-use RX (GPIO 3 after changing mode) pin since the board does not expose GPIOs
//https://www.forward.com.au/pfod/ESP8266/GPIOpins/ESP8266_01_pin_magic.html
const unsigned long DELAYTIME = 60000;
const unsigned long DELAYMULTIPLIER = 15;
const int MAX_WAIT_COUNT = 100;
const unsigned long MAX_POST_WAIT = 5000;
//Network settings
#include <ESP8266WiFi.h>  //http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
#define SSID "cedar142"
#define SSIDPWD  "Cedar,70427."
String server = "stphserver.azurewebsites.net";
//String server = "192.168.1.102";
String path = "/api/values";
//String port = "51081";
String port = "80";
#if USEDHT
//DHT settings
#include <DHT.h>
#define DHTTYPE DHT11
#if USE_TX_RX
#define DHTPIN  3
#else
#define DHTPIN  2
#endif

DHT dht(DHTPIN, DHTTYPE, 15);
#endif

#if USEBME     //BME settings
#include <Adafruit_BME280.h>
Adafruit_BME280 bme; // I2C
#endif

#include <Wire.h>
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


float temperature, ltemperature;
float pressure, lpressure;
float humidity, lhumidity;
unsigned long lastTime, lastPost;

WiFiClient client; //https://arduino-esp8266.readthedocs.io/en/2.4.0/esp8266wifi/client-class.html
void setup() {
  delay(1000);
#if DEBUG
  Serial.begin(115200);
#endif

#if USEDHT
#if USE_TX_RX
  pinMode(3, FUNCTION_3);
#endif
dht.begin();
#endif

#if USERAW
  rawSetup();
#endif

  lastTime = millis() - DELAYTIME;
}

void loop() {
  if (millis() > lastTime + DELAYTIME) {
    unsigned long processStart = millis();
    UpdateTPH();

    if (!USESENSOR || bigChange(temperature, ltemperature) || bigChange(pressure, lpressure) || bigChange(humidity, lhumidity) || millis() > (lastPost + (DELAYTIME * DELAYMULTIPLIER)) ) {
      String data = tphReading(temperature, pressure, humidity);
      WiFiWakeUp();
      postData(server, path, port, data);
      ltemperature = temperature;
      lpressure = pressure;
      lhumidity = humidity;
      WiFiSleep();
      lastPost = millis()  + (millis() - processStart);
    }

    lastTime = millis() + (millis() - processStart);
  }
}

void postData(String &host, String &path, String &port, String &data) {
  if (client.connect(host, port.toInt())) {
#if DEBUG
    Serial.println("Connected");
#endif
    int dataLength = data.length();
    String request = "POST " + path + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Content-Type: application/x-www-form-urlencoded\r\n" +
                     "Accept-Encoding: identity\r\n" +
                     "Accept-Language: en-US,en;q=0.9\r\n" +
                     "Accept: text/html,application/xhtml+xml,application/xml,application/json;\r\n" +
                     "Content-Length: " + String(dataLength) + "\r\n\r\n" +
                     data + "\n";
#if DEBUG
    Serial.println(request);
#endif
    client.print(request);
    bool endFound = false;
    int waitcount = 0;
    while (!client.available() && waitcount++ < MAX_WAIT_COUNT) {
      delay(10);
    }
    unsigned long waitmillis = millis() + MAX_POST_WAIT;

    while (client.connected() && millis() < waitmillis)
    {
      if (client.available())
      {
        String line = client.readStringUntil('\n');
#if DEBUG
        Serial.println(line);
#endif
      }
    }
    if(client.connected()){
      Serial.println("Stopping!");
      client.stop();
      
    }
    Serial.println("disconnected!");

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
    humidity = -1.11;
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

String getMACAddress() {
  String result = WiFi.macAddress();
  result.toLowerCase();
#if DEBUG
  Serial.println(result);
#endif
  return result;
}
bool bigChange(float x, float y) {
  return (abs(x - y) > 1);
}

String tphReading(float temp, float pres, float humid) {
  String data = "T=" + String(temp) + "&P=" + String(pres) + "&H=" + String(humid) + "&I=" + getMACAddress() + "&O=0";
  return data;
}
void WiFiWakeUp() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, SSIDPWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
#if DEBUG
    Serial.print(".");
#endif
  }
#if DEBUG
  Serial.println(WiFi.localIP());
#endif
}
void WiFiSleep() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Serial.println("Sleeping but still alive!");
}


#if USERAW
void rawSetup() {
  uint8_t osrs_t = 1;             //Temperature oversampling x 1
  uint8_t osrs_p = 1;             //Pressure oversampling x 1
  uint8_t osrs_h = 1;             //Humidity oversampling x 1
  uint8_t mode = 3;               //Normal mode
  uint8_t t_sb = 5;               //Tstandby 1000ms
  uint8_t filter = 0;             //Filter off
  uint8_t spi3w_en = 0;           //3-wire SPI Disable  ENABLE

  uint8_t ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | mode;
  uint8_t config_reg    = (t_sb << 5) | (filter << 2) | spi3w_en;
  uint8_t ctrl_hum_reg  = osrs_h;
#if USE_TX_RX
  Wire.begin(1, 3);
#else
  Wire.begin(0, 2);
#endif
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
#if DEBUG
  Serial.println("T=" + String(temp_act) + "  =>" + String(temperature));
  Serial.println("P=" + String(press_act) + "  =>" + String(pressure));
  Serial.println("H=" + String(hum_act) + "  =>" + String(humidity));
#endif
}
#endif

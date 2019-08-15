#include <Wire.h>
#include <DHT.h>
#include <Adafruit_MPL115A2/Adafruit_MPL115A2.h>
#include <LightDependentResistor.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

//Contains
const String WIFI_SSID = "WLAN-DYG8HA";
const String WIFI_PASS = "D80nm8A0Mgg7G8F";
const int NUMBER_OF_MEASUREMENTS = 10;
const int TIME_BETWEEN_MEASUREMENTS = 6000; //1 minute
const String API_URL = "https://weather-station-api.herokuapp.comn /v1/measurments";
const String API_FINGERPINT = "08 3B 71 72 02 43 6E CA ED 42 86 93 BA 7E DF 81 C4 BC 62 30"; //https://www.grc.com/fingerprints.htm
const String API_TOKEN = "1aVtaMl1gQ7qYg5Wa5htY1V2TVk8s9ZHLVykd8e9cSfUKQ50jSTNL2h5M7dX9V8VyyIPfmcQUVhecvXz";
const int API_STATION_ID = 1;

//Initialize Temperature and Humidity sensor on D3 pin
uint8_t DHTPin = D3;
DHT dht(DHTPin, DHT22);

//Initialize Pressure and Temperature sensor on D1 and D2 pins
Adafruit_MPL115A2 mpl115a2;

//Initialize Light sensor on A0 pin
LightDependentResistor photocell(A0, 1200, LightDependentResistor::GL5528);

/**
 * Setup function.
 */
void setup()
{
  Serial.begin(115200);

  //Initialize Temperature and Humidity sensor on D3 pin
  pinMode(DHTPin, INPUT);
  dht.begin();

  //Initialize Pressure and Temperature sensor on D1 and D2 pins
  mpl115a2.begin();

  //Initialize WIFI connection
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println('Waiting for WIFI connection...');
  }
}

/**
 * Loop function.
 */
void loop()
{
  float temperature[NUMBER_OF_MEASUREMENTS];
  float humidity[NUMBER_OF_MEASUREMENTS];
  float pressure[NUMBER_OF_MEASUREMENTS];
  float illuminance[NUMBER_OF_MEASUREMENTS];

  for (int i = 0; i < NUMBER_OF_MEASUREMENTS; i++)
  {
    temperature[i] = getTemperature();
    humidity[i] = getHumidity();
    pressure[i] = getPressure();
    illuminance[i] = getIlluminance();

    Serial.println(temperature[i]);
    Serial.println(humidity[i]);
    Serial.println(pressure[i]);
    Serial.println(illuminance[i]);
    Serial.println();

    delay(TIME_BETWEEN_MEASUREMENTS);
  }

  sendToAPI(0, 0, 0, 0);

  delay(5000);
}

/**
 * Get temperature. Temerature is avg of result of data from two sensors: DHT22 and MPL115A2
 * 
 * @return float|NULL
 */
float getTemperature()
{
  float t1 = dht.readTemperature();
  float t2 = mpl115a2.getTemperature();

  if (isnan(t1) && !isnan(t2))
  {
    return t2;
  }
  else if (!isnan(t1) && isnan(t2))
  {
    return t1;
  }
  else if (isnan(t1) && isnan(t2))
  {
    return NULL;
  }

  return (t1 + t2) / 2;
}

/**
 * Get humidity.
 *
 * @return float|NULL
 */
float getHumidity()
{
  float h = dht.readHumidity();

  if (isnan(h))
  {
    return NULL;
  }

  return h;
}

/**
 * Get pressure.
 * 
 * @reutrn float|NULL
 */
float getPressure()
{
  float p = mpl115a2.getPressure();

  if (isnan(p))
  {
    return NULL;
  }

  return p * 62.3F;
}

/**
 * Get illuminance.
 * 
 * @reutn float|NULL
 */
float getIlluminance()
{
  float i = photocell.getCurrentLux();

  if (isnan(i))
  {
    return NULL;
  }

  return i;
}

/**
  * Send data to API.
  * 
  * @param float temperature
  * @param float humidity
  * @param float pressure
  * @param float illuminance
  */
void sendToAPI(
    float temperature,
    float humidity,
    float pressure,
    float illuminance)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    http.begin(API_URL, API_FINGERPINT);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String params = "token=" + API_TOKEN + "&station_id=" + API_STATION_ID + "&temperature=" + temperature + "&humidity=" + humidity + "&pressure=" + pressure + "&illuminance=" + illuminance;

    int httpCode = http.POST(params);
    String payload = http.getString();

    if (httpCode > 0)
    {
      Serial.println(httpCode);
      Serial.println(payload);
    }
    else
    {
      Serial.println(http.errorToString(httpCode).c_str());
    }

    Serial.println();

    http.end();
  }
}

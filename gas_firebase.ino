#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

//Define Sensor Pins
#define gas_sensor_pin 27
#define dht_sensor_pin 33
#define buzzer_pin 26
//Ssid and Password
#define ssid_name "TURKSAT-KABLONET-D7F8-2.4G"
#define ssid_password "b6474ae6"

//Api Key and Databse URL
#define API_KEY "AIzaSyCHc7f_BJSoNJQXoEz8xnFcn88c9_FBn_k"
#define DATABASE_URL "https://auth-9fe80-default-rtdb.firebaseio.com/"

//User E-mail and Password
#define user_email "2004gundogmus2004@gmail.com"
#define user_password "123456789"

//Define Firebase Object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long SendDataPrevMillis = 0;


//Create DHT Sensor
DHT dht(dht_sensor_pin, DHT11);



void setup() {
  Serial.begin(115200);
  pinMode(gas_sensor_pin, INPUT);
  pinMode(buzzer_pin, OUTPUT);
  dht.begin();

  WiFi.begin(ssid_name, ssid_password);
  Serial.print("Connecting to \t\t");
  Serial.println(ssid_name);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi Connected");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = user_email;
  auth.user.password = user_password;

  config.token_status_callback = tokenStatusCallback;

  Firebase.reconnectNetwork(true);

  fbdo.setBSSLBufferSize(4096, 1024);
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
}

void loop() {
  if (Firebase.ready() && (millis() - SendDataPrevMillis > 2000 || SendDataPrevMillis == 0)) {
    SendDataPrevMillis = millis();

    Serial.printf("Gas leak: %s\n", Firebase.RTDB.setBool(&fbdo, F("/gas-leak"), digitalRead(gas_sensor_pin) == 1) ? "ok" : fbdo.errorReason().c_str());
    bool gas_sensor_state = digitalRead(gas_sensor_pin);

    while (gas_sensor_state) {
      alarm();
      Serial.println("Alarm");
      gas_sensor_state = digitalRead(buzzer_pin);
    }

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    Serial.printf("Temperature: %s\n", Firebase.RTDB.setFloat(&fbdo, F("/temperature"), temperature) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Humidity: %s\n", Firebase.RTDB.setFloat(&fbdo, F("/humidity"), humidity) ? "ok" : fbdo.errorReason().c_str());
  }
}

void alarm() {
  tone(buzzer_pin, 1000);
  delay(200);
  noTone(buzzer_pin);
  delay(200);
  tone(buzzer_pin, 1000);
  delay(200);
  noTone(buzzer_pin);
  delay(200);
}

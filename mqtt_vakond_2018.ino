#define DEBUG
#undef DEBUG

#define WLAN_SSID       "mywifissid"
#define WLAN_PASS       "mywifipass"
#define CLIENT_NAME     "client01"

#define AIO_SERVER      "my.mqtt.server.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "mqttuser"
#define AIO_KEY         "mqttpass"

#define X_INTERVAL_SEC 60
#define TURNAROUND_SEC 2

#include "user_config.h"

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// ######## Sensor specific includes ######## BEGIN
// ######## Sensor specific includes ######## END

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe my_hup = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/" CLIENT_NAME "/hup", MQTT_QOS_1);
Adafruit_MQTT_Subscribe global_hup = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/global/hup", MQTT_QOS_1);

// ######## Sensor specific variables ######## BEGIN
// ######## Sensor specific variables ######## END

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
#ifdef DEBUG
  Serial.begin(115200); delay(10);
  Serial.print("Connecting to "); Serial.println(WLAN_SSID);
#endif
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
#ifdef DEBUG
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
#endif
  mqtt.subscribe(&my_hup);
  mqtt.subscribe(&global_hup);
// ######## Sensor specific setup ######## BEGIN
// ######## Sensor specific setup ######## END
}

uint32_t x = 0;
uint32_t x_timer = 0;

void increment_timers() {
  x_timer++;
  if (x_timer >= (X_INTERVAL_SEC / TURNAROUND_SEC)) {
    x_timer = 0;
  }
}

// ######## Sensor specific functions ######## BEGIN
// ######## Sensor specific functions ######## END

void loop() {
  bool publish_success;
  Adafruit_MQTT_Subscribe *subscription;

  MQTT_connect();
  while ((subscription = mqtt.readSubscription(TURNAROUND_SEC * 1000))) {
    if (subscription == &my_hup || subscription == &global_hup) {
#ifdef DEBUG
      Serial.print(F("Got: "));
      Serial.println((char *)((*subscription).lastread));
#endif
      x_timer = 0;
    }
  }

  publish_success = true;
  if (x_timer == 0) {
// ######## Sensor specific publish ######## BEGIN
// ######## Sensor specific publish ######## END
  }
  
  increment_timers();

  if (!publish_success) {
    x_timer = 0;
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
#ifdef DEBUG
  Serial.print("Connecting to MQTT... ");
#endif
  uint8_t retries = 10;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
#ifdef DEBUG
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
#endif
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
#ifdef DEBUG
  Serial.println("MQTT Connected!");
#endif
}

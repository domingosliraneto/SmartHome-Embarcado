#include <WiFi.h>
#include <Ticker.h>
#include <IOXhop_FirebaseESP32.h>
#include <DHT.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#define NTP_OFFSET -3 * 60 * 60 // In seconds
#define NTP_INTERVAL 60 * 1000  // In miliseconds
#define NTP_ADDRESS "0.br.pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

#define FIREBASE_HOST "smarthome-9ccf9.firebaseio.com"
#define FIREBASE_AUTH "e5cNmEWhzDZjOGnE9yDbJbRkARDW2AkxyIHnvlo9"

#define WIFI_SSID "Domingos-2"
#define WIFI_PASSWORD "anna1234"

#define DEBUG

#define bomba_pin 21
#define rele1_pin 12
#define rele2_pin 14
#define rele3_pin 27
#define DHT_PIN 23
#define DHTTYPE 11

#define PUBLISH_INTERVAL 1000 * 60 * 1

DHT dht(DHT_PIN, DHTTYPE);
Ticker ticker;
bool publishNewState = true;

bool val_Bomba;
bool val_Rele1;
bool val_Rele2;
bool val_Rele3;

float humidity;
float temperature;

void publish()
{
    publishNewState = true;
}

void setupPins() {

    pinMode(bomba_pin, OUTPUT);
    pinMode(rele1_pin, OUTPUT);
    pinMode(rele2_pin, OUTPUT);
    pinMode(rele3_pin, OUTPUT);

    digitalWrite(bomba_pin, LOW);
    digitalWrite(rele1_pin, LOW);
    digitalWrite(rele2_pin, LOW);
    digitalWrite(rele3_pin, LOW);

    dht.begin();
}

void setupWifi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("connecting");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
}

void setupFirebase() {
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

    Firebase.setBool("Casa-Domingos/Bomba", true);
    Firebase.setBool("Casa-Domingos/Rele1", true);
    Firebase.setBool("Casa-Domingos/Rele2", true);
    Firebase.setBool("Casa-Domingos/Rele3", true);
}

void setup()
{
    Serial.begin(9600);

    setupPins();
    setupWifi();
    timeClient.begin();
    
    setupFirebase();

    ticker.attach_ms(PUBLISH_INTERVAL, publish);
}

void loop()
{
    timeClient.update();

    delay(50);

    val_Bomba = Firebase.getBool("Casa-Domingos/Bomba");
    digitalWrite(bomba_pin, val_Bomba ? HIGH : LOW);

    val_Rele1 = Firebase.getBool("Casa-Domingos/Rele1");
    digitalWrite(rele1_pin, val_Rele1 ? HIGH : LOW);

    val_Rele2 = Firebase.getBool("Casa-Domingos/Rele2");
    digitalWrite(rele2_pin, val_Rele2 ? HIGH : LOW);

    val_Rele3 = Firebase.getBool("Casa-Domingos/Rele3");
    digitalWrite(rele3_pin, val_Rele3 ? HIGH : LOW);

    if (publishNewState) {
        Serial.println("Novo status");
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();
        String formattedTime = timeClient.getFormattedTime();
        if (!isnan(humidity) && !isnan(temperature)) {
            Firebase.pushFloat("Casa-Domingos/Temperatura", temperature);
            Firebase.pushFloat("Casa-Domingos/Umidade", humidity);
            Firebase.pushString("Casa-Domingos/Datetime", formattedTime.substring(0,5));
            publishNewState = false;
        }
        else {
            Serial.println("Erro na publicação");
        }
    }

    delay(50);
}

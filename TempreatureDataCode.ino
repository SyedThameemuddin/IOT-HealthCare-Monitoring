#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHTPIN 23        // Pin where the DHT sensor is connected
#define DHTTYPE DHT11    // DHT 11 or DHT 22

const char* ssid = "IDEA_WIFI_2";  // Your Wi-Fi network name
const char* password = "ideawifi2";  // Your Wi-Fi password

String apiKey = "E0SZO2XMU6OSF6WA";  // Your ThingSpeak API key
const char* server = "http://api.thingspeak.com/update";

// IFTTT Webhook URL
const char* iftttWebhookURL = "https://maker.ifttt.com/trigger/tempreature_alert/with/key/caT3j1GCDuE7i6NMXRFw5c";  // Replace with your IFTTT Webhook key

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    dht.begin();
}

void loop() {
    delay(2000);  // Wait a few seconds between measurements

    float temperature = dht.readTemperature();
    
    if (isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    Serial.print("Temperature: ");
    Serial.println(temperature);

    // Send data to ThingSpeak
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(temperature);
        http.begin(url);
        int httpCode = http.GET();

        if (httpCode > 0) {
            Serial.println("Data sent to ThingSpeak successfully");
        } else {
            Serial.println("Error in sending data to ThingSpeak");
        }
        http.end();
    }

    // Trigger IFTTT Webhook alert if temperature exceeds 30°C
    if (temperature > 30) {  // Set your threshold here
        HTTPClient http;
        http.begin(iftttWebhookURL);
        int httpCode = http.GET();

        if (httpCode > 0) {
            Serial.println("Alert sent to IFTTT successfully");
        } else {
            Serial.println("Error in sending alert to IFTTT");
        }
        http.end();
    }
}



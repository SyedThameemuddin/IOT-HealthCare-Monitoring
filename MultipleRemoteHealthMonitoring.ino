#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>
#include <Wire.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Define patient structure
struct Patient {
    String patientID;
    String roomNo;
    int heartRate;
    float temperature;
};

// Create virtual patients
Patient patient1 = {"Patient 1", "Room 101", 0, 0};  // Real data
Patient patient2 = {"Patient 2", "Room 102", 0, 0};  // Simulated data

// Pulse Sensor setup
const int PULSE_INPUT = 34;  // GPIO34 for ESP32
const int THRESHOLD = 550;   // Threshold for heart rate detection
PulseSensorPlayground pulseSensor;

// DHT setup
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi and ThingSpeak setup
const char* ssid = "Airtel_Thameem_001";   
const char* password = "Allah7860"; 
String apiKeyPatient1 = "RZT1C7390VMNGL6H";  // ThingSpeak API Key for Patient 1
String apiKeyPatient2 = "WSPDMH8ATSC5FA1M";  // ThingSpeak API Key for Patient 2
const char* server = "http://api.thingspeak.com/update";

// IFTTT setup
const char* ifttt_event_name_1 = "patient_1_alert";  // Event for Patient 1
const char* ifttt_event_name_2 = "patient_2_alert";  // Event for Patient 2
const char* ifttt_key = "caT3j1GCDuE7i6NMXRFw5c";  
String ifttt_url_1 = "http://maker.ifttt.com/trigger/" + String(ifttt_event_name_1) + "/with/key/" + String(ifttt_key);
String ifttt_url_2 = "http://maker.ifttt.com/trigger/" + String(ifttt_event_name_2) + "/with/key/" + String(ifttt_key);

// Timing variables
unsigned long lastSwitchTime = 0;
const unsigned long switchInterval = 15000;  // 15 seconds
int currentPatient = 1; // Start with Patient 1

// Store the last valid heart rate for alerts
int lastValidHeartRate = 0;

void setup() {
    Serial.begin(115200);

    // Configure the PulseSensor
    pulseSensor.analogInput(PULSE_INPUT);
    pulseSensor.setThreshold(THRESHOLD);
    pulseSensor.begin();

    // Wi-Fi setup
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("...");
    }
    Serial.println("Connected to WiFi");

    // Initialize DHT sensor
    dht.begin();
}

void loop() {
    // Switch between patients every 15 seconds
    unsigned long currentTime = millis();
    if (currentTime - lastSwitchTime > switchInterval) {
        lastSwitchTime = currentTime;

        // Update current patient data
        if (currentPatient == 1) {
            updatePatientData(patient1);
            currentPatient = 2;
        } else {
            updatePatientData(patient2);
            currentPatient = 1;
        }
    }
}

void updatePatientData(Patient& patient) {
    // Update PulseSensor readings continuously
    pulseSensor.outputSample();

    // Check if a beat has been detected
    if (pulseSensor.sawStartOfBeat()) {
        patient.heartRate = pulseSensor.getBeatsPerMinute(); // Store latest heart rate
        Serial.print(patient.patientID);
        Serial.print(" Heart Rate: ");
        Serial.println(patient.heartRate); // Print heart rate to Serial
    }

    // Get temperature reading from DHT sensor
    patient.temperature = dht.readTemperature();
    if (isnan(patient.temperature)) {
        Serial.println("Failed to read from DHT sensor!");
    } else {
        Serial.print(patient.patientID);
        Serial.print(" Temperature: ");
        Serial.println(patient.temperature);
    }

    // Send data to ThingSpeak
    sendToThingSpeak(patient);

    // Check if heart rate exceeds 120 BPM or temperature exceeds 35°C and trigger IFTTT
    if (patient.heartRate > 120 || patient.temperature > 35) {
        sendAlert(patient);
    }
}

void sendToThingSpeak(Patient& patient) {
    String apiKey = (patient.patientID == "Patient 1") ? apiKeyPatient1 : apiKeyPatient2;
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(patient.temperature) + "&field2=" + String(patient.heartRate);
        http.begin(url);
        int httpCode = http.GET();

        if (httpCode > 0) {
            Serial.println("Data sent to ThingSpeak successfully for " + patient.patientID);
        } else {
            Serial.println("Error in sending data to ThingSpeak for " + patient.patientID);
        }
        http.end();
    }
}

void sendAlert(Patient& patient) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String ifttt_url = (patient.patientID == "Patient 1") ? ifttt_url_1 : ifttt_url_2;
        String ifttt_alert_url = ifttt_url + "?value1=" + String(patient.heartRate) + "&value2=" + String(patient.temperature);
        http.begin(ifttt_alert_url);
        int httpCode = http.POST("");  // Send a POST request to trigger the alert

        if (httpCode > 0) {
            Serial.println("IFTTT alert triggered successfully for " + patient.patientID);
        } else {
            Serial.println("Error in triggering IFTTT alert for " + patient.patientID);
        }
        http.end();
    }
}


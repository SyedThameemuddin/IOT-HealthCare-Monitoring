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

// IFTTT setup (replace with your IFTTT event name and key)
const char* ifttt_event_name_1 = "patient_1_alert";  // Event for Patient 1
const char* ifttt_event_name_2 = "patient_2_alert";  // Event for Patient 2
const char* ifttt_key = "caT3j1GCDuE7i6NMXRFw5c";  
String ifttt_url_1 = "http://maker.ifttt.com/trigger/" + String(ifttt_event_name_1) + "/with/key/" + String(ifttt_key);
String ifttt_url_2 = "http://maker.ifttt.com/trigger/" + String(ifttt_event_name_2) + "/with/key/" + String(ifttt_key);

// Timing variables
unsigned long lastSwitchTime = 0;
const unsigned long switchInterval = 15000;  // 15 seconds
int currentPatient = 1; // Start with Patient 1

// Store the last valid heart rate
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
        currentPatient = (currentPatient == 1) ? 2 : 1; // Switch between Patient 1 and Patient 2
        lastSwitchTime = currentTime;
        Serial.print("Switching to ");
        Serial.println((currentPatient == 1) ? "Patient 1" : "Patient 2");
    }

    // Update patient data
    updatePatientData((currentPatient == 1) ? patient1 : patient2);

    // Print to Serial Monitor
    Serial.print("Patient: ");
    Serial.print((currentPatient == 1) ? patient1.patientID : patient2.patientID);
    Serial.print(", Heart Rate: ");
    Serial.print((currentPatient == 1) ? patient1.heartRate : patient2.heartRate);
    Serial.print(", Temperature: ");
    Serial.println((currentPatient == 1) ? patient1.temperature : patient2.temperature);

    // Send data to ThingSpeak
    sendDataToThingSpeak((currentPatient == 1) ? patient1 : patient2);

    // Check thresholds and send alert if necessary
    if ((currentPatient == 1 && (patient1.heartRate > 120 || patient1.temperature > 38)) ||
        (currentPatient == 2 && (patient2.heartRate > 120 || patient2.temperature > 38))) {
        sendAlert((currentPatient == 1) ? patient1 : patient2);
    }

    delay(1000); // 1-second delay between data updates
}

void updatePatientData(Patient &patient) {
    if (currentPatient == 1) {
        // Real data for Patient 1
        patient.heartRate = pulseSensor.getBeatsPerMinute();
        patient.temperature = dht.readTemperature();
    } else {
        // Simulated data for Patient 2
        patient.heartRate = random(60, 100); // Random heart rate between 60-100 bpm
        patient.temperature = random(35, 38); // Random temperature between 35-38°C
    }
}

void sendDataToThingSpeak(Patient &patient) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String apiKey = (currentPatient == 1) ? apiKeyPatient1 : apiKeyPatient2;
        String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(patient.temperature) + "&field2=" + String(patient.heartRate);
        http.begin(url);
        int httpCode = http.GET();
        http.end();
    }
}

void sendAlert(Patient &patient) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String ifttt_alert_url = (currentPatient == 1) ? ifttt_url_1 : ifttt_url_2;
        String alertMessage = "Patient: " + patient.patientID + ", Room: " + patient.roomNo + ", Heart Rate: " + String(patient.heartRate) + ", Temperature: " + String(patient.temperature);
        ifttt_alert_url += "?value1=" + String(patient.heartRate) + "&value2=" + String(patient.temperature) + "&value3=" + alertMessage;
        http.begin(ifttt_alert_url);
        int httpCode = http.POST("");  
        if (httpCode > 0) {
            Serial.println("IFTTT alert triggered successfully");
        } else {
            Serial.println("Error in triggering IFTTT alert");
        }
        http.end();
    }
}

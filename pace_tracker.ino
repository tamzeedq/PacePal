#include <TinyGPS++.h>
#include <Firebase_Arduino_WiFiNINA.h>
#include "config.h" // Include the header file with sensitive information

#define GPSBaud 9600
#define Serial_Monitor_Baud 115200
#define TXPin 0
#define RXPin 1

TinyGPSPlus gps;
FirebaseData firebaseData;

double prevLat = NULL;
double prevLon = NULL;
double totalDistance = 0;         // Total distance travelled in KM
int status = WL_IDLE_STATUS;      // the WiFi radio's status
int cycleDelay = 1000;            // Delay between GPS fetches (in ms)

void setup() {
  // Start Serial Monitor
  Serial.begin(Serial_Monitor_Baud); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("Serial Monitor started");

  // Start HardwareSerial for GPS
  Serial1.begin(GPSBaud);
  while (!Serial1) {
    ; // wait for serial port to connect 
  }
  Serial.println("GPS Serial started");

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // Connect to Wi-Fi
  Serial.println("Connecting to Wi-Fi...");
  while (status != WL_CONNECTED) {
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(10000);
  }
  Serial.println("Connected to Wi-Fi");
  // printWifiData();


  // Set up Firebase connection
  Firebase.begin(DATABASE_URL, DB_SECRET, WIFI_SSID, WIFI_PASSWORD);
  Firebase.reconnectWiFi(true);


  // Wait for valid GPS data
  Serial.println(F("Waiting for valid GPS data..."));
  
  while (!checkDataValid()) {
    while (Serial1.available()) {
      gps.encode(Serial1.read());
    }
    if (checkDataValid()) {
      updatePrevLatLn();

      break;
    }
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Valid GPS data received!");

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Setup complete!");
}

void loop() {
  Serial.println("----------------------------");
  
  if (checkDataValid())  {
    digitalWrite(LED_BUILTIN, HIGH); //indicate on Nano that current readings are valid

    printDateTime(gps.date, gps.time); // Time stamp data

    printVal("Satellites: ", (float) gps.satellites.value(), gps.satellites.isValid());
    printVal("Latitude: ", gps.location.lat(), gps.location.isValid());
    printVal("Longitude: ", gps.location.lng(), gps.location.isValid());
    printVal("Altitude (m): ", gps.altitude.meters(), gps.altitude.isValid());

    double distanceTraveledM =
        TinyGPSPlus::distanceBetween(
            roundToDecimal(gps.location.lat(), 2),
            roundToDecimal(gps.location.lng(), 2),
            prevLat,
            prevLon
        );

    // Round the total distance to 3 decimal places before adding
    totalDistance += roundToDecimal(distanceTraveledM / 1000, 3);
    printVal("Distance: ", totalDistance, gps.location.isValid());
    printVal("Speed (m/s): ", roundToDecimal(distanceTraveledM / (cycleDelay / 1000.0), 3), gps.location.isValid());

    // Calculate pace in min/km if distance is greater than zero
    double paceMinPerKm = 0.0;
    if (distanceTraveledM > 0) {
      paceMinPerKm = roundToDecimal((cycleDelay / (60 * 1000.0)) / (distanceTraveledM / 1000.0), 3);
    }

    if (pushData(paceMinPerKm)) {
        Serial.println("Successfully updated Firebase");
    }

  } else {
    Serial.println("Data readings invalid");
    digitalWrite(LED_BUILTIN, LOW); //indicate on Nano that current readings are invalid
  }

  Serial.println("----------------------------");

  //update
  updatePrevLatLn();
  smartDelay(cycleDelay); 
}

// ===================== SERIAL MONITOR HELPERS ===================== 
static void printVal(const char* label, double val, bool valid)
{
  Serial.write(label);

  if (valid) {
    Serial.print(val);
  } else {
    Serial.write("Invalid");
  }

  Serial.println();
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }
  
  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  Serial.println();
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}

void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

// ===================== PUSH DATA TO FIREBASE ===================== 
bool pushData(double pace) {
    // Create JSON string to push to Firebase
    String jsonStr = "{";
    jsonStr += "\"lat\":" + String(gps.location.lat(), 2) + ",";
    jsonStr += "\"lng\":" + String(gps.location.lng(), 2) + ",";
    jsonStr += "\"totalDistance\":" + String(totalDistance, 3) + ",";
    jsonStr += "\"pace\":" + String(pace, 3);
    jsonStr += "}";

    // Debug the JSON string
    Serial.println("JSON String: " + jsonStr);

    // Construct the Firebase path using the run, date, and time
    char path[64];
    sprintf(path, "/run/%02d-%02d-%02d/%02d:%02d:%02d", 
            gps.date.day(), gps.date.month(), gps.date.year(), 
            gps.time.hour(), gps.time.minute(), gps.time.second());

    // Push data to Firebase
    if (Firebase.setJSON(firebaseData, String(path), jsonStr)) {
        Serial.println(firebaseData.dataPath());
        return true;
    } else {
        Serial.println("Error: " + firebaseData.errorReason());
        
        // Debug the JSON string
        Serial.println("JSON String: " + jsonStr);
        return false;
    }
}

// ===================== GPS HELPERS ===================== 
bool checkDataValid() {
  return gps.location.isValid() && gps.date.isValid() && gps.time.isValid();
}

// This delay ensures that the gps object is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial1.available())
      gps.encode(Serial1.read());
  } while (millis() - start < ms);
}

// Round a double to the specified number of decimal places
double roundToDecimal(double value, int decimalPlaces) {
    double scale = pow(10, decimalPlaces);
    return round(value * scale) / scale;
}

void updatePrevLatLn() {
  prevLat = roundToDecimal(gps.location.lat(), 2);
  prevLon = roundToDecimal(gps.location.lng(), 2);
}



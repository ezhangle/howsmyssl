/*
 * Do the equivalent of and send report on serial console.
 *  $ curl https://www.howsmyssl.com/a/check
 *
 * For Arduino boards ESP32, ESP8266 (AXTLS), ESP8266 (BearSSL),
 * MKR WiFi 1010, and Adafruit Feather M0 with ATWINC15000.
 *
 * BearSSL is much better than AXTLS and it is the default for ESP8266.
 * ESP32, BearSSL, and WiFiNINA (ESP32 inside) all have the "Probably OK" rating.
 * AXTLS has the rating "Improvable".
 */

#include <time.h>
#include <ArduinoJson.h>

#if defined(ADAFRUIT_FEATHER_M0)
// This board is similar to the MKR WiFi 1000. Let's assume if the
// board type is Adafruit Feather M0, it has the ATWINC1500 WiFi module.
#define ADAFRUIT_FEATHER_ATWINC1500
#endif

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_AVR_UNO_WIFI_REV2) || defined(ARDUINO_SAMD_NANO_33_IOT)
#include <SPI.h>
#include <WiFiNINA.h>
#define WIFININA
#define NO_WIFI_HARDWARE WL_NO_MODULE
#define WIFI_HARDWARE "WiFiNINA"

#elif defined(ADAFRUIT_FEATHER_ATWINC1500)
#include <SPI.h>
#include <WiFi101.h>
#define WIFI101
#define NO_WIFI_HARDWARE WL_NO_SHIELD
#define WIFI_HARDWARE "WiFi101"

#elif defined(ESP8266)
// In ESP8266 <= 2.4.2 do not define either of the following.
// In ESP8266 >= 2.5 define one or the other but not both.
//#define USING_AXTLS
#define USING_BEARSSL
#include <ESP8266WiFi.h>

#elif defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_SAM_DUE)
// Assume Adafruit Airlift shield
  #define SPIWIFI       SPI  // The SPI port
  #define SPIWIFI_SS    10   // Chip select pin
  #define ESP32_RESETN   5   // Reset pin
  #define SPIWIFI_ACK    7   // a.k.a BUSY or READY pin
  #define ESP32_GPIO0   -1
#include <SPI.h>
#include <WiFiNINA.h>
#define WIFININA
#define NO_WIFI_HARDWARE WL_NO_MODULE
#define WIFI_HARDWARE "WiFiNINA"

#endif

#if defined(ESP8266) && defined(USING_AXTLS)
// force use of AxTLS (BearSSL is now default)
#include <WiFiClientSecureAxTLS.h>
using namespace axTLS;

#elif !defined(WIFININA) && !defined(WIFI101)
#include <WiFiClientSecure.h>

#endif

const char ssid[]     = "SSID";
const char password[] = "PASSWORD";
#define server "randomfox.ca"
#define url_path "/floof/"
#define JSON_BUFFER (512)

#if defined(ESP8266)
#if defined(USING_AXTLS)
const char TLS_STACK[] = "ESP8266 AXTLS";
#elif defined(USING_BEARSSL)
const char TLS_STACK[] = "ESP8266 BearSSL";
#endif
#elif defined(ESP32)
const char TLS_STACK[] = "ESP32 TLS";
#elif defined(WIFININA)
const char TLS_STACK[] = "WiFiNINA";
#elif defined(WIFI101)
const char TLS_STACK[] = "WiFi101";
#else
#error "Board not supported"
#endif

// api.weather.gov root certificate authority, to verify the server
// change it to your server root CA
// SHA1 fingerprint is broken now!
//
#if defined(ESP32) || defined(USING_BEARSSL)
const char COMODOECCCertificationAuthority[] PROGMEM = R"====(
-----BEGIN CERTIFICATE-----
MIICiTCCAg+gAwIBAgIQH0evqmIAcFBUTAGem2OZKjAKBggqhkjOPQQDAzCBhTEL
MAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE
BxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNVBAMT
IkNPTU9ETyBFQ0MgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDgwMzA2MDAw
MDAwWhcNMzgwMTE4MjM1OTU5WjCBhTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdy
ZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09N
T0RPIENBIExpbWl0ZWQxKzApBgNVBAMTIkNPTU9ETyBFQ0MgQ2VydGlmaWNhdGlv
biBBdXRob3JpdHkwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAAQDR3svdcmCFYX7deSR
FtSrYpn1PlILBs5BAH+X4QokPB0BBO490o0JlwzgdeT6+3eKKvUDYEs2ixYjFq0J
cfRK9ChQtP6IHG4/bC8vCVlbpVsLM5niwz2J+Wos77LTBumjQjBAMB0GA1UdDgQW
BBR1cacZSBm8nZ3qQUfflMRId5nTeTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/
BAUwAwEB/zAKBggqhkjOPQQDAwNoADBlAjEA7wNbeqy3eApyt4jf/7VGFAkK+qDm
fQjGGoe9GKhzvSbKYAydzpmfz1wPMOG+FDHqAjAU9JM8SaczepBGR7NjfRObTrdv
GDeAU/7dIOA1mjbRxwG55tzd8/8dLDoWV9mSOdY=
-----END CERTIFICATE-----
)====";

#elif defined(ESP8266)
// Same as above but in binary DER format
const unsigned char COMODOECCCertificationAuthority_der PROGMEM [] = {
  0x30, 0xc2, 0x82, 0x02, 0xc2, 0x89, 0x30, 0xc2, 0x82, 0x02, 0x0f, 0xc2,
  0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x10, 0x1f, 0x47, 0xc2, 0xaf, 0xc2,
  0xaa, 0x62, 0x00, 0x70, 0x50, 0x54, 0x4c, 0x01, 0xc2, 0x9e, 0xc2, 0x9b,
  0x63, 0xc2, 0x99, 0x2a, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0xc2, 0x86, 0x48,
  0xc3, 0x8e, 0x3d, 0x04, 0x03, 0x03, 0x30, 0xc2, 0x81, 0xc2, 0x85, 0x31,
  0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x47, 0x42,
  0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x08, 0x13, 0x12, 0x47,
  0x72, 0x65, 0x61, 0x74, 0x65, 0x72, 0x20, 0x4d, 0x61, 0x6e, 0x63, 0x68,
  0x65, 0x73, 0x74, 0x65, 0x72, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03, 0x55,
  0x04, 0x07, 0x13, 0x07, 0x53, 0x61, 0x6c, 0x66, 0x6f, 0x72, 0x64, 0x31,
  0x1a, 0x30, 0x18, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x11, 0x43, 0x4f,
  0x4d, 0x4f, 0x44, 0x4f, 0x20, 0x43, 0x41, 0x20, 0x4c, 0x69, 0x6d, 0x69,
  0x74, 0x65, 0x64, 0x31, 0x2b, 0x30, 0x29, 0x06, 0x03, 0x55, 0x04, 0x03,
  0x13, 0x22, 0x43, 0x4f, 0x4d, 0x4f, 0x44, 0x4f, 0x20, 0x45, 0x43, 0x43,
  0x20, 0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69,
  0x6f, 0x6e, 0x20, 0x41, 0x75, 0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79,
  0x30, 0x1e, 0x17, 0x0d, 0x30, 0x38, 0x30, 0x33, 0x30, 0x36, 0x30, 0x30,
  0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x33, 0x38, 0x30, 0x31, 0x31,
  0x38, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0xc2, 0x81, 0xc2,
  0x85, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02,
  0x47, 0x42, 0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x08, 0x13,
  0x12, 0x47, 0x72, 0x65, 0x61, 0x74, 0x65, 0x72, 0x20, 0x4d, 0x61, 0x6e,
  0x63, 0x68, 0x65, 0x73, 0x74, 0x65, 0x72, 0x31, 0x10, 0x30, 0x0e, 0x06,
  0x03, 0x55, 0x04, 0x07, 0x13, 0x07, 0x53, 0x61, 0x6c, 0x66, 0x6f, 0x72,
  0x64, 0x31, 0x1a, 0x30, 0x18, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x11,
  0x43, 0x4f, 0x4d, 0x4f, 0x44, 0x4f, 0x20, 0x43, 0x41, 0x20, 0x4c, 0x69,
  0x6d, 0x69, 0x74, 0x65, 0x64, 0x31, 0x2b, 0x30, 0x29, 0x06, 0x03, 0x55,
  0x04, 0x03, 0x13, 0x22, 0x43, 0x4f, 0x4d, 0x4f, 0x44, 0x4f, 0x20, 0x45,
  0x43, 0x43, 0x20, 0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61,
  0x74, 0x69, 0x6f, 0x6e, 0x20, 0x41, 0x75, 0x74, 0x68, 0x6f, 0x72, 0x69,
  0x74, 0x79, 0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2a, 0xc2, 0x86, 0x48,
  0xc3, 0x8e, 0x3d, 0x02, 0x01, 0x06, 0x05, 0x2b, 0xc2, 0x81, 0x04, 0x00,
  0x22, 0x03, 0x62, 0x00, 0x04, 0x03, 0x47, 0x7b, 0x2f, 0x75, 0xc3, 0x89,
  0xc2, 0x82, 0x15, 0xc2, 0x85, 0xc3, 0xbb, 0x75, 0xc3, 0xa4, 0xc2, 0x91,
  0x16, 0xc3, 0x94, 0xc2, 0xab, 0x62, 0xc2, 0x99, 0xc3, 0xb5, 0x3e, 0x52,
  0x0b, 0x06, 0xc3, 0x8e, 0x41, 0x00, 0x7f, 0xc2, 0x97, 0xc3, 0xa1, 0x0a,
  0x24, 0x3c, 0x1d, 0x01, 0x04, 0xc3, 0xae, 0x3d, 0xc3, 0x92, 0xc2, 0x8d,
  0x09, 0xc2, 0x97, 0x0c, 0xc3, 0xa0, 0x75, 0xc3, 0xa4, 0xc3, 0xba, 0xc3,
  0xbb, 0x77, 0xc2, 0x8a, 0x2a, 0xc3, 0xb5, 0x03, 0x60, 0x4b, 0x36, 0xc2,
  0x8b, 0x16, 0x23, 0x16, 0xc2, 0xad, 0x09, 0x71, 0xc3, 0xb4, 0x4a, 0xc3,
  0xb4, 0x28, 0x50, 0xc2, 0xb4, 0xc3, 0xbe, 0xc2, 0x88, 0x1c, 0x6e, 0x3f,
  0x6c, 0x2f, 0x2f, 0x09, 0x59, 0x5b, 0xc2, 0xa5, 0x5b, 0x0b, 0x33, 0xc2,
  0x99, 0xc3, 0xa2, 0xc3, 0x83, 0x3d, 0xc2, 0x89, 0xc3, 0xb9, 0x6a, 0x2c,
  0xc3, 0xaf, 0xc2, 0xb2, 0xc3, 0x93, 0x06, 0xc3, 0xa9, 0xc2, 0xa3, 0x42,
  0x30, 0x40, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04,
  0x14, 0x75, 0x71, 0xc2, 0xa7, 0x19, 0x48, 0x19, 0xc2, 0xbc, 0xc2, 0x9d,
  0xc2, 0x9d, 0xc3, 0xaa, 0x41, 0x47, 0xc3, 0x9f, 0xc2, 0x94, 0xc3, 0x84,
  0x48, 0x77, 0xc2, 0x99, 0xc3, 0x93, 0x79, 0x30, 0x0e, 0x06, 0x03, 0x55,
  0x1d, 0x0f, 0x01, 0x01, 0xc3, 0xbf, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06,
  0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xc3, 0xbf, 0x04,
  0x05, 0x30, 0x03, 0x01, 0x01, 0xc3, 0xbf, 0x30, 0x0a, 0x06, 0x08, 0x2a,
  0xc2, 0x86, 0x48, 0xc3, 0x8e, 0x3d, 0x04, 0x03, 0x03, 0x03, 0x68, 0x00,
  0x30, 0x65, 0x02, 0x31, 0x00, 0xc3, 0xaf, 0x03, 0x5b, 0x7a, 0xc2, 0xac,
  0xc2, 0xb7, 0x78, 0x0a, 0x72, 0xc2, 0xb7, 0xc2, 0x88, 0xc3, 0x9f, 0xc3,
  0xbf, 0xc2, 0xb5, 0x46, 0x14, 0x09, 0x0a, 0xc3, 0xba, 0xc2, 0xa0, 0xc3,
  0xa6, 0x7d, 0x08, 0xc3, 0x86, 0x1a, 0xc2, 0x87, 0xc2, 0xbd, 0x18, 0xc2,
  0xa8, 0x73, 0xc2, 0xbd, 0x26, 0xc3, 0x8a, 0x60, 0x0c, 0xc2, 0x9d, 0xc3,
  0x8e, 0xc2, 0x99, 0xc2, 0x9f, 0xc3, 0x8f, 0x5c, 0x0f, 0x30, 0xc3, 0xa1,
  0xc2, 0xbe, 0x14, 0x31, 0xc3, 0xaa, 0x02, 0x30, 0x14, 0xc3, 0xb4, 0xc2,
  0x93, 0x3c, 0x49, 0xc2, 0xa7, 0x33, 0x7a, 0xc2, 0x90, 0x46, 0x47, 0xc2,
  0xb3, 0x63, 0x7d, 0x13, 0xc2, 0x9b, 0x4e, 0xc2, 0xb7, 0x6f, 0x18, 0x37,
  0xc2, 0x80, 0x53, 0xc3, 0xbe, 0xc3, 0x9d, 0x20, 0xc3, 0xa0, 0x35, 0xc2,
  0x9a, 0x36, 0xc3, 0x91, 0xc3, 0x87, 0x01, 0xc2, 0xb9, 0xc3, 0xa6, 0xc3,
  0x9c, 0xc3, 0x9d, 0xc3, 0xb3, 0xc3, 0xbf, 0x1d, 0x2c, 0x3a, 0x16, 0x57,
  0xc3, 0x99, 0xc2, 0x92, 0x39, 0xc3, 0x96
};
const unsigned int COMODOECCCertificationAuthority_der_len = 775;
#endif

#if defined(ESP32) || defined(ESP8266)
// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}
#endif

void weathergov()
{
#if defined(WIFININA) || defined(WIFI101)
WiFiSSLClient client;
#elif defined(ESP8266)
#if defined(USING_AXTLS)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored  "-Wdeprecated-declarations"
  WiFiClientSecure client;
#pragma GCC diagnostic pop
  client.setCACert_P(COMODOECCCertificationAuthority_der,
      COMODOECCCertificationAuthority_der_len);
#elif defined(USING_BEARSSL)
  BearSSL::WiFiClientSecure client;
  BearSSL::X509List cert(COMODOECCCertificationAuthority);
  client.setTrustAnchors(&cert);
#endif
#else
  // ESP32
  WiFiClientSecure client;
  client.setCACert(COMODOECCCertificationAuthority);
#endif

  client.setTimeout(10000);

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
#if defined(ESP8266) && defined(USING_AXTLS)
    // Verify validity of server's certificate
    if (client.verifyCertChain(server)) {
      Serial.println("Server certificate verified");
    } else {
      Serial.println("ERROR: certificate verification failed!");
      return;
    }
#endif

    client.print("GET " url_path " HTTP/1.1\r\n" \
               "Host: " server "\r\n" \
               "User-Agent: arduino/1.0.0\r\n" \
               "Connection: close\r\n\r\n");

    Serial.println("request sent");
    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    Serial.println(status);
    // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
    if (strcmp(status + 9, "200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      return;
    }

    // Transfer-Encoding: chunked
    // Skip next line which has the length of the next chunk
    client.readBytesUntil('\r', status, sizeof(status));

    const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_BUFFER;
    DynamicJsonDocument doc(capacity);

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, client);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
    const char* image = doc["image"]; // "http://randomfox.ca/images/31.jpg"
    const char* link = doc["link"]; // "http://randomfox.ca/?i=31"
    if (image) {
      Serial.print("image: ");
      Serial.println(image);
    }
    if (link) {
      Serial.print("link: ");
      Serial.println(link);
    }

    client.stop();
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup() {
#if defined(ADAFRUIT_FEATHER_ATWINC1500)
  //Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8,7,4,2);
#endif
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
#if defined(USBCON)
  while (!Serial) delay(1);
#endif
  delay(100);
  Serial.println();
  Serial.print("TLS Stack:");
  Serial.println(TLS_STACK);

#if defined(WIFININA) || defined(WIFI101)
  // check for the WiFi module:
  if (WiFi.status() == NO_WIFI_HARDWARE) {
    Serial.println(F("Communication with " WIFI_HARDWARE " module failed!"));
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  Serial.print(F(WIFI_HARDWARE " f/w version: "));
  Serial.println(fv);
  if (fv < "1.0.0") {
    Serial.println(F("Please upgrade the firmware"));
  }
#endif

  // attempt to connect to WiFi network:

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
#if defined(ESP8266) || defined(ESP32)
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);
  printWiFiStatus();

#if defined(ESP8266) || defined(ESP32)
  setClock();
#endif

  weathergov();
}

void loop() {
  // do nothing
}

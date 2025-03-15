#include <globals.h>
#include <secrets.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>

const char *wifi_ssid = SECRET_WIFI_SSID;
const char *wifi_password = SECRET_WIFI_PASSWORD;
int wifi_counter = 0;
int monitoring_counter = 0;
int monitoring_status = 0;
const char *monitoring_url = SECRET_MONITORING_CSV_URL;

int onoff_button_counter = ONOFF_BUTTON_COUNTER_MS;
bool onoff_button_clicked = false;
int onoff_button_state = LOW;

bool led_strips_active = true;
int led_state[LED_STRIP_ROWS][LED_STRIP_PIXELS] = {};
Adafruit_NeoPixel led_strips[LED_STRIP_ROWS] = {
  // we have one additional pixel on each strip, that is used for ESP32-related display (WiFi status, monitoring status)
  Adafruit_NeoPixel(LED_STRIP_PIXELS + 1, PIN_LED_STRIP_0, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LED_STRIP_PIXELS + 1, PIN_LED_STRIP_1, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(LED_STRIP_PIXELS + 1, PIN_LED_STRIP_2, NEO_GRB + NEO_KHZ800)
};

// ----------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  for(int index = 0; index < LED_STRIP_ROWS; index++) {
    led_strips[index].begin();
  }
  // initialize the button pin as an pull-up input
  pinMode(PIN_ONOFF_BUTTON, INPUT_PULLUP);
}

// ----------------------------------------------------------------------------------------------------
void loop() {
  wifi_counter = decrease_counter(wifi_counter, LOOP_DELAY_MS);
  if (wifi_counter == 0) {
    if (!wifi_connected()) {
      connect_to_wifi();
    }
    wifi_counter = WIFI_COUNTER_MS;
  }
  monitoring_counter = decrease_counter(monitoring_counter, LOOP_DELAY_MS);
  if (monitoring_counter == 0) {
    if (wifi_connected()) {
      fetch_monitoring_data();
    } else {
      Serial.println("Cannot fetch monitoring data, no WiFi connected!");
    }
    monitoring_counter = MONITORING_COUNTER_MS;
  }
  onoff_button_counter = decrease_counter(onoff_button_counter, LOOP_DELAY_MS);
  if (onoff_button_counter == 0) {
    if (onoff_button_clicked) {
      led_strips_active = !led_strips_active;
      update_led_strips();
      onoff_button_clicked = false;
    }
    onoff_button_counter = ONOFF_BUTTON_COUNTER_MS;
  }
  update_control_leds();
  int delayDivisions = 10;
  for(int index = 0; index < delayDivisions; index++) {
    int new_onoff_button_state = digitalRead(PIN_ONOFF_BUTTON);
    if (new_onoff_button_state != onoff_button_state) {
      if (new_onoff_button_state == HIGH) {
        onoff_button_clicked = true;
        Serial.println("Button clicked!");
      }
      onoff_button_state = new_onoff_button_state;
    }
    delay(LOOP_DELAY_MS / delayDivisions);
  }
}

// ----------------------------------------------------------------------------------------------------
void connect_to_wifi() {
  Serial.printf("Connecting to WiFi network %s\n", wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);

  wl_status_t wifi_status = WiFi.status();
  int wifi_connect_counter = WIFI_CONNECT_RETRIES;
  while (wifi_connect_counter > 0 && wifi_status != WL_CONNECTED) {
    Serial.printf("Waiting to connect - got status %s\n", wifi_status_to_string(wifi_status));
    delay(500);
    wifi_status = WiFi.status();
    wifi_connect_counter--;
  }
  if (wifi_status == WL_CONNECTED) {
    Serial.println("WiFi connected!");
  } else {
    Serial.printf("WiFi failed to connect, status is %s\n", wifi_status_to_string(wifi_status));
  }
}

const char* wifi_status_to_string(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD"; // 255, for compatibility with WiFi Shield library
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS"; // 0
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL"; // 1
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED"; // 2
    case WL_CONNECTED: return "WL_CONNECTED"; // 3
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED"; // 4
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST"; // 5
    case WL_DISCONNECTED: return "WL_DISCONNECTED"; // 6
  }
}

bool wifi_connected() {
  return WiFi.status() == WL_CONNECTED;
}

// ----------------------------------------------------------------------------------------------------
void fetch_monitoring_data() {
  Serial.println("Fetching monitoring data...");
  monitoring_status = LED_CONTROL_MONITORING_CONNECTING_CODE;
  update_control_leds();
  HTTPClient http;
  // Your Domain name with URL path or IP address with path
  http.begin(monitoring_url);
  // If you need server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    parse_monitoring_data(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    monitoring_status = LED_CONTROL_MONITORING_ERROR_CODE;
  }
  // Free resources
  http.end();
}

void parse_monitoring_data(String payload) {
  // Declare variables to store parsed data
  String line;
  int new_led_state[LED_STRIP_ROWS][LED_STRIP_PIXELS] = {}; 
  // new_led_state = new int[LED_STRIP_ROWS];
  // for(int index = 0; index < LED_STRIP_ROWS; index++) {
  //   new_led_state[index] = new int[LED_STRIP_PIXELS];
  // }

  // Split the CSV by lines
  int lineStart = 0;
  int lineEnd;
  int lineCount = 0;
  
  while ((lineEnd = payload.indexOf('\n', lineStart)) != -1) {
      line = payload.substring(lineStart, lineEnd);
      lineStart = lineEnd + 1;

      // Parse the first line (status,number)
      if (lineCount == 0) {
          int commaIndex = line.indexOf(',');
          String statusStr = line.substring(0, commaIndex);  // "status"
          String numberStr = line.substring(commaIndex + 1);
          monitoring_status = numberStr.toInt();  // Convert the number after the comma to an integer
          Serial.printf("Monitoring status code: %d\n", monitoring_status);
      }
      // Parse subsequent lines (numbers)
      else if (line.indexOf(',') > 0) {
          int startIndex = 0;
          int lineValues[3] = {};
          for(int index = 0; index < 3; index++) {
              int commaIndex = line.indexOf(',', startIndex);
              if (commaIndex == -1) commaIndex = line.length();  // Handle last value in the line
              lineValues[index] = line.substring(startIndex, commaIndex).toInt();
              startIndex = commaIndex + 1;
          }
          // Optionally, print the parsed numbers for debugging
          Serial.print("Monitoring line: ");
          for (int index = 0; index < 3; index++) {
              Serial.print(lineValues[index]);
              Serial.print(" ");
          }
          Serial.println();
          if (lineValues[0] >= 0 && lineValues[0] < LED_STRIP_ROWS && lineValues[1] >= 0 && lineValues[1] < LED_STRIP_PIXELS) {
             new_led_state[lineValues[0]][lineValues[1]] = lineValues[2];
          }
      }
      lineCount++;
  }

  for(int row = 0; row < LED_STRIP_ROWS; row++) {
    bool pixels_changed = false;
    for(int pixel = 0; pixel < LED_STRIP_PIXELS; pixel++) {
      if (new_led_state[row][pixel] != led_state[row][pixel]) {
        set_led_strip_pixel(row, pixel, new_led_state[row][pixel]);
        led_state[row][pixel] = new_led_state[row][pixel];
        pixels_changed = true;
      }
    }
    if (pixels_changed) {
      led_strips[row].show();
    }
  }
}

void update_led_strips() {
  for(int row = 0; row < LED_STRIP_ROWS; row++) {
    if (led_strips_active) {
      for(int pixel = 0; pixel < LED_STRIP_PIXELS; pixel++) {
        set_led_strip_pixel(row, pixel, led_state[row][pixel]);
      }
    } else {
      led_strips[row].clear();
    }
    led_strips[row].show();
  }
}

void update_control_leds() {
  if (led_strips_active) {
    if (wifi_connected()) {
      led_strips[LED_CONTROL_WIFI_ROW].setPixelColor(LED_CONTROL_WIFI_PIXEL, led_strips[LED_CONTROL_WIFI_ROW].Color(0, 255, 0));
    } else {
      led_strips[LED_CONTROL_WIFI_ROW].setPixelColor(LED_CONTROL_WIFI_PIXEL, led_strips[LED_CONTROL_WIFI_ROW].Color(255, 0, 0));
    }
    led_strips[LED_CONTROL_WIFI_ROW].show();
    switch (monitoring_status) {
      case LED_CONTROL_MONITORING_ERROR_CODE:
        set_led_strip_pixel(LED_CONTROL_MONITORING_ROW, LED_CONTROL_MONITORING_PIXEL, LED_CONTROL_MONITORING_ERROR_COLOR);
        break;
      case LED_CONTROL_MONITORING_CONNECTING_CODE:
        set_led_strip_pixel(LED_CONTROL_MONITORING_ROW, LED_CONTROL_MONITORING_PIXEL, LED_CONTROL_MONITORING_CONNECTING_COLOR);
        break;
      case LED_CONTROL_MONITORING_PARTIALLYOK_CODE:
        set_led_strip_pixel(LED_CONTROL_MONITORING_ROW, LED_CONTROL_MONITORING_PIXEL, LED_CONTROL_MONITORING_PARTIALLYOK_COLOR);
        break;
      case 200:
        // falls through
      case LED_CONTROL_MONITORING_OK_CODE:
        set_led_strip_pixel(LED_CONTROL_MONITORING_ROW, LED_CONTROL_MONITORING_PIXEL, LED_CONTROL_MONITORING_OK_COLOR);
        break;
      default:
        set_led_strip_pixel(LED_CONTROL_MONITORING_ROW, LED_CONTROL_MONITORING_PIXEL, LED_CONTROL_MONITORING_ERROR_COLOR);
        break;
    }
    led_strips[LED_CONTROL_MONITORING_ROW].show();
  }
}

// ----------------------------------------------------------------------------------------------------
int decrease_counter(int counter, int value) {
  counter -= value;
  if (counter < 0) {
    counter = 0;
  }
  return counter;
}

void set_led_strip_pixel(int row, int pixel, int colorNumber) {
  led_strips[row].setPixelColor(pixel, led_strips[row].Color((colorNumber >> 16) & 0xFF, (colorNumber >> 8) & 0xFF, colorNumber & 0xFF));
}
#include <Arduino.h>
#include <driver/adc.h>

#include "esp_adc_cal.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_sleep.h"

// eink.
#include "eink/display.h"
#include "eink/logger.h"

#define BATT_PIN 36

/**
 * Upper most button on side of device. Used to setup as wakeup source to start
 * from deepsleep. Pinout here
 * https://ae01.alicdn.com/kf/H133488d889bd4dd4942fbc1415e0deb1j.jpg
 */
gpio_num_t FIRST_BTN_PIN = GPIO_NUM_39;

int vref = 1100;

/**
 * RTC Memory var to get number of wakeups via wakeup source button
 * For demo purposes of rtc data attr
 **/
RTC_DATA_ATTR int pressed_wakeup_btn_index;

/**
 * That's maximum 30 seconds of runtime in microseconds
 */
int64_t maxTimeRunning = 30000000;

double_t get_battery_percentage() {
  // When reading the battery voltage, POWER_EN must be turned on
  // epd_poweron();

  delay(50);

  Serial.println(epd_ambient_temperature());

  uint16_t v = analogRead(BATT_PIN);
  Serial.print("Battery analogRead value is");
  Serial.println(v);
  double_t battery_voltage =
      ((double_t)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);

  double_t percent_experiment = ((battery_voltage - 3.7) / 0.5) * 100;

  // cap out battery at 100%
  // on charging it spikes higher
  if (percent_experiment > 100) {
    percent_experiment = 100;
  }

  String voltage = "Battery Voltage :" + String(battery_voltage) +
                   "V which is around " + String(percent_experiment) + "%";
  Serial.println(voltage);

  delay(50);

  return percent_experiment;
}

void start_deep_sleep_with_wakeup_sources() {
  epd_poweroff();
  delay(400);
  esp_sleep_enable_ext0_wakeup(FIRST_BTN_PIN, 0);

  Serial.println("Sending device to deepsleep");
  esp_deep_sleep_start();
}

/**
 * Function that prints the reason by which ESP32 has been awaken from sleep
 */
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      break;
  }
}

/**
 * Correct the ADC reference voltage. Was in example of lilygo epd47 repository
 * to calc battery percentage
 */
void correct_adc_reference() {
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
    vref = adc_chars.vref;
  }
}

// eink::Logger *logger;
eink::Display *display;

void setup() {
  // Serial.begin(115200);

  display = new eink::Display();

  correct_adc_reference();

  print_wakeup_reason();

  eink::Logger::Get().Printf("Hello, world instance\n");
  display->DrawRect(30, 30, 40, 40, 127);
  display->DrawText(60, 30, "Hello, world", 30, eink::FontSize::Size12);
  display->Update();

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    //   Serial.println("Woken up by wakeup source button");
    //   pressed_wakeup_btn_index++;
    //   String message = String("Woken up from deepsleep times: ");
    //   message.concat(String(pressed_wakeup_btn_index));
    //   display_full_screen_left_aligned_text(message.c_str());

  } else {
    //   /* Non deepsleep wakeup source button interrupt caused start e.g. reset
    //   btn
    //    */
    //   Serial.println("Woken up by reset button or power cycle");
    //   const char* message =
    //       "Hello there! You just turned me on.\nIn 30s I will go to
    //       deepsleep";
    //   display_center_message(message);
  }
}

void loop() {
  /*
   * Shutdown device after 30s always to conserve battery.
   * Basically almost no battery usage then until next wakeup.
   */
  if (esp_timer_get_time() > maxTimeRunning) {
    // Serial.println(
    //     "Max runtime of 30s reached. Forcing deepsleep now to save
    //     energy");
    //   display_center_message(
    //       "Sleeping now.\nWake me up from deepsleep again\nwith the first
    //       button " "on my side");
    //   delay(1500);

    start_deep_sleep_with_wakeup_sources();
  }
}
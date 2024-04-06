#include "secrets.h"
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/extra/esp32/telegram.h>
#include <eloquent_esp32cam/motion/detection.h>

using eloq::camera;
using eloq::wifi;
using eloq::motion::detection;
using eloq::telegram;

void setup() {
  delay(3000);
  Serial.begin(115200);

  // camera settings
  // replace with your own model!
  camera.pinout.aithinker();
  camera.brownout.disable();
  camera.resolution.vga();
  camera.quality.high();

  // configure motion detection
  // the higher the stride, the faster the detection
  // the higher the stride, the lesser granularity
  detection.stride(1);
  // the higher the threshold, the lesser sensitivity
  // (at pixel level)
  detection.threshold(5);
  // the higher the threshold, the lesser sensitivity
  // (at image level, from 0 to 1)
  detection.ratio(0.2);
  // optionally, you can enable rate limiting (aka debounce)
  // motion won't trigger more often than the specified frequency
  detection.rate.atMostOnceEvery(5).seconds();

  // init camera
  while (!camera.begin().isOk())
    Serial.println(camera.exception.toString());

  // connect to WiFi
  while (!wifi.connect().isOk())
    Serial.println(wifi.exception.toString());

  // connect to Telegram API
  while (!telegram.begin().isOk())
    Serial.println(telegram.exception.toString());

  Serial.println("Camera OK");
  Serial.println("Telegram OK");
}

void loop() {
  // capture picture
  if (!camera.capture().isOk()) {
    Serial.println("Capture error:");
    Serial.println(camera.exception.toString());
    return;
  }

  // run motion detection
  if (!detection.run().isOk()) {
    Serial.println("Motion detection error:");
    Serial.println(detection.exception.toString());
    return;
  }

  // on motion, perform action
  if (detection.triggered()) {
    if (telegram.to(TELEGRAM_CHAT).send(camera.frame).isOk()) {
      Serial.println("Photo sent to Telegram");
    } else {
      Serial.println("Sending error:");
      Serial.println(telegram.exception.toString());
    }
  }
}

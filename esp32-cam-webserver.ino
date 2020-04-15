#include "esp_camera.h"
#include <WiFi.h>

//ESP32 Dev Module
//PSRAM: disabled
//Partition scheme: Huge APP

#define CAMERA_MODEL_M5STACK_NO_PSRAM

// Select camera module used on the board
#define CAMERA_MODULE_OV2640
//#define CAMERA_MODULE_OV3660

#include "myconfig.h"

char myName[] = CAM_NAME;

// This will be displayed to identify the firmware
char myVer[] PROGMEM = __DATE__ " @ " __TIME__;

#include "camera_pins.h"

// Status and illumination LED's
#ifdef LAMP_PIN 
  int lampVal = 0; // Current Lamp value, range 0-100, Start off
#else 
  int lampVal = -1; // disable Lamp
#endif         
int lampChannel = 7;     // a free PWM channel (some channels used by camera)
const int pwmfreq = 50000;     // 50K pwm frequency
const int pwmresolution = 9;   // duty cycle bit range
// https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms
const int pwmIntervals = 100;  // The number of Steps between the output being on and off
float lampR;                   // The R value in the PWM graph equation (calculated in setup)

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("====");
  Serial.print("esp32-cam-webserver: ");
  Serial.println(myName);
  Serial.print("Code Built: ");
  Serial.println(myVer);

#ifdef LED_PIN  // If we have a notification LED set it to output
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LED_OFF); 
#endif

#ifdef LAMP_PIN
  ledcSetup(lampChannel, pwmfreq, pwmresolution); // configure LED PWM channel
  ledcWrite(lampChannel, lampVal);                // set initial value
  ledcAttachPin(LAMP_PIN, lampChannel);           // attach the GPIO pin to the channel 
  // Calculate the PWM scaling R factor: 
  // https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms
  lampR = (pwmIntervals * log10(2))/(log10(pow(2,pwmresolution)));
#endif

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_SVGA);
  s->set_vflip(s, 1);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  // Feedback that hardware init is complete and we are now attempting to connect
  Serial.println("");
  Serial.print("Connecting to Wifi Netowrk: ");
  Serial.println(ssid);
  flashLED(400);
  delay(100);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);  // Wait for Wifi to connect. If this fails wifi the code basically hangs here.
                 // - It would be good to do something else here as a future enhancement.
                 //   (eg: go to a captive AP config portal to configure the wifi)
  }

  // feedback that we are connected
  Serial.println("WiFi connected");
  Serial.println("");
  flashLED(200);
  delay(100);
  flashLED(200);
  delay(100);
  flashLED(200);

  // Start the Stream server, and the handler processes for the Web UI.
  startCameraServer();

  Serial.print("Camera Ready!  Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  flashLED(400);
}

// Notification LED 
void flashLED(int flashtime)
{
#ifdef LED_PIN                    // If we have it; flash it.
  digitalWrite(LED_PIN, LED_ON);  // On at full power.
  delay(flashtime);               // delay
  digitalWrite(LED_PIN, LED_OFF); // turn Off
#else
  return;                         // No notifcation LED, do nothing, no delay
#endif
} 


void loop() {
  // Just loop forever.
  // The stream and URI handler processes initiated by the startCameraServer() call at the
  // end of setup() will handle the camera and UI processing from now on.
  flashLED(400);
  delay(10000);
}

// Pin definitions for soem common ESP-CAM modules

// Common M5 Stack without PSRAM
#if defined(CAMERA_MODEL_M5STACK_NO_PSRAM)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM 15
#define XCLK_GPIO_NUM 27
#define SIOD_GPIO_NUM 25
#define SIOC_GPIO_NUM 23
#define Y9_GPIO_NUM 19
#define Y8_GPIO_NUM 36
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 39
#define Y5_GPIO_NUM 5
#define Y4_GPIO_NUM 34
#define Y3_GPIO_NUM 35
#define Y2_GPIO_NUM 17
#define VSYNC_GPIO_NUM 22
#define HREF_GPIO_NUM 26
#define PCLK_GPIO_NUM 21
#define LED_PIN 16
#define LED_ON HIGH
#define LED_OFF LOW 
// Note NO PSRAM,; so maximum working resolution is XGA 1024×768

#else
#error "Camera model not selected"
#endif

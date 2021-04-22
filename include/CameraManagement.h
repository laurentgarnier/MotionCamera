#include "Arduino.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h"          //disable brownout problems
#include "soc/rtc_cntl_reg.h" //disable brownout problems
#include "esp_http_server.h"
#include <FS.h>
#include "SD_MMC.h"
#include <EEPROM.h>

#define PART_BOUNDARY "123456789000000000000987654321"

#define CAMERA_MODEL_AI_THINKER

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

bool serverIsOn = false;

httpd_handle_t stream_httpd = NULL;

typedef struct {
    uint8_t * buffer;            
    size_t bufferLength;                 
} acquisitionResult;

void initCamera()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
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
    config.frame_size = FRAMESIZE_XGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    // sensor_t *s = esp_camera_sensor_get();

    // s->set_brightness(s, 0); // -2 to 2
    // s->set_contrast(s, 0);                     // -2 to 2
    // s->set_saturation(s, 0);                   // -2 to 2
    // s->set_special_effect(s, 0);               // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    // s->set_whitebal(s, 1);                     // 0 = disable , 1 = enable
    // s->set_awb_gain(s, 1);                     // 0 = disable , 1 = enable
    // s->set_wb_mode(s, 0);                      // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    // s->set_exposure_ctrl(s, 1);                // 0 = disable , 1 = enable
    // s->set_aec2(s, 0);                         // 0 = disable , 1 = enable
    // s->set_ae_level(s, 0);                     // -2 to 2
    // s->set_aec_value(s, 300);                  // 0 to 1200
    // s->set_gain_ctrl(s, 1);                    // 0 = disable , 1 = enable
    // s->set_agc_gain(s, 0);                     // 0 to 30
    // s->set_gainceiling(s, (gainceiling_t)0);   // 0 to 6
    // s->set_bpc(s, 0);                          // 0 = disable , 1 = enable
    // s->set_wpc(s, 1);                          // 0 = disable , 1 = enable
    // s->set_raw_gma(s, 1);                      // 0 = disable , 1 = enable
    // s->set_lenc(s, 1);                         // 0 = disable , 1 = enable
    // s->set_hmirror(s, 0);                      // 0 = disable , 1 = enable
    // s->set_vflip(s, 0);                        // 0 = disable , 1 = enable
    // s->set_dcw(s, 1);                          // 0 = disable , 1 = enable
    // s->set_colorbar(s, 0);                     // 0 = disable , 1 = enable
}

acquisitionResult takePicture()
{ 
    Serial.println("Acquisition");
    camera_fb_t * fb = esp_camera_fb_get();
    acquisitionResult acq;
    acq.bufferLength = fb->len;
    Serial.println("Allocation buffer " + String(fb->len));
    acq.buffer = (uint8_t *)malloc(fb->len);
    
    Serial.println("Memcpy " + String((int)&fb) );
    memcpy(acq.buffer, fb->buf, fb->len);
    Serial.println("Fin acquisition");
    esp_camera_fb_return(fb);
    
    return acq;
}

static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
    {
        return res;
    }

    while (true)
    {
        fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        }
        else
        {
            if (fb->width > 400)
            {
                if (fb->format != PIXFORMAT_JPEG)
                {
                    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                    esp_camera_fb_return(fb);
                    fb = NULL;
                    if (!jpeg_converted)
                    {
                        Serial.println("JPEG compression failed");
                        res = ESP_FAIL;
                    }
                }
                else
                {
                    _jpg_buf_len = fb->len;
                    _jpg_buf = fb->buf;
                }
            }
        }
        if (res == ESP_OK)
        {
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
            //res = httpd_resp_send_chunk(req, (const char *)publicPictureBuf, _jpg_buf_len);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (fb)
        {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        }
        else if (_jpg_buf)
        {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if (res != ESP_OK)
        {
            break;
        }
    }
    return res;
}

static httpd_uri_t getUri()
{
    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL};

        return index_uri;
}

void startCameraServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    httpd_uri_t index_uri = getUri();

    //Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(stream_httpd, &index_uri);
        serverIsOn = true;
    }
}

void stopCameraServer()
{
    httpd_unregister_uri_handler(stream_httpd, getUri().uri, getUri().method);
    if(httpd_stop(&stream_httpd) == ESP_OK)
        serverIsOn = false;
}

void restartESP32Cam()
{
    esp_sleep_enable_timer_wakeup(10000000);
    esp_deep_sleep_start();
}
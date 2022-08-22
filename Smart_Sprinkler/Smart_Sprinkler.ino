#include "config.h"
#include "esp_http_server.h"
#include "webPage.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiManager.h>

const char *ssidAP     = WIFI_AP_SSID;
const char *passwordAP = WIFI_AP_PASS;

/* #region  EEPROM config */
config_t config;
/* #endregion */

/* #region  Web Server config */
httpd_handle_t          main_server;
StaticJsonDocument<400> json_doc;
char                    json_output[200];
DeserializationError    json_error;
/* #endregion */

/* #region  Main Task var */
int      humi_raw_val = 0;
int      pVal         = 0;
bool     bumpState    = false;
uint64_t minute       = 1;
/* #endregion */

/* #region  WiFi Manager Config */
WiFiManager wm;
/* #endregion */

void setup()
{
    Serial.begin(UART_BAUDRATE);
    Serial.setDebugOutput(true);

    wm.setHostname(DEVICE_NAME);
    wm.setConfigPortalTimeout(30);
    if (!wm.autoConnect(WIFI_AP_SSID)) {
        WiFi.mode(WIFI_AP);
        LOG_PRINTLN("Configuring soft-AP...");
        WiFi.softAP(ssidAP);
    } else {
        LOG_PRINTLN("Configuring STA...");
    }

    InitWiFiManager();
    InitOTAConfig();
    InitPump();
    main_server = start_webserver();
    EEPROM.begin(128);
    EEPROM.get(EEPROM_ADDR, config);
    if (config.check != CHECK_DATA) {
        // Load Default value
        LOG_PRINTLN("\nLoad Default config value");
        config.check    = CHECK_DATA;
        config.lowp     = 25;         // 25%
        config.interval = 60;         // 60 min
        config.waterSec = 10;         // 10 s
        config.maxLimit = 1500;       // adc 1500
        config.minLimit = 2100;       // adc 2100
        config.mode     = MANUAL;     // manual mode
        EEPROM.put(EEPROM_ADDR, config);
        EEPROM.commit();
    }
}

bool InitOTAConfig()
{
    ArduinoOTA.setHostname("Smart-Sprinkler-PICO");
    ArduinoOTA.setPassword("12345678");
    ArduinoOTA.begin();
    DEBUG_PRINTLN("OTA ready!");
    return true;
}

bool InitPump()
{
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(HUMI_SNESOR_PIN, INPUT);
    // init turn off
    digitalWrite(PUMP_PIN, LOW);
    return true;
}

bool InitWiFiManager()
{
    pinMode(BTN_PIN, INPUT_PULLUP);
    return true;
}

void loop()
{
    ArduinoOTA.handle();
    doWiFiManager();
    mainTask();
}

void mainTask(void)
{
    static uint32_t getADC_Timer;
    static uint32_t waterSec_timer;
    static uint32_t minute_timer = millis() + 60000;
    static uint32_t show_ip_timer;
    static bool     start = false;

    if (millis() > minute_timer) {
        minute_timer = millis() + 60000;
        minute++;
    }

    switch (config.mode) {
    case AUTO: {
        if (pVal < config.lowp - (config.lowp * FUZZY_P * 0.01)) {
            digitalWrite(PUMP_PIN, HIGH);
        } else if (pVal > config.lowp + (config.lowp * FUZZY_P * 0.01)) {
            digitalWrite(PUMP_PIN, LOW);
        }
    } break;
    case MANUAL: {
        digitalWrite(PUMP_PIN, bumpState);
    } break;
    case TIMING: {
        if (minute >= config.interval) {
            minute         = 0;
            waterSec_timer = millis() + (config.waterSec * 1000);
            start          = true;
            digitalWrite(PUMP_PIN, HIGH);
        }
        if (start) {
            if (millis() > waterSec_timer) {
                minute_timer = millis() + 60000;
                start        = false;
                digitalWrite(PUMP_PIN, LOW);
            }
        } else {
            digitalWrite(PUMP_PIN, LOW);
        }
    } break;
    default:
        break;
    }

    if (millis() > getADC_Timer) {
        getADC_Timer = millis() + 100;
        humi_raw_val = analogRead(HUMI_SNESOR_PIN);
        pVal         = map(humi_raw_val, config.minLimit, config.maxLimit, 0, 100);
        pVal         = MIN(pVal, 100);
        pVal         = MAX(pVal, 0);
    }

    // show wifi ip
    if (millis() > show_ip_timer) {
        show_ip_timer = millis() + 10000;
        String ip     = "None";
        switch (WiFi.getMode()) {
        case WIFI_MODE_AP:
            ip = WiFi.softAPIP().toString();
            break;
        case WIFI_MODE_STA:
            ip = WiFi.localIP().toString();
            break;
        }
        LOG_PRINT("LocalIP: ");
        LOG_PRINTLN(ip);
    }
}

void doWiFiManager(void)
{
    // is configuration portal requested?
    if (digitalRead(BTN_PIN) == LOW) {
        stop_webserver(main_server);
        LOG_PRINTLN("Button Pressed, Starting Web Portal");
        wm.setHostname(DEVICE_NAME);
        wm.setConfigPortalTimeout(180);
        if (!wm.startConfigPortal(WIFI_AP_SSID)) {
            WiFi.mode(WIFI_AP);
            LOG_PRINTLN("Configuring soft-AP...");
            WiFi.softAP(ssidAP);
        }
        main_server = start_webserver();
    }
}

esp_err_t SetBumpON_handler(httpd_req_t *req)
{
    bumpState = true;
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);

    DEBUG_PRINTLN("SetBumpON_handler");
    return ESP_OK;
}

esp_err_t SetBumpOFF_handler(httpd_req_t *req)
{
    bumpState = false;
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    DEBUG_PRINTLN("SetBumpOFF_handler");
    return ESP_OK;
}

esp_err_t GetWaterVal_handler(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    json_doc.clear();
    json_doc["valP"] = pVal;
    json_doc["raw"]  = humi_raw_val;
    serializeJson(json_doc, json_output);
    httpd_resp_send(req, json_output, HTTPD_RESP_USE_STRLEN);
    DEBUG_PRINT("GetWaterVal_handler: ");
    DEBUG_PRINTLN(json_output);
    return ESP_OK;
}

esp_err_t GetSetVal_handler(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    json_doc.clear();
    json_doc["mode"]     = config.mode;
    json_doc["highVal"]  = config.maxLimit;
    json_doc["lowVal"]   = config.minLimit;
    json_doc["lowLimit"] = config.lowp;
    json_doc["interval"] = config.interval;
    json_doc["waterSec"] = config.waterSec;
    serializeJson(json_doc, json_output);
    httpd_resp_send(req, json_output, HTTPD_RESP_USE_STRLEN);
    DEBUG_PRINT("GetSetVal_handler: ");
    DEBUG_PRINTLN(json_output);
    return ESP_OK;
}

esp_err_t GetMainPage(httpd_req_t *req)
{
    DEBUG_PRINTLN("webpage loading");
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, MAIN_page, HTTPD_RESP_USE_STRLEN);
}

esp_err_t SetMode_handler(httpd_req_t *req)
{
    char   content[50];
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    config.mode = atoi(content);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);

    DEBUG_PRINT("SetMode_handler: ");
    DEBUG_PRINTLN(config.mode);
    return ESP_OK;
}

esp_err_t SetSave_handler(httpd_req_t *req)
{
    char   content[200];
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    json_doc.clear();
    json_error = deserializeJson(json_doc, content);
    if (!json_error) {
        config.maxLimit = json_doc["highVal"];
        config.minLimit = json_doc["lowVal"];
        config.lowp     = json_doc["lowLimit"];
        config.interval = json_doc["interval"];
        config.waterSec = json_doc["waterSec"];
        EEPROM.put(EEPROM_ADDR, config);
        EEPROM.commit();
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    }

    DEBUG_PRINT("SetSave_handler: ");
    DEBUG_PRINTLN(content);
    return ESP_OK;
}

httpd_uri_t uri_get_mainPage = {.uri = "/", .method = HTTP_GET, .handler = GetMainPage, .user_ctx = NULL};
httpd_uri_t uri_get_waterVal = {.uri = "/waterVal", .method = HTTP_GET, .handler = GetWaterVal_handler, .user_ctx = NULL};
httpd_uri_t uri_get_setVal   = {.uri = "/setVal", .method = HTTP_GET, .handler = GetSetVal_handler, .user_ctx = NULL};
httpd_uri_t uri_set_save     = {.uri = "/save", .method = HTTP_POST, .handler = SetSave_handler, .user_ctx = NULL};
httpd_uri_t uri_get_bumpOn   = {.uri = "/bumpOn", .method = HTTP_GET, .handler = SetBumpON_handler, .user_ctx = NULL};
httpd_uri_t uri_get_bumpOff  = {.uri = "/bumpOff", .method = HTTP_GET, .handler = SetBumpOFF_handler, .user_ctx = NULL};
httpd_uri_t uri_post_mode    = {.uri = "/mode", .method = HTTP_POST, .handler = SetMode_handler, .user_ctx = NULL};

httpd_handle_t start_webserver(void)
{
    httpd_config_t config   = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;

    httpd_handle_t server = NULL;

    // Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get_mainPage);
        httpd_register_uri_handler(server, &uri_get_waterVal);
        httpd_register_uri_handler(server, &uri_get_setVal);
        httpd_register_uri_handler(server, &uri_set_save);
        httpd_register_uri_handler(server, &uri_get_bumpOn);
        httpd_register_uri_handler(server, &uri_get_bumpOff);
        httpd_register_uri_handler(server, &uri_post_mode);
    }
    return server;
}

void stop_webserver(httpd_handle_t server)
{
    if (server) {
        httpd_stop(server);
    }
}
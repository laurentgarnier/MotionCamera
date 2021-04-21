#include <Arduino.h>

String ssid = "ssid";
String password = "pwd";

String serveurMqtt = "server IP";
String nomDevice = "device name";
String categorieDevice = "device category";

void decodeConfigFile(String data)
{
    Serial.println("Decodage configuration");
    Serial.println(data);

    // Les données arrivent en JSON sous la forme
    // {
    //     "ssid": "[SSID name]",
    //     "ssidPwd": "SSID_PWD",
    //     "deviceName": "[Device name]",
    //     "deviceCategory": "[Devicecategory]",
    //     "MQTTServerAddress": "[IP address]"
    // }
    DynamicJsonDocument json(256);
    DeserializationError error = deserializeJson(json, data);

    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    JsonObject obj = json.as<JsonObject>();

    ssid = obj["ssid"].as<String>();
    password = obj["ssidPwd"].as<String>();
    nomDevice = obj["deviceName"].as<String>();
    categorieDevice = obj["deviceCategory"].as<String>();
    serveurMqtt = obj["MQTTServerAddress"].as<String>();

    Serial.println("Config : " + ssid + " - " + password + " - " + nomDevice + " - " + categorieDevice + " - " + serveurMqtt);

}
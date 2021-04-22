#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>

String ssid = "ssid";
String password = "pwd";

String serveurMqtt = "server IP";
String nomDevice = "device name";
String categorieDevice = "device category";

String emailSenderAccount = "##############";
String emailSenderPassword = "##############";
String smtpServer = "##############";
int smtpServerPort = 000;
String emailSender = "##############";
String emailSubject = "##############";
String emailMessage = "##############";
String emailRecipient = "##############";

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
    //     "MQTTServerAddress": "[IP address]",
    //     "emailSenderAccount": "[email Sender Account]",
    //     "emailSenderPassword": "[email Sender Password]",
    //     "smtpServer": "[smtp Server address]",
    //     "smtpServerPort": [smtp server port],
    //     "emailSender": "[email Sender shown by recipients]",
    //     "emailSubject": "[email Subject]",
    //     "emailMessage": "[email Message]",
    //     "emailRecipient": "[email Recipients separates by ,]"
    // }
    DynamicJsonDocument json(1024);
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
    emailSenderAccount = obj["emailSenderAccount"].as<String>();
    emailSenderPassword = obj["emailSenderPassword"].as<String>();
    smtpServer = obj["smtpServer"].as<String>();
    smtpServerPort = obj["smtpServerPort"];
    emailSender = obj["emailSender"].as<String>();
    emailSubject = obj["emailSubject"].as<String>();
    emailMessage = obj["emailMessage"].as<String>();
    emailRecipient = obj["emailRecipient"].as<String>();

    Serial.println("Config : " + ssid + " - " + password + " - " + nomDevice + " - " + categorieDevice + " - " + serveurMqtt);
}

#endif
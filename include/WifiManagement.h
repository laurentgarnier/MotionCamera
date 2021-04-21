#include <WiFi.h>

IPAddress connectToWifi(String ssid, String password)
{
    Serial.println("");
    Serial.println("Connexion au Wifi " + ssid);
    
    WiFi.begin(const_cast<char*>(ssid.c_str()), const_cast<char*>(password.c_str()));

    // Tant que non connecté
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    // Connexion réussie
    IPAddress adresseIP = WiFi.localIP();
    Serial.println("");
    Serial.println("WiFi OK");
    Serial.print("Adresse IP : ");
    Serial.println(adresseIP);
    Serial.println("");
    return adresseIP;
}

String macToStr(const uint8_t *mac)
{
    String result;
    for (int i = 0; i < 6; ++i)
    {
        result += String(mac[i], 16);
        if (i < 5)
            result += ':';
    }
    return result;
}
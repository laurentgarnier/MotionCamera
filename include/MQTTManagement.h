#include <PubSubClient.h>

bool doitPrendrePhotos = false;
bool streamModeOn = false;

void connecterAuServeurMQTT(String nomDuDevice, String categorieDuDevice, String adresseMAC, PubSubClient *client)
{
    if (!client->connected())
    {
        Serial.println("");
        Serial.println("Connexion au serveur MQTT");
        if (client->connect(adresseMAC.c_str()))
        {
            Serial.println("Connexion au serveur OK");
            String topic = "Maison/" + categorieDuDevice + "/" + nomDuDevice + "/ToObject";
            if (client->subscribe(topic.c_str()))
            {
                Serial.println("Abonnement au topic " + topic + " OK");
            }
            else
            {
                Serial.println("Abonnement au topic " + topic + "  KO");
            }
        }
    }
    Serial.println("");
}

boolean envoyerMessageDeVie(String nomDuDevice, String adresseMAC, IPAddress adresseIP, PubSubClient *client, String categorieDuDevice)
{
    if (!client->connected())
        connecterAuServeurMQTT(nomDuDevice, categorieDuDevice, adresseMAC, client);

    String payload = "{\"Mac\" : \"" + adresseMAC + "\", \"IP\" : \"" + adresseIP.toString() + "\"}";
    String topic = "Maison/Discovery/" + categorieDuDevice + "/" + nomDuDevice;
    
    boolean result = client->publish(topic.c_str(), (char *)payload.c_str());
    Serial.println("Envoi du message de vie sur " + topic + " - Contenu " + payload + " - " + String(result));
    return result;
}

boolean publierMessage(String nomDuDevice, String categorieDuDevice, String message, String subject, PubSubClient *client)
{
    String topic = "Maison/" + categorieDuDevice + "/" + nomDuDevice + "/FromObject/" + subject ;
    boolean result = client->publish(topic.c_str(), message.c_str());
    Serial.println("Envoi du message " + message + " sur " + topic + " - " + String(result));
    return result;
}

void gererReceptionMessage(char *topic, byte *payload, unsigned int len)
{
  char msg[len];
  for (unsigned int i = 0; i < len; i++)
    msg[i] = payload[i];

  String data = String(msg);
  Serial.println(data);
  Serial.println("");

  // Les données arrivent en JSON sous la forme {"photo":[1/0],"streamMode":[0/1]}
  DynamicJsonDocument json(64);
  DeserializationError error = deserializeJson(json, data);

  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  JsonObject obj = json.as<JsonObject>();

  int photo = obj["photo"];
  int streamMode = obj["streamMode"];

  doitPrendrePhotos = (photo == 1);
  streamModeOn = (streamMode == 1);
}
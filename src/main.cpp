#include "Arduino.h"
#include <ArduinoJson.h>
#include "WifiManagement.h"
#include "CameraManagement.h"
#include "MQTTManagement.h"
#include "MailManagement.h"
#include "Configuration.h"
#include "SDCardManagement.h"

String adresseMAC;
IPAddress adresseIP;
WiFiClient clientWifi;
PubSubClient *clientMqtt;

int timingDernierEnvoiDesDonnees;
const int periodeEnvoiMessageDeVieEnMs = 10000; // Toutes les 10s

int indexAcquisition = 0;
const int nbPhoto = 10;
acquisitionResult acquisitions[nbPhoto];
int timingDerniereAcquisition;
const int periodeAcquisitionEnMs = 1000; // Toutes les 1s

String configFile = "/Configuration/Config.json";

void blinkLed(int nbTimes)
{
  for(int indexTimes = 0; indexTimes < nbTimes; nbTimes++)
  {
    uint8_t state = 0;
    if(indexTimes %2 == 0)
      state = 1;
    digitalWrite(4, state);
    delay(250);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  // Gestion de la carte SD pour stockage temporaire des images acquises
  mountSdCard();
  if (!isDirectoryExists("/Pictures"))
    createDir("/Pictures");

  if (isFileExists(configFile.c_str()))
  {
    size_t fileSize = getFileSize(configFile.c_str());
    char *fileData = (char *)malloc(fileSize);
    readFile(configFile.c_str(), fileData);
    decodeConfigFile(String((char *)fileData));
  }
  else
  {
    blinkLed(20);
  }

  // initialisation de la caméra
  initCamera();

  // Connexion Wi-fi
  adresseIP = connectToWifi(ssid, password);
  // Récupération de l'adresse MAC du device
  uint8_t mac[6];
  WiFi.macAddress(mac);
  adresseMAC = macToStr(mac);
  // abonnement au topic de réception des messages MQTT
  clientMqtt = new PubSubClient(serveurMqtt.c_str(), 1883, gererReceptionMessage, clientWifi);
  // Connexion au servuer MQTT
  connecterAuServeurMQTT(nomDevice, categorieDevice, adresseMAC, clientMqtt);
  // Envoi du 1er message de vie sur le topic de découverte
  envoyerMessageDeVie(nomDevice, adresseMAC, adresseIP, clientMqtt, categorieDevice);
  timingDernierEnvoiDesDonnees = millis();

  // Lancement serveur de streaming
  //startCameraServer();
}

void loop()
{
  int timingCourant = millis();

  // gestion du message de vie
  if ((timingCourant - timingDernierEnvoiDesDonnees) > periodeEnvoiMessageDeVieEnMs)
  {
    envoyerMessageDeVie(nomDevice, adresseMAC, adresseIP, clientMqtt, categorieDevice);
    timingDernierEnvoiDesDonnees = timingCourant;
  }

  // gestion des acquisitions
  if (doitPrendrePhotos)
  {
    // fin des acquisitions, envoi du mail
    if (indexAcquisition >= nbPhoto)
    {
      indexAcquisition = 0;
      doitPrendrePhotos = false;
      // sendPhoto(acquisitions, nbPhoto);
      // for (int i = 0; i < nbPhoto; i++)
      // {
      //   free(acquisitions[i].buffer);
      // //  acquisitions[i] = NULL;
      // }
    }
    else if ((timingCourant - timingDerniereAcquisition) > periodeAcquisitionEnMs)
    {
      // Acquisitions
      Serial.println("Acquisition " + String(indexAcquisition));
      String nomImage = String("/Pictures/Picture" + String(indexAcquisition) + ".jpg");
      acquisitionResult acquisition = takePicture();
      writeFile(nomImage.c_str(), acquisition.buffer, acquisition.bufferLength);
      free(acquisition.buffer);
      indexAcquisition++;
      timingDerniereAcquisition = timingCourant;
    }
  }

  // gestion de la perte de connexion WIFI, réinitialisation
  if (WiFi.status() != WL_CONNECTED)
  {
    // On vient de perdre la connexion WiFi, on reboot
    restartESP32Cam();
  }
  // boucle d'attente de réception des messages MQTT
  clientMqtt->loop();
}

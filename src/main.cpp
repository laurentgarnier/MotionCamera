#include "Arduino.h"
#include <ArduinoJson.h>
#include "WifiManagement.h"
#include "CameraManagement.h"
#include "MQTTManagement.h"
#include "SDCardManagement.h"
#include "MailManagement.h"
#include "Configuration.h"

String adresseMAC;
IPAddress adresseIP;
WiFiClient clientWifi;
PubSubClient *clientMqtt;

int timingDernierEnvoiDesDonnees;
const int periodeEnvoiMessageDeVieEnMs = 10000; // Toutes les 10s

int indexAcquisition = 0;
const int nbPhoto = 10;
int timingDerniereAcquisition;
const int periodeAcquisitionEnMs = 1000; // Toutes les 1s

String configFile = "/Configuration/Config.json";
String picturesDirectory = "/Pictures";
String pictureName = "Picture";

bool acquisitionsMailEncours = false;

void blinkLed(int nbTimes)
{
  for (int indexTimes = 0; indexTimes < nbTimes; nbTimes++)
  {
    uint8_t state = LOW;
    if (indexTimes % 2 == 0)
      state = HIGH;
    digitalWrite(4, state);
    delay(250);
  }
   digitalWrite(4, LOW);
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  
  pinMode(4, OUTPUT);
  //MailClient.sdBegin();
  // Gestion de la carte SD pour stockage temporaire des images acquises
  mountSdCard();
  if (isDirectoryExists(picturesDirectory.c_str()))
    removeDir(picturesDirectory.c_str());

  createDir(picturesDirectory.c_str());

  // Récupération de la configuration sur la carte SD
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

  // lancement du serveur de streaming si il n'est pas déjà lancé et si on n'est pas en cours
  // d'acquisition pour le mail
  if(streamModeOn && !serverIsOn && !acquisitionsMailEncours)
  {
    Serial.println("Request to start streaming server");
    // Lancement serveur de streaming
    startCameraServer();
  }

  if(!streamModeOn && serverIsOn)
  {
    Serial.println("Request to stop streaming server");
    stopCameraServer();
  }

  // gestion des acquisitions 
  //(uniquement si le serveur de streming n'est pas activé pour éviter le conflit de récupération du buffer caméra)
  if (doitPrendrePhotos && !serverIsOn)
  {
    if(serverIsOn)
    {
      Serial.println("Try to stop streaming server due to mail request");
      stopCameraServer();
      return;
    }

    acquisitionsMailEncours = true;
    // fin des acquisitions, envoi du mail
    if (indexAcquisition >= nbPhoto)
    {
      indexAcquisition = 0;
      doitPrendrePhotos = false;
      digitalWrite(4, LOW);
      sendMail(picturesDirectory, nbPhoto);
      acquisitionsMailEncours = false;
    }
    else if ((timingCourant - timingDerniereAcquisition) > periodeAcquisitionEnMs)
    {
      // Acquisitions
      Serial.println("Acquisition " + String(indexAcquisition));
      String nomImage = picturesDirectory + String("/") + pictureName + String(indexAcquisition) + ".jpg";
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

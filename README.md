# MotionCamera




# WARNING
Pour envoyer le mail avec les pièces jointes stockées sur la carte SD, il faut paramétrer la librairie ESP Mail client
libdeps\esp32cam\ESP Mail Client\src\ESP_Mail_FS.h
#include <SD_MMC.h>
#define ESP_Mail_DEFAULT_SD_FS SD_MMC //For ESP32 SDMMC
#define CARD_TYPE_SD_MMC 1

//#define ESP_Mail_DEFAULT_SD_FS SD //For ESP32 SDMMC
//#define CARD_TYPE_SD 1
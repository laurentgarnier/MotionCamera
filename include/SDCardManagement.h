// This post referred to this git. I just keep methods I need
// https://github.com/v12345vtm/CameraWebserver2SD/blob/master/CameraWebserver2SD/CameraWebserver2SD.ino

#include "FS.h"
#include "SD_MMC.h"

//Create a dir in SD card
void createDir(const char *path)
{
    Serial.printf("Creating Dir: %s\n", path);
    if (SD_MMC.mkdir(path))
    {
        Serial.println("Dir created");
    }
    else
    {
        Serial.println("mkdir failed");
    }
}

//delete a dir in SD card
void removeDir(const char *path)
{
    Serial.printf("Removing Dir: %s\n", path);
    if (SD_MMC.rmdir(path))
    {
        Serial.println("Dir removed");
    }
    else
    {
        Serial.println("rmdir failed");
    }
}

//Write a file in SD card
void writeFile(const char *path, uint8_t *buffer, size_t bufferLen)
{
    Serial.printf("Writing file: %s\n", path);

    File file = SD_MMC.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    else
    {
        file.write(buffer, bufferLen);
        Serial.println("File written");
    }
    file.close();
}

size_t getFileSize(const char *filePath)
{
    size_t fileSize = 0;
    File file = SD_MMC.open(filePath, FILE_READ);
    if (!file)
        Serial.println("Failed to open file for reading");
    else
        fileSize = file.size();

    if (file)
        file.close();
    return fileSize;
}

void readFile(const char *path, char *result)
{
    Serial.printf("Reading file: %s\n", path);

    File file = SD_MMC.open(path, FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }
    else
    {
        file.readBytes((char *)result, file.size());
        Serial.println("File read");
    }
    file.close();
}

//Delete a file in SD card
void deleteFile(const char *path)
{
    Serial.printf("Deleting file: %s\n", path);
    if (SD_MMC.remove(path))
    {
        Serial.println("File deleted");
    }
    else
    {
        Serial.println("Delete failed");
    }
}

bool mountSdCard()
{
    if (!SD_MMC.begin())
    {
        Serial.println("Card Mount Failed");
        return false;
    }
    Serial.println("Card Mount OK");
    return true;
}

void unmontSdCard()
{
    Serial.println("Card unmount");
    SD_MMC.end();
}

bool isDirectoryExists(const char *directory)
{
    Serial.printf("Check if directory: %s exists\n", directory);

    File root = SD_MMC.open(directory);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return false;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return false;
    }
    Serial.println("Directory exist");
    return true;
}

bool isFileExists(const char *filePath)
{
     Serial.printf("Check if file: %s exists\n", filePath);
    bool result = SD_MMC.exists(filePath);

    if(result)
         Serial.printf("file: %s exists\n", filePath);
    else
         Serial.printf("file: %s does not exists\n", filePath);
    return result;
}
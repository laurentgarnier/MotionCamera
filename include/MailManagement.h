#include "ESP_Mail_Client.h"
#ifndef CONFIGURATION_H
#include "Configuration.h"
#endif

SMTPSession smtp;

//Callback function to get the Email sending status
void smtpCallback(SMTP_Status info);

void sendMail(String picturesDirectory, byte nbFile)
{
  smtp.debug(1);
  smtp.callback(smtpCallback);

  ESP_Mail_Session session;
  session.server.host_name = smtpServer.c_str();
  session.server.port = smtpServerPort;
  session.login.email = emailSenderAccount.c_str();
  session.login.password = emailSenderPassword.c_str();

  SMTP_Message message;
  message.enable.chunking = true;
  message.sender.name = emailSender.c_str();
  message.sender.email = emailSenderAccount.c_str();

  message.subject = emailSubject.c_str();

  int indexVirgule = 0;
  int lastIndexVirgule = 0;
  do
  {
    indexVirgule = emailRecipient.indexOf(",", lastIndexVirgule);
    String recipient;

    if (indexVirgule > 0)
    {
      recipient = emailRecipient.substring(lastIndexVirgule, indexVirgule);
      // La recherche suivante commence après la virgule
      lastIndexVirgule = indexVirgule + 1;
    }
    else
    {
      recipient = emailRecipient.substring(lastIndexVirgule, emailRecipient.length());
    }
    recipient.trim();

    char *destinataire = (char *)malloc(recipient.length());
    strcpy(destinataire, recipient.c_str());

    // Serial.println("envoi a : " + recipient + " " + destinataire);

    message.addRecipient(destinataire, destinataire);

  } while (indexVirgule > 0);

  message.text.content = emailMessage.c_str();
  message.text.charSet = "utf-8";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_base64;
  message.priority = esp_mail_smtp_priority_normal;

  SMTP_Attachment att;

  int indexPicture = 0;
  while (indexPicture < nbFile)
  {
    String nomImage = picturesDirectory + String("/Picture") + String(indexPicture) + ".jpg";

    char *fileName = (char *)malloc(nomImage.length());
    strcpy(fileName, nomImage.c_str());

    // Serial.println("File : " + nomImage + " " + String(nomImage.length()) + " filename : " + fileName);

    att.descr.filename = fileName;
    att.descr.mime = "image/jpeg";
    att.file.path = fileName;
    att.file.storage_type = esp_mail_file_storage_type_sd;
    att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

    message.addParallelAttachment(att);

    message.resetAttachItem(att);
    indexPicture++;
  }

  if (!smtp.connect(&session))
    return;

  if (!MailClient.sendMail(&smtp, &message, true))
    Serial.println("Error sending Email, " + smtp.errorReason());
}

//Callback function to get the Email sending status
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      localtime_r(&result.timesstamp, &dt);

      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      Serial.printf("Recipient: %s\n", result.recipients);
      Serial.printf("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}
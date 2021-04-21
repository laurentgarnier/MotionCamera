#include <ESP_Mail_Client.h>

#define emailSenderAccount "maisoncroziergarnier@gmail.com"
#define emailSenderPassword "GMAC@s77L*74IL"
#define smtpServer "smtp.gmail.com"
#define smtpServerPort 465
#define emailSubject "J ai vu Filou"
#define emailRecipient "lologar69@gmail.com"

// The Email Sending data object contains config and data to send
SMTPSession smtp;
ESP_Mail_Session session;

// Callback function to get the Email sending status
void smtpCallback(SMTP_Status msg)
{
    //Print the current status
    Serial.println(msg.info());
}

void sendPhoto(acquisitionResult acquisitions[], int imagesNumber)
{
    Serial.println("Sending email...");
    session.server.host_name = smtpServer;
    session.server.port = smtpServerPort;
    session.login.email = emailSenderAccount;
    session.login.password = emailSenderPassword;

    SMTP_Message message;

    message.enable.chunking = true;
    message.sender.name = "Filou cam";
    message.sender.email = emailSenderAccount;
    message.subject = "J ai vu Filou";
    message.addRecipient("lolo", "lologar69@gmail.com");

    message.text.content = "Regarde mon Filou s'il est beau\r\nN'est-ce pas ?";
    message.text.charSet = "utf-8";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_base64;

    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_normal;

    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;
    SMTP_Attachment att;
    for (int indexBuffer = 0; indexBuffer < imagesNumber; indexBuffer++)
    {
        String nomImage = String("Filou" + String(indexBuffer) + ".jpg");
        att.descr.filename = nomImage.c_str();
        att.descr.mime = "image/jpg";
        att.blob.data = acquisitions[indexBuffer].buffer;
        att.blob.size = acquisitions[indexBuffer].bufferLength;
        att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;
        message.addAttachment(att);
       // message.resetAttachItem(att);
    }
    smtp.debug(1);
    if (!smtp.connect(&session))
        return;

    if (!MailClient.sendMail(&smtp, &message, true))
        Serial.println("Error sending Email, " + smtp.errorReason());
}

void sendPhoto(String photoPath)
{
    // Preparing email
    Serial.println("Sending email...");
    // smtp.callback(smtpCallback);

    if (!SD_MMC.begin())
    {
        Serial.println("/!\\ Echec initialisation carte SD");
    }
    File file = SD_MMC.open(photoPath, FILE_READ);
    int arraySize = static_cast<int>(file.size());
    static uint8_t buf[512];

    while (file.available())
    {
        file.read(buf, arraySize);
    }
    file.close();
    session.server.host_name = smtpServer;
    session.server.port = smtpServerPort;
    session.login.email = emailSenderAccount;
    session.login.password = emailSenderPassword;
    //  session.login.user_domain = "mydomain.net";

    SMTP_Message message;

    /* Enable the chunked data transfer with pipelining for large message if server supported */
    message.enable.chunking = true;

    /* Set the message headers */
    message.sender.name = "Filou cam";
    message.sender.email = emailSenderAccount;
    message.subject = "J ai vu Filou";
    message.addRecipient("lolo", "lologar69@gmail.com");

    /** The content transfer encoding e.g.
   * enc_7bit or "7bit" (not encoded)
   * enc_qp or "quoted-printable" (encoded)
   * enc_base64 or "base64" (encoded)
   * enc_binary or "binary" (not encoded)
   * enc_8bit or "8bit" (not encoded)
   * The default value is "7bit"
  */
    message.text.content = "Regarde mon Filou s'il est beau\r\nN'est-ce pas ?";
    message.text.charSet = "utf-8";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_base64;

    /** The message priority
   * esp_mail_smtp_priority_high or 1
   * esp_mail_smtp_priority_normal or 3
   * esp_mail_smtp_priority_low or 5
   * The default value is esp_mail_smtp_priority_low
  */
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_normal;

    /** The Delivery Status Notifications e.g.
   * esp_mail_smtp_notify_never
   * esp_mail_smtp_notify_success
   * esp_mail_smtp_notify_failure
   * esp_mail_smtp_notify_delay
   * The default value is esp_mail_smtp_notify_never
  */
    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

    /* The attachment data item */
    SMTP_Attachment att;
    att.descr.filename = "Filou.jpg";
    att.descr.mime = "image/jpg";
    att.blob.data = buf;
    att.blob.size = arraySize;

    // att.file.path = photoPath.c_str();
    // att.file.storage_type = esp_mail_file_storage_type_sd;

    /* Need to be base64 transfer encoding for inline image */
    att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

    /** The orange.png file is already base64 encoded file.
   * Then set the content encoding to match the transfer encoding
   * which no encoding was taken place prior to sending.
   */
    //att.descr.content_encoding = Content_Transfer_Encoding::enc_base64;

    // message.addInlineImage(att);
    message.addAttachment(att);
    smtp.debug(1);
    /* Connect to server with the session config */
    if (!smtp.connect(&session))
        return;

    /* Start sending the Email and close the session */
    if (!MailClient.sendMail(&smtp, &message, true))
        Serial.println("Error sending Email, " + smtp.errorReason());
}

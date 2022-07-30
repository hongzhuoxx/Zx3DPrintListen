#include <stdio.h>
#include <string.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "USB.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <ESP_Mail_Client.h>

// 监测电量的ADC
#define adc3 1


// https://github.com/dojyorin/arduino_base64
// Base64_Codec
#include "base64.hpp"

#if ARDUINO_USB_CDC_ON_BOOT
#define HWSerial Serial0
#define USBSerial Serial
#else
#define HWSerial Serial
USBCDC USBSerial;
#endif

// 定义背景色
#define TFT_GREY 0x5AEB
uint16_t TFT_BGColor = 0xFDA0;
uint16_t TFT_FGColor = 0x915C;
// 创建TFT对象
TFT_eSPI tft = TFT_eSPI();
// 定义
WiFiMulti wifiMulti;
// Email 客户端
// Define the global used SMTP Session object which used for SMTP transsport
SMTPSession smtp;
// Define the global used session config data which used to store the TCP session configuration
ESP_Mail_Session session;
// ------------------------------------------------------------------------
// 存储空间
String myNamespace = "zxnamespace";
// ------------------------------------------------------------------------
// 串口通讯数据
String upMsg = "";
// ------------------------------------------------------------------------
// 是否准备好
bool isRead = false;
// 是否准备好Email配置
bool isReadEmail = false;

// ------------------------------------------------------------------------
// --------配置参数

String title = "-";
String ssid = "-";
String pwd = "-";
String printip = "-";
String token = "-";

String emailhost = "-";
String emailport = "-";
String emailaccount = "-";
String emailpwd = "-";
String emailtag = "-";
// ------------------------------------------------------------------------
// --------通知控制
String NowJobName = "";
bool Notice_startState = false;
bool Notice_completion25 = false;
bool Notice_completion50 = false;
bool Notice_completion75 = false;
bool Notice_completion100 = false;
// --------任务状态
String JobState = "";
// --------监视工具电量低
bool Notice_powerlow = false;
// ------------------------------------------------------------------------
// --------ADC
// 用于监测电池电量的adc值
uint16_t adc_Value = 0;
String str_bat = "";

// 启动
void setup() {

  // 开启串口
  HWSerial.begin(115200);
  HWSerial.setDebugOutput(true);
  USBSerial.begin();
  USB.begin();

  // 背光引脚初始化
  pinMode(45, OUTPUT);
  digitalWrite(45, HIGH);


  //===============================================
  //===============================================

  // tft初始化
  tft.init();
  // 旋转屏幕
  tft.setRotation(3);
  // 检查配置项目
  isRead = CheckConfig();
  isReadEmail = CheckEmailConfig();
  // 设置显示文字
  SetTFT_Text(TFT_FGColor, 5, 10, 1.5, title);
  // 添加Wifi
  wifiMulti.addAP(ssid.c_str(), pwd.c_str());
  // 延迟
  delay(1000);
}

// 循环
void loop() {
  // 获取ADC数据
  adc_Value = analogRead(adc3);
  // 通过ADC计算电压
  double v = (adc_Value / 8191.0 * 3.3 - 0.34) * 3;

  // 计算电池剩余电量
  if (4.2 - v <= 0) {
    // 100%电量
    str_bat = "100";
    USBSerial.printf("E:100%");
    USBSerial.println("");
  } else
  {
    double e = (1.2 - (4.2 - v)) / 1.2 * 100;
    str_bat = String(e, 0);
    USBSerial.println(str_bat.c_str());
    USBSerial.println("");

    // 小工具电量低提醒
    if (e < 10 && !Notice_powerlow) {
      Notice_powerlow = true;
      USBSerial.println("powerlow");
      SendEmail("监测小工具状态报告-电量低", "3D打印机监视小工具的电量已经低于10%", "");
    }
  }

  // 循环接受串口数据
  while (USBSerial.available() > 0) {
    char ch = USBSerial.read();
    upMsg += (char)ch;
  }

  if (upMsg.length() >= 6) {
    // 截取命令头
    String sub_head = upMsg.substring(0, 5);
    // 截取数据
    String xdata = upMsg.substring(5);

    // 设置title命令
    if (sub_head.equals("#ccc#"))
    {
      ConfigClear();
    }
    // 设置title命令
    if (sub_head.equals("#stl#"))
    {
      SetConfig("title", xdata);
    }
    // 设置ssid
    if (sub_head.equals("#sss#"))
    {
      SetConfig("ssid", xdata);
    }
    // 设置密码
    if (sub_head.equals("#spw#"))
    {
      SetConfig("pwd", xdata);
    }
    // 设置打印机IP
    if (sub_head.equals("#sip#"))
    {
      SetConfig("printip", xdata);
    }
    // 设置token
    if (sub_head.equals("#stk#"))
    {
      SetConfig("token", xdata);
    }


    // 设置Email服务主机地址
    if (sub_head.equals("#seh#"))
    {
      SetConfig("emailhost", xdata);
    }
    // 设置Email服务主机端口
    if (sub_head.equals("#sep#"))
    {
      SetConfig("emailport", xdata);
    }
    // 设置Email SMTP 账户
    if (sub_head.equals("#sea#"))
    {
      SetConfig("emailaccount", xdata);
    }
    // 设置Email SMTP 密码
    if (sub_head.equals("#sew#"))
    {
      SetConfig("emailpwd", xdata);
    }
    // 设置Email接收地址
    if (sub_head.equals("#set#"))
    {
      SetConfig("emailtag", xdata);
    }
    // 设置背景色
    if (sub_head.equals("#sbg#"))
    {
      SetConfig("TFT_BGColor", xdata);
    }
    // 设置字颜色
    if (sub_head.equals("#sfg#"))
    {
      SetConfig("TFT_FGColor", xdata);
    }
    // 测试
    if (sub_head.equals("#tes#"))
    {
      //=========================================
      //======TEST
      //      ssid = "xxx";
      //      pwd = "xxx";
      //      printip = "xxx";
      //      emailhost = "xxx";
      //      emailport = "25";
      //      emailaccount = "xxx";
      //      emailpwd = "xxx";
      //      emailtag = "xxx";
      //      token = "xxx";
      //=========================================

      wifiMulti.addAP(ssid.c_str(), pwd.c_str());
      if ((wifiMulti.run() == WL_CONNECTED)) {
        String testData =  GetPrintCamBase64Img();
        //USBSerial.println(testData);
        SendEmail("3D打印机状态报告-测试", "测试发送邮件", testData);
      }
    }

    // 完成本次处理
    upMsg = "";

    return;
  }

  if (isRead) {
    // 监听打印机状态
    PrintStateListen();
  }
}

//将字符串("0xffff")的内容转换为 16进制
unsigned int hex_conver_dec(char *src)
{
  char *tmp = src;
  int len = 0;
  unsigned int hexad = 0;
  char sub = 0;

  while (1)
  {
    if (*tmp == '0')            //去除字符串 首位0
    {
      tmp++;
      continue;
    }
    else if (*tmp == 'X')
    {
      tmp++;
      continue;
    }
    else if (*tmp == 'x')
    {
      tmp++;
      continue;
    }
    else
      break;
  }

  len = strlen(tmp);
  for (len; len > 0; len--)
  {
    sub = toupper(*tmp++) - '0';
    if (sub > 9)
      sub -= 7;
    hexad += sub * power(16, len - 1);
  }

  return hexad;
}

//获取x的y次方; x^y
unsigned int power(int x, int y)
{
  unsigned int result = 1;
  while (y--)
    result *= x;
  return result;
}

// 设置配置项目
void SetConfig(String k, String v) {
  // 声明Preferences对象
  Preferences prefs;
  // 打开命名空间mynamespace
  prefs.begin(myNamespace.c_str());
  // 保存配置
  prefs.putString(k.c_str(), v.c_str());
  // 关闭当前命名空间
  prefs.end();
  // 返回结果
  USBSerial.println("OK");
  // 重启系统
  ESP.restart();
}

// 清空命名空间下所有配置
void ConfigClear() {
  // 声明Preferences对象
  Preferences prefs;
  // 打开命名空间mynamespace
  prefs.begin(myNamespace.c_str());
  // 清空配置
  prefs.clear();
  // 关闭当前命名空间
  prefs.end();
  // 返回结果
  USBSerial.println("All Data Clear");
  // 重启系统
  ESP.restart();
}

// 检查必要配置参数
bool CheckConfig() {

  Preferences prefs;
  prefs.begin(myNamespace.c_str());

  if (prefs.isKey("TFT_BGColor")) {
    TFT_BGColor = hex_conver_dec((char *)prefs.getString("TFT_BGColor").c_str());
  }
  if (prefs.isKey("TFT_FGColor")) {
    TFT_FGColor = hex_conver_dec((char *)prefs.getString("TFT_FGColor").c_str());
  }

  if (!prefs.isKey("title")) {
    prefs.end();
    SetTFT_Text(TFT_FGColor, 5, 10, 1.5, "Please Set Title");
    return false;
  }
  else {
    title = prefs.getString("title");
  }

  if (!prefs.isKey("ssid")) {
    prefs.end();
    SetTFT_Text(TFT_FGColor, 5, 10, 1.5, "Please Set SSID");
    return false;
  }
  else {
    ssid = prefs.getString("ssid");
  }

  if (!prefs.isKey("pwd")) {
    prefs.end();
    SetTFT_Text(TFT_FGColor, 5, 10, 1.5, "Please Set PWD");
    return false;
  }
  else {
    pwd = prefs.getString("pwd");
  }

  if (!prefs.isKey("printip")) {
    prefs.end();
    SetTFT_Text(TFT_FGColor, 5, 10, 1.5, "Please Set Print IP");
    return false;
  }
  else {
    printip = prefs.getString("printip");
  }

  if (!prefs.isKey("token")) {
    prefs.end();
    SetTFT_Text(TFT_FGColor, 5, 10, 1.5, "Please Set Token");
    return false;
  }
  else {
    token = prefs.getString("token");
  }

  prefs.end();
  return true;
}

// 检查Email配置
bool CheckEmailConfig() {

  Preferences prefs;
  prefs.begin(myNamespace.c_str());


  //String emailhost = "-";
  //String emailport = "-";
  //String emailaccount = "-";
  //String emailpwd = "-";
  //String emailtag = "-";

  // ==========>> emailhost
  if (!prefs.isKey("emailhost")) {
    return false;
  }
  else {
    emailhost = prefs.getString("emailhost");
  }
  // ==========>> emailport
  if (!prefs.isKey("emailport")) {
    return false;
  }
  else {
    emailport = prefs.getString("emailport");
  }
  // ==========>> emailaccount
  if (!prefs.isKey("emailaccount")) {
    return false;
  }
  else {
    emailaccount = prefs.getString("emailaccount");
  }
  // ==========>> emailpwd
  if (!prefs.isKey("emailpwd")) {
    return false;
  }
  else {
    emailpwd = prefs.getString("emailpwd");
  }
  // ==========>> emailtag
  if (!prefs.isKey("emailtag")) {
    return false;
  }
  else {
    emailtag = prefs.getString("emailtag");
  }

  prefs.end();
  return true;
}

// 获取http消息
void PrintStateListen() {
  // 准备连接wifi
  USBSerial.println("wifi run...");

  // 等待wifi连接
  if ((wifiMulti.run() == WL_CONNECTED)) {
    // 打印日志
    //USBSerial.print("[HTTP] begin...\n");
    // 定义http客户端对象
    HTTPClient http;
    // 设置请求地址
    http.begin("http://" + printip + "/api/job");
    // 设置超时时间
    http.setTimeout(3000);
    // 添加请求头
    http.addHeader("x-api-key", token);
    // 打印日志
    //USBSerial.print("[HTTP] GET...\n");
    // 发出GET请求
    int httpCode = http.GET();

    // 检查http返回状态
    if (httpCode > 0) {
      // 打印httpCode
      //USBSerial.printf("[HTTP] GET... code: %d\n", httpCode);

      // 请求成功
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument rdata(4096);
        deserializeJson(rdata, payload);
        String rstate = rdata["state"];
        String rprogress = rdata["progress"]["completion"];
        String jname = rdata["job"]["file"]["name"];
        if (rprogress.length() > 4) {
          rprogress = rprogress.substring(0, 4);
        }

        //USBSerial.println(rstate);
        ShowPrintState(rstate, rprogress, jname);
      }
    } else {
      //SetTFT_Text(TFT_FGColor, 5, 10, 1.5, "3DPrint Offline");
      //USBSerial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      // 离线状态
      ResetTFT(TFT_FGColor, 0, 0, 1);
      tft.println("   ---------> BAT[" + str_bat + "%]");
      tft.println("   " + title);
      tft.println("   ----------------");
      tft.println("   3DPrint Offline");
      tft.println("   =====================");
    }
    // 接触http
    http.end();
  }
  // 设置延迟
  delay(5000);
}

// 显示打印机状态
void ShowPrintState(String msg, String progress, String jobname)
{
  /**
     flags.operational true如果打印机正常运行，否则false
     flags.paused true如果打印机当前已暂停，否则false
     flags.printing true如果打印机当前正在打印，否则false
     printing from sd 打印sd卡中的模型
     flags.pausing true如果打印机当前正在打印并且正在暂停，否则false
     flags.cancelling true如果打印机当前正在打印并且正在暂停，否则false
     flags.sdReady true如果打印机的 SD 卡可用且已初始化，否则。这是 SD 状态的冗余信息。false
     flags.error true如果发生不可恢复的错误，否则false
     flags.ready true如果打印机正在运行，并且当前没有数据被流式传输到SD，那么准备接收指令
     flags.closedOrError true如果打印机断开连接（可能是由于错误），否则false
  */

  //String NowJobName = "";
  //bool Notice_startState = false;
  //bool Notice_completion50 = false;
  //bool Notice_completion100 = false;

  String msg_l = msg;
  msg_l.toLowerCase();
  double fpcc = atof(progress.c_str());
  //==================================================================
  //=============================处理消息==============================
  if (NowJobName.equals("")) {
    NowJobName = jobname;
  }

  if (NowJobName.equals(jobname) && fpcc >= 0 && fpcc < 5 && Notice_startState && Notice_completion100) {
    Notice_startState = false;
    Notice_completion25 = false;
    Notice_completion75 = false;
    Notice_completion50 = false;
    Notice_completion100 = false;
    NowJobName = jobname;
  }

  if (!NowJobName.equals(jobname)) {
    Notice_startState = false;
    Notice_completion25 = false;
    Notice_completion75 = false;
    Notice_completion50 = false;
    Notice_completion100 = false;
    NowJobName = jobname;
  }

  if (fpcc >= 0 && fpcc < 5 && (!Notice_startState) && jobname != NULL && jobname != "" && jobname != "null") {
    Notice_startState = true;
    USBSerial.println("start");
    String testData =  GetPrintCamBase64Img();
    SendEmail("3D打印机工作-开始新任务", "3D打印机已经开始新的工作,当前打印任务：" + jobname + "。", testData);
  }

  if (fpcc >= 25 && fpcc < 30 && (!Notice_completion25)) {
    Notice_completion25 = true;
    USBSerial.println("completion 25");
    String testData =  GetPrintCamBase64Img();
    SendEmail("3D打印机工作-进度25%", "3D打印机当前打印进度已过25%,当前打印任务：" + jobname + "。", testData);
  }

  if (fpcc >= 50 && fpcc < 55 && (!Notice_completion50)) {
    Notice_completion50 = true;
    USBSerial.println("completion 50");
    String testData =  GetPrintCamBase64Img();
    SendEmail("3D打印机工作-进度50%", "3D打印机当前打印进度已过50%,当前打印任务：" + jobname + "。", testData);
  }

  if (fpcc >= 75 && fpcc < 80 && (!Notice_completion75)) {
    Notice_completion75 = true;
    USBSerial.println("completion 75");
    String testData =  GetPrintCamBase64Img();
    SendEmail("3D打印机工作-进度75%", "3D打印机当前打印进度已过75%,当前打印任务：" + jobname + "。", testData);
  }

  if (fpcc >= 100 && (!Notice_completion100)) {
    Notice_completion100 = true;
    USBSerial.println("completion 100");
    String testData =  GetPrintCamBase64Img();
    SendEmail("3D打印机工作-已完成", "3D打印机已完成打印工作100%,当前打印任务：" + jobname + "。", testData);
  }

  if (!JobState.equals(jobname + msg_l)) {
    // 拼接任务状态-处理其他状态
    JobState = jobname + msg_l;

    if (!msg_l.equals("printing") && !msg_l.equals("operational")) {
      String testData =  GetPrintCamBase64Img();
      SendEmail("3D打印机状态报告-" + msg , "3D打印机已更新状态。任务：" + jobname + "。状态:" + msg, testData);
    }
  }
  //==================================================================

  // 完成任务
  if (msg_l.equals("operational") && fpcc >= 100) {
    ResetTFT(TFT_FGColor, 0, 0, 1);
    tft.println("   ---------> BAT[" + str_bat + "%]");
    tft.println("   " + title);
    tft.println("   " + msg);
    tft.println("   3DPrint Finish");
    tft.println("   =====================");
    return;
  }

  // 没有任务
  if (msg_l.equals("operational") && fpcc <= 0) {
    ResetTFT(TFT_FGColor, 0, 0, 1);
    tft.println("   ---------> BAT[" + str_bat + "%]");
    tft.println("   " + title);
    tft.println("   " + msg);
    tft.println("   No Job");
    tft.println("   =====================");
    return;
  }

  // 打印中
  if (msg_l.equals("printing")) {
    ResetTFT(TFT_FGColor, 0, 0, 1);
    tft.println("   ---------> BAT[" + str_bat + "%]");
    tft.println("   " + title);
    tft.println("   " + msg);
    tft.println("   Progress " + progress + "%");
    tft.println("   =====================");
    return;
  }

  // 打印中
  if (msg_l.equals("printing from sd")) {
    ResetTFT(TFT_FGColor, 0, 0, 1);
    tft.println("   ---------> BAT[" + str_bat + "%]");
    tft.println("   " + title);
    tft.println("   " + msg);
    tft.println("   Progress " + progress + "%");
    tft.println("   =====================");
    return;
  }

  // 其他状态
  ResetTFT(TFT_FGColor, 0, 0, 1);
  tft.println("   ---------> BAT[" + str_bat + "%]");
  tft.println("   " + title);
  tft.println("   " + msg);
  tft.println("   =====================");
}

// 显示数据消息
void SetTFT_Text(int color, int x, int y, int fsize, String text)
{
  // 重置
  ResetTFT(color, x, y, fsize);
  // 打印
  tft.println(text);
}

// 获取打印机摄像头照片
String GetPrintCamBase64Img() {
  //获取打印照片地址
  //http://ip/webcam/?action=snapshot
  // 定义http客户端对象
  HTTPClient http;
  // 设置请求地址
  http.begin("http://" + printip + "/webcam/?action=snapshot.jpg");
  // 设置超时时间
  http.setTimeout(5000);
  // 添加请求头
  //http.addHeader("x-api-key", token);
  // 打印日志
  //USBSerial.print("[HTTP] GET...\n");
  // 发出GET请求
  int httpCode = http.GET();
  String photoData = "";

  // 检查http返回状态
  if (httpCode > 0) {
    // 打印httpCode
    //USBSerial.printf("[HTTP] GET... code: %d\n", httpCode);

    // 请求成功
    if (httpCode == HTTP_CODE_OK) {
      int len = http.getSize();
      USBSerial.println("len");
      USBSerial.println(len);

      uint8_t buff[24] = { 0 };
      // create buffer for read
      WiFiClient * stream = http.getStreamPtr();
      // read all data from server
      //      while (http.connected() && (len > 0)) {
      //        // get available data size
      //        size_t size = stream->available();
      //        if (size) {
      //
      //          // read up to 128 byte
      //          int c = stream->readBytes(buff, ((size > 24) ? 24 : size));
      //
      //          char encoded[BASE64::encodeLength(c)];
      //          BASE64::encode((const uint8_t*)buff, c, encoded);
      //          String sTemp = encoded;
      //          photoData += sTemp;
      //
      //          if (len > 0) {
      //            len -= c;
      //          }
      //
      //        }
      //        else{
      //          break;
      //        }
      //        delay(1);
      //      }

      while (http.connected()) {
        // get available data size
        size_t size = stream->available();
        if (size) {

          // read up to 128 byte
          int c = stream->readBytes(buff, ((size > 24) ? 24 : size));

          char encoded[BASE64::encodeLength(c)];
          BASE64::encode((const uint8_t*)buff, c, encoded);
          String sTemp = encoded;
          photoData += sTemp;

          if (len > 0) {
            len -= c;
          }

        }
        else {
          break;
        }
        delay(1);
      }
    }
  }
  // 结束http
  http.end();
  return photoData;
}

// 重置TFT
void ResetTFT(int color, int x, int y, int fsize)
{
  // 设置黑色背景,并清空屏幕显示
  tft.fillScreen(TFT_BGColor);
  // 设置坐标和字体
  tft.setCursor(x, y, 2);
  // 设置文本的前景色和背景色
  tft.setTextColor(color);
  // 设置字体大小
  tft.setTextSize(fsize);
}

// 发送Email
void SendEmail(String emailTitle, String emailText, String imgData) {
  USBSerial.println("email start");
  //================================================================================================

  // Set the session config
  session.server.host_name = emailhost;
  session.server.port = emailport.toInt();
  session.login.email = emailaccount; // set to empty for no SMTP Authentication
  session.login.password = emailpwd; // set to empty for no SMTP Authentication

  // Define the SMTP_Message class variable to handle to message being transport
  SMTP_Message message;

  // Set the message headers
  message.sender.name = emailTitle;
  message.sender.email = emailaccount;
  message.subject = emailTitle;
  message.addRecipient(emailtag, emailtag);

  //================================================================================================

  String htmlMsg = "<p>3D打印机状态报告</p><p>" + emailText + "</p><p><a style=\"color:#ff0000;\" href=\"http://" + printip + "\">[点击跳转到3D打印机管理系统]</a></p><img style=\"width:500px;height:500px;\" src=\"data:image/jpeg;base64," + imgData + "\">";
  message.html.content = htmlMsg;
  message.html.charSet = "utf-8";

  //================================================================================================
  USBSerial.println("email Connect");
  // Connect to server with the session config
  smtp.connect(&session);
  USBSerial.println("email send");
  // Start sending Email and close the session
  if (!MailClient.sendMail(&smtp, &message))
    USBSerial.println("Error sending Email, " + smtp.errorReason());
}

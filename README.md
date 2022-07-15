# 3D打印机状态监测工具

---



## 功能介绍

该工具主要帮助用户进行3D打印状态监测，以及通过发送邮件提醒打印进度功能。

![Esp32](/docMedia/esp32.png)

![Email Msg](/docMedia/emailmsg.jpg)







---

## 必要条件

1. 准备一个3D打印机,我使用的是Ender3 S1 Pro。

2. 准备一个树莓派或香橙派或香蕉派等作为上位机,并安装 OctoPrint。至于为什么要使用 OctoPrint ？ 因为我刚接触3D打印机，还没有来得及了解其他系统。这个系统是我在网上了解到的第一个。关于 OctoPrint 安装到树莓派的方法网上有很多，在其官网也有。我是下载的 OctoPrint 树莓派系统镜像直接刷到树莓派SD卡。

3. 在树莓派上安装摄像头。我使用的是带补光灯的红外摄像头。

4. 在 OctoPrint 中创建一个Application key，用于调用 REST API。

5. 微雪 ESP32-S2-LCD-0.96 单片机。

6. 已经开通 SMTP 功能的邮箱(建议新建一个账号使用)。

---

## 开发环境

Arduino IDE

VS CODE

[ESP32-S2-Pico - Waveshare Wiki](https://www.waveshare.net/wiki/ESP32-S2-Pico#C.2FC.2B.2B.28Arduino.2CESP-IDF.29) 搭建开发环境

---

## 相关说明

[ESP32-S2-Pico - Waveshare Wiki](https://www.waveshare.net/wiki/ESP32-S2-Pico#LCD) LCD驱动设置

#### TFT_eSPI

- ESP32-S2-LCD-0.96上的LCD驱动使用了[ST7735](https://www.waveshare.net/w/upload/e/e2/ST7735S_V1.1_20111121.pdf),本小结使用TFT_eSPI库进行显示,注意提前安装arduino-esp32,TFT_eSPI等库
- 打开TFT_eSPI库文件夹(默认安装位置注意下图路径)下User_Setup_Select.h和User_Setups/Setup43_ST7735.h文件并按如图所示修改
- ![tft_config](/docMedia/ESP32-S2-Pico_014.jpg)
- 如果你发现你屏幕颜色显示的是反的，那么在 Setup43_ST7735.h 文件中还需要解开 \#define TFT_RGB_ORDER TFT_BGR   或 \#define TFT_RGB_ORDER TFT_RGB 注释(根据实际情况)。![设置屏幕颜色模式](/docMedia/设置屏幕颜色模式.png)

- 在TFT_eSPI的example文件的setup()函数中加入背光引脚的初始化
- ![setup](/docMedia/800px-ESP32-S2-Pico_015.jpg)



#### ConfigTool

该Html ([/ConfigTool_Serial/ConfigTool.html](ConfigTool_Serial/ConfigTool.html) ) 是利用Web串口工具与Esp32进行串口通讯，将Esp32插入电脑然后打开该本地网页即可。



建议浏览器 Edge Chrome

该网页工具中，每一项设置都会让ESP32重启并应用，至于为什么不做一次全部提交参数。这个完全是个人喜好。

---

## 3D打印外壳

![外壳](/docMedia/br01.jpg)

![外壳](/docMedia/br02.jpg)

![外壳](/docMedia/br03.jpg)



[打印文件](3DPrint) 

打印参数：

- 层高 0.2

- 填充 30

- 速度 30

  

***上壳需要支撑，我对xyz的距离支撑均为 0.2 但拆除的时候需要费点劲。***

***如果外壳组合的时候比较费力，可以使用壁纸刀稍微修一下插头。***



***以上参数仅供参考***

---

## 注意事项

1. 需要注意，并不是所有邮箱开通 SMTP 之后都可以正常使用，有的邮箱对文本和附件都有严格限制。我使用的是 163 邮箱（其他邮箱未测试）。
2. 建议在 Arduino IDE 中设置  工具->PSRAM->Enabled。
3. [/ConfigTool_Serial/ConfigTool.html](ConfigTool_Serial/ConfigTool.html)  建议使用 Chrome 或 Edge 打开，该页面是利用串口的设置工具。
4. 建议在 OctoPrint 中新建一个普通角色的账户，然后再生成其 Application key 进行使用。
5. 虽然该工具可以作为打印进度提醒工具，但我永远不会建议让3D打印机在家中无人的情况下进行工作。3D打印机工作温度高，请避免火灾风险！！！

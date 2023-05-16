#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT_U.h>
#include "../lib/Login.h"
#include "../lib/Logo.h"
#include <Adafruit_SSD1306.h>
#include <IRremote.hpp>

void sendSamsungSmartHubMacro(bool aDoSelect);
void IRSendWithDelay(uint8_t aCommand, uint16_t aDelayMillis);


#define ADDRESS_OF_SAMSUNG_REMOTE   0x0707 // 红外发射器地址
#define SCREEN_WIDTH 128 // 屏幕宽度
#define SCREEN_HEIGHT 64 // 屏幕高度

#define OLED_MOSI   9   // D1
#define OLED_CLK   10   // D0
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

#define DHTTYPE DHT11   // DHT 11 (AM2302)
#define DHTPIN 4        // DHT 引脚

#define FMD_PIN 3
#define INTPIN 2        // 中断引脚

#define IR_RECEIVE_PIN 5     // 红外接收器
#define IR_SEND_PIN 6        // 红外发射器

// 创建 SSD1306 实例
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS = 1500; // 循环时间

float Temperature, Humidity;                            // 当前环境的温湿度
float Temperature_limit = 30, Humidity_limit = 90;      // 温湿度预设值

bool switch_fmd = false;        // 蜂鸣器开关

// 初始化串口
void init_UART() {
    Serial.begin(115200, SERIAL_8E2);
}

// 初始化 SSD1306
void init_SSD1306() {
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("failed");
        for (;;);
    } else {
        Serial.print("ssd1306 is ok");
    }
}

// 初始化温湿度传感器
void init_dht() {
    dht.begin();
    Serial.println(F("DHT11 is ok"));
}

// 打印温湿度传感器信息
void print_info_dht() {
    
// 打印温度传感器的信息
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    Serial.println(F("------------------------------------"));
    Serial.println(F("Temperature Sensor"));
    Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F(" C"));
    Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F(" C"));
    Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F(" C"));
    Serial.println(F("------------------------------------"));
// 打印湿度传感器的信息
    dht.humidity().getSensor(&sensor);
    Serial.println(F("Humidity Sensor"));
    Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
    Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
    Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
    Serial.println(F("------------------------------------"));

}
void chmode(uint16_t x);
void setup() {

// 一些初始化设置
    init_UART();
    init_SSD1306();
    init_dht();
    pinMode(FMD_PIN, OUTPUT);
    delay(5000);
// 启用红外发射器
    IrSender.begin(IR_SEND_PIN);
// 启用红外接收器
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

// -----------------------开机显示-----------------------------
// 字体设置
    display.setTextColor(WHITE);
    display.setTextSize(2);
// 显示姓名学号
    display.clearDisplay();
    display.drawBitmap(0, 0, gImage_Login, 128, 64, WHITE);
    display.display();
    delay(1500);
// 显示logo
    display.clearDisplay();
    display.drawBitmap(26, 0, gImage_logo, 77, 64, WHITE);
    display.display();
    delay(1500);
// ------------------------------------------------------------

}

void display_DHlimit() {

// --------------oled 显示---------------
    display.clearDisplay();
    display.setCursor(0, 20);
    display.print("Temp limit is");
    display.println(Temperature_limit);
    display.print("Humi limit is");
    display.println(Humidity_limit);
    if(switch_fmd) {
        display.println("fmd on");
    } else {
        display.println("fmd off");
    }
    display.display();
    delay(1500);
// ---------------串口显示----------------
    Serial.print("Temp limit is:");
    Serial.println(Temperature_limit);

    Serial.print("Humi limit is:");
    Serial.println(Humidity_limit);

    if(switch_fmd) {
        Serial.println("fmd on");
    } else {
        Serial.println("fmd off");
    }
// --------------------------------------
}

// 执行接收到的红外信号命令
void chmode(uint16_t x) {
    switch (x) {
// 温度上限加一
    case 0x47:
        Temperature_limit ++;
        if(Temperature_limit > 60) {
            Temperature_limit = 60;
        } 
        break;
// 温度上限减一
    case 0x48:
        Temperature_limit --;
        if(Temperature_limit < -10) {
            Temperature_limit = -10;
        }
// 湿度上限加一
    case 0x49:
        Humidity_limit ++;
        if(Humidity_limit > 100) {
            Humidity_limit = 100;
        }
// 湿度上限减一
    case 0x4A:
        Humidity_limit --;
        if(Humidity_limit < 0) {
            Humidity_limit = 0;
        }
// 报警开启
    case 0x4B:
        switch_fmd = true;
// 报警关闭
    case 0x4C:
        switch_fmd = false;
    default:
        break;
    }
// 打印当前温湿度预设信息
    display_DHlimit();
    delay(1500);
}

// 报警
void warning() {

    tone(FMD_PIN, 882, 1000);

}

// 得到温湿度值并打印到串口和屏幕
void get_temperature_humidity() {

// 得到温度并输出值
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
        Serial.println(F("Error reading temperature!"));
    } else {
        Temperature = event.temperature;
        Serial.print(F("Temperature: "));
        Serial.print(Temperature);
        Serial.println(F(" C"));
    }

// 得到湿度并输出值
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
        Serial.println(F("Error reading humidity!"));
    } else {
        Humidity = event.relative_humidity;
        Serial.print(F("Humidity: "));
        Serial.print(Humidity);
        Serial.println(F("%"));
    }

// ---------------oled 屏幕输出温湿度信息-------------
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print(F("Temperature: "));
    display.print(Temperature);
    display.println(F(" C"));
    display.print(F("Humidity: "));
    display.print(Humidity);
    display.println(F("%"));
    display.display();
// ---------------------------------------------------
}

void loop() {
    
// 得到当前环境的温湿度
    get_temperature_humidity();

// 得到接受到的红外信号
    if (IrReceiver.decode()) {
// ------------打印得到的信息--------------
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRSendUsage(&Serial);
        Serial.println();
// ---------------------------------------
// 执行接收到的红外信号命令
        chmode(IrReceiver.decodedIRData.command);

        IrReceiver.restartAfterSend();
        IrReceiver.resume();
    }

// 蜂鸣器报警条件
    if(Temperature > Temperature_limit || Humidity > Humidity_limit) {
        warning();
    }

    delay(delayMS);
}

// 红外发射器发送一条指令
void IRSendWithDelay(uint8_t aCommand, uint16_t aDelayMillis) {
    
    IrSender.sendSamsung(ADDRESS_OF_SAMSUNG_REMOTE, aCommand, 1);
    Serial.print(F("Send Samsung command 0x"));
    Serial.println(aCommand);
    delay(aDelayMillis);
    
}

// 设置串口事件
void serialEvent() {

    while (Serial.available()) {
        uint8_t aCommand = Serial.read();
        IRSendWithDelay(aCommand, 1);  
    }

}

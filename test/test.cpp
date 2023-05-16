#include <Arduino.h>
#include <DHT_U.h>
#include <Adafruit_GFX.h>
#include "../lib/Login.h"
#include "../lib/Logo.h"
#include <IRremote.hpp>

void sendSamsungSmartHubMacro(bool aDoSelect);
void IRSendWithDelay(uint8_t aCommand, uint16_t aDelayMillis);

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

#define IR_RECEIVE_PIN 5            // 红外接收器
#define IR_TRANSMITTER_PIN 6        // 红外发射器

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS; // 循环时间

float Temperature, Humidity;                // 当前环境的温湿度
float Temperature_limit = 30, Humidity_limit = 90;    // 温湿度预设值

bool switch_fmd = false;        // 蜂鸣器开关

// 初始化串口
void init_UART() {
    Serial.begin(9600, SERIAL_8E2);
}

// 初始化 SSD1306
void init_SSD1306() {
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
    delayMS = sensor.min_delay / 1000;

}
void chmode(uint16_t x);
void setup() {

    // 一些初始化设置
    pinMode(FMD_PIN, OUTPUT);
    pinMode(INTPIN, OUTPUT);
    digitalWrite(INTPIN, 0);
    delay(5000);
    init_UART();
    init_SSD1306();
    init_dht();
    print_info_dht();
    delay(1000);
    // 启用红外接收器
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

}

void display_DHlimit() {
}

void chmode(uint16_t x) {
    switch (x) {
    case 0x47:
        Temperature_limit ++;
        if(Temperature_limit > 60) {
            Temperature_limit = 60;
        } 
        break;
    case 0x48:
        Temperature_limit --;
        if(Temperature_limit < -10) {
            Temperature_limit = -10;
        }
    case 0x49:
        Humidity_limit ++;
        if(Humidity_limit > 100) {
            Humidity_limit = 100;
        }
    case 0x4A:
        Humidity_limit --;
        if(Humidity_limit < 0) {
            Humidity_limit = 0;
        }
    case 0x4B:
        switch_fmd = true;
    case 0x4C:
        switch_fmd = false;
    default:
        break;
    }
    display_DHlimit();
    digitalWrite(INTPIN, 0);
    delay(1500);
}

void warning() {

    tone(FMD_PIN, 882, 1000);
    delay(5000);

}

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

}

void loop() {

    delay(delayMS);
    
    get_temperature_humidity();

    if (IrReceiver.decode()) {
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRSendUsage(&Serial);
        Serial.println();
        chmode(IrReceiver.decodedIRData.command);
        IrReceiver.restartAfterSend();
        IrReceiver.resume();
    }

    if(Temperature > Temperature_limit || Humidity > Humidity_limit) {
        warning();
    }

}


// 设置串口事件
void serialEvent() {

    while (Serial.available()) {
        
    }

}
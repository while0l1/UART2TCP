#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#define SYSRuntime D4

int tcpPort = 8266; //服务器端口
char valFromPC; //接收TCP发过来的数据
String valFromMCU = ""; //接收串口发过来的数据
int exitFlag; //退出配置模式标志
unsigned int Baud = 115200; //串口波特率

WiFiServer server(tcpPort);
WiFiClient client;

//开启热点
//开启TCP服务
void StartServer()
{
    // 连接WIFI
    WiFi.mode(WIFI_STA);
    // 选取一种连接路由器的方式 
    // WiFi.begin(ssid, password);
    WiFi.beginSmartConfig();

    server.begin(tcpPort); //开启TCP服务器
    server.setNoDelay(true);
}

void setup()
{
    //初始化串口
    Serial.begin(Baud);
    Serial.setRxBufferSize(1024 * 10);
    Serial.setTimeout(2);
    pinMode(SYSRuntime, OUTPUT);
    delay(10);
    //启动TCP服务
    StartServer();

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        // Serial.print(".");
    }

    MDNS.begin("esp-switch-"+WiFi.macAddress());
    MDNS.addService("iotdevice", "tcp", tcpPort);
    MDNS.addServiceTxt("iotdevice", "tcp", "mykEY","myvALUE");
}

void loop()
{
    MDNS.update();
    if (server.hasClient()) { //有新的设备连接上
        client = server.available();
    }
    while (client.available()) //TCP接收单片机发过来的信息，串口发给电脑
    {
        digitalWrite(SYSRuntime, LOW);
        valFromPC = client.read();
        Serial.print(valFromPC);
    }

    while (Serial.available()) { //串口接收电脑发送过来的信息，TCP发送给单片机
        digitalWrite(SYSRuntime, LOW);
        valFromMCU = Serial.readString();
        client.print(valFromMCU); //把串口数据通过TCP发出去
    }
}
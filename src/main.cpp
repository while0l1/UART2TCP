#include <ESP8266WiFi.h>

String ssid = "UART2TCP"; //WiFi名称
String password = "1234567890"; //WiFi密码

IPAddress mylocalIP(192, 168, 5, 2); //本地IP地址
IPAddress mygateWay(192, 168, 5, 1); //网关
IPAddress mysubnet(255, 255, 255, 0); //子网掩码
int tcpPort = 8266; //服务器端口
char valFromPC; //接收TCP发过来的数据
String valFromMCU = ""; //接收串口发过来的数据
int exitFlag; //退出配置模式标志
unsigned int Baud = 9600; //串口波特率

WiFiServer server(tcpPort);
WiFiClient client;

//开启热点
//开启TCP服务
void StartServer()
{
    WiFi.mode(WIFI_AP); //AP模式（仅打开热点）
    WiFi.softAPConfig(mylocalIP, mygateWay, mysubnet); //配置热点
    WiFi.softAP(ssid, password, 1, 0, 10); //设置SSID，密码，最大连接数等

    //输出配置信息
    Serial.println("");
    Serial.println("WiFi Created!");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Password: ");
    Serial.println(password);
    Serial.println("IP address is: 192.168.5.2");
    Serial.print("Server port is: ");
    Serial.println(tcpPort);

    server.begin(tcpPort); //开启TCP服务器
    server.setNoDelay(true);
}

//配置模式
void SettingMode()
{
    exitFlag = 1;
    Serial.println("-----------------UART to TCP Beta--------------------");
    Serial.println("You can type \"HELP\" for help");
    Serial.print('>');
    while (exitFlag == 1) {
        while (Serial.available()) {
            valFromMCU = Serial.readString(); //读取串口数据
            if (valFromMCU.substring(0, 5) == "SSID:") { //修改热点名称
                ssid = valFromMCU.substring(5);
                Serial.print('>');
            } else if (valFromMCU.substring(0, 4) == "PSW:") { //修改热点密码
                password = valFromMCU.substring(4);
                Serial.print('>');
            } else if (valFromMCU.substring(0, 5) == "PORT:") { //修改端口
                tcpPort = valFromMCU.substring(5).toInt();
                Serial.print('>');
            } else if (valFromMCU.substring(0, 5) == "BAUD:") { //修改波特率
                Baud = valFromMCU.substring(5).toInt();
            } else if (valFromMCU.substring(0, 4) == "HELP") { //打印帮助信息
                Serial.println("Example:");
                Serial.println("Set you SSID name as UART2TCP:   SSID:UART2TCP");
                Serial.println("Set you password as 1234567890:  PSW:1234567890");
                Serial.println("Set you TCP port as 8266:        PORT:8266");
                Serial.println("Set you baudrate as 9600:        BAUD:9600");
                Serial.println("Exit setting mode:               EXIT");
            } else if (valFromMCU.substring(0, 4) == "EXIT") { //退出配置
                exitFlag = 0;
            }
        }
    }
    StartServer(); //重新启动服务
}

void setup()
{
    //初始化串口
    Serial.begin(Baud);
    Serial.setRxBufferSize(1024 * 10);
    Serial.setTimeout(10);
    delay(10);

    //启动TCP服务
    StartServer();
}

void loop()
{
    if (server.hasClient()) { //有新的设备连接上
        client = server.available();
        client.setNoDelay(true);
    }
    while (client.available()) //TCP接收单片机发过来的信息，串口发给电脑
    {
        valFromPC = client.read();
        Serial.print(valFromPC);
    }
    while (Serial.available()) { //串口接收电脑发送过来的信息，TCP发送给单片机
        delay(5);
        valFromMCU = Serial.readString();
        if (valFromMCU.substring(0, 8) == ":Setting") { //进入配置模式
            Serial.println("Entering setting mode");
            SettingMode();
        } else {
            client.print(valFromMCU); //把串口数据通过TCP发出去
        }
    }
}
#include <ESP8266WiFi.h>

String ssid = "UART2TCP"; //这里是我的wifi，你使用时修改为你要连接的wifi ssid
String password = "1234567890"; //你要连接的wifi密码

IPAddress mylocalIP(192, 168, 5, 2);
IPAddress mygateWay(192, 168, 5, 1);
IPAddress mysubnet(255, 255, 255, 0);
int tcpPort = 8266; //修改为你建立的Server服务端的端口号
char valFromPC;
String valFromMCU = "";
int exitFlag;
unsigned int Baud = 9600;

WiFiServer server(tcpPort);
WiFiClient client;

void StartServer()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(mylocalIP, mygateWay, mysubnet);
    WiFi.softAP(ssid, password, 1, 0, 10);
    Serial.println("");
    Serial.println("WiFi Created!");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Password: ");
    Serial.println(password);
    Serial.println("IP address is: 192.168.5.2");
    Serial.print("Server port is: ");
    Serial.println(tcpPort);
    server.begin();
    server.setNoDelay(true);
}

void SettingMode()
{
    exitFlag = 1;
    Serial.println("-----------------UART to TCP Beta--------------------");
    Serial.println("You can type \"HELP\" for help");
    Serial.print('>');
    while(exitFlag == 1)
    {
      while(Serial.available())
      {
      valFromMCU = Serial.readString();
      if(valFromMCU.substring(0,5) == "SSID:")
      {
        ssid = valFromMCU.substring(5);
        Serial.print('>');
      }
      else if(valFromMCU.substring(0,4) == "PSW:")
      {
        password = valFromMCU.substring(4);
        Serial.print('>');
      }
      else if(valFromMCU.substring(0,5) == "PORT:")
      {
        tcpPort = valFromMCU.substring(5).toInt();
        Serial.print('>');
      }
      else if(valFromMCU.substring(0,5) == "BAUD:")
      {
        Baud = valFromMCU.substring(5).toInt();
      }
      else if(valFromMCU.substring(0,4) == "HELP")
      {
        Serial.println("Example:");
        Serial.println("Set you SSID name as UART2TCP:   SSID:UART2TCP");
        Serial.println("Set you password as 1234567890:  PSW:1234567890");
        Serial.println("Set you TCP port as 8266:        PORT:8266");
        Serial.println("Set you baudrate as 9600:        BAUD:9600");
        Serial.println("Exit setting mode:               EXIT");
      }
      else if(valFromMCU.substring(0,4) == "EXIT")
      {
        exitFlag = 0;
      }
      }
    }
    StartServer();
}

void setup()
{
    Serial.begin(Baud);
    Serial.setRxBufferSize(1024 * 10);
    Serial.setTimeout(10);
    delay(10);
    StartServer();
}

void loop()
{
    if (server.hasClient()) {
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
        if (valFromMCU.substring(0,8) == ":Setting") {
            Serial.println("Entering setting mode");
            SettingMode();
        }
        else
        {
          client.print(valFromMCU);
        }
        
    }
}
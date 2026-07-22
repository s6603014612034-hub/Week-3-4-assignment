#include <WiFi.h>
#include "ThingSpeak.h"

char ssid[] = "Chokdee";                    
char pass[] = "s69wq4v8";                         
unsigned long myChannelNumber = 3432817;            
const char * myWriteAPIKey = "UU2Q33MAADB42FSL";    

// --- เพิ่มการตั้งค่าสำหรับ Node-RED ---
const char* nodeRedHost = " 10.189.174.248"; // ⚠️ เปลี่ยนเป็น IP คอมพิวเตอร์ของคุณ
const uint16_t nodeRedPort = 2000;       // Port 2000 ตามที่ตั้งในโหนดย tcp in

int keyIndex = 0;

WiFiClient client;          // สำหรับ ThingSpeak
WiFiClient nodeRedClient;   // สำหรับ Node-RED (เพิ่มใหม่)

void setup() 
{
  Serial.begin(115200);  
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() 
{
  // มั่นใจว่า Wi-Fi ยังเชื่อมต่ออยู่
  wifi_reconnect();

  // 1. สุ่มข้อมูลเซนเซอร์จำลอง
  int val1 = random(100); // แทนค่าอุณหภูมิ (Temp)
  int val2 = random(200); // แทนค่าความชื้น (Humi)

  // 2. ส่งข้อมูลไปที่ ThingSpeak
  ThingSpeak.setField(1, val1);
  ThingSpeak.setField(2, val2);
  thingspeak_multi_write();

  // 3. ส่งข้อมูลไปที่ Node-RED ผ่าน TCP Socket (รูปแบบ: ค่า1,ค่า2)
  send_to_nodered(val1, val2);

  // ThingSpeak กำหนดให้ส่งข้อมูลห่างกันอย่างน้อย 15 วินาที
  delay(20000); 
}

void wifi_reconnect() 
{
  if (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED) 
    {
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }
}

// ฟังก์ชันส่งข้อมูลไป Node-RED (เพิ่มใหม่)
void send_to_nodered(int v1, int v2) 
{
  Serial.print("Connecting to Node-RED... ");
  if (nodeRedClient.connect(nodeRedHost, nodeRedPort)) 
  {
    Serial.println("Connected!");
    // สร้าง String ในรูปแบบ "ค่า1,ค่า2" เช่น "45,120" เพื่อให้โหนด function 14 ของ Node-RED ใช้ .split(",") ตัดคำได้
    String dataStr = String(v1) + "," + String(v2);
    nodeRedClient.print(dataStr);
    nodeRedClient.stop(); // ปิดการเชื่อมต่อชั่วคราวหลังจากส่งเสร็จ เพื่อให้โหนด tcp in รับรู้ว่าสิ้นสุดข้อความ
    Serial.println("Sent to Node-RED: " + dataStr);
  } 
  else 
  {
    Serial.println("Connection to Node-RED failed.");
  }
}

void thingspeak_multi_write() 
{
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) 
  {
    Serial.println("Multichannel update successful.");
  }
  else 
  {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}

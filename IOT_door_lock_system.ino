//########################## Library ESP8266 and Firebase ##########################//
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

//########################## Library Keypad ##########################//
#include <Keypad.h>

//########################## Library OLED ##########################//
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <SPI.h>

//########################## Library TIME LOCAL ##########################//
#include <time.h>

//########################## Config Firebase ##########################//
#define FIREBASE_HOST "password-request-system.firebaseio.com"
#define FIREBASE_AUTH "tbtchEClvgjLNASBQwvsJQJjNej6NYbEL8gLaitg"
                       
//########################## Config connect WiFi
#define WIFI_SSID "123"
#define WIFI_PASSWORD "567891011"

//########################## Config variable BUZZER, LED ##########################//
#define BUZZER 15
#define LED_GREEN 15
#define LED_RED 3

//##########################  Config Keypad ##########################//
char keys[4][4] = {
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};
byte rowPin[4] = {16, 5, 4, 0};
byte colPin[4] = {2, 14, 12, 13};
Keypad keypad = Keypad(makeKeymap(keys), rowPin, colPin, 4, 4);

//########################## config OLED no reset ##########################//
Adafruit_SSD1306 OLED(-1);

//########################## config time local ##########################//
int timezone = 7 * 3600;                    // setting TimeZone ตามเวลาประเทศไทย
int dst = 0;                                //กำหนดค่า Date Swing Time
char ntp_server1[20] = "pool.ntp.org";
char ntp_server2[20] = "time.nist.gov";
char ntp_server3[20] = "time.uni.net.th";

//########################## config variable response from firebase, string request ##########################//
String response, _res = "";
String _str, _str2, _str3 = "";
String password, _password = "";
String _uidKey = "";

void setup() {
  Serial.begin(115200); //setup Serial baudrate

  OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C); //setup OLED address
  
  pinMode(BUZZER,OUTPUT); //setup BUZZER OUTPUT
  pinMode(LED_GREEN,OUTPUT); //setup LED_GREEN OUTPUT
  pinMode(LED_RED,OUTPUT); //setup LED_RED OUTPUT

  digitalWrite(BUZZER,LOW); //give BUZZER LOW or OFF
  digitalWrite(LED_GREEN,LOW); //give LED_GREEN LOW or OFF
  digitalWrite(LED_RED,LOW); //give LED_RED LOW or OFF
  
  WiFi.mode(WIFI_STA); //use WiFi mode STA, ให้เป็นโหมดที่ไม่ใช่เราเตอร์หรือไม่ใช่ ACESS POINT 
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //setup WIFI_SSID, WIFI_PASSWORD
  Serial.print("connecting");
  
  while (WiFi.status() != WL_CONNECTED) { //Loop wait connect WiFi
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP()); //Show IP Network

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); //setup FIREBASE_HOST, FIREBASE_AUTH

  configTime(timezone, dst, ntp_server1, ntp_server2, ntp_server3); //setup time local
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) { //loop wait pulling time
    Serial.print(".");
    delay(1000);
  }
  
  OLED.clearDisplay();  // เคลียร์หน้าจอ
  OLED.setTextColor(WHITE);  // กำหนดให้ตัวหนังสือสีขาว 
  OLED.setCursor(0, 0);  // กำหนดให้แสดงข้อความที่ตำแหน่ง x=0 , y=0
  OLED.setTextSize(2);  // กำหนดขนาดตัวอักษาว่าจะเอากี่เท่า (2 เท่า)
  OLED.print("Ready!!!");  // แสดงคำว่า Ready!!! ออกสู่หน้าจอ
  OLED.display(); //ทำการแสดงไปบนหน้าจอตามที่กำหนดไว้ข้างต้น
  delay(5000);  // หน่วงเวลาไป 5 วีนาที(5000 millisecond)
  OLED.clearDisplay();  //  เคลียร์หน้าจอ
  OLED.display(); //ทำการแสดงไปบนหน้าจอตามที่กำหนดไว้ข้างต้น
}

void loop() {
  digitalWrite(LED_GREEN,LOW);
  digitalWrite(LED_RED,LOW);
  
  OLED.setTextColor(WHITE);
  OLED.setTextSize(1);
  OLED.setCursor(0, 0);
  OLED.print("IoT door lock system");
  OLED.display();
  
  char key = keypad.getKey(); // กำหนดให้ตัวแปร character key รับค่ามาจาก keypad 
  
  if(key){ //ถ้า key มีค่าหรือมีการกดเข้ามา ให้..
    digitalWrite(BUZZER, HIGH); //Buzzer หรือเสียงทำงาน(HIGH)
    delayMicroseconds(300); //หน่วงเวลาไป 300 Microseconds
    digitalWrite(BUZZER, LOW);  //Buzzer หรือเสียงหยุดทำงาน(LOW)
    delayMicroseconds(300); //หน่วงเวลาไป 300 Microseconds
    
    if(key == '#'){ //ถ้าkey มีค่าเท่ากับ # ให้..
      OLED.clearDisplay(); // เคลียร์หน้าจอ
      
      Serial.print("\npassword : ");
      Serial.println(password);
      
      response = check_pass(password); //ให้ตัวแปร response มีค่าเท่ากับค่าของการ return จาก fucntion check_pass โดยมีค่าพารามิเตอร์คือ password
      password = "\0";  //ให้ password เท่ากับค่าว่าง
      
      OLED.setCursor(0, 0);
      OLED.print("IoT door lock system");
      OLED.setCursor(0, 12);
      OLED.print("Wait for check.");
      OLED.display(); //ทำการแสดงบน OLED ตามที่กำหนดไว้ข้างต้น
      
      if(response == "false"){  //ถ้าหากตัวแปร response ที่รับค่ามาจาก return function check_pass มีค่าเท่ากับ false ให้..
        Serial.println("Password Incorrect!");
        
        OLED.setCursor(0, 24);
        OLED.print("Password Incorrect!");
        OLED.display();
        
        digitalWrite(LED_GREEN,LOW);
        digitalWrite(LED_RED,HIGH);
        delay(7000);
        
      }else if(response == "true"){ //ถ้าหากตัวแปร response ที่รับค่ามาจาก return function check_pass มีค่าเท่ากับ true ให้..
        Serial.println("Password correct!");
        
        OLED.setCursor(0, 24);
        OLED.print("Password correct!");
        OLED.display();
        
        digitalWrite(LED_GREEN,HIGH);
        digitalWrite(LED_RED,LOW);
        delay(7000);
        
      }
       OLED.clearDisplay();
       OLED.display();
    }else{ //ถ้าเป็นกรณีอื่นๆที่ไม่ใช่การกด # ให้..
      if((password.length() >= 0)&&(password.length() <= 3)){ //ถ้า password มีค่าระหว่าง 0 ถึง 3 ให้..
        password += key; //ให้เก็บค่า key ที่รับมาไว้ในตัวแปร password
        Serial.print("Press # for confirm");
        Serial.println(password); 
        OLED.setCursor(0, 0);
        OLED.print("IoT door lock system");    
        OLED.setCursor(0, 12);
        OLED.print("Press # for confirm");
        OLED.setCursor(0, 24);
        OLED.print(password);
        OLED.display();
      }else{  //ถ้าเป็นกรณีอื่นๆที่ password ไม่ได้มีค่าระหว่าง 0 ถึง 3 ให้..
        OLED.clearDisplay();
        OLED.setCursor(0, 0);
        OLED.print("IoT door lock system");
        Serial.println("Limit Input!");
        Serial.println("press # for confirm");
        OLED.setCursor(0, 12);
        OLED.print("Limit Input!");
        OLED.setCursor(0, 24);
        OLED.print("Press # for confirm");
        OLED.display();
      }
    }
    key = '\0'; //ให้ key มีค่าว่าง
  }
}

String check_pass(String password){ //function check_pass โดยมีพารามิเตอร์ที่รับมาเป็น string
  _password = String(password); // ให้ตัวแปร _password มีค่าเท่ากับค่าพารามิเตอร์ที่รับมาโดยเปลี่ยนเป็น string  
  
  _str  = "pin/";
  _str += _password;
  _str += "/uidKey"; //ให้ตัวแปร  _str = "pin"+_password+"/uidKey"

  _str2  = "pin/";
  _str2 += _password;
  _str2 += "/statusUsed"; //ให้ตัวแปร  _str2 = "pin"+_password+"/statusUsed"

  String uidKey = Firebase.getString(_str); //ทำการดึงค่า uidKey จากฐานข้อมูลมาตรวจสอบโดยใช้ path จาก _str
  int status_used = Firebase.getInt(_str2); //ทำการดึงค่า status_used จากฐานข้อมูลมาตรวจสอบโดยใช้ path จาก _str2
  
  bool isFailed = Firebase.failed(); //ตัวแปรตรวจสอบการทำงานของfirebase 
  if(isFailed) {  //ถ้า error ให้..
    Serial.println(Firebase.error());
  }
  Serial.print("GET uidKey : ");
  Serial.println(_str);
  Serial.print("GET status_used : ");
  Serial.println(_str2);
  Serial.print("uidKey :");
  Serial.println(uidKey);
  Serial.print("status_used :");
  Serial.println(status_used);
  
  if((uidKey.length() > 0)&&(status_used > 0)){ //ถ้าความยาวของตัวอักษรในตัวแปร uidKey มีค่ามากกว่า 0 และ status_used มีค่ามากกว่า 0 ให้..
    Serial.println("Repeat password.");
    _res = "false"; //ตัวแปร _res = false
  }else if((uidKey.length() <= 0)&&(status_used <= 0)){ //ถ้าความยาวของตัวอักษรในตัวแปร uidKey มีค่าน้อยกว่าหรือเท่ากับ 0 และ status_used มีค่าน้อยกว่าหรือเท่ากับ 0 ให้..
    _res = "false"; //ตัวแปร _res = false
  }else if((uidKey.length() > 0)&&(status_used <= 0)){ //ถ้าความยาวของตัวอักษรในตัวแปร uidKey มีค่ามากกว่า 0 และ status_used มีค่าน้อยกว่าหรือเท่ากับ 0 ให้..
    Serial.println("wait update status..");
    _res = update_status(uidKey,_password); //ตัวแปร _res เท่ากับค่าที่ return จาก function update_status โดยมีพารามิเตอร์คือ uidKey กับ _password
  }
  return _res; //return ค่าตัวแปร _res ให้กับฟังก์ชัน
}

String update_status(String uidKey, String password){
  _uidKey = String(uidKey);
  _password = String(password);
  
  _str  = "request/";
  _str += _uidKey;
  _str += "/statusUsed";

  _str2  = "pin/";
  _str2 += _password;
  _str2 += "/statusUsed";

  _str3 = "pin/";
  _str3 += _password;
  _str3 += "/timeUsed";

  String nowTime = NowDate()+"."+NowTime(); //ให้ตัวแปร nowTime มีค่าเท่ากับค่าที่ return มาจากฟังก์ชัน NowDate และ NowTime โดยมี . คั้นกลาง 
  
  Firebase.setInt(_str, 1); //ทำการกำหนดค่าบนฐานข้อมูลแบบ Integer ตาม path _str โดยให้มีค่าเท่ากับ 1
  Firebase.setInt(_str2, 1); //ทำการกำหนดค่าบนฐานข้อมูลแบบ Integer ตาม path _str2 โดยให้มีค่าเท่ากับ 1
  Firebase.setString(_str3, nowTime); //ทำการกำหนดค่าบนฐานข้อมูลแบบ String ตาม path _str3 โดยให้มีค่าเท่ากับ nowTime
  
  bool isSuccess = Firebase.success(); //ตัวแปรตรวจสอบการทำงานของ firebase ว่า success หรือไม่
  bool isFailed = Firebase.failed();  //ตัวแปรตรวจสอบการทำงานของ firebase ว่า failed หรือไม่
  if(isFailed) {  //ถ้า failed ให้..
    Serial.println("Update status faile.");
    _res = "false";   //ตัวแปร _res = false
    Serial.println(Firebase.error());
  }
  if(isSuccess){  //ถ้า success ให้..
    Serial.println("Update Status success.");
    _res = "true";  //ตัวแปร _res = true
  }
  return _res; //return ค่าตัวแปร _res ให้กับฟังก์ชัน
} 

String NowDate() {
  
  time_t now = time(nullptr); //ให้ตัวแปร now มีค่าเท่ากับ time local 
  struct tm* p_tm = localtime(&now); //แปลงให้อยู่ในรูปแบบของตัวแปร structure ของเวลา
  String dateNow = ""; //กำหนดค่า dateNow เท่ากับค่าว่าง
  dateNow += String(p_tm->tm_mday);
  dateNow += "/";
  dateNow += String(p_tm->tm_mon + 1);
  dateNow += "/";
  dateNow += String(p_tm->tm_year+ 1900); // dateNow = day+"/"+(month+1)+"/"+(year+1900)
  return dateNow;  //return ค่าตัวแปร dateNow ให้กับฟังก์ชัน
}

String NowTime() {
  
  time_t now = time(nullptr); //ให้ตัวแปร now มีค่าเท่ากับ time local 
  struct tm* p_tm = localtime(&now); //แปลงให้อยู่ในรูปแบบของตัวแปร structure ของเวลา
  String timeNow = ""; //กำหนดค่า timeNow เท่ากับค่าว่าง
  timeNow = String(p_tm->tm_hour < 10 ? "0" : ""); // timeNow มีค่าเท่ากับค่าชั่วโมงและถ้าชั่วโมงมีค่าน้อยกว่า 10 ให้เติม 0 ข้างหน้า
  timeNow += String(p_tm->tm_hour);
  timeNow += ":";
  timeNow += String(p_tm->tm_min < 10 ? "0" : "");
  timeNow += String(p_tm->tm_min);  //timeNow = hour+":"+min
  return timeNow; //return ค่าตัวแปร timeNow ให้กับฟังก์ชัน
}

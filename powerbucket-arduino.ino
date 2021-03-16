#include <WiFi.h>
#include <HTTPClient.h>
#include <string.h>
//camera stuff:
#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include "esp_camera.h"
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30         /* Time ESP32 will go to sleep (in seconds) */
#define LED_BUILTIN 4

/* Tony's House 
const char* ssid     = "Masterskaya-2.4";
const char* password = "KafeAnastasiia";
*/

/* Tony's Phone
const char* ssid     = "T-Money S6";
const char* password = "tmoney16";
*/

/* Joe's House  */
const char* ssid      = "Fios-9G8kR2.4";
const char* password  = "draw22ink266mow";


/* Forrest 
const char* ssid = "GeorgetteForrest";
const char* password = "Steggers";
*/

//Your Domain name with URL path or IP address with path
//String serverName = "http://127.0.0.1:8000/accounts/login/";
String loginUrl = "http://powerbucket-test.herokuapp.com/accounts/login/";
String indexUrl = "http://powerbucket-test.herokuapp.com/readings/";
String settingsUrl = "https://powerbucket-test.herokuapp.com/readings/change_settings/";
char * userName   = "josephabbate";
char * userPass   = "Soccerturtle3";
const char * headerkeys[] = {"Set-Cookie"};
size_t headerkeyssize = 1; //sizeof(headerkeys) / sizeof(char*);

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void startCameraServer();
RTC_DATA_ATTR int bootCount = 0; // deep sleep saved state

void setup() {

  
  // baud rate
  Serial.begin(115200);
  pinMode (LED_BUILTIN, OUTPUT);//Specify that LED pin is output
  
  // tutorial disables brownout detection
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());


  // Check boot Deep Sleep saved state
  if (bootCount == 0) {
    Serial.println("First Start, bootCount = 0");
  }
  else {
    Serial.println("Waking up, bootCount != 0");
  }
  
  //Camera setup
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM; 
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }





    
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
    camera_fb_t * fb = NULL;

  // prepare flash
  rtc_gpio_hold_dis(GPIO_NUM_4);
  digitalWrite(LED_BUILTIN, HIGH);
  // take picture
  fb = esp_camera_fb_get();
  // turn off flash
  digitalWrite(LED_BUILTIN, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  
  

  if (bootCount == 0) {
      startCameraServer();
    
      Serial.print("Camera Ready! Use 'http://");
      Serial.print(WiFi.localIP());
      Serial.println("' to connect");
    
      Serial.println("Starting webserver delay");
      delay(20000);
      Serial.println("Finishing webserver delay");
  }


  
  /* After webserver, run Power Bucket script */

  
  HTTPClient http;
  http.setReuse(true);
  String serverPath = loginUrl;
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());
  // allows us to see the csrf token from the initial GET below
  http.collectHeaders(headerkeys, headerkeyssize);
  // $CURL_BIN $LOGIN_URL > /dev/null
  int httpCode = http.GET();
  // specifies format that we'll give post request, 
  // "csrfmiddlewaretoken=$DJANGO_TOKEN&username=$YOUR_USER&password=$YOUR_PASS"
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // grab csrf token from the header
  int index = 0;
  char*  cookie = strdup(http.header(index).c_str());
  // this first call extracts "csrftoken"
  char* csrftoken = strtok(cookie, "=;");
  // this call extracts the csrf token from the Set-Cookie header
  csrftoken = strtok(NULL, "=;");
  // Put csrf token into response cookie 
  char newcookie[1000];
  strcpy(newcookie, "csrftoken=");
  strcat(newcookie, csrftoken);
  http.addHeader("Cookie", newcookie);
  // Post the data (including csrf token)
  char httpRequestData[1000];
  strcpy(httpRequestData, "csrfmiddlewaretoken=");
  strcat(httpRequestData, csrftoken);
  strcat(httpRequestData, "&username=");
  strcat(httpRequestData, userName);
  strcat(httpRequestData, "&password=");
  strcat(httpRequestData, userPass);
  http.POST(httpRequestData);
  String payload = http.getString();
  
  // below will be blank unless there's an error
  Serial.println(payload);
  // set sessionid
  char*  test = strdup(http.header(index).c_str());
  //Serial.println("Second cookie");
  //Serial.println(test);
  // this first call extracts "sessionid"
  char* sessionid = strtok(test, "=;");
  // this call extracts the sessionid from the Set-Cookie header
  sessionid = strtok(NULL, "=;");
  strcat(newcookie, "; sessionid=");
  strcat(newcookie, sessionid);
  
  String serverName="powerbucket-test.herokuapp.com";
  String serverExtension="/readings/submission/";
  const int serverPort=80;
  // make and send the message to post the pic to the website 
  WiFiClient client;
  if (client.connect(serverName.c_str(), serverPort)) {

    
    Serial.println("Connected to Website (heroku)");
  

    // this can be anything you want - just a boundary for sending the message
    String boundary="PowerBucket";
    String csrf_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"csrfmiddlewaretoken\"\r\n\r\n"+String(csrftoken)+"\r\n";
    String time_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"time\"\r\n\r\n2021-02-14 18:51:03\r\n";
    String firstNum_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"firstNum\"\r\n\r\n"+String(1)+"\r\n";
    String secondNum_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"secondNum\"\r\n\r\n"+String(1)+"\r\n";
    String thirdNum_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"thirdNum\"\r\n\r\n"+String(1)+"\r\n";
    String fourthNum_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"fourthNum\"\r\n\r\n"+String(1)+"\r\n";
    String fifthNum_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"fifthNum\"\r\n\r\n"+String(1)+"\r\n";
    String pic_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"picture\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String pic_tail="\r\n--"+boundary+"--\r\n";
    String head=csrf_head+pic_head; //time_head+firstNum_head+secondNum_head+thirdNum_head+fourthNum_head+fifthNum_head+pic_head;
    String tail=pic_tail;
    uint32_t imageLen = fb->len;
    //uint32_t extraLen = head.length() + tail.length(); //new_head.length() + 2*tail.length();
    //uint32_t totalLen = imageLen + extraLen;
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    uint32_t totallen=head.length()+imageLen+tail.length();
    
    client.println("POST " + serverExtension + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Referer: http://powerbucket-test.herokuapp.com/accounts/login/");
    client.println("Cookie: " + String(newcookie));
    client.println("Content-Length: "+String(totallen));
    client.println("Content-Type: multipart/form-data; boundary="+boundary);
    client.println();
    client.print(head);
    for (size_t n=0; n<fbLen; n=n+1024) {
      if (n+1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client.write(fbBuf, remainder);
      }
    }  
    client.print(tail);
    esp_camera_fb_return(fb);

    client.stop();
    
    // for debugging - get the website response
    int timoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;
    String getAll;
    String getBody;
  
    //Serial.println(getBody);
    delay(10000);  // this delay gives 10s for uploading to website

    
  }
  else {
    Serial.println("Failed to connect to Website (heroku)");
  }

  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  Serial.println("trying to deep sleep");
  bootCount = 1;
  esp_deep_sleep_start();
  
}

void loop() {
  
}

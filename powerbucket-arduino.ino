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
#define TIME_TO_SLEEP  60         /* Time ESP32 will go to sleep (in seconds) */
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
String serverName="powerbucket-test.herokuapp.com"; //"http://127.0.0.1:8000/";
char * userName   = "173sh-arduino";
char * userPass   = "173sh-arduino";
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

  unsigned long start_time=millis();
  // tutorial disables brownout detection
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  WiFi.begin(ssid, password);
  Serial.println("Connecting: "+String(millis()-start_time)+"ms");
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
    //startCameraServer();
  
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
  
    Serial.println("Starting webserver delay");
    delay(30000);
    Serial.println("Finishing webserver delay");
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

  
  Serial.println("Begin camera startup: "+String(millis()-start_time)+"ms");
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  camera_fb_t * fb = NULL;
  Serial.println("Begin setting flash: "+String(millis()-start_time)+"ms");

  // prepare flash
  rtc_gpio_hold_dis(GPIO_NUM_4);
  digitalWrite(LED_BUILTIN, HIGH);
  // take picture
  Serial.println("Begin delaying before taking picture: "+String(millis()-start_time)+"ms");
  delay(5000);
  Serial.println("Begin taking picture: "+String(millis()-start_time)+"ms");
  fb = esp_camera_fb_get();
  // turn off flash
  Serial.println("Begin turning off flash: "+String(millis()-start_time)+"ms"); 
  digitalWrite(LED_BUILTIN, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }

  
  
  /* After webserver, run Power Bucket script */

/*
  Serial.println("Begin camera startup: "+String(millis()-start_time)+"ms");
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  camera_fb_t * fb = NULL;
  Serial.println("Begin setting flash: "+String(millis()-start_time)+"ms");

  // prepare flash
  rtc_gpio_hold_dis(GPIO_NUM_4);
  digitalWrite(LED_BUILTIN, HIGH);
  // take picture
  Serial.println("Begin delaying before taking picture: "+String(millis()-start_time)+"ms");
  delay(5000);
  Serial.println("Begin taking picture: "+String(millis()-start_time)+"ms");
  fb = esp_camera_fb_get();
  // turn off flash
  Serial.println("Begin turning off flash: "+String(millis()-start_time)+"ms"); 
  digitalWrite(LED_BUILTIN, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  */

  const int serverPort=80;
  // make and send the message to post the pic to the website 
  WiFiClient client;
  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connected to website: "+String(millis()-start_time)+"ms");
    client.println("GET /accounts/login/ HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Accept: */*");
    client.println("Referer: http://powerbucket-test.herokuapp.com/accounts/login/");
    client.println();
    // for debugging - get the website response
    int timoutTimer = 10000;
    long startTimer = millis();
    String getBody;
    
    while ((startTimer + timoutTimer) > millis()) {
      //Serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        getBody+=String(c);
      }
    }
    //Serial.println(getBody);
    // this call extracts the csrf token from the Set-Cookie header
    String delimiter = "csrftoken=";
    int token_start = getBody.indexOf(delimiter) + delimiter.length();//+length(delimiter);
    int token_end = getBody.indexOf(";",token_start);
    String csrftoken = getBody.substring(token_start,token_end);
    //Serial.println("Cookie: "+csrftoken);
    Serial.println();
    Serial.println("Extracted csrf token: "+String(millis()-start_time)+"ms");

    String postData="csrfmiddlewaretoken="+csrftoken+"&username="+userName+"&password="+userPass;
    client.println("POST /accounts/login/ HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Accept: */*");
    client.println("Referer: http://powerbucket-test.herokuapp.com/accounts/login/");
    client.println("Cookie: csrftoken="+csrftoken);
    client.println("Content-Length: "+String(postData.length()));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();
    client.println(postData);
    client.println();
    
    // for debugging - get the website response
    startTimer = millis();
    getBody="";
    
    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        getBody+=String(c);
      }
    }
    //Serial.println(getBody);
    // this first call extracts "csrftoken"
    // this call extracts the csrf token from the Set-Cookie header
    delimiter = "sessionid=";
    token_start = getBody.indexOf(delimiter) + delimiter.length();//+length(delimiter);
    token_end = getBody.indexOf(";",token_start);
    String sessionid = getBody.substring(token_start,token_end);
    //Serial.println("Session id: "+sessionid);
    Serial.println();
    Serial.println("Logged in and extracted session id: "+String(millis()-start_time)+"ms");

    // write submission
    // this can be anything you want - just a boundary for sending the message
    String boundary="PowerBucket";
    Serial.println();
    Serial.println("csrftoken: "+csrftoken);
    String csrf_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"csrfmiddlewaretoken\"\r\n\r\n"+csrftoken+"\r\n";
    // hack: if the time is before the beginning of the year 2000, website sets the time for you to be now
    String time_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"time\"\r\n\r\n1999-01-01 00:00:00\r\n";
    String firstNum_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"firstNum\"\r\n\r\n"+String(1)+"\r\n";
    String secondNum_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"secondNum\"\r\n\r\n"+String(1)+"\r\n";
    String thirdNum_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"thirdNum\"\r\n\r\n"+String(1)+"\r\n";
    String fourthNum_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"fourthNum\"\r\n\r\n"+String(1)+"\r\n";
    String fifthNum_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"fifthNum\"\r\n\r\n"+String(1)+"\r\n";
    String pic_head="--"+boundary+"\r\nContent-Disposition: form-data; name=\"picture\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String pic_tail="\r\n--"+boundary+"--\r\n";
    
    //String head=csrf_head+pic_head; 
    String head=csrf_head+time_head+firstNum_head+secondNum_head+thirdNum_head+fourthNum_head+fifthNum_head+pic_head;
    String tail=pic_tail;

    Serial.println("Done prepping POST data: "+String(millis()-start_time)+"ms");
    
    uint32_t imageLen = fb->len;
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    uint32_t totallen=head.length()+imageLen+tail.length();
    
    client.println("POST /readings/submission/ HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Referer: http://powerbucket-test.herokuapp.com/accounts/login/");
    client.println("Cookie: csrftoken=" + csrftoken + "; sessionid=" + sessionid);
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
    
    startTimer = millis();
    getBody="";

    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        getBody+=String(c);
      }
    }
    Serial.println(getBody);
    
    Serial.println("Posted reading submission to website: "+String(millis()-start_time)+"ms");
    esp_camera_fb_return(fb);
  }
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  Serial.println("trying to deep sleep");
  bootCount = 1;
  esp_deep_sleep_start();
}

void loop() {
  
}

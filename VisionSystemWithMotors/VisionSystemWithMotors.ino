  
/*
ESP32-CAM object detecting and Tracking with Tensorflow.js
This code modified from the people tracking code from ChungYi Fu
below is his code:
https://github.com/fustyles/Arduino/blob/master/ESP32-CAM_Tensorflow.js/ESP32-CAM_coco-ssd_PeopleTracking/ESP32-CAM_coco-ssd_PeopleTracking.ino  
other references:
https://github.com/tensorflow/tfjs-models/blob/master/coco-ssd/src/classes.ts

*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"         
#include "soc/soc.h"            //don't restart if the power is not stable
#include "soc/rtc_cntl_reg.h"   //don't restart if the power is not stable
#include "ESP32Servo.h"

Servo ServoOne;
int mspeed = 1500;

const char* ssid     = "*******";   //input your network SSID
const char* password = "*******";   //input your network password
/*Servo1 horazonal -> gpio2 
  Servo2 vertical -> gpio13
  initial servo angle value in dutycycle
*/
int angle1Value1 = 4850;
int angle1Value2 = 4850;

String Feedback="";   //feedback from client
String Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9=""; //command index
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0; //receiver status

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

WiFiServer server(80);//server port

void ExecuteCommand()
{

  if (cmd!="getstill") {
    Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
    Serial.println("");
  }
  
  if (cmd=="ip") {
    Feedback+="STA IP: "+WiFi.localIP().toString();
  }  
  else if (cmd=="mac") {
    Feedback="STA MAC: "+WiFi.macAddress();
  }  
  else if (cmd=="resetwifi") {  
    WiFi.begin(P1.c_str(), P2.c_str());
    Serial.print("Connecting to ");
    Serial.println(P1);
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 
    Serial.println("");
    Serial.println("STAIP: "+WiFi.localIP().toString());
    Feedback="STAIP: "+WiFi.localIP().toString();
  }    
  else if (cmd=="restart") {
    ESP.restart();
    randomPattern();
    rightFirst();
  }    
  else if (cmd=="flash") {
    ledcAttachPin(4, 4);  
    ledcSetup(4, 5000, 8);   
     
    int val = P1.toInt();
    ledcWrite(4,val);  
  }  
  else if (cmd=="framesize") { 
    sensor_t * s = esp_camera_sensor_get();  
    if (P1=="QQVGA")
      s->set_framesize(s, FRAMESIZE_QQVGA);
    else if (P1=="HQVGA")
      s->set_framesize(s, FRAMESIZE_HQVGA);
    else if (P1=="QVGA")
      s->set_framesize(s, FRAMESIZE_QVGA);
    else if (P1=="CIF")
      s->set_framesize(s, FRAMESIZE_CIF);
    else if (P1=="VGA")
      s->set_framesize(s, FRAMESIZE_VGA);  
    else if (P1=="SVGA")
      s->set_framesize(s, FRAMESIZE_SVGA);
    else if (P1=="XGA")
      s->set_framesize(s, FRAMESIZE_XGA);
    else if (P1=="SXGA")
      s->set_framesize(s, FRAMESIZE_SXGA);
    else if (P1=="UXGA")
      s->set_framesize(s, FRAMESIZE_UXGA);           
    else 
      s->set_framesize(s, FRAMESIZE_QVGA);     
  }
  else if (cmd=="quality") { 
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_quality(s, val);
  }
  else if (cmd=="contrast") {
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_contrast(s, val);
  }
  else if (cmd=="brightness") {
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt();  
    s->set_brightness(s, val);  
  }
  else if (cmd=="servo1") {
    int val = P1.toInt();
    //ledcAttachPin(12, 3);  
    //ledcSetup(3, 50, 16);      
    if (val > 8000)
       val = 8000;
    else if (val < 1700)
      val = 1700;   
    val = 1700 + (8000 - val);   
    /*if(val >4850)
      {ServoOne.attach(2);
      ServoOne.writeMicroseconds(1250);
      delay(3000);
      ServoOne.writeMicroseconds(1500);
      delay(500);}
      else{
        ServoOne.attach(2);
      ServoOne.writeMicroseconds(1850);
      delay(3000);
      ServoOne.writeMicroseconds(1500);
      delay(500);  
      }*/
      if(val >4850)
      rightFirst();
      if(val<4850)
      leftFirst();
      //ledcWrite(3, val); 
    Serial.println("servo1="+String(val));
  }  
  else if (cmd=="servo2") {
    int val = P1.toInt();
   // ledcAttachPin(13, 5);  
   // ledcSetup(5, 50, 16);      
    if (val > 8000)
       val = 8000;
    else if (val < 1700)
      val = 1700;   
    val = 1700 + (8000 - val);   
    while(val) randomPattern();
    Serial.println("servo2="+String(val));
   
  }   
  else {
    Feedback="Command is not defined.";
  }
  if (Feedback=="") Feedback=Command;  
}

void randomPattern(){
  ledcAttachPin(13, 5);  
  ledcSetup(5, 50, 16);    
  int r = random(1800,7650);
  ledcWrite(5,r);
  delay(4500);
}

void rightFirst(){
      ServoOne.attach(2);
      ServoOne.writeMicroseconds(1250);
      delay(3000);
      ServoOne.writeMicroseconds(1500);
      delay(500);
       ServoOne.writeMicroseconds(1850);
      delay(3000);
      ServoOne.writeMicroseconds(1500);
      delay(500);  
}
void leftFirst(){
      ServoOne.attach(2);
      ServoOne.writeMicroseconds(1850);
      delay(3000);
      ServoOne.writeMicroseconds(1500);
      delay(500);  
       ServoOne.writeMicroseconds(1250);
      delay(3000);
      ServoOne.writeMicroseconds(1500);
      delay(500);
      }
  
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //turn off the setting that restart if the power supply is not stable
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);  //start output
  Serial.println();

  //camera pin setting
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
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
    randomPattern();
    rightFirst();
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);  //UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA  
  
  //Servo
  ledcAttachPin(12, 3);  
  ledcSetup(3, 50, 16);
  ledcWrite(3, 4850);

  //ServoOne.attach(2);

  ledcAttachPin(13, 5);  
  ledcSetup(5, 50, 16);
  ledcWrite(5, 4850);  
  
  //flash
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);    
  
  WiFi.mode(WIFI_AP_STA);

  WiFi.begin(ssid, password);    //connect wifi

  delay(1000);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      if ((StartTime+10000) < millis()) break;    
  } 

  if (WiFi.status() == WL_CONNECTED) {    
//    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);   //set SSID to display client's ip         
    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());  

    for (int i=0;i<5;i++) {   //if wifi connected, flash quickly 
      ledcWrite(4,10);
      delay(200);
      ledcWrite(4,0);
      delay(200);    
    }         
  }
  else {
//    WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);         

    for (int i=0;i<2;i++) {    //if wifi not connected, flash slowly
      ledcWrite(4,10);
      delay(1000);
      ledcWrite(4,0);
      delay(1000);    
    }
  }       

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);  

  server.begin();          
}

//self manage web interface
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
  <!DOCTYPE html>
  <head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <script src="https:\/\/ajax.googleapis.com/ajax/libs/jquery/1.8.0/jquery.min.js"></script>
  <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow/tfjs@1.3.1/dist/tf.min.js"> </script>
  <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow-models/coco-ssd@2.1.0"> </script>   
  </head><body>
  <img id="ShowImage" src="" style="display:none">
  <canvas id="canvas" width="0" height="0"></canvas>  
  <table>
  <tr>
    <td><input type="button" id="restart" value="Restart"></td> 
    <td colspan="2"><input type="button" id="getStill" value="Start Detect" style="display:none"></td> 
  </tr>
  <tr>  
    <td>Object</td> 
    <td colspan="2">
        <select id="object" onchange="count.innerHTML='';">
          <option value="person" selected="selected">cat</option>
          <option value="cat">cat</option>
          <option value="dog">dog</option>
        </select>
    </td>
    <td><span id="count" style="color:red"><span></td>
  </tr>
  <tr>
    <td>ScoreLimit</td> 
    <td colspan="2">
      <select id="score">
        <option value="1.0">1</option>
        <option value="0.9">0.9</option>
        <option value="0.8">0.8</option>
        <option value="0.7">0.7</option>
        <option value="0.6">0.6</option>
        <option value="0.5">0.5</option>
        <option value="0.4">0.4</option>
        <option value="0.3">0.3</option>
        <option value="0.2">0.2</option>
        <option value="0.1">0.1</option>
        <option value="0" selected="selected">0</option>
      </select>
    </td>
  </tr>
  <tr>
    <td>MirrorImage</td> 
    <td colspan="2">  
      <select id="mirrorimage">
        <option value="1">yes</option>
        <option value="0">no</option>
      </select>
    </td>
  </tr>   
  <tr>
    <td>Resolution</td> 
    <td colspan="2">
      <select id="framesize">
        <option value="UXGA">UXGA(1600x1200)</option>
        <option value="SXGA">SXGA(1280x1024)</option>
        <option value="XGA">XGA(1024x768)</option>
        <option value="SVGA">SVGA(800x600)</option>
        <option value="VGA">VGA(640x480)</option>
        <option value="CIF">CIF(400x296)</option>
        <option value="QVGA" selected="selected">QVGA(320x240)</option>
        <option value="HQVGA">HQVGA(240x176)</option>
        <option value="QQVGA">QQVGA(160x120)</option>
      </select> 
    </td>
  </tr>    
  <tr>
    <td>Flash</td>
    <td colspan="2"><input type="range" id="flash" min="0" max="255" value="0"></td>
  </tr>
  <tr>
    <td>Quality</td>
    <td colspan="2"><input type="range" id="quality" min="10" max="63" value="10"></td>
  </tr>
  <tr>
    <td>Brightness</td>
    <td colspan="2"><input type="range" id="brightness" min="-2" max="2" value="0"></td>
  </tr>
  <tr>
    <td>Contrast</td>
    <td colspan="2"><input type="range" id="contrast" min="-2" max="2" value="0"></td>
  </tr>
  <tr>
    <td>Rotate</td>
    <td align="left" colspan="2">
      <select onchange="document.getElementById('canvas').style.transform='rotate('+this.value+')';">
        <option value="0deg">0deg</option>
        <option value="90deg">90deg</option>
        <option value="180deg">180deg</option>
        <option value="270deg">270deg</option>
      </select>
    </td>
  </tr>  
  </table>
  <div id="result" style="color:red"><div>
  </body>
  </html> 
  
  <script>
    var getStill = document.getElementById('getStill');
    var ShowImage = document.getElementById('ShowImage');
    var canvas = document.getElementById("canvas");
    var context = canvas.getContext("2d"); 
    var object = document.getElementById('object');
    var score = document.getElementById("score");
    var mirrorimage = document.getElementById("mirrorimage");    
    var count = document.getElementById('count');     
    var result = document.getElementById('result');
    var flash = document.getElementById('flash'); 
    var lastValue="";
    var myTimer;  
    var restartCount=0;     
    var Model;
    var angle1Value1 = 4850;
    var angle1Value2 = 4850;        
    getStill.onclick = function (event) { 
      clearInterval(myTimer);   
      myTimer = setInterval(function(){error_handle();},5000);
      ShowImage.src=location.origin+'/?getstill='+Math.random();
    }
    function error_handle() {
      restartCount++;
      clearInterval(myTimer);
      if (restartCount<=2) {
        result.innerHTML = "Get still error. <br>Restart ESP32-CAM "+restartCount+" times.";
        myTimer = setInterval(function(){getStill.click();},10000);
      }
      else
        result.innerHTML = "Get still error. <br>Please close the page and check ESP32-CAM.";
    }     
    ShowImage.onload = function (event) {
      clearInterval(myTimer);
      restartCount=0;      
      canvas.setAttribute("width", ShowImage.width);
      canvas.setAttribute("height", ShowImage.height);
      
      if (mirrorimage.value==1) {
        context.translate((canvas.width + ShowImage.width) / 2, 0);
        context.scale(-1, 1);
        context.drawImage(ShowImage, 0, 0, ShowImage.width, ShowImage.height);
        context.setTransform(1, 0, 0, 1, 0, 0);
      }
      else
        context.drawImage(ShowImage,0,0,ShowImage.width,ShowImage.height);
      if (Model) {
        DetectImage();
      }          
    }     
    
    restart.onclick = function (event) {
      fetch(location.origin+'/?restart=stop');
    }    
    framesize.onclick = function (event) {
      fetch(document.location.origin+'/?framesize='+this.value+';stop');
    }  
    flash.onchange = function (event) {
      fetch(location.origin+'/?flash='+this.value+';stop');
    } 
    quality.onclick = function (event) {
      fetch(document.location.origin+'/?quality='+this.value+';stop');
    } 
    brightness.onclick = function (event) {
      fetch(document.location.origin+'/?brightness='+this.value+';stop');
    } 
    contrast.onclick = function (event) {
      fetch(document.location.origin+'/?contrast='+this.value+';stop');
    }                             
    
    function ObjectDetect() {
      result.innerHTML = "Please wait for loading model.";
      cocoSsd.load().then(cocoSsd_Model => {
        Model = cocoSsd_Model;
        result.innerHTML = "";
        getStill.style.display = "block";
      }); 
    }
    
    function DetectImage() {
      Model.detect(canvas).then(Predictions => {    
        var s = (canvas.width>canvas.height)?canvas.width:canvas.height;
        var objectCount=0;  //record the total number of detected objects
        var trackState = 0;  //if any object detected 
        var x, y, width, height;
        
        //console.log('Predictions: ', Predictions);
        if (Predictions.length>0) {
          result.innerHTML = "";
          for (var i=0;i<Predictions.length;i++) {
            const x = Predictions[i].bbox[0];
            const y = Predictions[i].bbox[1];
            const width = Predictions[i].bbox[2];
            const height = Predictions[i].bbox[3];
            context.lineWidth = Math.round(s/200);
            context.strokeStyle = "#00FFFF";
            context.beginPath();
            context.rect(x, y, width, height);
            context.stroke(); 
            context.lineWidth = "2";
            context.fillStyle = "red";
            context.font = Math.round(s/30) + "px Arial";
            context.fillText(Predictions[i].class, x, y);
            //context.fillText(i, x, y);
            result.innerHTML+= "[ "+i+" ] "+Predictions[i].class+", "+Math.round(Predictions[i].score*100)+"%, "+Math.round(x)+", "+Math.round(y)+", "+Math.round(width)+", "+Math.round(height)+"<br>";
            if (Predictions[i].class==object.value&&Predictions[i].score>=score.value&&trackState==0) {   
              trackState = 1;  //只偵測第一人
              
              var midX = Math.round(x)+Math.round(width)/2;  //get the mid horizontal index of detected object
              if (midX>(40+320/2)) {   //the index is slight on the right, too small to move the servo
                if (midX>260) {  //if the index is on right,servo start to move 
                  if (mirrorimage.value==1) {angle1Value1-=550;}else{angle1Value1+=550;}
                } else {
                  if (mirrorimage.value==1) {angle1Value1-=375;}else{angle1Value1+=375;}
                }
                if (angle1Value1 > 7650) angle1Value1 = 7650;
                if (angle1Value1 < 2050) angle1Value1 = 2050;
                $.ajax({url: document.location.origin+'?servo1='+angle1Value1, async: false}); 
              }
              else if (midX<(320/2)-40) {  //the index is slight on the left, too small to move the servo
                if (midX<60) {  ///if the index is on left, servo start to move
                  if (mirrorimage.value==1) {angle1Value1+=550;}else{angle1Value1-=550;}
                } else {
                  if (mirrorimage.value==1) {angle1Value1+=375;}else{angle1Value1-=375;}
                }
                if (angle1Value1 > 7650) angle1Value1 = 7650;
                if (angle1Value1 < 2050) angle1Value1 = 2050;                 
                $.ajax({url: document.location.origin+'?servo1='+angle1Value1, async: false}); 
              }
                
              var midY = Math.round(y)+Math.round(height)/2; //get the mid vertical index of detected object
              if (midY>(240/2+30)) {  //the index is slight on the bottom, too small to move the servo
                if (midY>195) {  //if the index is on bottom,servo start to move
                  angle1Value2-=300;
                } else {
                  angle1Value2-=150; 
                }
                if (angle1Value2 > 7650) angle1Value2 = 7650;
                if (angle1Value2 < 2050) angle1Value2 = 2050;                  
                $.ajax({url: document.location.origin+'?servo2='+angle1Value2, async: false}); 
              }
              else if (midY<(240/2)-30) {   //the index is slight on the top, too small to move the servo
                if (midY<45) { //if the index is on the top,servo start to move
                  angle1Value2+=300;
                } else {
                  angle1Value2+=150;   
                }
                if (angle1Value2 > 7650) angle1Value2 = 7650;
                if (angle1Value2 < 2050) angle1Value2 = 2050;                  
                $.ajax({url: document.location.origin+'?servo2='+angle1Value2, async: false});  
              }    
            }
                      
            if (Predictions[i].class==object.value) {
              objectCount++;
            }  
          }
          count.innerHTML = objectCount;
        }
        else {
          result.innerHTML = "Unrecognizable";
          count.innerHTML = "0";
        }
    
      //if (count.innerHTML != lastValue) { 
        lastValue = count.innerHTML;
        //ifr.src = document.location.origin+'/?detectCount='+object.value+';'+String(objectCount)+';stop';        
      //}
        try { 
          document.createEvent("TouchEvent");
          setTimeout(function(){getStill.click();},250);
        }
        catch(e) { 
          setTimeout(function(){getStill.click();},150);
        } 
      });
    }
    function getFeedback(target) {
      var data = $.ajax({
      type: "get",
      dataType: "text",
      url: target,
      success: function(response)
        {
          result.innerHTML = response;
        },
        error: function(exception)
        {
          result.innerHTML = 'fail';
        }
      });
    }     
    window.onload = function () { ObjectDetect(); }    
  </script>   
)rawliteral";



void loop() {
  vision();
}

void vision(){
  Feedback="";Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;
  
   WiFiClient client = server.available();

  if (client) { 
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();             
        
        getCommand(c);   
                
        if (c == '\n') {
          if (currentLine.length() == 0) {    
            
            if (cmd=="getstill") {
    
              camera_fb_t * fb = NULL;
              fb = esp_camera_fb_get();  
              if(!fb) {
                Serial.println("Camera capture failed");
                delay(1000);
                ESP.restart();
              }
  
              client.println("HTTP/1.1 200 OK");
              client.println("Access-Control-Allow-Origin: *");              
              client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
              client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
              client.println("Content-Type: image/jpeg");
              client.println("Content-Disposition: form-data; name=\"imageFile\"; filename=\"picture.jpg\""); 
              client.println("Content-Length: " + String(fb->len));             
              client.println("Connection: close");
              client.println();
              
              uint8_t *fbBuf = fb->buf;
              size_t fbLen = fb->len;
              for (size_t n=0;n<fbLen;n=n+1024) {
                if (n+1024<fbLen) {
                  client.write(fbBuf, 1024);
                  fbBuf += 1024;
                }
                else if (fbLen%1024>0) {
                  size_t remainder = fbLen%1024;
                  client.write(fbBuf, remainder);
                }
              }  
              
              esp_camera_fb_return(fb);
            
              pinMode(4, OUTPUT);
              digitalWrite(4, LOW);               
            }
            else {
            
              client.println("HTTP/1.1 200 OK");
              client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
              client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
              client.println("Content-Type: text/html; charset=utf-8");
              client.println("Access-Control-Allow-Origin: *");
              client.println("Connection: close");
              client.println();
              
              String Data="";
              if (cmd!="")
                Data = Feedback;
              else {
                Data = String((const char *)INDEX_HTML);
              }
              int Index;
              for (Index = 0; Index < Data.length(); Index = Index+1000) {
                client.print(Data.substring(Index, Index+1000));
              }           
              
              client.println();
            }
                        
            Feedback="";
            break;
          } else {
            currentLine = "";
          }
        } 
        else if (c != '\r') {
          currentLine += c;
        }

        if ((currentLine.indexOf("/?")!=-1)&&(currentLine.indexOf(" HTTP")!=-1)) {
          if (Command.indexOf("stop")!=-1) {  
            client.println();
            client.println();
            client.stop();
          }
          currentLine="";
          Feedback="";
          ExecuteCommand();
        }
      }
    }
    delay(1);
    client.stop();
  }
}
void getCommand(char c)
{
  if (c=='?') ReceiveState=1;
  if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
  
  if (ReceiveState==1)
  {
    Command=Command+String(c);
    
    if (c=='=') cmdState=0;
    if (c==';') strState++;
  
    if ((cmdState==1)&&((c!='?')||(questionstate==1))) cmd=cmd+String(c);
    if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) P1=P1+String(c);
    if ((cmdState==0)&&(strState==2)&&(c!=';')) P2=P2+String(c);
    if ((cmdState==0)&&(strState==3)&&(c!=';')) P3=P3+String(c);
    if ((cmdState==0)&&(strState==4)&&(c!=';')) P4=P4+String(c);
    if ((cmdState==0)&&(strState==5)&&(c!=';')) P5=P5+String(c);
    if ((cmdState==0)&&(strState==6)&&(c!=';')) P6=P6+String(c);
    if ((cmdState==0)&&(strState==7)&&(c!=';')) P7=P7+String(c);
    if ((cmdState==0)&&(strState==8)&&(c!=';')) P8=P8+String(c);
    if ((cmdState==0)&&(strState>=9)&&((c!=';')||(semicolonstate==1))) P9=P9+String(c);
    
    if (c=='?') questionstate=1;
    if (c=='=') equalstate=1;
    if ((strState>=9)&&(c==';')) semicolonstate=1;
  }
}

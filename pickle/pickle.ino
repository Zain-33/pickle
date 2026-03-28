#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>

/* ================= OLED ================= */
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_SDA       21
#define OLED_SCL       22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RoboEyes<Adafruit_SSD1306> roboEyes(display);

/* ================= L298N MOTOR PINS ================= */
#define LF   25   // IN1  - Left  Forward
#define LB   26   // IN2  - Left  Backward
#define ENA  32   // PWM  - Left  Speed

#define RF   27   // IN3  - Right Forward
#define RB   14   // IN4  - Right Backward
#define ENB  33   // PWM  - Right Speed

#define MOTOR_SPEED 200   // 0-255, tune for your motors

/* ================= HC-SR04 ================= */
#define TRIG_PIN             18
#define ECHO_PIN             19
#define OBSTACLE_DISTANCE_CM 20

/* ================= WIFI ================= */
WebServer server(80);
DNSServer  dnsServer;

/* ================= STATE ================= */
volatile bool manualActive = false;

/* ================= MODE ================= */
enum AutoMode { AUTO_OFF, AUTO_SOFT, AUTO_NORMAL };
volatile AutoMode autoMode = AUTO_NORMAL;
AutoMode prevMode = AUTO_NORMAL;  // track changes for eye switching

/* ================= MOTOR HELPERS ================= */
void setSpeed(bool on) {
  analogWrite(ENA, on ? MOTOR_SPEED : 0);
  analogWrite(ENB, on ? MOTOR_SPEED : 0);
}

void stopMotors() {
  digitalWrite(LF, LOW); digitalWrite(LB, LOW);
  digitalWrite(RF, LOW); digitalWrite(RB, LOW);
  setSpeed(false);
}

/* drive in direction c for ms milliseconds then stop */
void drive(byte c, int ms) {
  setSpeed(true);
  switch (c) {
    case 1: digitalWrite(LF,HIGH);digitalWrite(LB,LOW); digitalWrite(RF,HIGH);digitalWrite(RB,LOW); break; // fwd
    case 2: digitalWrite(LF,LOW); digitalWrite(LB,HIGH);digitalWrite(RF,LOW); digitalWrite(RB,HIGH);break; // back
    case 3: digitalWrite(LF,LOW); digitalWrite(LB,HIGH);digitalWrite(RF,HIGH);digitalWrite(RB,LOW); break; // left
    case 4: digitalWrite(LF,HIGH);digitalWrite(LB,LOW); digitalWrite(RF,LOW); digitalWrite(RB,HIGH);break; // right
    default: stopMotors(); return;
  }
  delay(ms);
  stopMotors();
}

/* WiFi-triggered drive — sets direction, no built-in delay */
void motorWifi(byte c) {
  switch (c) {
    case 0: stopMotors(); return;
    case 1: setSpeed(true);digitalWrite(LF,HIGH);digitalWrite(LB,LOW); digitalWrite(RF,HIGH);digitalWrite(RB,LOW); break;
    case 2: setSpeed(true);digitalWrite(LF,LOW); digitalWrite(LB,HIGH);digitalWrite(RF,LOW); digitalWrite(RB,HIGH);break;
    case 3: setSpeed(true);digitalWrite(LF,LOW); digitalWrite(LB,HIGH);digitalWrite(RF,HIGH);digitalWrite(RB,LOW); break;
    case 4: setSpeed(true);digitalWrite(LF,HIGH);digitalWrite(LB,LOW); digitalWrite(RF,LOW); digitalWrite(RB,HIGH);break;
  }
}

/* ================= RANDOM MOTOR (Autonomous) ================= */
void MOTOR(byte c, int t1, int t2, int times) {
  setSpeed(true);
  for (int i = 0; i < times; i++) {
    switch (c) {
      case 0: digitalWrite(LF,LOW); digitalWrite(LB,LOW); digitalWrite(RF,LOW); digitalWrite(RB,LOW); break;
      case 1: digitalWrite(LF,LOW); digitalWrite(LB,HIGH);digitalWrite(RF,LOW); digitalWrite(RB,HIGH);break;
      case 2: digitalWrite(LF,HIGH);digitalWrite(LB,LOW); digitalWrite(RF,HIGH);digitalWrite(RB,LOW); break;
      case 3: digitalWrite(LF,LOW); digitalWrite(LB,HIGH);digitalWrite(RF,HIGH);digitalWrite(RB,LOW); break;
      case 4: digitalWrite(LF,HIGH);digitalWrite(LB,LOW); digitalWrite(RF,LOW); digitalWrite(RB,HIGH);break;
      case 5: digitalWrite(LF,LOW); digitalWrite(LB,HIGH);digitalWrite(RF,LOW); digitalWrite(RB,LOW); break;
      case 6: digitalWrite(LF,LOW); digitalWrite(LB,LOW); digitalWrite(RF,LOW); digitalWrite(RB,HIGH);break;
      case 7: digitalWrite(LF,HIGH);digitalWrite(LB,LOW); digitalWrite(RF,LOW); digitalWrite(RB,LOW); break;
      case 8: digitalWrite(LF,LOW); digitalWrite(LB,LOW); digitalWrite(RF,HIGH);digitalWrite(RB,LOW); break;
    }
    delay(t1);
    stopMotors();
    delay(t2);
  }
}

/* ================= WIGGLE DANCE ================= */
/*
  Medium rhythmic pattern (~2.4s total):
  1. Left  x2  (150ms each) — the shake
  2. Right x2  (150ms each) — the shake back
  3. Forward   (120ms)      — the bump
  4. Backward  (120ms)      — the bump back
  5. Half-spin left (280ms) — the flair finish
*/
void wiggleDance() {
  drive(3, 150); delay(80);
  drive(3, 150); delay(80);
  drive(4, 150); delay(80);
  drive(4, 150); delay(80);
  drive(1, 120); delay(80);
  drive(2, 120); delay(80);
  drive(3, 280); delay(80);
  stopMotors();
}

/* ================= HC-SR04 ================= */
long getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (dur == 0) return 999;                  // no echo = clear path
  return dur * 0.034 / 2;
}

/* ================= OBSTACLE AVOIDANCE ================= */
void avoidObstacle() {
  stopMotors();
  delay(150);
  drive(2, 300);  // back up
  delay(100);
  if (random(2) == 0) drive(3, random(300, 600));  // random turn
  else                 drive(4, random(300, 600));
  stopMotors();
}

/* ================= SLEEP EYES ================= */
/*
  Draws two thick horizontal lines (closed eyes) + floating zzZ
  directly on the OLED, bypassing RoboEyes entirely.
  Called once on mode transition to AUTO_OFF.
*/
void drawSleepEyes() {
  display.clearDisplay();

  // Left closed eye
  for (int t = 0; t < 3; t++) {
    display.drawLine(18, 31 + t, 54, 31 + t, SSD1306_WHITE);
  }
  // Right closed eye
  for (int t = 0; t < 3; t++) {
    display.drawLine(74, 31 + t, 110, 31 + t, SSD1306_WHITE);
  }

  // Floating z z Z
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(100, 20);
  display.print("z");
  display.setCursor(108, 13);
  display.print("z");
  display.setTextSize(2);
  display.setCursor(114, 4);
  display.print("Z");

  display.display();
}

/* ================= WEB UI ================= */
void handleRoot() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{
  margin:0;height:100vh;
  background:radial-gradient(circle at top,#0f2027,#000);
  color:#00ffe1;
  font-family:Arial;
  display:flex;align-items:center;justify-content:center;
}
.panel{
  width:260px;padding:20px;border-radius:18px;
  background:rgba(0,255,225,0.05);
  border:1px solid rgba(0,255,225,0.4);
  box-shadow:0 0 25px rgba(0,255,225,0.3);
}
h2{text-align:center;margin:0 0 14px;letter-spacing:2px;}
.grid{
  display:grid;
  grid-template-columns:1fr 1fr 1fr;
  grid-template-rows:60px 60px 60px;
  gap:10px;
}
button{
  border:none;border-radius:12px;
  font-size:16px;font-weight:bold;
  background:linear-gradient(145deg,#0ff,#00b3a4);
  cursor:pointer;
}
.stop{background:linear-gradient(145deg,#ff5555,#aa0000);color:#fff;}
.empty{background:none;pointer-events:none;}
.mode{margin-top:12px;display:flex;gap:6px;}
.mode button{flex:1;font-size:12px;opacity:0.6;padding:8px 2px;}
.mode button.active{
  opacity:1;
  background:linear-gradient(145deg,#00ff9c,#00c46a);
  box-shadow:0 0 12px rgba(0,255,180,0.8);
}
.footer{margin-top:14px;text-align:center;font-size:11px;opacity:0.6;letter-spacing:1px;}
</style>
</head>
<body>
<div class="panel">
  <h2>🥒 PICKLE</h2>
  <div class="grid">
    <div class="empty"></div>
    <button onclick="fetch('/f')">▲</button>
    <div class="empty"></div>
    <button onclick="fetch('/l')">◀</button>
    <button class="stop" onclick="fetch('/s')">STOP</button>
    <button onclick="fetch('/r')">▶</button>
    <div class="empty"></div>
    <button onclick="fetch('/b')">▼</button>
    <div class="empty"></div>
  </div>
  <div class="mode">
    <button id="btn_sleep"  onclick="setMode('off')">SLEEP</button>
    <button id="btn_wiggle" onclick="setMode('soft')">WIGGLE</button>
    <button id="btn_auto"   onclick="setMode('normal')" class="active">AUTONOMOUS</button>
  </div>
  <div class="footer">🥒 Pickle v1.1</div>
</div>
<script>
function clearActive(){
  ['btn_sleep','btn_wiggle','btn_auto'].forEach(function(id){
    document.getElementById(id).classList.remove('active');
  });
}
function setMode(mode){
  fetch('/mode_' + mode);
  clearActive();
  if(mode==='off')    document.getElementById('btn_sleep').classList.add('active');
  if(mode==='soft')   document.getElementById('btn_wiggle').classList.add('active');
  if(mode==='normal') document.getElementById('btn_auto').classList.add('active');
}
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", page);
}

/* ================= SERVER ================= */
void setupServer() {
  server.on("/", handleRoot);
  server.on("/f", []() { manualActive=true; motorWifi(1); server.send(200); manualActive=false; });
  server.on("/b", []() { manualActive=true; motorWifi(2); server.send(200); manualActive=false; });
  server.on("/l", []() { manualActive=true; motorWifi(3); server.send(200); manualActive=false; });
  server.on("/r", []() { manualActive=true; motorWifi(4); server.send(200); manualActive=false; });
  server.on("/s", []() { manualActive=true; stopMotors();  server.send(200); manualActive=false; });
  server.on("/mode_off",    []() { autoMode=AUTO_OFF;    server.send(200); });
  server.on("/mode_soft",   []() { autoMode=AUTO_SOFT;   server.send(200); });
  server.on("/mode_normal", []() { autoMode=AUTO_NORMAL; server.send(200); });
  server.onNotFound(handleRoot);
  server.begin();
}

/* ================= SETUP ================= */
void setup() {
  // Motor pins
  pinMode(LF,OUTPUT); pinMode(LB,OUTPUT); pinMode(ENA,OUTPUT);
  pinMode(RF,OUTPUT); pinMode(RB,OUTPUT); pinMode(ENB,OUTPUT);
  stopMotors();

  // HC-SR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  roboEyes.setMood(DEFAULT);

  randomSeed(esp_random());

  WiFi.softAP("pickle");
  dnsServer.start(53, "*", WiFi.softAPIP());
  setupServer();
}

/* ================= LOOP ================= */
void loop() {
  server.handleClient();
  dnsServer.processNextRequest();

  AutoMode currentMode = autoMode; // snapshot volatile once per loop

  /* --- Mode transition handler --- */
  if (currentMode != prevMode) {
    if (currentMode == AUTO_OFF) {
      // Entering sleep: draw closed eyes once, stop RoboEyes updates
      drawSleepEyes();
    } else {
      // Leaving sleep: re-init RoboEyes so animated eyes resume
      roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
      roboEyes.setAutoblinker(ON, 3, 2);
      roboEyes.setIdleMode(ON, 2, 2);
      roboEyes.setMood(DEFAULT);
    }
    prevMode = currentMode;
  }

  /* --- RoboEyes update only when awake --- */
  if (currentMode != AUTO_OFF) {
    roboEyes.update();
  }

  /* --- Movement logic --- */
  static unsigned long lastTick  = 0;
  static unsigned long lastSonar = 0;
  static long dist = 999;

  // Non-blocking sonar read every 80ms
  if (millis() - lastSonar > 80) {
    lastSonar = millis();
    dist = getDistanceCM();
  }

  if (!manualActive && millis() - lastTick > 40) {
    lastTick = millis();

    if (currentMode == AUTO_SOFT) {
      if (dist < OBSTACLE_DISTANCE_CM) {
        avoidObstacle();
      } else if (random(150) == 1) {
        wiggleDance();  // perform the full dance pattern
      }
    }
    else if (currentMode == AUTO_NORMAL) {
      if (dist < OBSTACLE_DISTANCE_CM) {
        avoidObstacle();
      } else if (random(100) == 1) {
        MOTOR(random(9), random(5,50), random(10,100), random(20));
      }
    }
    // AUTO_OFF: no movement, sleep eyes already drawn
  }
}

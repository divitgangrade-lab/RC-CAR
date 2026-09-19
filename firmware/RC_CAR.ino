#include <WiFi.h>
#include <WebServer.h>

// Change these pins to match your wiring. GPIOs must not be connected directly
// to motors; connect them to the motor driver's logic inputs.
constexpr int DRIVE_IN1 = 26;
constexpr int DRIVE_IN2 = 27;
constexpr int DRIVE_PWM = 25;
constexpr int STEER_IN1 = 32;
constexpr int STEER_IN2 = 33;
constexpr int STEER_PWM = 14;

constexpr int DRIVE_CHANNEL = 0;
constexpr int STEER_CHANNEL = 1;
constexpr int PWM_FREQUENCY = 20000;
constexpr int PWM_RESOLUTION = 8;
constexpr unsigned long COMMAND_TIMEOUT_MS = 500;

const char *AP_NAME = "RC-Car";
const char *AP_PASSWORD = "rc-car-1234";

WebServer server(80);
unsigned long lastCommand = 0;

const char PAGE[] PROGMEM = R"HTML(
<!doctype html><html lang="en"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>RC Tacoma controller</title><style>
body{font-family:system-ui;text-align:center;background:#101827;color:#fff;margin:0;padding:20px}button{font-size:2rem;width:100px;height:75px;margin:6px;border:0;border-radius:14px;background:#2563eb;color:#fff;touch-action:none}button:active{background:#16a34a}.stop{background:#dc2626}.row{display:flex;justify-content:center}.hint{color:#aab6ca}#status{min-height:1.5em}
</style><h1>RC Tacoma</h1><p class="hint">Hold a button or use Arrow keys / WASD</p><p id="status">Stopped</p>
<div class="row"><button data-t="100" data-s="0">▲</button></div><div class="row"><button data-t="0" data-s="-100">◀</button><button class="stop" id="stop">■</button><button data-t="0" data-s="100">▶</button></div><div class="row"><button data-t="-100" data-s="0">▼</button></div>
<script>
let timer=0, active=false;
const status=document.querySelector('#status');
function send(t,s){fetch(`/api/control?throttle=${t}&steering=${s}`,{cache:'no-store'}).catch(()=>{});status.textContent=`Throttle ${t}, steering ${s}`}
function start(t,s){active=true;send(t,s);clearInterval(timer);timer=setInterval(()=>send(t,s),150)}
function stop(){active=false;clearInterval(timer);send(0,0);status.textContent='Stopped'}
document.querySelectorAll('button[data-t]').forEach(b=>{let t=+b.dataset.t,s=+b.dataset.s;b.addEventListener('pointerdown',e=>{e.preventDefault();start(t,s)});['pointerup','pointercancel','pointerleave'].forEach(x=>b.addEventListener(x,stop))});
document.querySelector('#stop').addEventListener('click',stop);
const keys={ArrowUp:[100,0],w:[100,0],ArrowDown:[-100,0],s:[-100,0],ArrowLeft:[0,-100],a:[0,-100],ArrowRight:[0,100],d:[0,100]};
document.addEventListener('keydown',e=>{let v=keys[e.key];if(v&&!e.repeat){e.preventDefault();start(...v)}});document.addEventListener('keyup',e=>{if(keys[e.key]){e.preventDefault();stop()}});window.addEventListener('blur',stop);document.addEventListener('visibilitychange',()=>{if(document.hidden)stop()});
</script></html>
)HTML";

int clampCommand(int value) { return constrain(value, -100, 100); }

void output(int in1, int in2, int channel, int value) {
  value = clampCommand(value);
  if (value == 0) {
    digitalWrite(in1, LOW); digitalWrite(in2, LOW); ledcWrite(channel, 0); return;
  }
  digitalWrite(in1, value > 0 ? HIGH : LOW);
  digitalWrite(in2, value > 0 ? LOW : HIGH);
  ledcWrite(channel, map(abs(value), 0, 100, 0, 255));
}

void stopCar() { output(DRIVE_IN1, DRIVE_IN2, DRIVE_CHANNEL, 0); output(STEER_IN1, STEER_IN2, STEER_CHANNEL, 0); }

void handleControl() {
  int throttle = server.hasArg("throttle") ? server.arg("throttle").toInt() : 0;
  int steering = server.hasArg("steering") ? server.arg("steering").toInt() : 0;
  output(DRIVE_IN1, DRIVE_IN2, DRIVE_CHANNEL, throttle);
  output(STEER_IN1, STEER_IN2, STEER_CHANNEL, steering);
  lastCommand = millis();
  server.send(204);
}

void setup() {
  pinMode(DRIVE_IN1, OUTPUT); pinMode(DRIVE_IN2, OUTPUT);
  pinMode(STEER_IN1, OUTPUT); pinMode(STEER_IN2, OUTPUT);
  ledcSetup(DRIVE_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION); ledcAttachPin(DRIVE_PWM, DRIVE_CHANNEL);
  ledcSetup(STEER_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION); ledcAttachPin(STEER_PWM, STEER_CHANNEL);
  stopCar();
  WiFi.mode(WIFI_AP); WiFi.softAP(AP_NAME, AP_PASSWORD);
  server.on("/", HTTP_GET, [](){ server.send_P(200, "text/html", PAGE); });
  server.on("/api/control", HTTP_GET, handleControl);
  server.onNotFound([](){ server.send(404, "text/plain", "Not found"); });
  server.begin();
  lastCommand = millis();
}

void loop() {
  server.handleClient();
  if (millis() - lastCommand > COMMAND_TIMEOUT_MS) stopCar();
}

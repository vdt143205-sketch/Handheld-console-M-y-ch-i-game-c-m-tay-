#include <SoftwareSerial.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// --- GIAO TIẾP VỚI ESP8266 (SoftwareSerial) ---
// Uno Pin 2 (RX) <---nối---> ESP8266 TX
// Uno Pin 3 (TX) <---nối---> ESP8266 RX
SoftwareSerial espSerial(2, 3); 

// --- CẤU HÌNH MÀN HÌNH ILI9341 (Hardware SPI) ---
#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
// Chân cố định SPI Uno: MOSI=11, CLK=13, MISO=12
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// --- CẤU HÌNH NÚT BẤM (Dùng chân Analog làm Digital) ---
#define BTN_UP    A0 
#define BTN_RIGHT A1 
#define BTN_DOWN  A2 
#define BTN_LEFT  A3 
#define BTN_FIRE  A4 

int lastBtn = -1;

void drawUI() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(3); // Xoay ngang
  
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("UNO GAMEPAD");
  
  tft.drawRect(5, 40, 310, 190, ILI9341_WHITE);
  
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 180); tft.print("A:UP B:RIGHT C:DOWN");
  tft.setCursor(20, 210); tft.print("D:LEFT E:FIRE");
}

void drawStatus(String status, uint16_t color) {
  tft.fillRect(10, 60, 300, 30, ILI9341_BLACK);
  tft.setCursor(20, 65);
  tft.setTextColor(color);
  tft.setTextSize(2);
  tft.println(status);
}

void drawScore(String scoreText) {
  tft.fillRect(10, 120, 300, 30, ILI9341_BLACK);
  tft.setCursor(20, 125);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println(scoreText);
}

void setup() {
  Serial.begin(115200); // Debug máy tính
  espSerial.begin(9600); // Giao tiếp ESP8266 (Baudrate thấp để ổn định)

  // 1. Màn hình
  tft.begin();
  drawUI();
  drawStatus("KHOI DONG...", ILI9341_ORANGE);

  // 2. Nút bấm (INPUT_PULLUP)
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_FIRE, INPUT_PULLUP);
}

void loop() {
  // --- 1. NHẬN DỮ LIỆU TỪ ESP (Server -> ESP -> Uno) ---
  if (espSerial.available()) {
    String data = espSerial.readStringUntil('\n');
    data.trim();
    
    // Xử lý các thông báo từ ESP
    if (data == "WIFI_CONNECTED") {
      drawStatus("DA KET NOI WIFI", ILI9341_BLUE);
    } 
    else if (data == "WS_CONNECTED") {
      drawStatus("DA KET NOI SERVER", ILI9341_GREEN);
    }
    else if (data == "WS_DISCONNECTED") {
      drawStatus("MAT KET NOI!", ILI9341_RED);
    }
    // Xử lý điểm số hoặc thông tin game
    else if (data.startsWith("update") || data.startsWith("fight") || data.startsWith("reflex")) {
       if(data.length() > 20) data = data.substring(0, 20); // Cắt ngắn nếu dài quá
       drawScore(data);
    }
    else if (data.indexOf("win") != -1) {
       drawStatus("CHIEN THANG !!!", ILI9341_MAGENTA);
    }
  }

  // --- 2. GỬI LỆNH TỪ NÚT BẤM (Uno -> ESP -> Server) ---
  int currentBtn = -1;
  
  if (digitalRead(BTN_UP) == LOW)    currentBtn = 0; // A
  else if (digitalRead(BTN_RIGHT) == LOW) currentBtn = 1; // B
  else if (digitalRead(BTN_DOWN) == LOW)  currentBtn = 2; // C
  else if (digitalRead(BTN_LEFT) == LOW)  currentBtn = 3; // D
  else if (digitalRead(BTN_FIRE) == LOW)  currentBtn = 99; // E (Fire)

  if (currentBtn != -1) {
    if (currentBtn != lastBtn) {
      // Gửi chuỗi lệnh sang ESP qua SoftwareSerial
      if (currentBtn == 99) espSerial.println("pad:fire");
      else espSerial.println("pad:" + String(currentBtn));
      
      // Hiệu ứng nháy ô trên màn hình
      tft.fillCircle(280, 20, 8, ILI9341_RED);
      delay(50);
      tft.fillCircle(280, 20, 8, ILI9341_BLACK);
    }
    lastBtn = currentBtn;
    delay(50); // Chống rung phím
  } else {
    lastBtn = -1;
  }
}
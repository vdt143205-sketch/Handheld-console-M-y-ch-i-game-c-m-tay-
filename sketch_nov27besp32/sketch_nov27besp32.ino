/*
 * ESP32 GAME CONSOLE - ILI9341 STABLE EDITION
 * FIX: Giảm nhiễu màn hình, tăng tốc SPI, tối ưu vẽ.
 */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <WiFi.h>

// --- 1. CẤU HÌNH PHẦN CỨNG ---
#define TFT_CS   15
#define TFT_DC   2
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_RST  4
#define TFT_MISO 19
#define RXD2     26
#define TXD2     27
#define SPK_PIN  25

Adafruit_ILI9341 tft = Adafruit_ILI9341(
  TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO
);

// --- 2. MÀU SẮC ---
#define C_BLACK   0x0000
#define C_WHITE   0xFFFF
#define C_GREEN   0x07E0
#define C_RED     0xF800
#define C_BLUE    0x001F
#define C_YELLOW  0xFFE0
#define C_CYAN    0x07FF

// --- 3. BIẾN HỆ THỐNG ---
int u, d, l, r, s, e;
int appState = 0;
int gameID = 1;
int score = 0;
bool gameOver = false;

// --- HÀM TỐI ƯU ---
void beep(int f, int d) { tone(SPK_PIN, f, d); }

// Xóa - vẽ mới nhanh hơn & ít flicker
void drawRectOpt(int oldX, int oldY, int newX, int newY, int w, int h, uint16_t color) {
  if (oldX != newX || oldY != newY)
    tft.fillRect(oldX, oldY, w, h, C_BLACK);
  tft.fillRect(newX, newY, w, h, color);
}

// ====================== SETUP ======================
void setup() {
  Serial.begin(115200);
  Serial2.begin(38400, SERIAL_8N1, RXD2, TXD2);

  // FIX NHIỄU 1: Tắt WiFi & Bluetooth
  WiFi.mode(WIFI_OFF);
  WiFi.setSleep(false);
  btStop();

  // Bật TFT ở 40MHz (ổn định nhất cho ILI9341)
  tft.begin(40000000);
  tft.setRotation(0);

  drawMenu();
  beep(600, 100);
}

// ======================= LOOP ======================
void loop() {
  if (Serial2.available()) {
    String str = Serial2.readStringUntil('\n');
    sscanf(str.c_str(), "%d,%d,%d,%d,%d,%d", &u, &d, &l, &r, &s, &e);
  }

  if (appState == 0) loopMenu();
  else if (appState == 1) loopPong();
  else if (appState == 2) loopSnake();
  else if (appState == 3) loopSpace();

  delay(10); // FPS ổn định, giảm nhiễu update
}

// ======================================================
// MENU
// ======================================================
void drawMenu() {
  tft.fillScreen(C_BLACK);
  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(C_CYAN);
  tft.setCursor(80, 40);
  tft.print("GAME MENU");

  tft.setFont(); tft.setTextSize(2);

  tft.setTextColor(gameID==1 ? C_YELLOW : C_WHITE);
  tft.setCursor(100, 90);
  tft.print(gameID==1 ? "> PONG" : "  PONG");

  tft.setTextColor(gameID==2 ? C_YELLOW : C_WHITE);
  tft.setCursor(100, 120);
  tft.print(gameID==2 ? "> SNAKE" : "  SNAKE");

  tft.setTextColor(gameID==3 ? C_YELLOW : C_WHITE);
  tft.setCursor(100, 150);
  tft.print(gameID==3 ? "> SPACE" : "  SPACE");

  tft.setTextSize(1); tft.setTextColor(C_WHITE);
  tft.setCursor(60, 210);
  tft.print("A:UP  C:DOWN  E:SELECT");
}

void loopMenu() {
  static int lastID = -1;

  if (u) { gameID--; delay(120); beep(600,30); }
  if (d) { gameID++; delay(120); beep(600,30); }
  if (gameID < 1) gameID = 3;
  if (gameID > 3) gameID = 1;

  if (gameID != lastID) {
    drawMenu();
    lastID = gameID;
  }

  if (s) {
    beep(1000,100);
    if (gameID==1) { appState=1; initPong(); }
    if (gameID==2) { appState=2; initSnake(); }
    if (gameID==3) { appState=3; initSpace(); }
    delay(250);
  }
}

// ======================================================
// GAME 1: PONG
// ======================================================
int padY, ballX, ballY, bVX, bVY;

void initPong() {
  tft.fillScreen(C_BLACK);
  padY = 100;
  ballX = 160; 
  ballY = 120;
  bVX = 4; 
  bVY = 3;
  score = 0;
  gameOver = false;
}

void loopPong() {
  if (e) { appState=0; drawMenu(); delay(300); return; }
  if (gameOver) { showGameOver(); return; }

  int oldPY = padY;
  int oldBX = ballX, oldBY = ballY;

  if (u) padY -= 6;
  if (d) padY += 6;
  if (padY < 0) padY = 0;
  if (padY > 200) padY = 200;

  // Bóng
  ballX += bVX;
  ballY += bVY;

  if (ballY < 5 || ballY > 235) bVY = -bVY;
  if (ballX > 315) bVX = -bVX;

  if (ballX < 18 && ballY > padY && ballY < padY+40) {
    bVX = abs(bVX)+1;
    score++;
    beep(500,20);
  }
  if (ballX < 0) gameOver = true;

  // Vẽ tối ưu
  drawRectOpt(10, oldPY, 10, padY, 5, 40, C_GREEN);
  tft.fillCircle(oldBX, oldBY, 4, C_BLACK);
  tft.fillCircle(ballX, ballY, 4, C_WHITE);

  // Score (giảm flicker)
  tft.fillRect(0,0,100,18,C_BLACK);
  tft.setCursor(4,4);
  tft.setTextColor(C_WHITE);
  tft.print("Score:");
  tft.print(score);
}

// ======================================================
// GAME 2: SNAKE
// ======================================================
int snkX[100], snkY[100], snkLen, dir, fdX, fdY;

void initSnake() {
  tft.fillScreen(C_BLACK);
  tft.drawRect(0,20,320,220,C_BLUE);

  snkLen=3;
  snkX[0]=16; snkY[0]=12;
  dir=1;
  score=0;
  gameOver=false;

  fdX=random(2,30);
  fdY=random(3,22);
}

void loopSnake() {
  if (e) { appState=0; drawMenu(); delay(300); return; }
  if (gameOver) { showGameOver(); return; }

  if (u && dir!=2) dir=0;
  if (d && dir!=0) dir=2;
  if (l && dir!=1) dir=3;
  if (r && dir!=3) dir=1;

  static unsigned long t=0;
  if (millis() - t > 90) {
    t = millis();

    tft.fillRect(snkX[snkLen-1]*10, snkY[snkLen-1]*10, 10, 10, C_BLACK);

    for(int i=snkLen-1;i>0;i--) {
      snkX[i]=snkX[i-1];
      snkY[i]=snkY[i-1];
    }

    if(dir==0) snkY[0]--;
    if(dir==1) snkX[0]++;
    if(dir==2) snkY[0]++;
    if(dir==3) snkX[0]--;

    if(snkX[0]<1) snkX[0]=30;
    if(snkX[0]>30) snkX[0]=1;
    if(snkY[0]<3) snkY[0]=22;
    if(snkY[0]>22) snkY[0]=3;

    if(snkX[0]==fdX && snkY[0]==fdY) {
      snkLen++;
      score+=10;
      fdX=random(2,30);
      fdY=random(3,22);
      beep(1000,50);
    }

    for(int i=1;i<snkLen;i++)
      if(snkX[0]==snkX[i] && snkY[0]==snkY[i]) gameOver=true;

    tft.fillRect(snkX[0]*10, snkY[0]*10, 10, 10, C_GREEN);
    tft.fillRect(fdX*10, fdY*10, 10, 10, C_RED);

    tft.fillRect(0,0,100,18,C_BLACK);
    tft.setCursor(4,4);
    tft.setTextColor(C_WHITE);
    tft.print("Score:");
    tft.print(score);
  }
}

// ======================================================
// GAME 3: SPACE SHOOTER
// ======================================================
int pX=20, pY=100;
struct Bullet { int x,y; bool act; } bul[5];
struct Enemy { int x,y; bool act; } ene[5];

void initSpace() {
  tft.fillScreen(C_BLACK);
  pX=20; pY=100;
  score=0; gameOver=false;

  for (int i=0;i<5;i++) {
    bul[i].act=false;
    ene[i] = {330, random(20,220), true};
  }
}

void loopSpace() {
  if (e) { appState=0; drawMenu(); delay(300); return; }
  if (gameOver) { showGameOver(); return; }

  int oldPX=pX, oldPY=pY;

  if(u) pY-=5;
  if(d) pY+=5;
  if(l) pX-=5;
  if(r) pX+=5;

  if(pY<20) pY=20;
  if(pY>220) pY=220;

  drawRectOpt(oldPX, oldPY, pX, pY, 15, 10, C_CYAN);

  static int cd=0; 
  if(cd>0) cd--;

  if(s && cd==0) {
    for(int i=0;i<5;i++)
      if(!bul[i].act) {
        bul[i]={pX+15,pY+4,true};
        cd=8;
        beep(800,20);
        break;
      }
  }

  for(int i=0;i<5;i++) {
    // Bullet
    if(bul[i].act) {
      tft.fillRect(bul[i].x, bul[i].y, 6, 2, C_BLACK);
      bul[i].x+=10;
      if(bul[i].x>320) bul[i].act=false;
      else tft.fillRect(bul[i].x, bul[i].y, 6, 2, C_YELLOW);
    }

    // Enemy
    tft.fillCircle(ene[i].x, ene[i].y, 6, C_BLACK);
    ene[i].x -= 3;
    if(ene[i].x < 0) gameOver=true;

    for(int k=0;k<5;k++) {
      if(bul[k].act && abs(bul[k].x-ene[i].x)<10 && abs(bul[k].y-ene[i].y)<10) {
        bul[k].act=false;
        ene[i] = {330, random(20,220), true};
        score+=10;
        beep(200,50);
      }
    }

    tft.fillCircle(ene[i].x, ene[i].y, 6, C_RED);
  }

  tft.fillRect(0,0,100,18,C_BLACK);
  tft.setCursor(4,4);
  tft.setTextColor(C_WHITE);
  tft.print("Score:");
  tft.print(score);
}

// ======================================================
// GAME OVER
// ======================================================
void showGameOver() {
  tft.setCursor(100,100);
  tft.setTextColor(C_RED);
  tft.setTextSize(3);
  
  tft.print("GAME OVER");

  tft.setCursor(120,140);
  tft.setTextColor(C_WHITE);
  tft.setTextSize(2);
  tft.print(score);

  if(s) {
    if(appState==1) initPong();
    if(appState==2) initSnake();
    if(appState==3) initSpace();
    delay(300);
  }
}

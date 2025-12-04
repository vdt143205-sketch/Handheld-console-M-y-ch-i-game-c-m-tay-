/*
 * ARDUINO BUTTON CONTROLLER - 2 PLAYER EDITION
 * Mapping: A=Up, B=Right, C=Down, D=Left, E=Shoot, F=Exit
 * Tốc độ: 38400
 */
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 9); // RX, TX

// Định nghĩa chân nút bấm trên Shield V1.A 
#define BTN_A 2 // LÊN
#define BTN_B 3 // PHẢI
#define BTN_C 4 // XUỐNG
#define BTN_D 5 // TRÁI
#define BTN_E 6 // BẮN
#define BTN_F 7 // THOÁT

void setup() {
  mySerial.begin(38400);
  // Cấu hình toàn bộ nút là Input Pullup
  for(int i=2; i<=7; i++) pinMode(i, INPUT_PULLUP);
}

void loop() {
  // Đọc trạng thái nút (Nhấn = 1, Nhả = 0)
  int u = !digitalRead(BTN_A); // A -> Up
  int r = !digitalRead(BTN_B); // B -> Right
  int d = !digitalRead(BTN_C); // C -> Down
  int l = !digitalRead(BTN_D); // D -> Left
  int s = !digitalRead(BTN_E); // E -> Shoot/Select
  int x = !digitalRead(BTN_F); // F -> Exit

  // Gửi gói tin: <Up,Down,Left,Right,Shoot,Exit>
  // Ví dụ: 1,0,0,0,1,0 (Đang bấm Lên và Bắn)
  mySerial.print(u); mySerial.print(",");
  mySerial.print(d); mySerial.print(",");
  mySerial.print(l); mySerial.print(",");
  mySerial.print(r); mySerial.print(",");
  mySerial.print(s); mySerial.print(",");
  mySerial.println(x);

  delay(30); // Tốc độ phản hồi 33Hz
}
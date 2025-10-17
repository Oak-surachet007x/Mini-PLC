#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#define ledRed 2 // สถานะมอเตอร์ หยุดทำงาน
#define ledYellow 3 //สถานะตรวจพบวัตถุ
#define ledGreen 4 //สถานะมอเตอร์ ทำงาน
#define relayPin 11 // ควบคุมการจ่ายไฟ Mosfets IRLZ44N

// กำหนด Pin ที่เชื่อมต่อกับเซ็นเซอร์
const int TRIG_PIN = 6;
const int ECHO_PIN = 7;

// --- ตัวแปรสำคัญที่ต้องตั้งค่า (Calibration) ---
// *** ให้วัดระยะทางจริงจากเซ็นเซอร์ถึงสายพาน (หน่วยเป็น cm) แล้วใส่ค่าที่นี่ ***
// *** แนะนำให้เปิด Serial Monitor ดูก่อนเมื่อไม่มีวัตถุ เพื่อหาค่าที่แม่นยำที่สุด ***
const float distanceToConveyor = 9.2; // <<<<<<<< แก้ไขค่านี้! (ตัวอย่างคือ 25 cm)

// กำหนดค่าเผื่อ (Threshold) เพื่อกรองสัญญาณรบกวน (หน่วยเป็น cm)
// ถ้าวัตถุเตี้ยกว่าค่านี้ จะไม่ถูกนับ
float objectHeightThreshold = 1.0; 

// ตัวแปรสำหรับคำนวณ
long duration;
float distance;
float objectHeight;

void setup()
{
  Serial.begin(9600);
  //LED
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);

  //RelayModule
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  digitalWrite(ledGreen, HIGH);


  //LCD i2c                     // initialize the lcd 
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
 

  // กำหนดโหมดของ Pin
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
}


void loop()
{

  // สร้างสัญญาณ Pulse เพื่อสั่งให้เซ็นเซอร์ทำงาน
  // 1. ปิด Trig Pin ก่อนเพื่อความแน่นอน
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // 2. ส่งสัญญาณ Pulse สั้นๆ (10 ไมโครวินาที) ไปที่ Trig Pin
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 3. อ่านระยะเวลาที่ Echo Pin เป็น HIGH (คือระยะเวลาที่คลื่นเสียงเดินทางไป-กลับ)
  duration = pulseIn(ECHO_PIN, HIGH);

  // คำนวณระยะทาง (หน่วยเป็น cm)
  // สูตร: ระยะทาง = (ระยะเวลา * ความเร็วเสียง) / 2 (เพราะคลื่นเดินทางไปและกลับ)
  // ความเร็วเสียงประมาณ 343 m/s หรือ 0.0343 cm/µs
  distance = duration * 0.0343 / 2;
  Serial.println(distance);

  // ตรวจสอบว่ามีวัตถุเคลื่อนที่ผ่านหรือไม่
  // โดยเช็คว่าระยะที่วัดได้ สั้นกว่าระยะอ้างอิงถึงสายพาน (และหักลบค่าเผื่อเล็กน้อย) หรือไม่
  if (distance < (distanceToConveyor - objectHeightThreshold)) {
    
    // คำนวณความสูงของวัตถุ
    objectHeight = distanceToConveyor - distance;

    // แสดงผลความสูงของวัตถุที่ตรวจพบ
    digitalWrite(relayPin, HIGH); // สั่งหยุดจ่ายไฟ
    digitalWrite(ledYellow, HIGH);
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledRed, HIGH);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Obj Height :");
    lcd.setCursor(1,1);
    lcd.print(objectHeight);
    Serial.print("ตรวจพบวัตถุ! ความสูง: ");
    Serial.print(objectHeight);
    Serial.println(" cm");
    delay(5000); //หน่วงเวลาให้แสดงค่า
    digitalWrite(relayPin, LOW); // สั่งเปิดจ่ายไฟ
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledGreen, HIGH);
    digitalWrite(ledRed, LOW);
    
  } else {
    // ถ้าไม่มีวัตถุ อาจจะให้แสดง "---" หรือแสดงระยะทางปัจจุบันก็ได้
    Serial.println("--- ไม่มีวัตถุ ---");
    digitalWrite(relayPin, LOW); // สั่งเปิดจ่ายไฟ
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledGreen, HIGH);
    digitalWrite(ledRed, LOW);
  }

  // หน่วงเวลาเล็กน้อยก่อนทำการวัดครั้งต่อไป เพื่อให้ระบบเสถียร
  delay(100);

}

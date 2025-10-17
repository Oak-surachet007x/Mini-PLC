# Mini-PLC
ระบบสายพานคัดแยก ด้วยการคัดแยกความสูงของวัตถุ
## introduction :memo::speech_balloon:
- Arduino uno R4 ในการเขียนโปรแกรมและรับค่าข้อมูลจากเซนเซอร์
- Ultrasonic sensor ทำหน้าที่วัดความสูงของวัตถุและตรวจจับวัตถุ เมื่อมีวัตถุเข้ามาใกล้ตัวเซนเตอร์
- LED แสดงสถานะการทำงานของระบบสายพาน
  - LED Green = สถานะมอเตอร์ทำงาน
  - LED Yellow = สถานะตรวจพบวัตถุ
  - LED Red = สถานะมอเตอร์หยุดทำงาน
- LCD I2C แสดงค่าความสูงของวัตถุที่วัดได้
- Relay Module ทำหน้าที่จ่ายไฟจากแรงดันไฟฟ้าภายนอก และควบคุมการจ่ายไฟของมอเตอร์
- Mosfets IRLZ44N เป็นสวิตซ์ควบคุมความเร็วของมอเตอร์
- potentiometer ทำหน้าที่ควบคุมความเร็วรอบของมอเตอร์

## Circult Diagram
![Circult](https://github.com/Oak-surachet007x/Mini-PLC/blob/main/circuit%20diagram.png)


***for Mini Project 05366036 3(2-2) Microcontroller &amp; Interfacing.  ( Industry Physics,   School of Science KMITL )***


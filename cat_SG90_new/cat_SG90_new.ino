/*
В функции smoothMove из предыдущего примера за резкость отвечает параметр speedDelay. 
Чем он меньше, тем быстрее движение. 
Однако, для максимально резкого рывка (на который только способен мотор), 
лучше использовать стандартный метод writeMicroseconds без всяких циклов.
Вот код, реализующий вашу схему: плавно на 10°, плавно в 0°, плавно на 20° и резкий возврат.

Математика расчета:
Весь диапазон (180°): 2500-500 = 2000 мкс.
Сколько мкс в 1 градусе:  2000/180 = 11.11 мкс.
Для 10 градусов: 10х11.11  = 111 мкс.

Следовательно, чтобы повернуться на 10 градусов от начальной точки (500), 
нужно подать: 500+111 =   611 мкс.
*/

#include <Servo.h>
Servo myservo;

const int MIN_MS = 500;
const int MAX_MS = 2500;
int currentPos   = 500; 
int buzzerPin    = D8;      // Пин для наушника

void setup() {
  myservo.attach(D4, MIN_MS, MAX_MS);
  myservo.writeMicroseconds(currentPos);
  delay(1000); // Даем время встать в начало
}

void smoothMove(int targetAngle, int speedDelay) {
  int targetUs = map(targetAngle, 0, 180, MIN_MS, MAX_MS);
  while (currentPos != targetUs) {
    if (currentPos < targetUs) currentPos++;
    else currentPos--;
    myservo.writeMicroseconds(currentPos);
    delay(speedDelay);
  }
}

// Функция для мгновенного рывка (без плавности)
void instantMove(int targetAngle) {
  currentPos = map(targetAngle, 0, 180, MIN_MS, MAX_MS);
  myservo.writeMicroseconds(currentPos);
  // Небольшая пауза обязательна, чтобы мотор физически успел долететь
  delay(200); 
}

void mouseChirp() {
  // Серия коротких высокочастотных писков
  for (int i = 0; i < 3; i++) {
    tone(buzzerPin, (unsigned int)random(4000, 6000), 50);
    //tone(buzzerPin, random(4000, 6000), 50); // Случайный писк
    delay(70);
    noTone(buzzerPin);
    delay(50);
  }
}

void manualChirp(int pin, int freq, int duration) {
  long period = 1000000L / freq;
  long startTime = millis();
  while (millis() - startTime < duration) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(period / 2);
    digitalWrite(pin, LOW);
    delayMicroseconds(period / 2);
  }
}

// исправленная стрекоза
void cricketSound() {
  for (int i = 0; i < 10; i++) { // 10 циклов стрекотания
    // Быстрый "вжик" вверх по частоте
    for (unsigned int f = 3000; f < 5000; f += 200) {
      tone(buzzerPin, f, 15); 
      delay(10);
    }
    noTone(buzzerPin);
    delay(30); // Короткая пауза между стрекотанием
  }
}

// пробуйте «птичье чириканье» (свип вниз)
void birdChirp() {
  for (unsigned int f = 7000; f > 4000; f -= 100) {
    tone(buzzerPin, f, 10);
    delay(5);
  }
  noTone(buzzerPin);
}


void loop() {
  birdChirp();
  delay(1000);
  cricketSound();
  delay(1000);

  // 1. Плавно на 10 градусов
  smoothMove(10, 15); 
  delay(1000);
  manualChirp(buzzerPin, 5000, 50);

  // 2. Плавно обратно в 0
  smoothMove(0, 15);
  delay(1000);
  manualChirp(buzzerPin, 5000, 50);

  // 3. Плавно на 30 градусов
  smoothMove(30, 15);
  delay(1000);
  manualChirp(buzzerPin, 5000, 50);
  // 4. БЫСТРЫЙ, НО БЕРЕЖНЫЙ ВОЗВРАТ в 0
  // Вместо мгновенного рывка ставим задержку 2-3 мс.
  // Это уберет "ударный" шум и спасет пластиковые шестерни.
  smoothMove(0, 3); 

  mouseChirp(); // Пищим перед тем как убежать
  //smoothMove(60, 5); 

// 3. Плавно на 30 градусов
  smoothMove(60, 15);
  delay(1000);



  // 4. РЕЗКИЙ РЫВОК в 0 (эффект "сбежавшей мыши")
  instantMove(0);
  
  delay(3000); // Пауза перед новым циклом
}

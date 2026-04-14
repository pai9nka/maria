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
  // — это инициализирует генератор случайных чисел шумом со свободного пина.
  randomSeed(analogRead(0)); 

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


/*
Да! Все = ОК!  Хотелось внести в звуки элемент разнообразия 
типа random по длительности и частотам
Это отличная идея! Random сделает игрушку «живой», 
и кот не сможет предугадать её поведение. 
Если звуки будут каждый раз немного разными, 
это не даст ему быстро привыкнуть к алгоритму.
Чтобы это работало красиво, мы создадим функцию playRandomSound(), 
которая будет выбирать один из типов звуков и каждый раз генерировать 
его с новыми случайными параметрами.
*/

void playRandomSound() {
  int soundType = random(0, 3); // Выбираем тип звука: 0, 1 или 2

  if (soundType == 0) {
    // Случайный мышиный писк
    int count = random(2, 5); // От 2 до 4 писков
    for (int i = 0; i < count; i++) {
      unsigned int freq = random(5000, 9000); // Высокий писк
      tone(buzzerPin, freq, random(30, 80)); 
      delay(random(50, 150));
    }
  } 
  else if (soundType == 1) {
    // Случайная птичка (свип вниз)
    unsigned int startF = random(6000, 9000);
    unsigned int endF = startF - random(2000, 3000);
    for (unsigned int f = startF; f > endF; f -= 150) {
      tone(buzzerPin, f, 10);
      delay(7);
    }
  } 
  else {
    // Случайное стрекотание
    int clicks = random(5, 15);
    for (int i = 0; i < clicks; i++) {
      tone(buzzerPin, (unsigned int)random(3000, 5000), 10);
      delay(random(20, 60));
    }
  }
  noTone(buzzerPin);
}


void loop() {
  
  int clicks =2;
  int i = 0;

  for (int i = 0; i < clicks; i++) {
     playRandomSound();
     delay(random(500, 1600));
    }



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
  smoothMove(170, 15);
  delay(1000);



  // 4. РЕЗКИЙ РЫВОК в 0 (эффект "сбежавшей мыши")
  instantMove(0);
  
  delay(3000); // Пауза перед новым циклом
}

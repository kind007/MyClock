#include <OneWire.h>
#include <IRLremote.h> // https://github.com/NicoHood/IRLremote
#include <ci2c.h> // https://github.com/SMFSW/cI2C
#include <FAB_LED.h> // https://github.com/sonyhome/FAB_LED

CNec IRLremote;
I2C_SLAVE DS3231;
OneWire DS18B20(11);
ws2812b<B,4> LED;

#define PORT595 PORTB
#define PORT168 PORTD
#define A0_168 7
#define A1_168 6
#define A2_168 5
#define DATA 0
#define SH_CP 1
#define ST_CP 2
#define BEEPER_PIN 4

#define START 1
#define BLINK_ON 2
#define STOP 3
// ИК пульт
#define pinIR 2
#define IR_1 0x45
#define IR_2 0x46
#define IR_3 0x47
#define IR_4 0x44
#define IR_5 0x40
#define IR_6 0x43 
#define IR_7 0x07
#define IR_8 0x15
#define IR_9 0x09
#define IR_0 0x19
#define IR_STAR 0x16
#define IR_GRID 0x0D
#define IR_UP 0x18
#define IR_DOWN 0x52
#define IR_LEFT 0x08
#define IR_RIGHT 0x5A
#define IR_OK 0x1C
#define STAR 10
#define GRID 11
#define ARROW_UP 12
#define ARROW_DOWN 13
#define ARROW_LEFT 14
#define ARROW_RIGHT 15
#define OK 16


// menu
#define HOUR 1
#define MIN 2
#define SEC 3
#define YEAR 4
#define MONTH 5
#define DATE 6
#define TIMER 7
#define UP 1
#define DOWN 0
#define MENU_WAIT 120  // время, через которое произвести выход из настроек, если пользователь не активен * 0,5сек

#define S_TIME 1
#define S_DATE 2
#define S_TEMP 3
#define S_TIMER 4
#define S_BIGDAY 5

// БЕГУЩАЯ СТРОКА
// Ссылки на слова, отображаемые в знаменательные дни в PROGMEM
#define NY 1
#define D31 2

uint8_t NewSymbol;
uint8_t RunText;
uint8_t BufferPos;
uint8_t EndLine;
uint8_t RunTextLoop;

const uint8_t POINTER[] PROGMEM = {0,0,3}; // Указатель на начало в TEXT_INDEX
const uint8_t TEXT_INDEX[] PROGMEM = {0,14,254,12,27,14,254}; // Указатель на слова в TEXT (254 символ остановки перебора слов)
const uint8_t TEXT[] PROGMEM = {28, 27, 20, 17, 29, 13, 15, 24, 44, 18, 25, 12, 30, 255, // 0 ПОЗДРАВЛЯЕМ C (255 символ конца слова/строки)
                                26, 27, 15, 40, 25, 12, 16, 27, 17, 27, 25, 52, 255, //14 НОВЫМ ГОДОМ 
                                26, 13, 30, 31, 32, 28, 13, 43, 38, 21, 25, 255}; // 27 НАСТУПАЮЩИМ
//
const uint8_t scrPos[] PROGMEM = {9,6,3,8,5,2};
const uint8_t dow1[] PROGMEM = {28,15,30,36,28,30,15};
const uint8_t dow2[] PROGMEM = {26,31,29,31,31,14,30};
const uint8_t dowArray[] PROGMEM = {0,3,2,5,0,3,5,1,4,6,2,4};
const uint8_t daysArray [] PROGMEM = {31,28,31,30,31,30,31,31,30,31,30,31};

const uint8_t font[][8] PROGMEM = { // ШРИФТ
  {193,156,152,144,132,140,193,255},  // 0
  {243,227,211,243,243,243,192,255},  // 1
  {195,153,249,227,207,159,129,255},  // 2
  {195,153,249,227,249,153,195,255},  // 3
  {241,225,201,153,128,249,249,255},  // 4
  {129,159,131,249,249,153,195,255},  // 5
  {227,207,159,131,153,153,195,255},  // 6
  {129,249,249,243,231,231,231,255},  // 7
  {195,153,153,195,153,153,195,255},  // 8
  {195,153,153,193,249,243,199,255},  // 9
  {255,231,231,255,255,231,231,255},  // :    10
  {255,255,255,255,255,231,231,255},  // .    11
  {255,255,255,255,255,255,255,255},  // Пробел   12
  {231,195,153,153,129,153,153,255},  // А    13
  {131,159,159,131,153,153,131,255},  // Б    14
  {131,153,153,131,153,153,131,255},  // В    15
  {129,129,159,159,159,159,159,255},  // Г    16
  {241,225,193,201,201,201,128,156},  // Д    17
  {129,159,159,131,159,159,129,255},  // Е    18
  {182,148,193,227,193,148,182,255},  // Ж    19
  {195,153,249,227,249,153,195,255},  // З    20
  {153,153,145,129,137,153,153,255},  // И    21
  {247,156,152,144,132,140,156,255},  // Й    22
  {153,147,135,143,135,147,153,255},  // К    23
  {241,225,201,153,153,153,153,255},  // Л    24
  {156,136,128,128,148,156,156,255},  // М    25
  {153,153,153,129,153,153,153,255},  // Н    26
  {195,153,153,153,153,153,195,255},  // О    27
  {129,153,153,153,153,153,153,255},  // П    28
  {131,153,153,153,131,159,159,255},  // Р    29
  {195,153,159,159,159,153,195,255},  // С    30
  {129,231,231,231,231,231,231,255},  // Т    31
  {153,153,153,193,249,153,195,255},  // У    32
  {195,165,165,165,195,231,231,255},  // Ф    33
  {153,153,195,231,195,153,153,255},  // Х    34
  {153,153,153,153,153,153,128,252},  // Ц    35
  {153,153,153,193,249,249,249,255},  // Ч    36
  {36,36,36,36,36,36,0,255},          // Ш    37
  {36,36,36,36,36,36,0,252},          // Щ    38
  {143,207,207,193,204,204,193,255},  // Ъ    39
  {156,156,156,132,152,152,132,255},  // Ы    40
  {159,159,159,131,153,153,131,255},  // Ь    41
  {195,185,252,192,252,185,195,255},  // Э    42
  {99,73,73,9,73,73,99,255},          // Ю    43
  {193,153,153,153,193,201,153,255},  // Я    44
  {255,227,201,201,227,255,255,255},  // ГРАДУС   45
  {207,207,131,207,207,203,231,255},  // t    46
  {255,231,231,129,231,231,255,255},  // +    47
  {255,255,255,129,129,255,255,255},  // -    48
  {147,1,1,1,131,199,239,255},        // сердечко   49
  {195,153,249,243,231,255,231,255},  // ?    50
  {207,231,243,249,243,231,207,255},  // >    51
  {231,195,195,231,231,255,231,255}   // !    52
  };

const uint8_t numPixels = 7; 
grb pixels[numPixels] = {};

uint8_t pos, DG, r, c, j, fn;

uint8_t Screen [13][8]; // Экранный массив 12 сегментов 8 на 8 и один буфер
uint8_t CurScreen[12]; // Текущие символы отображаемые на экране
uint8_t NewScreen[12]; // Символы, готовящиеся к отображению
uint8_t ChangePix[12]; // Символы изменяемые анимацией "сдвиг вниз"
uint8_t Blink[12]; // "Моргание" символов на экране при установке времени/даты
uint8_t CurrTime[8]; // байты DS3231

int16_t Termo; // Измеренная температура
uint8_t TimerNumber[3]; // цифры установленного таймера для индикации
uint8_t TimerPos; // Позиция устанавливаемой цифры
uint16_t TimerValue; // Установленное время таймера
uint8_t BEEPER; // Сработал таймер
uint8_t SOUND; // Состояние вывода пищалки
uint8_t PIK; // "пикаем" при нажатии кнопок на ИК пульте
uint8_t IR_BLOCK; // Блокировка принятия команд с ИК пульта во время переходных процессов отображения инфо на экране

uint8_t IsMenu; // Активно меню изменения даты/времени 

uint8_t bigDay; // День совпал со знаменательным, выводим сообщения с соответствующим номером
uint8_t BD_COUNT; // Счетчик для отображения надписи о bigDay, показываем один за каждые 5 отображения времени

uint8_t tictak = 7; // Движение огонька по овалу, имитация стрелки

uint32_t Timer; // Тики по 100 миллисекунд
uint32_t TimerSec; // Секундный таймер
uint32_t TimerLED; // Таймер на светодиоды для радуги
uint16_t TimerFunc; // Время исполнения процедур в мкс, для расчета задержки динамической индикации
uint8_t rotator; 
uint8_t screenPage = 0; // Номер отображаемой страницы на экране (Время, дата и т.д.)
uint8_t screenTime; // Время отображение страницы в секундах

int8_t MenuTimer; // Время ожидания ввода от пользователя новых даты/времени
uint8_t MenuNumber; // Изменяемое число

uint16_t screenDelay = 1900; // Задержка отображения строки при динамической индикации мкс

void ScreenRefresh(uint8_t n){ // Загружаем символ в экранный массив
 for (c = 0; c < 8; c++) {
    Screen[n][c] = pgm_read_byte(&(font[CurScreen[n]][c]));
 }
}

void ScreenRefreshFull(){ // Загружаем все символы в экранный массив
  for (fn = 0; fn <12; fn++){
    ScreenRefresh(fn);
  }
}

void IntroStep() { // Выводим приветственные надписи
  switch (screenPage) {
    case 1: // Привет     28, 29, 21, 15, 18, 31, 
      CurScreen[0] = 12;CurScreen[1] = 12;CurScreen[2] = 12;CurScreen[3] = 28;CurScreen[4] = 29;CurScreen[5] = 21;CurScreen[6] = 15;CurScreen[7] = 18;CurScreen[8] = 31;CurScreen[9] = 12;CurScreen[10] = 12;CurScreen[11] = 12;
      TimerFunc = 2400;
      for (fn = 0; fn < 7; fn++){
        pixels[fn].r = 255;
        pixels[fn].g = 255;
        pixels[fn].b = 0;
      }
        LED.sendPixels(numPixels, pixels);
      break;
    case 2: // очень
      CurScreen[0] = 12;CurScreen[1] = 27;CurScreen[2] = 36;CurScreen[3] = 18;CurScreen[4] = 26;CurScreen[5] = 41;CurScreen[6] = 12;CurScreen[7] = 12;CurScreen[8] = 12;CurScreen[9] = 12;CurScreen[10] = 12;CurScreen[11] = 12;
      TimerFunc = 700;
      break;
    case 3: // рады
      CurScreen[0] = 12;CurScreen[1] = 27;CurScreen[2] = 36;CurScreen[3] = 18;CurScreen[4] = 26;CurScreen[5] = 41;CurScreen[6] = 12;CurScreen[7] = 29;CurScreen[8] = 13;CurScreen[9] = 17;CurScreen[10] = 40;CurScreen[11] = 12;
      TimerFunc = 1500;
      break; 
    case 4: // вас
      CurScreen[0] = 12;CurScreen[1] = 15;CurScreen[2] = 13;CurScreen[3] = 30;CurScreen[4] = 12;CurScreen[5] = 12;CurScreen[6] = 12;CurScreen[7] = 12;CurScreen[8] = 12;CurScreen[9] = 12;CurScreen[10] = 12;CurScreen[11] = 12;
      TimerFunc = 700;
      break;
    case 5: // видеть
      CurScreen[0] = 12;CurScreen[1] = 15;CurScreen[2] = 13;CurScreen[3] = 30;CurScreen[4] = 12;CurScreen[5] = 15;CurScreen[6] = 21;CurScreen[7] = 17;CurScreen[8] = 18;CurScreen[9] = 31;CurScreen[10] = 41;CurScreen[11] = 12;
      TimerFunc = 1500;
      break;
    case 6: // но
      CurScreen[0] = 12;CurScreen[1] = 26;CurScreen[2] = 27;CurScreen[3] = 12;CurScreen[4] = 12;CurScreen[5] = 12;CurScreen[6] = 12;CurScreen[7] = 12;CurScreen[8] = 12;CurScreen[9] = 12;CurScreen[10] = 12;CurScreen[11] = 12;
      TimerFunc = 700;
      break;
    case 7: // нам
      CurScreen[0] = 12;CurScreen[1] = 26;CurScreen[2] = 27;CurScreen[3] = 12;CurScreen[4] = 26;CurScreen[5] = 13;CurScreen[6] = 25;CurScreen[7] = 12;CurScreen[8] = 12;CurScreen[9] = 12;CurScreen[10] = 12;CurScreen[11] = 12;
      TimerFunc = 700;
      break;
    case 8: // уже
      CurScreen[0] = 12;CurScreen[1] = 26;CurScreen[2] = 27;CurScreen[3] = 12;CurScreen[4] = 26;CurScreen[5] = 13;CurScreen[6] = 25;CurScreen[7] = 12;CurScreen[8] = 32;CurScreen[9] = 19;CurScreen[10] = 18;CurScreen[11] = 12;
      TimerFunc = 1500;
      break;
    case 9: // пора
      CurScreen[0] = 12;CurScreen[1] = 28;CurScreen[2] = 27;CurScreen[3] = 29;CurScreen[4] = 13;CurScreen[5] = 12;CurScreen[6] = 12;CurScreen[7] = 12;CurScreen[8] = 12;CurScreen[9] = 12;CurScreen[10] = 12;CurScreen[11] = 12;
      TimerFunc = 700;
      break;
    case 10: // идти
      CurScreen[0] = 12;CurScreen[1] = 28;CurScreen[2] = 27;CurScreen[3] = 29;CurScreen[4] = 13;CurScreen[5] = 12;CurScreen[6] = 21;CurScreen[7] = 17;CurScreen[8] = 31;CurScreen[9] = 21;CurScreen[10] = 12;CurScreen[11] = 12;
      TimerFunc = 2000;
      break;  
  }
}

void StartRunText() { // Запускаем бегущую строку
  BufferPos = pgm_read_byte(&POINTER[bigDay]);
  RunText = pgm_read_byte(&TEXT_INDEX[BufferPos++]);
  RunTextLoop = 1;
}

uint8_t GetNextSymbol() {// Берем новую букву для бегущей строки
  uint8_t Temp;
  if (EndLine) { // кончилась строка, выдаем пробелы
    EndLine--;
    if (!EndLine) {
      RunTextLoop = 0;
      screenPage = 0;
      BD_COUNT = 0;
      NextScreen();
    }
    return 12;
  }
  Temp = pgm_read_byte(&TEXT[RunText]); // берем следующий символ
  if (Temp == 255) { // если конец слова смотрим есть ли дальше
    Temp = 12;
    if (pgm_read_byte(&TEXT_INDEX[BufferPos]) != 254){ // есть, крутим
      RunText = pgm_read_byte(&TEXT_INDEX[BufferPos++]);
    } else { // нет - добавляем пробелов и возврат к часам
      EndLine = 12;
    }
  } else {
  RunText++; // берем следущую букву
  }
  return Temp;
}

void ShiftToLeft() { // Бегущая строка
uint8_t row, col;
if (!NewSymbol) { // Берем новый символ в буфер
  DG = GetNextSymbol();
  for (row = 0; row < 8; row++){
      Screen[12][row] = pgm_read_byte(&(font[DG][row]));   
   }
}
for (col = 0; col < 12; col++) {
  for (row = 0; row < 8; row++){
    Screen[col][row] <<= 1; // Двигаем точки влево
    bitWrite(Screen[col][row], 0, bitRead(Screen[col+1][row],7)); // Берем из предыдущего символа пропадающую часть после сдвига
  }
}
for (row = 0; row < 8; row++){
  Screen[12][row] <<= 1; // Двигаем буфер
}
if (++NewSymbol == 8) NewSymbol = 0; // Если сдвинули 8 раз, нужен новый символ
}

void BLINK(uint8_t oper, int n, ...){
  va_list vl;
  va_start(vl,n);
  for (fn=0; fn<n; fn++)
  {
    Blink[va_arg(vl,int)] = oper;
  }
  va_end(vl);
}

void Blinker() { // "Моргаем" необходимые символы
  for (fn = 0; fn < 12; fn++) {
    switch (Blink[fn]) {
      case START:
        CurScreen[fn] = 12;
        Blink[fn] = BLINK_ON;
        ScreenRefresh(fn);       
      break; 
      case BLINK_ON:
        CurScreen[fn] = NewScreen[fn];
        Blink[fn] = START;
        ScreenRefresh(fn);       
      break; 
      case STOP:
        CurScreen[fn] = NewScreen[fn];
        Blink[fn] = 0;
        ScreenRefresh(fn);       
      break; 
    }
  }
}

void BLANK (int n, ...) { // очищаем нужные сегменты
  va_list vl;
  va_start(vl,n);
  for (fn=0; fn<n; fn++)
  {
    NewScreen[va_arg(vl,int)] = 12;
  }
  va_end(vl);
}

uint8_t bcd2dec(uint8_t bcd){
    return ((bcd / 16) * 10) + (bcd % 16);
}

uint8_t dec2bcd(const uint8_t val){
    return ((val / 10 * 16) + (val % 10));
}

uint8_t dow(uint16_t y, uint8_t m, uint8_t d){ // день недели
    uint8_t dow;

    y -= m < 3;
    dow = ((y + y/4 - y/100 + y/400 + pgm_read_byte(dowArray+(m-1)) + d) % 7);

    if (dow == 0)
    {
        return 7;
    }

    return dow;
}

uint8_t leapyear(uint16_t y) { // високосный год
  return (((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0));
}

uint8_t dim(uint16_t year, uint8_t month){// кол-во дней в месяце
    uint8_t days;
    days = pgm_read_byte(daysArray + month - 1);
    if ((month == 2) && leapyear(year))
    {
        ++days;
    }
    return days;
}

void UpdateNumber(const uint8_t n){// меняем значения при установке даты/время
int8_t Temp, A1, A2, A3, A4 = 0;
  if (IsMenu == HOUR) {
    A1 = 2; A2 = 3; A3 = 23; 
  } else if (IsMenu == MIN) {    
    A1 = 5; A2 = 6; A3 = 59;
  } else if (IsMenu == SEC) {    
    A1 = 8; A2 = 9; A3 = 59;
  } else if (IsMenu == YEAR) {
    A1 = 9; A2 = 10; A3 = 99; A4 = 19;
  } else if (IsMenu == MONTH) {
    A1 = 6; A2 = 7; A3 = 12; A4 = 1;
  } else if (IsMenu == DATE) {
    A1 = 3; A2 = 4; A3 = dim(CurrTime[3]+2000,CurrTime[2]); A4 = 1;
  } else return;
  if (n == UP) {
    if (++MenuNumber > A3) MenuNumber = A4;
  } else {
    if (--MenuNumber == (A4-1)) MenuNumber = A3;
  }
    
  NewScreen[A2] = MenuNumber % 10;
  Temp = MenuNumber / 10;
  NewScreen[A1] = Temp % 10;        
  MenuTimer = MENU_WAIT;
}

void CheckIrButton(uint8_t btn) 
{// обработка кнопок ИК пульта
  if (btn == 255) return;
  PIK = 2; // издаем пик в знак подтверждение получения команды от ИК
  switch (btn) {
    case OK:
      if (!IsMenu && !BEEPER) {
        if (screenPage == S_TEMP) Termo = CurScreen[4]*100+CurScreen[5]*10+CurScreen[7]; // Запоминаем текущую температуру, чтобы вернуть назад на экран.
        IsMenu = TIMER;
        TimerValue = 0; ChangePix[9] = 0; ChangePix[8] = 0;
        CurScreen[0] = 31; CurScreen[1] = 13; CurScreen[2] = 22; CurScreen[3] = 25; CurScreen[4] = 18; CurScreen[5] = 29; CurScreen[6] = 10; CurScreen[7] = 12; CurScreen[8] = 0; CurScreen[9] = 0; CurScreen[10] = 0; CurScreen[11] = 12; // ТАЙМЕР: 000
        ScreenRefreshFull();
        NewScreen[8] = 0; NewScreen[9] = 0; NewScreen[10] = 0;
        BLINK(START,3,8,9,10);
        MenuTimer = MENU_WAIT;
        TimerPos = 0; TimerNumber[0] = 0; TimerNumber[1] = 0; TimerNumber[2] = 0; 
      } else if (IsMenu == TIMER) { // Запускаем таймер
        TimerValue = (TimerNumber[2] * 100 + TimerNumber[1] * 10 + TimerNumber[0]) * 60;
        IsMenu = 0;
        screenTime = 0;
        if ((TimerValue == 0) && (screenPage == S_TIMER)) screenPage = S_TIME;
        BLINK(STOP,3,8,9,10);
        IR_BLOCK = true;
      }
      if (BEEPER) { // Выключаем бипер
        BEEPER = 1; SOUND = true;
      }
    break;
    case GRID:
      if (IsMenu) {
        if (IsMenu == TIMER) {
          if (TimerPos == 0) MenuTimer = 1;
          NewScreen[8] = 0; NewScreen[9] = 0; NewScreen[10] = 0;
          TimerPos = 0; TimerNumber[0] = 0; TimerNumber[1] = 0; TimerNumber[2] = 0; 
        } else {
          MenuTimer = 1;
          screenTime = 0;
        }
      }
    break;
    case STAR:
      if (screenPage == S_TIME && IsMenu != TIMER) {
        MenuTimer = MENU_WAIT;
        switch (IsMenu) {
          case 0:
            IsMenu = HOUR; 
            BLINK(START,2,2,3);
            MenuNumber = NewScreen[2] * 10 + NewScreen[3];
            NewScreen[8] = 0; NewScreen[9] = 0;
            CurScreen[8] = 0; CurScreen[9] = 0;
            ScreenRefresh(8); ScreenRefresh(9);
          break;
          case HOUR:
            IsMenu = MIN;
            BLINK(STOP,2,2,3);
            BLINK(START,2,5,6);
            CurrTime[2] = dec2bcd(MenuNumber); // Часы для записи
            MenuNumber = NewScreen[5] * 10 + NewScreen[6];
          break;
          case MIN:
            IsMenu = SEC;
            BLINK(STOP,2,5,6);
            BLINK(START,2,8,9);
            CurrTime[1] = dec2bcd(MenuNumber); // Минуты для записи
            MenuNumber = 0;
          break;
          case SEC:
            IsMenu = 0;
            BLINK(STOP,2,8,9);
            CurrTime[0] = dec2bcd(MenuNumber); // Секунды для записи
            I2C_write(&DS3231, 0, &CurrTime[0], 3); // Пишем новые сек, мин, час
            screenTime = 0;
          break;
        }
      }
      if (screenPage == S_DATE && IsMenu != TIMER) {
        MenuTimer = MENU_WAIT;
        switch (IsMenu) {
          case 0:
            IsMenu = YEAR;
            BLINK(START,2,9,10);
            CurScreen[1] = 12; CurScreen[0] = 12;
            ScreenRefresh(0); ScreenRefresh(1);
            MenuNumber = NewScreen[9] * 10 + NewScreen[10];
          break;
          case YEAR:
            IsMenu = MONTH;
            BLINK(STOP,2,9,10);
            BLINK(START,2,6,7);
            CurrTime[3] = MenuNumber;
            MenuNumber = NewScreen[6] * 10 + NewScreen[7];
          break;
          case MONTH:
            IsMenu = DATE;
            BLINK(STOP,2,6,7);
            BLINK(START,2,3,4);
            CurrTime[2] = MenuNumber;
            uint8_t CheckDay;
            CheckDay = dim(CurrTime[3]+2000, MenuNumber); // Проверяем, чтобы текущее число было не больше возможного, после ввода года и месяца
            if ((NewScreen[3] * 10 + NewScreen[4]) > CheckDay) {
                    NewScreen[4] = CheckDay % 10;
                    CheckDay /= 10;
                    NewScreen[3] = CheckDay % 10;
             }
            MenuNumber = NewScreen[3] * 10 + NewScreen[4];
          break;
          case DATE:
            IsMenu = 0;
            BLINK(STOP,2,3,4);
            CurrTime[1] = dec2bcd(MenuNumber); // число
            CurrTime[0] = dec2bcd(dow(CurrTime[3]+2000,CurrTime[2],MenuNumber)); // день
            CurrTime[2] = dec2bcd(CurrTime[2]); // месяц
            CurrTime[3] = dec2bcd(CurrTime[3]); // год
            I2C_write(&DS3231, 3, &CurrTime[0], 4); // Пишем новые день, число, месяц, год
            screenTime = 0;
          break;
        }
      }
    break;
    case ARROW_UP: 
       if (!IsMenu) break;
       UpdateNumber(UP);
    break;
    case ARROW_DOWN: 
       if (!IsMenu) break;
       UpdateNumber(DOWN);
    break;
    case ARROW_LEFT: 
       if (!IsMenu) break;
       for (fn=0;fn<10;fn++) {
          UpdateNumber(UP);
       }
    break;
    case ARROW_RIGHT: 
       if (!IsMenu) break;
       for (fn=0;fn<10;fn++) {
          UpdateNumber(DOWN);
       }
    break;
    default:
      if (IsMenu == TIMER && btn <= 9 && TimerPos < 3) {
        MenuTimer = MENU_WAIT;
        if (TimerPos == 0 && btn == 0) break;
        TimerNumber[2] = TimerNumber[1];
        TimerNumber[1] = TimerNumber[0];
        TimerNumber[0] = btn;
        NewScreen[10] = TimerNumber[0];
        NewScreen[9] = TimerNumber[1];
        NewScreen[8] = TimerNumber[2];
        TimerPos++;
      }
    break;
  }
}

uint8_t GetIR() 
{// Берем код нажатой кнопки
  if (IRLremote.available())
  {
    auto data = IRLremote.read();
    if (data.address == 0xFF00) {
      switch (data.command) {
        case IR_1:
          return 1;
        break;
        case IR_2:
          return 2;
        break;
        case IR_3:
          return 3;
        break;
        case IR_4:
          return 4;
        break;
        case IR_5:
          return 5;
        break;
        case IR_6:
          return 6;
        break;
        case IR_7:
          return 7;
        break;
        case IR_8:
          return 8;
        break;
        case IR_9:
          return 9;
        break;
        case IR_0:
          return 0;
        break;
        case IR_STAR:
          return STAR;
        break;
        case IR_GRID:
          return GRID;
        break;
        case IR_UP:
          return ARROW_UP;
        break;
        case IR_DOWN:
          return ARROW_DOWN;
        break;
        case IR_LEFT:
          return ARROW_LEFT;
        break;
        case IR_RIGHT:
          return ARROW_RIGHT;
        break;
        case IR_OK:
          return OK;
        break;
      }
    }
  } 
  return 255;
}

uint8_t CheckDay() 
{ // Проверяем день на важное событие
  switch (CurrTime[2]) {
    case 1:
      if (CurrTime[1] == 1) return NY;
    break;
    case 2:
    break;
    case 3:
    break;
    case 4:
    break;
    case 5:
    break;
    case 6:
    break;
    case 7:
    break;
    case 8:
    break;
    case 9:
    break;
    case 10:
    break;
    case 11:
    break;
    case 12:
      if (CurrTime[1] == 31) return D31;
    break;
  }
return 0;
}

void NextScreen() 
{// листаем инфо на экране
  IR_BLOCK = true;
  screenPage++;
  screenTime = 0;
  if (screenPage == S_TEMP) {
    if (Termo == 850) screenPage++; // Не выводим температуру, если не удалось правильно прочитать данные
  } 
  if (screenPage == 4) {
    if (TimerValue != 0) return;
    screenPage = 5;
  }
  if (screenPage == 5){
    if (bigDay) {
      BD_COUNT++;
      if (BD_COUNT == 1) { return; } else {screenPage = 1;}
    } else {
      screenPage = 1;
    }
  }
}

void GetTermo(uint8_t action) 
{ // берем инфо с термодатчика
  int16_t value;
  switch (action) {
    case 1:
      if (DS18B20.reset()) {
        DS18B20.skip();
        DS18B20.write(0x44);
      }
    break;
    case 2:
      if (DS18B20.reset()){
        DS18B20.skip();
        DS18B20.write(0xBE);
        Termo = -1;
      }
    break;
    case 3:
      if (Termo == -1) {
        value = DS18B20.read();
        value = value | (DS18B20.read()<<8);  
        Termo = (value*10)>>4;
      } else {Termo = 850;}
    break;
  }
}

void ScreenUpdate() 
{// Выводим инфо на экран в зависимости от текущей страницы
  screenTime++;
  if (screenTime == 2) {
    IRLremote.read(); // очищаем, если что-то нажали во время запрета
    IR_BLOCK = false;
  }
  switch (screenPage) {
   case S_TIME:
      I2C_read(&DS3231, 0, &CurrTime[0], 3); // Запрашиваем три байта из часов (сек, мин, чаc)
      for (fn = 0; fn < 3; fn++) { // Составляем строку на экран
            CurrTime[fn] = bcd2dec(CurrTime[fn]); 
            NewScreen[pgm_read_byte(scrPos+fn)] = CurrTime[fn] % 10;
            CurrTime[fn] /= 10;
            NewScreen[pgm_read_byte(scrPos+fn+3)] = CurrTime[fn] % 10;    
      }  
      if (screenTime == 1) {
        NewScreen[4] = 10; NewScreen[7] = 10;// Двоеточия
        BLANK(4,0,1,10,11);
      } else if (screenTime == 10) {
        NextScreen(); // 
      }
      break;
   case S_DATE:
      I2C_read(&DS3231, 3, &CurrTime[0], 4); // Запрашиваем четыре байта из часов (день, число, месяц, год)
      for (fn = 0; fn < 4; fn++) { 
          CurrTime[fn] = bcd2dec(CurrTime[fn]); 
      }
      if (screenTime == 1) bigDay = CheckDay();
      GetTermo(screenTime);
      NewScreen[0] = pgm_read_byte(dow1 + CurrTime[0]-1);
      NewScreen[1] = pgm_read_byte(dow2 + CurrTime[0]-1);
      NewScreen[4] = CurrTime[1] % 10;
      CurrTime[1] /= 10;
      NewScreen[3] = CurrTime[1] % 10;
      NewScreen[5] = 11; // точка
      NewScreen[7] = CurrTime[2] % 10;
      CurrTime[2] /= 10;
      NewScreen[6] = CurrTime[2] % 10;
      NewScreen[8] = 11;// точка
      NewScreen[10] = CurrTime[3] % 10;
      CurrTime[3] /= 10;
      NewScreen[9] = CurrTime[3] % 10;
      BLANK(2,2,11);
      if (screenTime == 5) NextScreen();
      break;
   case S_TEMP:
      if (screenTime == 1) {
        NewScreen[2] = 46; NewScreen[3] = 47; NewScreen[6] = 11;
        NewScreen[7] = Termo % 10;
        Termo /= 10;
        NewScreen[5] = Termo % 10;
        Termo /= 10;
        NewScreen[4] = Termo % 10;
        NewScreen[8] = 45; NewScreen[9] = 30;
        BLANK(4,0,1,10,11);
        //IR_BLOCK = false;
      } else if (screenTime == 5) NextScreen();
      break;
   case S_TIMER: // Если запущен таймер, отображаем оставшееся время
      NewScreen[0] = 31; NewScreen[1] = 13; NewScreen[2] = 22; NewScreen[3] = 25; NewScreen[4] = 18; NewScreen[5] = 29; NewScreen[6] = 10; NewScreen[7] = 12; NewScreen[11] = 12; // ТАЙМЕР:
      uint16_t Temp;
      Temp = TimerValue / 60;
      if (Temp == 0) {
        NewScreen[10] = 1;
        NewScreen[9] = 51;
        NewScreen[8] = 12;
      } else {
        NewScreen[10] = Temp % 10; Temp /= 10;
        NewScreen[9] = Temp % 10; Temp /= 10;
        NewScreen[8] = Temp % 10;
        if (NewScreen[8] == 0) {
          NewScreen[8] = 12;
          if (NewScreen[9] == 0) NewScreen[9] = 12;
        }
      }
      if (screenTime == 5) NextScreen();
   break;
   case S_BIGDAY:
     for (fn=0;fn<12;fn++){
        CurScreen[fn] = 12;}
     StartRunText();
     return;
   break;
  }
  
  for (fn = 0; fn < 12; fn++) { // Включаем анимационный сдвиг вниз измененных цифр, или все символы при смене страницы экрана
    if (((CurScreen[fn] != NewScreen[fn]) && (ChangePix[fn] == 0)) || (screenTime == 1)) ChangePix[fn] = 8;      
  }
}

void TicTak () 
{ // Бегущий огонек по овалу часов, имитация секундной стрелки
  tictak--;
  pixels[tictak].r = 255;
  pixels[tictak].g = 0;
  pixels[tictak].b = 0;
  (tictak == 6) ? fn = 0 : fn = tictak + 1;   
  pixels[fn].r = 50;
  pixels[fn].g = 50;
  pixels[fn].b = 50;
  if (tictak == 0) tictak = 7;
  LED.sendPixels(numPixels, pixels);
}

void Wheel(uint8_t pos, uint8_t WheelPos) 
{ // меняем цвета радуги
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   pixels[pos].r = 255 - WheelPos * 3;
   pixels[pos].g = 0;
   pixels[pos].b = WheelPos * 3;
  } else if (WheelPos < 170) {
    WheelPos -= 85;
   pixels[pos].r = 0;
   pixels[pos].g = WheelPos * 3;
   pixels[pos].b = 255 - WheelPos * 3;
  } else {
   WheelPos -= 170;
   pixels[pos].r = WheelPos * 3;
   pixels[pos].g = 255 - WheelPos * 3;
   pixels[pos].b = 0;
  }
}

void rainbowCycle() 
{ // Радуга на заднем фоне при знаменательных датах
  for(fn=0; fn< numPixels; fn++) {
      Wheel(fn,((fn * 256 / numPixels) + j) & 255);
  }
  LED.sendPixels(numPixels, pixels);
  if (++j == 256*5-1) j = 0;
}

void Rotator() 
{ // в определенные временные интервалы что то делаем

  rotator++;
    
  if (PIK >= 1) {// Пикаем при нажатии кнопки пульта
      digitalWrite(BEEPER_PIN, HIGH);
      if (PIK == 1) digitalWrite(BEEPER_PIN, LOW);
      PIK--;
  }   
  // Эффект прокрутки символа вниз
  for (fn = 0; fn <12; fn++){
     if (ChangePix[fn] > 0) {
        DG = NewScreen[fn]; // НОВОЕ
        pos = ChangePix[fn]-1;
        for (int8_t n = 0; n < 8; n++){
          Screen[fn][n] = pgm_read_byte(&(font[DG][pos]));
          pos++;
          
          if (pos == 8) {
            pos = 0;
            DG = CurScreen[fn]; /// Старое
          }
        }
      ChangePix[fn]--;
      if (ChangePix[fn] == 0) {
        CurScreen[fn] = NewScreen[fn];
      }
     }
  }
  if (TimerValue > 0) { //во время работы таймера стрелка по кругу бежит быстрее
    TicTak();
  }
  if (!(rotator % 5)) { 
      if (!IR_BLOCK) CheckIrButton(GetIR()); // Смотрим сигналы с ИК пульта
      if (IsMenu) {
          if (--MenuTimer == 0) {
            IsMenu = false; 
            screenTime = 0;
            BLINK(STOP,12,0,1,2,3,4,5,6,7,8,9,10,11);    
          }
      }
      Blinker(); // Проверка на необходимость "моргания" символов
      if (BEEPER) { // Пищим если сработал таймер
        SOUND = !SOUND;
        digitalWrite(BEEPER_PIN, SOUND);
        BEEPER--;
      }
  }
  if (rotator == 240) rotator = 0;
}

 void ScreenShow() 
 { // Выводим символы на матрицу из экранного массива
      ((r-1) & (1 << 0)) ? PORT168 |= (1 << A0_168) : PORT168 &= ~(1 << A0_168);
      ((r-1) & (1 << 1)) ? PORT168 |= (1 << A1_168) : PORT168 &= ~(1 << A1_168);
      ((r-1) & (1 << 2)) ? PORT168 |= (1 << A2_168) : PORT168 &= ~(1 << A2_168);

    PORT595 |= (1 << ST_CP);

    for (c = 0; c < 12; c++){
      for (fn=0; fn<8; fn++) {
        (Screen[c][r] & (1<<(7-fn))) ? PORT595 |= (1<<DATA) : PORT595 &= ~(1<<DATA);
        PORT595 |= (1<<SH_CP);
        PORT595 &= ~(1<<SH_CP);
      }
    }
    
    PORT595 &= ~(1 << ST_CP);

    if (++r==8) {
      r=0;
      if (RunTextLoop) ShiftToLeft();
    }
}

void Intro() 
{ // Заставка при включении
  while (screenPage < 11){
    if (millis() - Timer > TimerFunc) {
      screenPage++;
      IntroStep();
      ScreenRefreshFull();
      Timer = millis();
    }
    ScreenShow();
    delayMicroseconds(screenDelay);
  }
  screenPage = 0;
  NextScreen();
}

void setup() 
{
// нужные порты на вход/выход
DDRD = 0b11110000; 
DDRB = 0b00110111;
IRLremote.begin(pinIR); // инициализация ИК приемник на соответствующем пине
I2C_init(I2C_FM);   // инициализация I2C в режим Fast Mode (400KHz)
I2C_slave_init(&DS3231, 0x68, I2C_8B_REG); // Часики с 68 адресом
for (fn=0;fn<12;fn++){
    CurScreen[fn] = 12;
    NewScreen[fn] = 12;
}
 
Timer = millis();
TimerSec = Timer;

Intro();

}

void loop() 
{
 
 ScreenShow();

 TimerFunc = micros();

 if (bigDay && !TimerValue) {// в знаменательный день выводим радугу
  if (millis() - TimerLED >= 15) {
      rainbowCycle();
      TimerLED = millis();  
  }
 }
 
 if (millis() - Timer >= 100) {
    Rotator();
    Timer = millis();  
 }

  if (millis() - TimerSec >= 1000) {
    if (!IsMenu && !RunTextLoop) ScreenUpdate();
    if (TimerValue > 0) {
      if (--TimerValue == 0) {
         BEEPER = 180;
      }
    } else if (!bigDay) TicTak();
  TimerSec = millis();
 }
 
 TimerFunc = micros() - TimerFunc;
 if (TimerFunc < screenDelay) delayMicroseconds(screenDelay - TimerFunc);
}

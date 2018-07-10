#include <LiquidCrystal.h>
#include <EEPROM.h>
LiquidCrystal lcd(11, 6, 5, 4, 3, 2); //rs, e, d4, d5, d6, d7

static byte addon_letters[16];
void init_rus(const char* letters_use )
{
  // custom characters
  static byte letters[][8]   = {
        { B11111, B10000, B10000, B11111, B10001, B10001, B11111, B00000 },//Б
        { B11111, B10000, B10000, B10000, B10000, B10000, B10000, B00000 },//Г
        { B01111, B01001, B01001, B01001, B01001, B11111, B10001, B00000 },//Д
        { B10101, B10101, B10101, B01110, B10101, B10101, B10101, B00000 },//Ж
        { B01110, B10001, B00001, B00110, B00001, B10001, B01110, B00000 },//З
        { B10001, B10001, B10011, B10101, B11001, B10001, B10001, B00000 },//И
        { B10101, B10101, B10011, B10101, B11001, B10001, B10001, B00000 },//Й
        { B00111, B01001, B10001, B10001, B10001, B10001, B10001, B00000 },//Л
        { B11111, B10001, B10001, B10001, B10001, B10001, B10001, B00000 },//П
        { B10001, B10001, B10001, B01111, B00001, B10001, B01110, B00000 },//У
        { B01110, B10101, B10101, B10101, B01110, B00100, B00100, B00000 },//Ф
        { B10001, B10001, B10001, B10001, B10001, B10001, B11111, B00001 },//Ц
        { B10001, B10001, B10001, B01111, B00001, B00001, B00001, B00000 },//Ч
        { B10101, B10101, B10101, B10101, B10101, B10101, B11111, B00000 },//Ш
        { B10101, B10101, B10101, B10101, B10101, B10101, B11111, B00001 },//Щ
        { B10000, B10000, B10000, B11110, B10001, B10001, B11110, B00000 },//Ь
        { B11000, B01000, B01110, B01001, B01001, B01001, B01110, B00000 },//Ъ
        { B10001, B10001, B10001, B11101, B10101, B10101, B11101, B00000 },//Ы
        { B11110, B00001, B00001, B01111, B00001, B00001, B11110, B00000 },//Э
        { B10111, B10101, B10101, B11101, B10101, B10101, B10111, B00000 },//Ю
        { B01111, B10001, B10001, B01111, B10001, B10001, B10001, B00000 },//Я
  };
  static char chars[] = {'Б','Г','Д','Ж','З','И','Й','Л','П','У','Ф','Ц','Ч','Ш','Щ','Ь','Ъ','Ы','Э','Ю','Я'};
  static byte empty[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int index = 0, cl = sizeof(chars)/sizeof(char),i,j,symb;
  memset(addon_letters,0,sizeof(addon_letters));
  for( j = 0; j < strlen(letters_use) && j < 16; j++ )
          lcd.createChar(j, empty);

  for( j = 0; j < strlen(letters_use) && j < 16; j++ )
  {
          symb = -1;
          for( i=0; i < cl; i++ ) if( chars[i] == letters_use[j] ) { symb = i; addon_letters[index] = letters_use[j]; break; }
          if( symb != -1 ) { lcd.createChar(index, letters[symb]); index++; }
  }
}
////////////////////////////////////////////////////////////////////
// print russian chars
////////////////////////////////////////////////////////////////////
void print_rus(char *str) {
  static char rus_letters[] = {'А','В','Е','Ё','К','М','Н','О','Р','С','Т','Х'};
  static char trans_letters[] = {'A','B','E','E','K','M','H','O','P','C','T','X'};
  int lcount = sizeof(rus_letters)/sizeof(char), i, j;
  for( i=0; i<strlen(str); i++ )
  { 
        if( byte(str[i]) == 208 ) continue; // 208 ignore
        int found = 0;
        for(j=0; j < 16; j++) if( addon_letters[j] != 0 && byte(str[i]) == byte(addon_letters[j]) ) { lcd.write(j); found = 1; break; }
        if(!found) for(j=0; j < lcount; j++) if( byte(str[i]) == byte(rus_letters[j]) ) { lcd.write(trans_letters[j]); found = 1; break; }
        if(!found) lcd.write(byte(str[i]));
  }  
}
void print_rus(int x, int y, char *str) {
  lcd.setCursor(x, y);
  print_rus(str);
}
static byte degree[8] = { B01100, B10010, B10010, B01100, B00000, B00000, B00000, B00000 };//degree

// задаем константы
float umax = 21.00;       //максимальное напряжение
float umin = 0.00;        //минимальное напряжение
const float maxI = 6.00;  //максимальный ток
int button1 = 13;          //кнопка 1
int button2 = 12;          //кнопка 2
float ah = 0.0000;        //Cчетчик Ампер*часов
const int down = 10;      //выход валкодера 1/2
const int up =  8;        //выход валкодера 2/2
const int pwm = 9;        //выход ШИМ 
const int power = 7;      //управление релюхой
long previousMillis = 0;  //храним время последнего обновления дисплея
long maxpwm = 0;          //циклы поддержки максимального ШИМ
long interval = 200;      // интервал обновления информации на дисплее, мс
int mig = 0;              //Для енкодера (0 стоим 1 плюс 2 минус)
float level = 0;          //"уровень" ШИМ сигнала
float com = 100;
long com2 = 0;
int mode = 0;            //режим (0 обычный, спабилизация тока, защита по току)
float Ioutmax = 1.0;     //заданный ток
int set = 0;             //пункты меню, отображение защиты...
int knopka_a = 0;        //состояние кнопок
int knopka_b = 0;
int knopka_ab = 0;
boolean off = false;
boolean red = false;      //красный светодиод
boolean blue = false;     //синий светодиод
float counter = 5;       // переменная хранит заданное напряжение
int disp = 0;            //режим отображения 0 ничего, 1 мощьность, 2 режим, 3 установленный ток, 4 шим уровень
float Uout ;             //напряжение на выходе




bool bounce = 0;
 bool btn, btn_old;
 uint32_t past = 0 ;
 bool flag = 0;
uint32_t past_flag = 0 ;
 const uint32_t time = 1500 ;




int incomingByte;         //входящий символ внешнего управления


void EEPROM_float_write(int addr, float val) // запись в ЕЕПРОМ
{  
  byte *x = (byte *)&val;
  for(byte i = 0; i < 4; i++) EEPROM.write(i+addr, x[i]);
}

float EEPROM_float_read(int addr) // чтение из ЕЕПРОМ
{    
  byte x[4];
  for(byte i = 0; i < 4; i++) x[i] = EEPROM.read(i+addr);
  float *y = (float *)&x;
  return y[0];
}

void setup() {   
pinMode(button1, INPUT);
pinMode(button2, INPUT);
digitalWrite(button1, 1);
digitalWrite(button2, 1);
         
cli();
DDRB |= 1<<1 | 1<<2;
PORTB &= ~(1<<1 | 1<<2);
TCCR1A = 0b00000010;
//TCCR1A = 0b10100010;
TCCR1B = 0b00011001;
ICR1H = 255;
ICR1L = 255;
sei();
int pwm_rez = 13;
pwm_rez = pow(2, pwm_rez);
ICR1H = highByte(pwm_rez);
ICR1L = lowByte(pwm_rez);

Serial.begin(9600);  
  //задаем режимы работы пинов
  pinMode(pwm, OUTPUT);  
  pinMode(down, INPUT);  
  pinMode(up, INPUT);  
 
  pinMode(power, OUTPUT); 
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  // поддерживаем еденицу на входах от валкодера
  digitalWrite(up, 1); 
  digitalWrite(down, 1);

  //запуск дисплея
  lcd.begin(16, 2);     
  init_rus("ПИЮБГЖДЩ");  
  print_rus(0,0, "  3АГРY3КА... ");
  delay(500);

  //загружаем настройки из памяти МК
  counter = EEPROM_float_read(0);
  Ioutmax = EEPROM_float_read(4);
  mode = EEPROM_float_read(12);
  disp = EEPROM_float_read(10);

  if(isnan(counter)){ //если нет настоек по умолчанию
    //загружаем их в память
     EEPROM_float_write(0, 12);
     EEPROM_float_write(4, 4);
     EEPROM_float_write(12, 0);
     EEPROM_float_write(10, 1);
    //и считываем для дальнейшей работы
    counter = EEPROM_float_read(0);
    Ioutmax = EEPROM_float_read(4);
    mode = EEPROM_float_read(12);
    disp = EEPROM_float_read(10);
  }  
  //включаем реле
  digitalWrite(power, 1);
}  //конец setup

//функции при вращении енкодера
void uup(){ //енкодер +
  if(set==0){//обычный режим - добавляем напряжения
     if(counter<umax) counter = counter+0.1;//добавляем
  }
  if(set==1){ //переключаем режим работы вперед
    mode = mode+1;
    if(mode>2) mode=2;
  }
  if(set==2){ //настройка тока, добавляем ток
    iplus();
  }  
  if(set==3){//сброс счетчика А*ч
    ah = 0;
    set = 0;
    disp = 5;
  }
  if(set==4){//сохранение текущих настроек в память
    save();
  }
  if(set==10){//сохранение текущих настроек в память
counter = 4.2;
mode = 1;
Ioutmax = 1;
disp = 3;
set = 0;
  }
   if(set==11){//сохранение текущих настроек в память
counter = 5;
mode = 1;
Ioutmax = 1;
disp = 3;
set = 0;
  }
   if(set==12){//сохранение текущих настроек в память
counter = 12;
mode = 1;
Ioutmax = 1;
disp = 3;
set = 0;
  }
}

void udn(){ //енкодер -
  if(set==0){
   if(counter>umin) counter = counter-0.1; //убавляем напнряжение
  }
  if(set==1){
   mode = mode-1; //переключаем режим работы назад
   if(mode<0) mode=0;
  }  
  if(set==2) iminus(); //убавляем ток
}

void iplus(){ 
   Ioutmax = Ioutmax+0.01;
   if(Ioutmax>0.2) Ioutmax=Ioutmax+0.04;
   if(Ioutmax>1) Ioutmax=Ioutmax+0.05;   
   if(Ioutmax>maxI) Ioutmax=maxI;
}

void iminus(){ 
    Ioutmax = Ioutmax-0.01;
   if(Ioutmax>0.2) Ioutmax=Ioutmax-0.04;
   if(Ioutmax>1) Ioutmax=Ioutmax-0.05;   
   if(Ioutmax<0.03) Ioutmax=0.03;
}

void save(){
      lcd.clear();
     print_rus(0,0, "   СОХРАНЕНИЕ   ");
     print_rus(0,1, "       ОК       ");        
     EEPROM_float_write(0, counter);
     EEPROM_float_write(4, Ioutmax);
     EEPROM_float_write(12, mode);
     EEPROM_float_write(10, disp);
     //мигаем светодиодами
      digitalWrite(A4, 1);  
      digitalWrite(A5, 1);  
      delay(1500);
      digitalWrite(A4, 0); 
      digitalWrite(A5, 0);   
      set = 0; //выходим из меню
}

void loop() //основной цикл работы МК
{  
  unsigned long currentMillis = millis();   
  /* Вншнее управление */
  if (Serial.available() > 0) {  //если есть доступные данные
        // считываем байт
        incomingByte = Serial.read(); 
    }else{
      incomingByte = 0;
    }    
    if(incomingByte==97){ //a
    if(counter>umin+0.1)counter = counter-0.1; //убавляем напнряжение    
    }   
        if(incomingByte==98){ //b  
          if(counter<umax) counter = counter+0.1;//добавляем     
    }    
    if(incomingByte==99){ //c   
        iminus();
    }     
    if(incomingByte==100){ //d
         iplus();
    }
    if(incomingByte==101) mode = 0;
    if(incomingByte==102) mode = 1; 
    if(incomingByte==103) mode = 2;
    if(incomingByte==104) save();
    if(incomingByte==105){
      digitalWrite(power, 1); //врубаем реле если оно было выключено
      delay(100);
      digitalWrite(A4, 0); //гасим красный светодиод 
      Serial.print("t0;");
      off = false;
      set = 0;//выходим из меню
   }  
   if(incomingByte==106) off = true;
   if(incomingByte==107) ah = 0;
   /* конец внешнего управления */

  //получаем значение напряжения и тока в нагрузке
  float Ucorr = -0.09; //коррекция напряжения, при желании можно подстроить
  float Uout = analogRead(A1) * ((5.0 + Ucorr) / 1023.0) * 5.0; //узнаем напряжение на выходе
  float Iout = analogRead(A0) / 100.00; // узнаем ток в нагрузке
  //корекция показаний амперметра
  if(Iout==0.01) Iout =  0.03; else 
  if(Iout==0.02) Iout =  0.04; else
  if(Iout==0.03) Iout =  0.05; else
  if(Iout==0.04) Iout = 0.06; else
  if(Iout>=0.05) Iout = Iout + 0.02;
  if(Iout>=0.25)Iout = Iout + 0.01;  
  
  /* ЗАЩИТА и выключение */  
  if (((Iout > ( counter + 0.3 ) * 2.0) | Iout>10.0  | off) & set<4 & millis()>100 ){ // условия защиты
     digitalWrite(power, 0); //вырубаем реле
     level = 0; //убираем ШИМ сигнал
     digitalWrite(A4, 1);                
     Serial.print("I0;U0;r1;W0;");
     Serial.println(' ');
     set = 6;
  }
   //Зашита от длительного максимального шим
    if (level==8190 & off==false)
    {  
      if(set<4)//если уже не сработала защита
      { 
        maxpwm++; //добавляем +1 к счетчику
        digitalWrite(A4, 1); //светим красным для предупреждения о максимальном ШИМ
      }  
    }
    else //шим у нас не максимальный, поэтому поубавим счетчик
    {
      maxpwm--;
      if(maxpwm<0)//если счетчик дошел до нуля
      {
        maxpwm = 0; //таким его и держим
        if(set<4) digitalWrite(A4, 0); // гасим красный светодиод. Перегрузки нет.
      }
    }
  //Зашита от Что то пошло не так. Например сгорел транзистор.
    if (level==0 & off==false & counter<Uout & Iout>0)
    {  
      if(set<4)//если уже не сработала защита
      { 
        digitalWrite(power, 0); //вырубаем реле
     level = 0; //убираем ШИМ сигнал
     digitalWrite(A4, 1);                
     Serial.print("I0;U0;r1;W0;");
     Serial.println(' ');
        set = 8;
      }  
    }
  /* ЗАЩИТА КОНЕЦ */
  
  
  // считываем значения с входа валкодера
  boolean regup = digitalRead(up);
  boolean regdown = digitalRead(down);
  
  if(regup<regdown) mig = 1; // крутится в сторону увеличения
  if(regup>regdown) mig = 2; // крутится в сторону уменшения
  if(!regup & !regdown) //момент для переключения
  { 
    if(mig==1) uup();//+
    if(mig==2) udn(); //-
    mig = 0; //сбрасываем указатель направления
  }

if(mode==0 | mode==1) //если управляем только напряжением (не режим стабилизации тока)
{   
  //Сравниваем напряжение на выходе с установленным, и принимаем меры..
  if(Uout>counter)
  {
    float raz = Uout - counter; //на сколько напряжение на выходе больше установленного...
    if(raz>0.05)
    {
      level = level - raz * 20; //разница большая управляем грубо и быстро!
    }else{
       if(raz>0.015)  level = level -  raz * 3 ; //разница небольшая управляем точно
    }
  }
  if(Uout<counter)
  {
    float raz = counter - Uout; //на сколько напряжение меньше чем мы хотим
    if(raz>0.05)
    {
      level = level + raz * 20; //грубо
    }else{
      if(raz>0.015)  level = level + raz * 3 ; //точно
    }
  }

  if(mode==1&&Iout>Ioutmax) //режим защиты по току, и он больше чем мы установили
  { 
    digitalWrite(power, 0); //вырубаем реле
    Serial.print("t2;");    
    digitalWrite(A4, 1);   //зажигаем красный светодиод
    level = 0; //убираем ШИМ сигнал
    set=5; //режим ухода в защиту...
  }
  
}else{ //режим стабилизации тока
  if(Iout>=Ioutmax)
  {
    //узнаем запас разницу между током в нагрузке и установленным током
    float raz = (Iout - Ioutmax); 
    if(raz>0.3) //очень сильно превышено (ток больше заданного более чем на 0,3А)
    {
      level = level - raz * 20; //резко понижаем ШИМ
    }else{    
      if(raz>0.05) //сильно превышено (ток больше заданного более чем на 0,1А)
      {
        level = level - raz * 5; //понижаем ШИМ
      }else{
        if(raz>0.00) level = level - raz * 2; //немного превышен (0.1 - 0.01А) понижаем плавно
      }
    }
    digitalWrite(A5, 1);  //зажигаем синий светодиод
  }else{ //режим стабилизации тока, но ток у нас в пределах нормы, а значит занимаемся регулировкой напряжения
    digitalWrite(A5, 0);//синий светодиод не светится
    if(Uout>counter){ //Сравниваем напряжение на выходе с установленным, и принимаем меры..
      float raz = Uout - counter; //на сколько напряжение на выходе больше установленного...
      if(raz>0.1){
        level = level - raz * 20; //разница большая управляем грубо и быстро!
      }else{
        if(raz>0.015)  level = level - raz * 5; //разница небольшая управляем точно
      }
    }
    if(Uout<counter){
      float raz = counter - Uout; //на сколько напряжение меньше чем мы хотим
      float iraz = (Ioutmax - Iout); //учитываем запас по току
      if(raz>0.1 & iraz>0.1){
        level = level + raz * 20; //грубо
      }else{
        if(raz>0.015)  level = level + raz ; //точно
      }
    }
  }
}//конец режима стабилизации тока     
    
if(off) level = 0;
if(level<0) level = 0; //не опускаем ШИМ ниже нуля
if(level>8190) level = 8190; //не поднимаем ШИМ выше 13 бит
//Все проверили, прощитали и собственно отдаем команду для силового транзистора.
if(ceil(level)!=255) analogWrite(pwm, ceil(level)); //подаем нужный сигнал на ШИМ выход (кроме 255, так как там какая-то лажа!!!)



/* УПРАВЛЕНИЕ */
if (digitalRead(button1)==0 && digitalRead(button2)==0 && knopka_ab==0 ) { // нажата ли кнопка a и б вместе
  knopka_ab = 1;
  lcd.clear();
  lcd.setCursor (0, 0);
  lcd.print("V2.2  25/02/2017");
  lcd.setCursor (0, 1);
  lcd.print("www.start.net.ua"); 
  delay(1700);
  knopka_ab = 0;
}

if (! bounce && btn != digitalRead(button1)) { // если прошел фронт изм на выводн
        bounce = 1;                              // выставить флаг
        past = millis();                         // сделать временую засветку
      }
      else if ( bounce && millis() - past >= 5 ) { // если прошло антидребезговое время
        bounce = 0;                                // то снять флаг
        btn_old = btn ;
        btn = digitalRead(button1) ;                   // прочитать реальное значение на выводе
        if (btn_old && ! btn) {
          flag = 1;
          past_flag = millis();
        }
        if (! btn_old && btn && flag && millis() - past_flag < time  ) {
          flag = 0;      
 lcd.clear();
 if(set<10) set = 10;
 else{
 if(set==10) set = set+1;
 else{
 if(set==11) set = set+1;
 else{
 if(set==12) {
 set = 0;
   lcd.clear();
  }
 }
 }
 }  
      }
      if (flag && millis() - past_flag >= time ) {
        flag = 0;     
//counter = 5;
//mode = 1;
//Ioutmax = 2;
//disp = 3;
  disp = disp + 1; //поочередно переключаем режим отображения информации
  if(disp==6) disp = 0; //дошли до конца, начинаем снова
  lcd.clear();
  }
      }

//if (digitalRead(button1)==0 && knopka_a==0) { // нажата ли кнопка А (disp)
//  knopka_a = 1;
//  disp = disp + 1; //поочередно переключаем режим отображения информации
//  if(disp==6) disp = 0; //дошли до конца, начинаем снова
//}

if (digitalRead(button2)==0 && knopka_b==0) { // нажата ли кнопка Б (menu)
  knopka_b = 1;
  set = set+1; //перебираем меню
  if(set>4 | off) {//Задействован один из режимов защиты, а этой кнопкой мы его вырубаем. (или мы просто дошли до конца меню)
  off = false;
  digitalWrite(power, 1); //врубаем реле если оно было выключено
  delay(100);
  digitalWrite(A4, 0); //гасим красный светодиод 
  Serial.print("t0;r0;");
  Serial.println(' ');
  set = 0;//выходим из меню
  }
  lcd.clear();//чистим дисплей
}
//сбрасываем значения кнопок.
if(digitalRead(button2)==1&&knopka_b==1) knopka_b = 0;
if(digitalRead(button1)==1&&knopka_a==1) knopka_a = 0;

/* COM PORT */
if(currentMillis - com2 > com) {
    // сохраняем время последнего обновления
    com2 = currentMillis;      
    //Считаем Ампер*часы
    ah = ah + (Iout / 36000);
    Serial.print('U');    Serial.print(Uout);               Serial.print(';');    
    Serial.print('I');    Serial.print(Iout);               Serial.print(';');    
    Serial.print('i');    Serial.print(Ioutmax);            Serial.print(';');    
    Serial.print('u');    Serial.print(counter);            Serial.print(';');    
    Serial.print('W');    Serial.print(level);              Serial.print(';');    
    Serial.print('c');    Serial.print(ah);                 Serial.print(';');    
    Serial.print('m');    Serial.print(mode);               Serial.print(';');    
    Serial.print('r');    Serial.print(digitalRead(A4));    Serial.print(';');    
    Serial.print('b');    Serial.print(digitalRead(A5));    Serial.print(';');    
    Serial.println(' ');  
  }  
  
  /* ИНДИКАЦИЯ LCD */
  if(set==0){ //стандартный екран  
    //выводим уснановленное напряжение на дисплей
    lcd.setCursor (0, 1);
    lcd.print("U>"); 
    if(counter<10) lcd.print(" "); //добавляем пробел, если нужно, чтобы не портить картинку
    lcd.print (counter,1); //выводим установленное значение напряжения
    lcd.print ("B "); //пишем что это вольты 
  
    //обновление информации
    if(currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;  // сохраняем время последнего обновления
      //выводим актуальные значения напряжения и тока на дисплей
      lcd.setCursor (0, 0);
      lcd.print("U=");
      if(Uout<9.99) lcd.print(" ");
      lcd.print(Uout,2);
      lcd.print("B I=");
      lcd.print(Iout, 2);
      lcd.print("A ");
      //дополнительная информация
      lcd.setCursor (8, 1);
      if(disp==0)  lcd.print("         "); //ничего
      if(disp==1){  //мощность
        lcd.print(" ");
        lcd.print (Uout * Iout,2); 
        lcd.print("W   ");
      }  
      if(disp==2){  //режим БП
        if(mode==0)print_rus(8,1,"СТАНДАРТ"); 
        if(mode==1)print_rus(8,1,"  3АЩИТА  "); 
        if(mode==2)print_rus(8,1,"СТАБ.ТОК");
      }  
      if(disp==3){  //максимальный ток
        lcd.print (" I>"); 
        lcd.print (Ioutmax, 2); 
        lcd.print ("A ");
      }
      if(disp==4){  // значение ШИМ
        lcd.print ("pwm:"); 
        lcd.print (ceil(level), 0); 
        lcd.print ("  ");
      }
      if(disp==5){  // значение ШИМ
        lcd.print ("R="); 
        lcd.print (Iout / Uout,2); 
        lcd.print ("  ");
      }
      if(disp==6){  // cчетчик А*ч
        if(ah<1){
          if(ah<=0.01) lcd.print (" ");
          if(ah<=0.1) lcd.print (" ");
          lcd.print (ah*1000, 1); 
          lcd.print ("mAh  ");
        }else{
          if(ah<=10) lcd.print (" ");
          lcd.print (ah, 3); 
          lcd.print ("Ah  ");
        }
      }
    }
  }

  /* ИНДИКАЦИЯ МЕНЮ */
  if(set==1)//выбор режима
  {
    print_rus(0,0, "> МЕНЮ 1/4   ");
    lcd.setCursor (0, 1);
    print_rus(0,1,"РЕЖИМ: ");
    //режим (0 обычный, спабилизация тока, защита по току)
    if(mode==0)  print_rus(7,1,"СТАНДАРТ ");
    if(mode==1)  print_rus(7,1,"3АЩИТА   ");
    if(mode==2)  print_rus(7,1,"СТАБ.ТОКА");
  }
  if(set==2){//настройка тока
    print_rus(0,0, "> МЕНЮ 2/4   ");
    lcd.setCursor (0, 1);
    print_rus(0,1,"MAX ТОК: ");
    lcd.print(Ioutmax);
    lcd.print("A");
   }
  if(set==3){//спрашиваем хочет ли юзер сбросить счетчик
    lcd.setCursor (0, 0);
    print_rus(0,0, "> МЕНЮ 3/4   ");
    lcd.setCursor (0, 1);
    print_rus(0,1,"СБРОС A*h?    ->");
  }

  if(set==4){//спрашиваем хочет ли юзер сохранить настройки
    lcd.setCursor (0, 0);
    print_rus(0,0, "> МЕНЮ 4/4   ");
    lcd.setCursor (0, 1);
    print_rus(0,1,"СОХРАНЕНИЕ?   ->");
  }
    if(set==10){//спрашиваем хочет ли юзер сохранить настройки
    print_rus(0,0, "> ПРЕДYСТАНОВКА");
    lcd.setCursor (0, 1);
    print_rus(0,1,"4.2В 1А 3АЩИТА->");
  }
    if(set==11){//спрашиваем хочет ли юзер сохранить настройки
    print_rus(0,0, "> ПРЕДYСТАНОВКА");
    lcd.setCursor (0, 1);
    print_rus(0,1,"5В 1А 3АЩИТА  ->");
  }
    if(set==12){//спрашиваем хочет ли юзер сохранить настройки
    print_rus(0,0, "> ПРЕДYСТАНОВКА");
    lcd.setCursor (0, 1);
    print_rus(0,1,"12В 1А 3АЩИТА ->");
  }    
  
  /* ИНДИКАЦИЯ ЗАЩИТЫ */
  if(set==5){//защита. вывод инфы
    lcd.setCursor (0, 0);
    print_rus(0,0, "[3АЩИТА ПО ТОКY]");
    lcd.setCursor (0, 1);
    lcd.print("Iout");
    lcd.print(">Imax(");
    lcd.print(Ioutmax);
    lcd.print("A)"); 
    level=0;
    Serial.print("I0;U0;r1;W0;");
    Serial.println(' ');
  }
  if(set==7){//защита. вывод инфы
    lcd.setCursor (0, 0);
    print_rus(0,0, "[3АЩИТА MAX PWM]");
    lcd.setCursor (0, 1);
    lcd.print("Iout");
    lcd.print(">Imax(");
    lcd.print(Ioutmax);
    lcd.print("A)"); 
    level=0;
    Serial.print("I0;U0;r1;W0;");
    Serial.println(' ');
  }
    if(set==8){//защита. вывод инфы
    lcd.setCursor (0, 0);
    print_rus(0,0, "[4ТО ТО  НЕ ТАК]");
    lcd.setCursor (0, 1);
    lcd.print("Iout");
    lcd.print(">Imax(");
    lcd.print(Ioutmax);
    lcd.print("A)"); 
    level=0;
    Serial.print("I0;U0;r1;W0;");
    Serial.println(' ');
  } 
 if(set==6){//защита. вывод инфы критическое падение напряжения
    Serial.print("I0;U0;r1;W0;");
    digitalWrite(A4, true);
    Serial.println(' ');
    level=0;
    lcd.setCursor (0, 0);
    if (off==false){ 
      print_rus(0,0, "[  ПЕРЕГРY3КА  ]");
      lcd.setCursor (0, 1);    
      if((Iout > (counter + 0.3)*2.0) | Iout>maxI){ //и обьясняем юзеру что случилось
          Serial.print("t1;");
          lcd.print("  Iout >= Imax  ");          
      }     
    }else{   
      lcd.print("[      OFF     ]");
      lcd.setCursor (0, 1);
      Serial.print("t4;");
    }
  }  
}


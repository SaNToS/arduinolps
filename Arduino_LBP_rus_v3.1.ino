#include <LiquidCrystal_1602_RUS.h>;
#include <EEPROM.h>
#include "GyverEncoder.h"
#include "TimerOne.h"

LiquidCrystal_1602_RUS lcd(11, 6, 5, 4, 3, 2); //rs, e, d4, d5, d6, d7

  // задаем константы
float umax = 21.00;       //максимальное напряжение
float umin = 0.00;        //минимальное напряжение
const float maxI = 7.00;  //максимальный ток
int button1 = 13;          //кнопка 1
int button2 = 12;          //кнопка 2
float ah = 0.0000;        //Cчетчик Ампер*часов
const int down = 10;      //выход энкодера 1/2
const int up =  8;        //выход энкодера 2/2
const int pwm = 9;        //выход ШИМ 
const int power = 7;      //управление релюхой
long previousMillis = 0;  //храним время последнего обновления дисплея
long maxpwm = 0;          //циклы поддержки максимального ШИМ
long interval = 100;      // интервал обновления информации на дисплее, мс
int mig = 0;              //Для енкодера (0 стоим 1 плюс 2 минус)
float level = 0;          //"уровень" ШИМ сигнала
float com = 100;
long com2 = 0;
int mode = 1;            //режим (0 обычный, спабилизация тока, защита по току)
float Ioutmax = 1.0;     //заданный ток
int set = 0;             //пункты меню, отображение защиты...
int knopka_a = 0;        //состояние кнопок
int knopka_b = 0;
int knopka_ab = 0;
boolean off = false;
boolean red = false;      //красный светодиод
boolean blue = false;     //синий светодиод
float counter = 5;       // переменная хранит заданное напряжение
int ri = 1;
boolean fr = 0;
int disp = 0;            //режим отображения 0 ничего, 1 мощьность, 2 режим, 3 установленный ток, 4 шим уровень
float Uout;             //напряжение на выходе
float Icrash;
 
 bool bounce = 0;
 bool btn, btn_old;
 uint32_t past = 0 ;
 bool flag = 0;
 uint32_t past_flag = 0 ;
 const uint32_t time = 300 ;
int incomingByte;         //входящий символ внешнего управления

// таймаут на скорость isFastR & isFastL. По умолч. 50 
//(чем меньше число тем быстрее надо крутить, и наоборот)
int timeout = 25; 
float counterFast = 0.5; // Установка напряжения при быстром вращении энкодера + -
Encoder enc1(up, down);


// Массивы символов для прогресс-бара
byte p20[8] = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
};
 
byte p40[8] = {
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
};
 
byte p60[8] = {
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
};
 
byte p80[8] = {
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
};
 
byte p100[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

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
  pinMode(down, INPUT_PULLUP);  
  pinMode(up, INPUT_PULLUP);  
 
  pinMode(power, OUTPUT); 
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  // поддерживаем еденицу на входах от энкодера
  digitalWrite(up, 1); 
  digitalWrite(down, 1);

  //запуск дисплея
  lcd.begin(16, 2);                    // Настройка количества столбцов и строк на ЖК-дисплее
       lcd.setCursor(1, 0);
  lcd.print("   LOADING...  ");         // Печать текста на ЖК-дисплее
  //delay(500);
   //Определение пользовательских символов
  lcd.createChar(0, p20);
  lcd.createChar(1, p40);
  lcd.createChar(2, p60);
  lcd.createChar(3, p80);
  lcd.createChar(4, p100);

lcd.setCursor(0,1);                  // Установка курсора во вторую строку, первый столбец
  lcd.print("                ");       // Очистка второй строки 16-ю пробелами
  for (int i = 0; i<16; i++)
  {
    // Итерация по каждому символу на второй строке
    for (int j=0; j<5; j++)
    {
      lcd.setCursor(i, 1);            // Установка курсора в заданную позицию
      lcd.write(j);                   // Обновление прогресс-бара
      delay(25);                     // Ожидание 100 мс.
    }  
  }
lcd.clear();

  //загружаем настройки из памяти МК
  counter = EEPROM_float_read(0);
  Ioutmax = EEPROM_float_read(4);
     mode = EEPROM_float_read(12);
     disp = EEPROM_float_read(10);
	   ri = EEPROM_float_read(15);
	
  if(isnan(counter)){ //если нет настоек по умолчанию
    //загружаем их в память
    EEPROM_float_write(0, 12);
    EEPROM_float_write(4, 0.3);
    EEPROM_float_write(12, 0);
    EEPROM_float_write(10, 1);
	EEPROM_float_write(15, 0);
    //и считываем для дальнейшей работы
    counter = EEPROM_float_read(0);
    Ioutmax = EEPROM_float_read(4);
       mode = EEPROM_float_read(12);
       disp = EEPROM_float_read(10);
	     ri = EEPROM_float_read(15);
  }  
  
  //включаем реле
  digitalWrite(power, 1);
  
  //Инициализация антидребезга
  	enc1.setType(TYPE2);    // тип энкодера TYPE1 одношаговый, TYPE2 двухшаговый. Если ваш энкодер работает странно, смените тип
	Timer1.initialize(1000);            // установка таймера на каждые 1000 микросекунд (= 1 мс)
	Timer1.attachInterrupt(timerIsr);   // запуск таймера
	enc1.setFastTimeout(timeout);    // таймаут на скорость isFastR. По умолч. 50

}  //конец setup


void timerIsr() {   // прерывание таймера
enc1.tick();     // отработка теперь находится здесь
}


//функции при вращении енкодера

void uup(){ //енкодер +

  if(set==0){//обычный режим - добавляем напряжения
	counterUp();
  }
  
  if(set==1){ //переключаем режим работы вперед
		mode = mode + 1;
		if(mode >= 3) mode = 0;
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

if(fr){ // Листаем предустановки (+)
		if(set==10){
			set = set+1;
		} else if(set==11) {
			set = set+1;
		} else if(set==12) { 
			set = 10; 
		}
	}
}

unsigned long CurrentTime, LastTime;
void counterUp(){
    if (!enc1.isFastR()){
        if(counter<umax) counter = counter + 0.1; //добавляем
      }else{ 
    CurrentTime = millis();
    if (CurrentTime - LastTime > 400) {
            LastTime = CurrentTime;
            counter = counter + 0.1; //добавляем
        }else{
            if(counter<umax) counter = counter + counterFast; //добавляем
        }
    }
    if(counter>umax) counter = umax;
}

void counterDown(){
    if (!enc1.isFastL()){
        if(counter>umin) counter = counter - 0.1; //отнимаем
    }else{
        CurrentTime = millis();
        if (CurrentTime - LastTime > 400) {
            LastTime = CurrentTime;
            counter = counter - 0.1;
        }else{
            if(counter>umin) {
                if(counter > 0.5){
                    counter = counter - counterFast; //отнимаем
                }else{
                    counter = counter - 0.1;
                }
            }
        }
    }
    if(counter < 0.1) counter = 0.0;
}

void udn(){ //енкодер -

if(set==0){//обычный режим - добавляем напряжения
	counterDown();
  }
	
	if(set==1){
		mode = mode -1; //переключаем режим работы назад
		if(mode < 0) mode = 2;
	}  
	if(set==2) iminus(); //убавляем ток
  
	if(fr){ // Листаем предустановки (-)
		if(set==12){
			set = set-1;
		} else if(set==11) {
			set = set-1;
		} else if(set==10) { 
			set = 12; 
		}
	}
}

void iplus(){  // + Прибавить ток
 
	if(Ioutmax + 0.05 > maxI){
		ri = ri;
	}else if(Ioutmax < maxI ) {
		ri = ri + 1;
	}
   if(ri < 19) {
	   Ioutmax = Ioutmax + 0.01;
   }else if(ri < 35) {
	   Ioutmax=Ioutmax+ 0.05;
   }else if(ri > 34){
	   Ioutmax=Ioutmax+ 0.10;
   } 
   if(Ioutmax>=maxI) { 
	   Ioutmax=maxI;
   }
}

void iminus(){  // - Отнять ток
 
 if(Ioutmax - 0.01 < 0.03){
	ri = 1;
 }else{
	 if(ri < 1) ri = 1;
	ri = ri - 1;
 }
 if(ri < 18) {
		Ioutmax = Ioutmax - 0.01;
   }else if(ri > 17 && ri < 34) {
		Ioutmax=Ioutmax - 0.05;
   }else if(ri > 32) {
	    Ioutmax=Ioutmax - 0.10;
   } 
   if(Ioutmax<0.03) { 
   Ioutmax=0.03;
   }
}

void save(){
    lcd.clear();
    lcd.setCursor (0, 0);
    lcd.setCursor(3, 0);
	lcd.print(L"Сохранение");
	lcd.setCursor(7, 1);
	lcd.print(L"ОК");        
    EEPROM_float_write(0, counter);
    EEPROM_float_write(4, Ioutmax);
    EEPROM_float_write(12, mode);
    EEPROM_float_write(10, disp);
	EEPROM_float_write(15, ri);
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
Serial.println(digitalRead(up));
Serial.println(digitalRead(down));
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
  //float Ucorr = -0.09; //коррекция напряжения, при желании можно подстроить
//  float Uout = analogRead(A1) * ((5.0 + Ucorr) / 1023.0) * 5.0; //узнаем напряжение на выходе
  
  float Uout = analogRead(A1) * (5.0  / 1023.0) * 5.0; //узнаем напряжение на выходе
  
  float Iout = analogRead(A0) / 100.00; // узнаем ток в нагрузке
  //корекция показаний амперметра
  if(Iout==0.01) Iout =  0.03; else 
  if(Iout==0.02) Iout =  0.04; else
  if(Iout==0.03) Iout =  0.05; else
  if(Iout==0.04) Iout = 0.06; else
  if(Iout>=0.05) Iout = Iout + 0.02;
  if(Iout>=0.25)Iout = Iout + 0.01;  
  
  /* ЗАЩИТА и выключение */  
  if (((Iout > ( counter + 0.3 ) * 2.0) | Iout>maxI  | off) & set<4 & millis()>800 ){ // условия защиты
     Icrash=Iout;
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
    if(raz>0.1 & Uout>5) //очень сильно превышено (ток больше заданного более чем на 0,6А)
    {
      level = 0; //резко понижаем ШИМ
      }else{
    if(raz>0.1 & Uout>1) //очень сильно превышено (ток больше заданного более чем на 0,3А)
    {
      level = level - 1; //резко понижаем ШИМ
      }else{
    if(raz>0.05) //очень сильно превышено (ток больше заданного более чем на 0,3А)
    {
      level = level - raz * 30; //резко понижаем ШИМ
    }else{
    if(raz>0.03) //очень сильно превышено (ток больше заданного более чем на 0,3А)
    {
      level = level - raz * 20; //резко понижаем ШИМ
    }else{
    if(raz>0.02) //очень сильно превышено (ток больше заданного более чем на 0,3А)
    {
      level = level - raz * 10; //резко понижаем ШИМ
    }else{    
      if(raz>0.01) //сильно превышено (ток больше заданного более чем на 0,1А)
      {
        level = level - raz * 2; //понижаем ШИМ
      }else{
        if(raz>0.00) level = level - raz; //немного превышен (0.1 - 0.01А) понижаем плавно
      }
    }
    }
    }
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
        level = level + raz * 5; //грубо
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
  lcd.print(L"V3.1  26/07/2019");
  lcd.setCursor(0, 1);
  lcd.print(L"www.start.net.ua");
  delay(5700);
  knopka_ab = 0;
  fr=0; // Выйти с предустановок
  set = 0;//выходим из меню
  lcd.clear();//чистим дисплей
	flag = 0;
}

if (! bounce && btn != digitalRead(button1)) { // если прошел фронт изм на выводн
        bounce = 1;                              // выставить флаг
        past = millis();                         // сделать временую засветку
      }
      else if ( bounce && millis() - past >= 5 ) { // если прошло антидребезговое время
        bounce = 0;                                // то снять флаг
        btn_old = btn ;
        btn = digitalRead(button1) ;   // прочитать реальное значение на выводе
        if (btn_old && ! btn) {
          flag = 1;
          past_flag = millis();
        }
        
		if (! btn_old && btn && flag && millis() - past_flag < time  ) { //быстро нажали 
		flag = 0;     

		    disp = disp + 1; //поочередно переключаем режим отображения информации
		    if(disp==7) disp = 0; //дошли до конца, начинаем снова
		    lcd.clear(); 
			
			if(set==10){//сохранение текущих настроек предцстановок 1\3 в память
				counter = 3.9;
				mode = 1;
				Ioutmax = 1;
				ri = 34;
				disp = 3;
				set = 0;
			}
			if(set==11){//сохранение текущих настроек предцстановок 2\3 в память
				counter = 5;
				mode = 1;
				Ioutmax = 1;
				ri = 34;
				disp = 3;
				set = 0;
			}
			if(set==12){//сохранение текущих настроек предцстановок 3\3 в память
				counter = 12;
				mode = 1;
				Ioutmax = 1;
				ri = 34;
				disp = 3;
				set = 0;
			}
			
			fr=0; // Выйти с предустановок
		}
	  
      if (flag && millis() - past_flag >= time) { //нажали и задержали ~1 сек
        flag = 0;      
		lcd.clear();
			 if(set<10) set = 10;
			fr = 1; // Войти в меню предустановок
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
  
  fr=0; // Выйти с предустановок
  lcd.clear();//чистим дисплей
}
//сбрасываем значения кнопок.
if(digitalRead(button2)==1&&knopka_b==1) knopka_b = 0;
if(digitalRead(button1)==1&&knopka_a==1) knopka_a = 0;



// обязательная функция отработки. Должна постоянно опрашиваться
enc1.tick();
if (enc1.isTurn()) { // если был совершён поворот (индикатор поворота в любую сторону)

}
if (enc1.isRight()) uup();//+        // если был поворот вправо >>>
if (enc1.isLeft()) udn(); //-        // если был поворот влево  <<<

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
    if(counter<9.99) lcd.print(" "); //добавляем пробел, если нужно, чтобы не портить картинку
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
      if(mode==0)lcd.print   ("Стандарт"); 
      if(mode==1)lcd.print  ("  Защита"); 
      if(mode==2)lcd.print ("Стаб.ток");
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
      if(disp==6){  // значение R
        lcd.print ("R="); 
        lcd.print (Uout / Iout,2); 
        lcd.print ("  ");
      }
      if(disp==5){  // cчетчик А*ч
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
    lcd.setCursor (0, 0);
 lcd.print(L"> Меню 1/4    ");
 lcd.setCursor (0, 1);
 lcd.print(L"Режим: ");
 //режим (0 обычный, спабилизация тока, защита по току)
 if(mode==0)  lcd.print(L" Стандарт");
 if(mode==1)  lcd.print(L" Защита  ");
 if(mode==2)  lcd.print(L" Стаб.ток");
}
  if(set==2){//настройка тока
 lcd.setCursor (0, 0);
 lcd.print(L"> Меню 2/4   ");
 lcd.setCursor (0, 1);
 lcd.print("MAX ток: ");
 lcd.print(Ioutmax);
 lcd.print("A ");
   }
 if(set==3){//спрашиваем хочет ли юзер сохранить настройки
 lcd.setCursor (0, 0);
 lcd.print(L"> Меню 3/4      ");
 lcd.setCursor (0, 1);
 lcd.print(L"Сброс A*h?   ->");
}
if(set==4){//спрашиваем хочет ли юзер сохранить настройки
 lcd.setCursor (0, 0);
 lcd.print(L"> Меню 4/4      ");
 lcd.setCursor (0, 1);
 lcd.print(L"Сохранить?   ->");
}
    if(set==10){//спрашиваем хочет ли юзер сохранить настройки
    lcd.setCursor (0, 0);
    lcd.print(L"> Пред-устан 1/3");
    lcd.setCursor (0, 1);
    lcd.print("3.9В 1А 3ащита->");
  }
    if(set==11){//спрашиваем хочет ли юзер сохранить настройки
    lcd.setCursor (0, 0);
    lcd.print(L"> Пред-устан 2/3");
    lcd.setCursor (0, 1);
    lcd.print("  5В 1А 3ащита->");
  }
    if(set==12){//спрашиваем хочет ли юзер сохранить настройки
    lcd.setCursor (0, 0);
    lcd.print("> Пред-устан 3/3");
    lcd.setCursor (0, 1);
    lcd.print(" 12В 1А 3ащита->");
  }    
  
  /* ИНДИКАЦИЯ ЗАЩИТЫ */
  if(set==5){//защита. вывод инфы
    lcd.setCursor (0, 0);
    lcd.print("[Защита по току]");
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
    lcd.print("Защита от MAXpwm");
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
    lcd.print("Что-то не так!!!");
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
      lcd.print("[  Перегрузка  ]");
      lcd.setCursor (0, 1);    
      if((Iout > (counter + 0.3)*2.0) | Iout>maxI){ //и обьясняем юзеру что случилось
          Serial.print("t1;");
              lcd.print("Iout = ");
       lcd.print(Icrash);          
      }     
    }else{   
      lcd.print("[     Выкл.    ]");
      lcd.setCursor (0, 1);
      Serial.print("t4;");
    }
  }  
}

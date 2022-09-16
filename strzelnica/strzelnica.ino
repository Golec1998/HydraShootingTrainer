#include <Adafruit_NeoPixel.h>
#include "U8glib.h"

#define ilosc_diod 4
#define sterowanie_diod 9
#define potwierdz 10
#define re1 11
#define re2 12
#define zmien 13
#define brzeczyk 6
#define czulosc 100
#define wersja "v1.0"

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI 
Adafruit_NeoPixel ledy = Adafruit_NeoPixel(ilosc_diod, sterowanie_diod, NEO_GRB + NEO_KHZ800);

/* ---------------------------------------------------------- ZMIENNE ----------------------------------------------------------------- */

const int
  czujka[5]{A0, A1, A2, A3};

int
  odczyt[5]{0, 0, 0, 0},
  poprzedniOdczyt[5]{0, 0, 0, 0},
  trafienia[2] = {0, 0},
  sumTrafienia = 0,
  punkty = 0,
  iloscTarcz = 4,
  tarcza[2] = {0, 0},
  sredniCzas = 0,
  poprzedniCzas = 0,
  ustawCzas = 0,
  opoznienie = 250,
  settime[2] = {0, 0},
  wymiana = 0,
  opcjeTryb1tab[2] = {1, 1}, iteracjaOpcji1[2] = {1, 1}, minOpcja1[2] = {1, 1}, maxOpcja1[2] = {2, 5},
  opcjeTryb2tab[3] = {1, 0, 5}, iteracjaOpcji2[3] = {1, 1, 5}, minOpcja2[3] = {1, 0, 5}, maxOpcja2[3] = {5, 10, 50},
  tryb = 1, iloscTrybow = 2,
  iloscOpcjiTrybu = sizeof(opcjeTryb1tab)/sizeof(int),
  opcja = 0,
  i = 0;
    
bool
  setStartTime = true,
  re = true,
  re1o = true,
  re2o = true,
  czyWymienic = false,
  z = true;

String
  tryby[2] = {"Tryb 1", "Tryb 2"},
  opcjeTryb1[2] = {"Gracze", "W. traf."},
  opcjeTryb2[3] = {"W. traf.", "Il. wym.", "Max. traf."},
  wyswietl = "";

void tryb1();
void tryb2();

/* ---------------------------------------------------------- EKRAN ----------------------------------------------------------------- */

void ekran(String coWyswietlic = "")
{
  u8g.setFont(u8g_font_unifont);

  if(coWyswietlic == "powitanie")
  {
    u8g.setPrintPos(30, 30);
    u8g.print("Strzelnica");
    u8g.setPrintPos(30, 44);
    u8g.print(wersja);
  }
  else
  {
    u8g.setPrintPos(5, 20);
    u8g.print(coWyswietlic);
  }
}

/* ---------------------------------------------------------- SETUP ----------------------------------------------------------------- */

void setup()
{
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }

  for(i = 0; i < 4; i++)
  {
    pinMode(czujka[i], INPUT);
  }
  pinMode(sterowanie_diod, OUTPUT);
  pinMode(brzeczyk, OUTPUT);
  pinMode(potwierdz, INPUT_PULLUP);
  pinMode(re1, INPUT_PULLUP);
  pinMode(re2, INPUT_PULLUP);
  pinMode(zmien, INPUT_PULLUP);
  
  Serial.begin(9600);
  ledy.begin();

  u8g.firstPage();
  do{
    ekran("powitanie");
  }while(u8g.nextPage());
  delay(2500);
  for(i = 0; i < 5; i++)
    ledy.setPixelColor(i, 0, 0, 0);
  ledy.show();
  tone(brzeczyk, 1000, 200);
}

/* ---------------------------------------------------------- LOOP ----------------------------------------------------------------- */

void loop()
{
  re1o = digitalRead(re1);
  re2o = digitalRead(re2);

  if(re2o)
    re = true;
  else if(re)
  {
    if(re1o)
    {
      opcja--;
    }
    else
    {
      opcja++;
    }
    re = false;
  }
  switch(tryb)
  {
    case 1: iloscOpcjiTrybu = sizeof(opcjeTryb1tab)/sizeof(int) + 1; break;
    case 2: iloscOpcjiTrybu = sizeof(opcjeTryb2tab)/sizeof(int) + 1; break;
  }
  if(opcja < 0)
    opcja = iloscOpcjiTrybu;
  else if(opcja > iloscOpcjiTrybu)
    opcja = 0;

  if(digitalRead(zmien) == LOW && z)
  {
    if(opcja == 0)
    {
      tryb++;
      if(tryb > iloscTrybow)
        tryb = 1;
    }
    else if(opcja == 1)
    {
      iloscTarcz++;
      if(iloscTarcz > 4)
        iloscTarcz = 1;
    }
    else
      switch(tryb)
      {
        case 1:
        {
          opcjeTryb1tab[opcja - 2] += iteracjaOpcji1[opcja - 2];
          if(opcjeTryb1tab[opcja - 2] > maxOpcja1[opcja - 2])
            opcjeTryb1tab[opcja - 2] = minOpcja1[opcja - 2];
        }break;
        case 2:
        {
          opcjeTryb2tab[opcja - 2] += iteracjaOpcji2[opcja - 2];
          if(opcjeTryb2tab[opcja - 2] > maxOpcja2[opcja - 2])
            opcjeTryb2tab[opcja - 2] = minOpcja2[opcja - 2];
        }break;
      }
    z = false;
  }
  else
    z = true;
    
  if(opcja == 0)
    wyswietl = tryby[tryb - 1];
  else if(opcja == 1)
    wyswietl = "I. Tarcz: " + String(iloscTarcz);
  else
    switch(tryb)
    {
      case 1:
      {
        wyswietl = opcjeTryb1[opcja - 2] + ": " + String(opcjeTryb1tab[opcja - 2]);
      }break;
      case 2:
      {
        wyswietl = opcjeTryb2[opcja - 2] + ": " + String(opcjeTryb2tab[opcja - 2]);
      }break;
    }
    
  u8g.firstPage();
  do {
    ekran(wyswietl);
  }while(u8g.nextPage());

  if(digitalRead(potwierdz) == LOW)
    switch(tryb)
    {
      case 1: tryb1(); break;
      case 2: tryb2(); break;
    }

  delay(20);
}

/* ---------------------------------------------------------- TRYB 1 ----------------------------------------------------------------- */

void tryb1()
{
  tone(brzeczyk, 1000, 1000);
  u8g.firstPage();
  do {
    ekran("Start Tryb 1");
  }while(u8g.nextPage());
  
  trafienia[0] = 0;
  trafienia[1] = 0;
  
  for(i = 0; i < iloscTarcz; i++)
    ledy.setPixelColor(i, 255, 255, 255);
  ledy.show();
  delay(3000);
  for(i = 0; i < iloscTarcz; i++)
    ledy.setPixelColor(i, 0, 0, 0);
  delay(1000);
  tarcza[0] = random(iloscTarcz);
  if(opcjeTryb1tab[0] == 2)
    do
      tarcza[1] = random(iloscTarcz);
    while(tarcza[1] == tarcza[0]);
  else
    tarcza[1] = -1;
  for(i = 0; i < iloscTarcz; i++)
    if(i == tarcza[0])
      ledy.setPixelColor(i, 255, 0, 0);
    else if(i == tarcza[1])
      ledy.setPixelColor(i, 0, 255, 0);
    else
      ledy.setPixelColor(i, 0, 0, 0);

  while(digitalRead(zmien) == HIGH)
  {
    u8g.firstPage();
    do {
      ekran("Aktywny Tryb 1");
    }while(u8g.nextPage());
    
    for(i = 0; i < iloscTarcz; i++)
      odczyt[i] = analogRead(czujka[i]);

    Serial.print(odczyt[0]);
    Serial.print(" ");
    Serial.print(odczyt[1]);
    Serial.print(" ");
    Serial.print(odczyt[2]);
    Serial.print(" ");
    Serial.println(odczyt[3]);
      
    if(odczyt[tarcza[0]] > poprzedniOdczyt[tarcza[0]] && odczyt[tarcza[0]] > czulosc)
      trafienia[0]++;
    if(trafienia[0] >= opcjeTryb1tab[1])
    {
      trafienia[0] = 0;
      for(i = 0; i < iloscTarcz; i++)
        if(i != tarcza[1])
          ledy.setPixelColor(i, 0, 0, 0);
      settime[0] = millis() / 10;
      do  
        tarcza[0] = random(iloscTarcz);
      while(tarcza[0] == tarcza[1]);
    }
    
    if(opcjeTryb1tab[0] == 2)
    {
      if(odczyt[tarcza[1]] > poprzedniOdczyt[tarcza[1]] && odczyt[tarcza[1]] > czulosc)
        trafienia[1]++;
      if(trafienia[1] >= opcjeTryb1tab[1])
      {
        trafienia[1] = 0;
        for(i = 0; i < iloscTarcz; i++)
          if(i != tarcza[0])
            ledy.setPixelColor(i, 0, 0, 0);
        settime[1] = millis() / 10;
        do  
          tarcza[1] = random(iloscTarcz);
        while(tarcza[1] == tarcza[0]);
      }
    }
    
    if(millis() / 10 - settime[0] > opoznienie / 10)
      ledy.setPixelColor(tarcza[0], 255, 0, 0);
    if(millis() / 10 - settime[1] > opoznienie / 10 && opcjeTryb1tab[0] == 2)
      ledy.setPixelColor(tarcza[1], 0, 255, 0);
    
  
    for(i = 0; i < iloscTarcz; i++)
      poprzedniOdczyt[i] = odczyt[i];
    delay(10);
    ledy.show();
  }
  
  u8g.firstPage();
  do {
    ekran("Koniec Tryb 1");
  }while(u8g.nextPage());
  
  for(i = 0; i < iloscTarcz; i++)
    ledy.setPixelColor(i, 255, 255, 255);
  ledy.show();
  delay(3000);
  for(i = 0; i < iloscTarcz; i++)
    ledy.setPixelColor(i, 0, 0, 0);
  ledy.show();
  tone(brzeczyk, 1000, 1000);
}

/* ---------------------------------------------------------- TRYB 2 ----------------------------------------------------------------- */

void tryb2()
{
  tone(brzeczyk, 1000, 1000);
  u8g.firstPage();
  do {
    ekran("Start Tryb 2");
  }while(u8g.nextPage());
  
  trafienia[0] = 0;
  sumTrafienia = 0;
  wymiana = 0;
  
  for(i = 0; i < iloscTarcz; i++)
    ledy.setPixelColor(i, 255, 255, 255);
  ledy.show();
  delay(3000);
  for(i = 0; i < iloscTarcz; i++)
    ledy.setPixelColor(i, 0, 0, 0);
  delay(1000);
  tarcza[0] = random(iloscTarcz);
  
  for(i = 0; i < iloscTarcz; i++)
    if(i == tarcza[0])
      ledy.setPixelColor(i, 255, 0, 0);
    else
      ledy.setPixelColor(i, 0, 0, 0);

  while(digitalRead(zmien) == HIGH && sumTrafienia < opcjeTryb2tab[2])
  {
    u8g.firstPage();
    do {
      ekran("Aktywny Tryb 2");
    }while(u8g.nextPage());
    
    for(i = 0; i < iloscTarcz; i++)
      odczyt[i] = analogRead(czujka[i]);

    Serial.print(odczyt[0]);
    Serial.print(" ");
    Serial.print(odczyt[1]);
    Serial.print(" ");
    Serial.print(odczyt[2]);
    Serial.print(" ");
    Serial.print(odczyt[3]);
    Serial.print(" ");
    Serial.println(odczyt[4]);
      
    if(odczyt[tarcza[0]] > poprzedniOdczyt[tarcza[0]] && odczyt[tarcza[0]] > czulosc)
    {
      if(czyWymienic)
      {
        ledy.setPixelColor(tarcza[0], 255, 0, 0);
        czyWymienic = false;
      }
      else
        trafienia[0]++;
      tone(brzeczyk, 1000, 200);
    }
    if(trafienia[0] >= opcjeTryb2tab[0])
    {
      trafienia[0] = 0;
      for(i = 0; i < iloscTarcz; i++)
        ledy.setPixelColor(i, 0, 0, 0);
      settime[0] = millis() / 10;
      tarcza[0] = random(iloscTarcz);
      sumTrafienia++;
      if(random(opcjeTryb2tab[2]) < opcjeTryb2tab[1] && wymiana < opcjeTryb2tab[1])
      {
        czyWymienic = true;
        wymiana++;
      }
    }

    if(millis() / 10 - settime[0] > opoznienie / 10 && czyWymienic)
      ledy.setPixelColor(tarcza[0], 0, 255, 0);
    else if(millis() / 10 - settime[0] > opoznienie / 10)
      ledy.setPixelColor(tarcza[0], 255, 0, 0);
  
    for(i = 0; i < iloscTarcz; i++)
      poprzedniOdczyt[i] = odczyt[i];
    delay(10);
    ledy.show();
  }
  
  u8g.firstPage();
  do {
    ekran("Koniec Tryb 2");
  }while(u8g.nextPage());
  
  for(i = 0; i < iloscTarcz; i++)
    ledy.setPixelColor(i, 255, 255, 255);
  ledy.show();
  delay(3000);
  for(i = 0; i < iloscTarcz; i++)
    ledy.setPixelColor(i, 0, 0, 0);
  ledy.show();
  tone(brzeczyk, 1000, 1000);
}

/* 
 * Svetlyachok Mid
 *
 * ESP 8266 D1 mini
 * Дисплей ILI9486 240 х 320 - Библиотека TFT_eSPI: User_Setup.h - #define ILI9486_DRIVER  \\\ tft.setRotation(1) или tft.setRotation(3);
 * Дисплей ILI9341 240 х 320 - Библиотека TFT_eSPI: User_Setup.h - #define ILI9341_DRIVER  \\\ tft.setRotation(0) или tft.setRotation(2);
 * 
 * 
 * 
 * 
 * Источник:
 * Author: Ken Ross
 * https://github.com/verykross
 *
 *
 *
 * pins:   TFT SPI ILI9341   ------------ TOUCH ----------- Wemos D1 and ESP8266 
 *
 *        Красный     VCC    ------------------------------  3V3  // в некоторых источниках 5V
 *        Чёрный      GND    ------------------------------  GND 
 *        Желтый      CS     ------------------------------  D8
 *        Голубой     RST    ------------------------------  RST
 *        Зелёный     DC     ------------------------------  D3
 *    Г --Белый       MOSI   -----------  T_DIN  ----------  D7
 *    | Г Коричневый  SCK    -----------  T_CLK  ----------  D5       
 *    | | Серый       LED (BL) ----------------------------  D1  // управление яркостью дисплея 
 * Г- | | Оранжевый   MISO   -----------  T_DO   ----------  D6 
 * |  | |                                   
 * |  | |                                  TOUCH                                
 * |  | L Коричневый  T_CLK                                  D5
 * |  |   Фиолетовый  T_CS   ------------------------------  D2
 * |  L --Белый       T_DIN                                  D7
 * L------Оранжевый   T_DO                                   D6 
 *                    T_IRQ           не подключен
*/

#define FIRMWARE_VERSION "0.3.2"  // Current version of Svetlyachok, displayed at startup

#include <ESP8266WebServer.h>

#include <ArduinoJson.h>
#include <string.h>
#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESP8266mDNS.h>
#include <Time.h>
#include <EEPROM.h>
#include <ESP8266HTTPUpdateServer.h>  // для OTA обновлений

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

// ======== Gyver библиотеки - БД + Settings ========

#include <GyverDBFile.h>
#include <LittleFS.h>
// база данных для хранения настроек
// будет автоматически записываться в файл при изменениях
GyverDBFile db(&LittleFS, "/data.db");

#include <SettingsGyver.h>
// указывается заголовок меню, подключается база данных
SettingsGyver sett("Светлячок", &db);

// ключи для хранения в базе данных
DB_KEYS(
    kk,
    wifi_ssid,
    wifi_pass,
    apply_btn,
    ns_url,
    ns_token,
    timezone,
    yellow_low,
    yellow_high,
    red_low,
    red_high,
    hour_day,
    hour_night,
    bright_day,
    bright_night,
    rotation_mon
);

// билдер! Тут строится наше окно настроек
void build(sets::Builder& b) {
    // можно узнать, было ли действие по виджету
    if (b.build.isAction()) {
        Serial.print("Set: 0x");
        Serial.print(b.build.id, HEX);
        Serial.print(" = ");
        Serial.println(b.build.value);
    }

    {
        sets::GuestAccess g(b); // Настройки Wi-Fi
        b.Input(kk::wifi_ssid, "Имя Wi-Fi");
        b.Pass(kk::wifi_pass, "Пароль Wi-Fi");
        if (b.Button(kk::apply_btn, "Сохранить")) {
            db.update();  // сохраняем БД не дожидаясь таймаута
            ESP.restart();
        }
    }

    {
        sets::GuestAccess g(b); // Настройки Nightscout (Найтскаут)
        b.Input(kk::ns_url, "URL адрес NS");
        b.Pass(kk::ns_token, "Токен NS (опционально)");
//        b.LED(kk::led);
//        b.Paragraph(kk::paragr);
//        b.Input(kk::input);
//        b.Input(kk::ssid, "SSID");
//        b.Pass(kk::password);
//        b.Pass(kk::pass);
//        b.Color(kk::color);
//        b.Switch(kk::sw);
//        b.Date(kk::datew);
//        b.Time(kk::timew);
//        b.DateTime(kk::datetime);
//        b.Slider(kk::timezone1);
//        b.Select(kk::sel, "", "foo;bar;test");
//        if (b.Button(kk::btn)) Serial.println("btn 0");
    }

    {
        sets::GuestAccess g(b);  // Настройки устройства
//        b.Input(kk::timezone, "Часовой пояс");
        b.Slider(kk::timezone, "Часовой пояс", +2, +12, 1);
        b.Input(kk::yellow_low, "Предупредительный порог о низкой ГК");
        b.Input(kk::yellow_high, "Предупредительный порог о высокой ГК");
        b.Input(kk::red_low, "Alarm порог о низкой ГК");
        b.Input(kk::red_high, "Alarm порог о высокой ГК");
//        b.LED(kk::led);
//        b.Paragraph(kk::paragr);
//        b.Input(kk::input);
//        b.Input(kk::ssid, "SSID");
//        b.Pass(kk::password);
//        b.Pass(kk::pass);
//        b.Color(kk::color);
//        b.Switch(kk::sw);
//        b.Date(kk::datew);
//        b.Time(kk::timew);
//        b.DateTime(kk::datetime);
//        b.Slider(kk::slider);
//        b.Select(kk::sel, "", "foo;bar;test");
//        if (b.Button(kk::btn)) Serial.println("btn 0");
    }


    {
        sets::GuestAccess g(b);  // Яркость дисплея
        b.Slider(kk::hour_day, "Время увеличения яркости, час.", 5, 10, 1);
        b.Slider(kk::hour_night, "Время снижения яркости, час.", 19, 23, 1);
        b.Slider(kk::bright_day, "Яркость днём, 0 - 255", 5, 255, 5);
        b.Slider(kk::bright_night, "Яркость ночью, 0 - 255", 5, 255, 5);
    }

//    if (b.Switch(kk::rotation_mon, "Переворот экрана")) {  // виден только админу
//       db.update();  // сохраняем БД не дожидаясь таймаута
//       ESP.restart();
//    }

    b.Switch(kk::rotation_mon, "Перевёрнутый экран");  // виден только админу

}


// это апдейтер. Функция вызывается, когда вебморда запрашивает обновления
void update(sets::Updater& upd) {
    // можно отправить значение по имени (хэшу) виджета
//    upd.update(kk::lbl1, random(100));
//    upd.update(kk::lbl2, millis());

    // примечание: при ручных изменениях в базе данных отправлять новые значения не нужно!
    // библиотека сделает это сама =)
}

// ======== Конец объявления Gyver библиотек - БД + Settings ========

#include "timesNewRoman24.h"
#include "timesNewRoman48.h"

// Имена шрифтов являются ссылками на массивы, поэтому их НЕ следует заключать в кавычки ""
#define FONT_RUS_24 timesNewRoman24
#define FONT_RUS_48 timesNewRoman48

#include <TFT_eSPI.h> 
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();  

#define CALIBRATION_FILE "/TouchCalData1" // имя файла, используемое для хранения данных калибровки. Можно изменить его, чтобы создать новые файлы калибровки. Имя файла SPIFFS должно начинаться с «/»
#define REPEAT_CAL false // REPEAT_CAL = true вместо false, чтобы запустить калибровку еще раз, в противном случае она будет выполнена только один раз
//#define REPEAT_CAL true  // для повторной калибровки
#define LED_BRIGHTNESS 150     // яркость дисплея при старте
#define NSAPI_INTERVAL 60000UL  // интервал между запросами NS API (60 секунд)

// настройки яркости дисплея
//byte slider_day   = 255;           // яркость день 0 - 255
//byte slider_night = 25;           // яркость ночь 0 - 255
//byte night_       = 22;            // ночь - время уменьшения яркости дисплея
//byte day_         = 7;             // день - время увеличения яркости дисплея

IPAddress local_IP(10,10,10,10);
IPAddress gateway(10,10,10,1);
IPAddress subnet(255,255,255,0);

///ESP8266WebServer server(80);
//ESP8266HTTPUpdateServer httpUpdater;  // для OTA обновлений

WiFiEventHandler stationConnectedHandler;

WiFiClientSecure client;  // Создаём объект клиента класса WiFiClientSecure

int nsConnectCount = 0;
int nsRetryDelay = 15;            // Первоначальная задержка повтора в секундах
int nsRetryMax = 9;               // Max # of retries before asking for help // Максимальное количество попыток перед обращением за помощью
int nsRetryNoticeThreshold = 20;  // The point at which the display will indicate a retry delay // Точка, в которой на дисплее будет указана задержка повтора

bool askReset = false;
bool nsConnectFailed = false;
bool nsFirstUpdate = true;
bool wifiInitialized = true;
bool connectionValidated = false;
bool apMode = false;
bool failDataNS = false;          // Если данные из NS не обновляются более заданного порога (10 мин)
bool dayTime = false;  // Дневное время

//unsigned long curMiliCount = 0;  // Ранее применялась в секции loop
//unsigned long prevMiliCount = 0;  // Ранее применялась в секции loop
//int nsApiInterval = 60000;        // Вызвать NS API каждую минуту // Ранее применялась в секции loop
long long tmptime = 0;   ///////////////// временная величина для хранения времени последнего обновления данных

//struct settings {
//  int initialized;
//  char ssid[30];            // имя wi-fi сети
//  char password[30];        // пароль wi-fi сети
//  char nsurl[50];           // url адрес NS
//  char nstoken[50];         // токен NS (опционально)
//  char timezone[10];        // часовой пояс
//  char yellow_low[10];      // значение гликемии ниже этого порога будет отображаться жёлтым цветом
//  char yellow_high[10];     // значение гликемии выше этого порога будет отображаться жёлтым цветом
//  char red_low[10];         // значение гликемии ниже этого порога будет отображаться красным цветом
//  char red_high[10];        // значение гликемии выше этого порога будет отображаться красным цветом 
//} user_settings = {};

//const int errorLedPin = 14; // это соответствует выводу D5 на ESP8266 (будущее использование)
//const int busyLedPin = 12;  // это соответствует выводу D6

// Определить NTP-клиент, чтобы мы могли получить текущее время UTC
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// This are the specific API value being retrieved; changing this will require // Это конкретное извлекаемое значение API; для его изменения потребуется
// changes to the parseReadings() method so best not to change it. // изменения в методе parseReadings(), поэтому лучше его не менять.
char apiName[] = "/api/v2/properties/bgnow,delta.display,pump.data,pump.uploader";
String serverName = "";
String apiToken = "";

// The following 5 values represent what is presented on the display // Следующие 5 значений представляют то, что отображается на дисплее
String batteryLevel=""; // Battery level of the phone running Loop // Уровень заряда батареи телефона, работающего в режиме Loop
int battery = 0;       /////// Уровень заряда батареи телефона, работающего в режиме Loop
String lastUpdate="";   // How long since last Nightscout update // Сколько времени прошло с последнего обновления Nightscout
String bgLevel="";      // Current Blood Glucose level // Текущий уровень глюкозы в крови
String insLevel="";     // Insuline level reported by pump // Уровень инсулина, сообщаемый помпой
int insLevelPmp = 0;     // Уровень инсулина в помпе в формате Int
String arrowDir="";     // Arrow trend direction // Стрелка направления тренда
String levelDelta="";   // Delta change amount // Сумма изменения дельты
float BG = 0;             // Текущее значение ГК в формате float   ///// ЦИКЛИЧЕСКИЙ РЕБУТ ВОЗМОЖНО ИЗ ЗА ЭТОГО !!!!!!!!!!!!!!!
//String lastUpdate1="";  /////////// для отладки
//String timestamp="";  /////////// для отладки

//------------------------------------------------------------------------------------------

void setup(void) {

  Serial.begin(115200); // Use serial port
//  pinMode(TFT_BL, OUTPUT);

    // ======== SETTINGS ========
  sett.begin();
  sett.onBuild(build);
  sett.onUpdate(update);

  sett.setPass(F("diamonitor"));  // пароль веб морды

    // настройки вебморды
    // sett.config.requestTout = 3000;
    // sett.config.sliderTout = 500;
    // sett.config.updateTout = 1000;

    // ======== DATABASE ========
#ifdef ESP32
  LittleFS.begin(true);
#else
  LittleFS.begin();
#endif
    // запуск БД и чтение из файла
  db.begin();
    // инициализация БД начальными значениями
  db.init(kk::wifi_ssid, "");
  db.init(kk::wifi_pass, "");
  db.init(kk::ns_url, "");
  db.init(kk::ns_token, "");
  db.init(kk::timezone, "+3");
  db.init(kk::yellow_low, "5.0");
  db.init(kk::yellow_high, "8.0");
  db.init(kk::red_low, "4.0");
  db.init(kk::red_high, "10.0");
  db.init(kk::hour_day, "7");
  db.init(kk::hour_night, "22");
  db.init(kk::bright_day, "255");
  db.init(kk::bright_night, "25");
  db.init(kk::rotation_mon, false);
//    db.init(kk::pass, "some pass");
//    db.init(kk::uintw, 64u);
//    db.init(kk::intw, -10);
//    db.init(kk::int64w, 1234567ll);
//    db.init(kk::color, 0xff0000);
//    db.init(kk::toggle, (bool)1);
//    db.init(kk::selectw, (uint8_t)1);
//    db.init(kk::date, 1719941932);
//    db.init(kk::timew, 60);
//    db.init(kk::datime, 1719941932);

  db.dump(Serial);

    // ========

  analogWrite(TFT_BL, LED_BRIGHTNESS); // первоначальная яркость дисплея
//  tft.init(); // Initialise the TFT screen
  tft.begin();

// Установите поворот экрана перед калибровкой
  if(db[kk::rotation_mon]) {  // перевёрнутый экран  
    tft.setRotation(2);  // перевёрнутый экран
    } else {
      tft.setRotation(0); // НЕ перевёрнутый экран
    }  

//  tft.setRotation(2); /// ILI9341
//  tft.setRotation(0); // ILI9341 240 х 320  
  touch_calibrate(); // Калибровка сенсорного экрана и получение коэффициентов масштабирования
  tft.fillScreen(TFT_BLACK); // очистка экрана (заливка чёрным цветом)

//  if (!SPIFFS.begin()) {  // Инициализируем файловую систему для загрузки русского шрифта /// Старый метод через SPIFS !!!
//      while (1) yield();
//  }      


//  tft.loadFont("timesNewRoman24"); // Загружаем русский шрифт // Стараый метод
  tft.loadFont(FONT_RUS_24); // Загружаем русский шрифт

  tft.setTextColor(TFT_WHITE,TFT_BLACK); // TFT_BLACK нужно использовать для загружаемых RUS шрифтов со сглаживанием
 
    // По умолчанию фоновая заливка отключена
    // Добавление параметра «true» к функции setTextColor() заполняет фон символа
    // Этот дополнительный параметр предназначен только для сглаженных шрифтов!
//  tft.setTextColor(TFT_GREEN, TFT_BLACK, true);

//  tft.setTextSize(1);   // Для Eng шрифтов
//  tft.setTextFont(4);   // Для Eng шрифтов

  // Let the user know we're awake // Дайте пользователю знать, что мы проснулись
  tft.setCursor(0, 20);

  tft.println("Светлячок");
//  tft.setCursor(0,32);
  tft.println("запускается...");
//  tft.setTextSize(1);
  tft.println();
  tft.print("Version ");
  tft.println(FIRMWARE_VERSION);

  // Setup the pin to control the Busy LED // Настройте вывод для управления светодиодом Busy
//  pinMode(busyLedPin, OUTPUT);
//  pinMode(LED_BUILTIN, OUTPUT);
//  digitalWrite(LED_BUILTIN, HIGH);

//  EEPROM.begin(sizeof(struct settings) );
//  EEPROM.get( 0, user_settings );

   // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
 // timeClient.setTimeOffset(25200);  
//  timeClient.setTimeOffset(atoi(user_settings.timezone) * 3600); // atoi() преобразует char в integer 
  timeClient.setTimeOffset(db[kk::timezone].toInt() * 3600); // String.toInt() преобразует String в int 

//  serverName = user_settings.nsurl;
//  if(String(user_settings.nstoken) != ""){
//    apiToken = "?Token=" + String(user_settings.nstoken);
//  }

  serverName = String(db[kk::ns_url]);
  if(String(db[kk::ns_token]) != ""){
    apiToken = "?Token=" + String(db[kk::ns_token]);
  }

//  if(user_settings.initialized != 1) {
//    wifiInitialized = false;
//  } else {
//    wifiInitialized = true;
//  }
  
  Serial.println("Svetlyachok");
  Serial.print("Version ");
  Serial.println(FIRMWARE_VERSION);
  Serial.println("---------------------");

//  Serial.printf("Connecting to WiFi... %s \n", user_settings.ssid);
  Serial.printf("Connecting to WiFi... %s \n", db[kk::wifi_ssid]);
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.hostname("Svetlyachok");
//  WiFi.begin(user_settings.ssid, user_settings.password);
  WiFi.begin(db[kk::wifi_ssid], db[kk::wifi_pass]);

  int tries = 0;
  apMode = false;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.printf("%i (%i)...", tries, WiFi.status());
    if (tries++ > 20) {
      Serial.println("Switching to Access Point mode ... ");
      WiFi.disconnect(true);
      delay(1000);
      Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
      WiFi.mode(WIFI_AP);
      WiFi.softAP("Svetlyachok", "12345678");
      Serial.println("Starting Access Point at 10.10.10.10 ...");
      stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
      apMode = true;
      break;
    }
  }

  if(WiFi.getMode() == WIFI_AP){
    Serial.println("Could not connect to WiFi - ask user to connect to Svetlyachok.");

    tft.fillScreen(TFT_BLACK);    
    tft.setCursor(0, 20);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE); 
    tft.println("Подключитесь к WiFi");
    tft.println("точке доступа:");
    tft.println();
  //  tft.setCursor(0, 16);
    tft.println("Svetlyachok");
    tft.println();
  //  tft.setCursor(0, 32);
    tft.println("Пароль: 12345678");

  } else {
    Serial.println("Connected to WiFi: ");
    printWiFiStatus();

    // Since we're connected to WiFi, let's display our IP address // Поскольку мы подключены к WiFi, давайте отобразим наш IP-адрес
    // In case the user wants to connect to our configuration portal // В случае, если пользователь захочет подключиться к нашему порталу конфигурации

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 20);
//    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE,TFT_BLACK); 
    tft.println("Светлячок");
  //  tft.setCursor(0, 32);
    tft.println("подключен к Wi-Fi...");
//    tft.setTextSize(1);
    tft.println();
    tft.println("IP адрес:");
    tft.println();
    tft.setTextColor(TFT_GREEN, TFT_BLACK, true); // Доп параметр true - если нужен цветной текст для сглаженных шрифтов
    tft.println(WiFi.localIP());
    MDNS.begin("diamonitor");

    // Setup mDNS so we can serve our web page at http://bgbuddy.local/ // Настройте mDNS, чтобы мы могли обслуживать нашу веб-страницу по адресу http://diamonitor.local/
    // which is easier to remember than our IP address. Note that this // который легче запомнить, чем наш IP-адрес. Обратите внимание, что это
    // will only work if you have one Svetlyachok on your network, otherwise // будет работать только если в вашей сети есть один Svetlyachok, в противном случае
    // you'll still need to use the IP address to access each one. // вам все равно придется использовать IP-адрес для доступа к каждому из них.
//    MDNS.begin("svetlyachok");
    delay(2000);  // Задержка чтобы увидеть на экране IP адрес
  }

  tft.unloadFont();  // Выгружаем русский шрифт, освобождаем память

  // Start the internal web server for the configuration page.
//  server.on("/",       [](){web();});
///  server.on("/",  web);
//  server.on("/",  handlePortal);
///  httpUpdater.setup(&server);
///  server.begin();

  client.setInsecure();

}

//------------------------------------------------------------------------------------------

void loop(void) {

  MDNS.update();

  // тикер, вызывать в лупе
  sett.tick();

///  server.handleClient();

  // If user had connected to the Svetlyachok access point, // Если пользователь подключился к точке доступа «Светлячок»,
  // remind them of the IP address to browse to for setup. // напомните им IP-адрес, по которому следует перейти для настройки.
  if (askReset == true) {

//    tft.loadFont("timesNewRoman24"); // Загружаем русский шрифт
    tft.loadFont(FONT_RUS_24); // Загружаем русский шрифт

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);  
    tft.setCursor(0, 20);
//    tft.setTextSize(1);
//    tft.setTextColor(TFT_WHITE);
    tft.println("Вы подключились к");
    tft.println("WiFi   Svetlyachok");
    tft.println();
    tft.println();
    tft.println("Для первоначальной");
    tft.println("настройки параметров");
    tft.println("наберите в браузере");
    tft.println("адрес:");
    tft.println();
    tft.println("10.10.10.10");  

    tft.unloadFont();  // Выгружаем русский шрифт, освобождаем память

    askReset = false;
  }  

  // Make the API call immediately after startup or roughly every minute // // Вызов API выполняется сразу после запуска или примерно каждую минуту
//  curMiliCount = millis();
//  if(nsFirstUpdate || curMiliCount - prevMiliCount >= nsApiInterval){
//    prevMiliCount = curMiliCount;
//    if(apMode == false){
//      checkNSapi();
//    }
//  }
  PeriodCheckNS(NSAPI_INTERVAL); // Вызов NS API - выполняется сразу после запуска или каждую минуту

}

//------------------------------------------------------------------------------------------

// Turns on/off LEDs to indicate activity/busy status // // Включает/выключает светодиоды для индикации активности/занятости
void showBusy(bool ledOn){
  if(ledOn){
//    digitalWrite(LED_BUILTIN, LOW);
//    digitalWrite(busyLedPin, HIGH); // Busy LED On
  } else {
//    digitalWrite(LED_BUILTIN, HIGH);
//    digitalWrite(busyLedPin, LOW); // Busy LED Off
  }
}

//------------------------------------------------------------------------------------------

//  Вызывает API Nightscout для получения обновленной информации
void checkNSapi(){

  if(wifiInitialized) {
    Serial.println("--- Calling Nightscout API ...");
    nsFirstUpdate = false;

    // If we've previously connected, the settings are likely correct so adjust our patience // // Если мы уже подключались, настройки, скорее всего, верны, поэтому наберитесь терпения
    if(connectionValidated) {
      nsRetryDelay = 30;
      nsRetryMax = 50;
    }
  
    showBusy(true);

    if (!client.connect(serverName, 443) && !nsConnectFailed) {  // Если нет соединения с NS, например из-за отсутвия Wi-Fi 
      Serial.printf("Attempt %i of %i failed.\n", nsConnectCount + 1, nsRetryMax);
      showBusy(false);

      failDataNS = true;    // Флаг отсутствия связи с NS для зачёркивания показаний ГК 
      lastUpdate = getElapsedTime(tmptime);  /// Для обновления времени отсутствия связи
      displayInfo();   //// Для одновления показаний

//      Serial.print("lastUpdate: ");  // Отображение времени обновления данных    ////////////////////////
//      Serial.println(lastUpdate);   ////////////////////////////////////////////////////////////////////
//      Serial.print("lastUpdate1: "); ///////////////////////////////////////////////////////////////////
//      Serial.println(lastUpdate1); ////////////////////////////////////////////////////////////////////

      if(connectionValidated && (nsConnectCount >= nsRetryNoticeThreshold && nsConnectCount < nsRetryMax)){

        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 20);
        tft.setTextFont(4);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE);
        tft.println("API Update");
        tft.println("Failing...");
        tft.printf("Retry %i\n", nsConnectCount);

      }

      if(nsConnectCount >= nsRetryMax) {
        nsConnectCount = 0;      
        nsConnectFailed = true;

        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 20);
        tft.setTextFont(4);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE);
        tft.println("NS Connect Fail");
        tft.println("Browse to:");
        tft.println(WiFi.localIP());
        tft.println("to check settings.");

      } 

      Serial.printf("Retry in %i seconds...\n", nsRetryDelay);
      nsConnectCount++;
      delay(nsRetryDelay * 1000);
      return;
    }
  
    String rawJSON = "";
    HTTPClient http;
    String apiStr = "https://" + serverName + apiName + apiToken;
    
    http.begin(client, apiStr);
    int respCode = http.GET();

    if (respCode > 0) {
      rawJSON = http.getString();
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(respCode);
    }

    // Read the JSON response data
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, rawJSON);

    http.end();

    if (error) {
      Serial.print(F("Failed to deserialize JSON data: "));
      Serial.println(error.f_str());

      Serial.println("------------------------------");
      Serial.print("API Str: ");
      Serial.println(apiStr);
      Serial.print("Raw JSON: ");
      Serial.println(rawJSON);

      failDataNS = true;    // Флаг отсутствия связи с NS для зачёркивания показаний ГК 
      lastUpdate = getElapsedTime(tmptime);  /// Для обновления времени отсутствия связи
      displayInfo();   //// Для обновления показаний

      return;
    } else {
      parseReadings(doc);
//      failDataNS = false;
    }

//    failDataNS = false;
    showBusy(false);
    displayInfo();

    // We know we can successfully connect and process the data - huzzah! Мы знаем, что можем успешно подключить и обработать данные – ура!
    connectionValidated = true;
    nsConnectCount = 0;
    
    Serial.println("Update Complete");
    Serial.println("------------------------------");
  }  
  
}

//------------------------------------------------------------------------------------------

// В основном используется в качестве средства отладки для подключения по WiFi.
// Выводит подключенный SSID и IP-адрес на последовательный монитор.
void printWiFiStatus() {
  // Распечатать SSID сети, к которой мы подключились
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Распечатать IP-адрес WiFi
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Распечатать уровень принятого сигнала
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.println();
}

//------------------------------------------------------------------------------------------

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  Serial.print("Station connected: ");
  Serial.println(macToString(evt.mac));
  Serial.print("AID: ");
  Serial.println(evt.aid);

  if (evt.aid > 0) {
    askReset = true;
  }
}

//------------------------------------------------------------------------------------------

// Служебная функция для возврата отформатированного MAC-адреса
String macToString(const unsigned char* mac) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

//------------------------------------------------------------------------------------------

// Этот метод вызывается, когда пользователь подключается к Svetlyachok через веб-браузер
// и предоставляет простой веб-интерфейс для настройки.
//void handlePortal() {
///void web() {  
//

//------------------------------------------------------------------------------------------

// Обновить дисплей с текущей информацией, включая рисование стрелки тренда
void displayInfo(){

  int arrowOffset = 0; // смещение стрелки в зависимости от её направления
  int XarrowOffset = 0; // смещение координаты X стрелки
  int arrowAngle = 0; // угол стрелки
  bool hasDirection = false; // рисовать первую стрелку
  bool doubleArrow = false; // рисовать вторую стрелку
  uint16_t glColor = TFT_GREEN; // цвет цифр значения ГК
  uint16_t deltaColor = TFT_WHITE; // цвет цифр значения Дельта изменения ГК
//  uint16_t batteryColor = TFT_WHITE; // цвет цифр значения Заряд аккумулятора телефона ААПС
//  uint16_t insLevelColor = TFT_WHITE; // цвет цифр значения Уровень инсулина помпы
  
  String directarr = "";

///  ---------------- Определить угол и смещение стрелок направления ГК -------------------------------------------------

  if (String(arrowDir) == "Flat") {
    hasDirection = true;
    arrowAngle = 90;
    arrowOffset = -10;
  }  else if (String(arrowDir) == "FortyFiveUp") {
    hasDirection = true;
    arrowAngle = 45;
    arrowOffset = -5;
  }  else if (String(arrowDir) == "FortyFiveDown") {
    hasDirection = true;
    arrowAngle = 135;
    arrowOffset = -20;
  } else if (String(arrowDir) == "DoubleUp") {
    hasDirection = true;
    arrowAngle = 0;
    arrowOffset = 0;
    doubleArrow = true;
  } else if (String(arrowDir) == "DoubleDown") {
    hasDirection = true;
    arrowAngle = 179;
    arrowOffset = -22;
    doubleArrow = true;
  }  else if (String(arrowDir) == "SingleUp") {
    hasDirection = true;
    arrowAngle = 0;
    arrowOffset = 0;
  }  else if (String(arrowDir) == "SingleDown") {
    hasDirection = true;
    arrowAngle = 179;
    arrowOffset = -22;
  }

///  ----------------

  tft.fillScreen(TFT_BLACK);  // Очистить экран

  timeClient.update();
  Serial.print("Formatted Time: ");
  Serial.println(timeClient.getFormattedTime());  
//  int currentHour = timeClient.getHours();
  display_brightness();       // Установить яркость экрана в зависимости от времени суток

///  ---------------- Отображение заряда батареи помпы -------------------------------------------------

  tft.setCursor(5,12);
  tft.setTextFont(4);
  tft.setTextSize(1);

  if(batteryLevel != "") {  // Если есть значение batteryLevel
    if(failDataNS) {    // Если нет показаний из NS более 10 мин
      tft.setTextColor(TFT_DARKGREY, TFT_BLACK); //  Темносерый шрифт на чёрном фоне
    } else {  // Если есть показания из NS
      tft.setTextColor(TFT_WHITE, TFT_BLACK);  //  Белый шрифт на чёрном фоне

//    if(battery>=15) {    
//      batteryColor=TFT_WHITE; // Normal
//      tft.setTextColor(TFT_WHITE, TFT_BLACK);
//    }
      if(battery<15 && battery>10) {   // меньше 15 И больше 10 
//      batteryColor=TFT_YELLOW; // warning is YELLOW
        tft.setTextColor(TFT_BLACK, TFT_YELLOW);
      }
      if(battery<=10) {
//      batteryColor=TFT_RED; // alert is RED
        tft.setTextColor(TFT_WHITE, TFT_RED);
      }
      }
  tft.print(battery);
  tft.print(" %");

  if(failDataNS) {   // Если нет показаний из NS более 10 мин
    tft.fillRect(0, 20, 70, 3, TFT_DARKGREY);  // Зачеркнуть показания % заряда батареи (прямоугольник X Y координаты левой верхней точки, ширина, высота, цвет)    
  }
  }

///  ---------------- Отображение прошедшего времени от получения данных ГК --------------------------

  tft.setCursor(5,40);
//  tft.loadFont("timesNewRoman24"); // Загружаем русский шрифт
  tft.loadFont(FONT_RUS_24); // Загружаем русский шрифт
  tft.setTextColor(TFT_WHITE,TFT_BLACK); // TFT_BLACK нужно использовать для загружаемых RUS шрифтов со сглаживанием

  if(failDataNS) {    // Если нет показаний из NS более 10 мин
//    tft.setTextColor(TFT_WHITE, TFT_RED); // Белый шрифт на красном фоне
    tft.setTextColor(TFT_YELLOW);
  } else {  // Если есть показания из NS  
//    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Белый шрифт на чёрном фоне
    tft.setTextColor(TFT_WHITE);
    }

  tft.print(lastUpdate);   // Отображение прошедшего времени от получения данных ГК
//  tft.fillRect(0, 50, 170, 5, TFT_RED);  // Закрасить прямоугольник под показаниями % заряда батареи (X Y координаты левой верхней точки, ширина, высота, цвет)   // Для настройки позиции 

//  tft.unloadFont();  // Выгружаем русский шрифт, освобождаем память

//  Serial.print("lastUpdate: ");  
//  Serial.println(lastUpdate);
//  Serial.print("lastUpdate1: ");
//  Serial.println(lastUpdate1);
//  Serial.print("timestamp: ");
//  Serial.println(timestamp);

///  ---------------- Отображение уровня инсулина в резервуаре помпы --------------------------------

  tft.setCursor(5,280);
//  tft.setTextFont(4);
//  tft.setTextSize(1);

//  if(insLevel != "") {  // Если есть значение insLevel
  if(insLevelPmp != 0) {  // Если есть значение insLevel
    if(failDataNS) {    // Если нет показаний из NS более 10 мин   //  добавить проверку if(dayTime) - если день то цвет GREY если ночь то цвет DARKGREY
      tft.setTextColor(TFT_DARKGREY, TFT_BLACK, true); // Доп параметр true - если нужен цветной текст для сглаженных шрифтов
    } else {
      tft.setTextColor(TFT_WHITE, TFT_BLACK, true); // Доп параметр true - если нужен цветной текст для сглаженных шрифтов

      if(insLevelPmp<15 && insLevelPmp>10) {   // меньше 15 И больше 10 
//        batteryColor=TFT_YELLOW; // warning is YELLOW
        tft.setTextColor(TFT_BLACK, TFT_YELLOW, true); // Доп параметр true - если нужен цветной текст для сглаженных шрифтов
//        tft.setTextColor(TFT_YELLOW);
      }
      if(insLevelPmp<=10) {
//        batteryColor=TFT_RED; // alert is RED
        tft.setTextColor(TFT_WHITE, TFT_RED, true); // Доп параметр true - если нужен цветной текст для сглаженных шрифтов
//        tft.setTextColor(TFT_RED);
      }
    }
    tft.print("Инс.:  ");
//    tft.println((insLevel) + (" Ед.")); 
    tft.print(insLevelPmp);
    tft.print(" Ед.");  

//    tft.print("Res.:  ");
//    tft.print(insLevelPmp);
//    tft.print(" U");
//    tft.print(" -"); tft.print(insLevel);
    
    }

//  } else {
//    tft.setTextColor(TFT_RED);
//    tft.print("Нет данных от помпы!");
//  }

  tft.unloadFont();  // Выгружаем русский шрифт, освобождаем память

  if((failDataNS) and (insLevelPmp != 0)) {   // Если нет показаний из NS более 10 мин И есть значение insLevel
    tft.fillRect(0, 290, 140, 3, TFT_DARKGREY);  //Зачеркнуть показания insLevel (прямоугольник X Y координаты левой верхней точки, ширина, высота, цвет)    
  }

//  tft.fillRect(0, 290, 140, 5, TFT_RED);  // Зачеркнуть показания insLevel (прямоугольник X Y координаты левой верхней точки, ширина, высота, цвет)   // Для настройки позиции 
//  tft.unloadFont();  // Выгружаем русский шрифт, освобождаем память

///  ---------------- Отображение значения ГК (уровня сахара) -------------------------------------------

  tft.setCursor(0,120); // положение на экране показаний ГК
  tft.setTextFont(8);
  tft.setTextSize(1);

  if(failDataNS) {   // Если нет показаний из NS более 10 мин
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK); //  Темносерый шрифт на чёрном фоне
    glColor=TFT_DARKGREY; // Тёмносерый цвет показаний ГК    
    deltaColor=TFT_DARKGREY; // Тёмносерый цвет показаний Дельты
//    tft.fillRect(10, 158, 200, 5, TFT_DARKGREY);  // Зачеркнуть показания ГК (прямоугольник X Y координаты левой верхней точки, ширина, высота, цвет)
  } else {  // Если есть показания из NS
    tft.setTextColor(TFT_GREEN, TFT_BLACK);  // Зелёный шрифт на чёрном фоне
    glColor=TFT_GREEN;
    deltaColor=TFT_WHITE;
//    Serial.println("ГК: зелёный порог");
//    Serial.print("ГК: ");
//    Serial.println(bgLevel);
//    Serial.print("yellow_low: ");
//    Serial.println(db[kk::yellow_low]);
//    Serial.print("yellow_high: ");
//    Serial.println(db[kk::yellow_high]); 

    // Если порог 1 сахар - жёлтые показания
//    if(BG < atof(user_settings.yellow_low) || BG > atof(user_settings.yellow_high)) {   // Лучше сюда добавить исключение - проверку внесены ли параметры в пороги сигнализации ГК
//    if(bgLevel < db[kk::yellow_low] || bgLevel > db[kk::yellow_high]) {   // Лучше сюда добавить исключение - проверку внесены ли параметры в пороги сигнализации ГК
    if(BG < db[kk::yellow_low].toFloat() || BG > db[kk::yellow_high].toFloat()) {   // Лучше сюда добавить исключение - проверку внесены ли параметры в пороги сигнализации ГК
      tft.setTextColor(TFT_YELLOW, TFT_BLACK); // warning is YELLOW 
      glColor=TFT_YELLOW;
//      Serial.println("ГК: желтый порог");
//      Serial.print("ГК: ");
//      Serial.println(bgLevel);
//      Serial.print("yellow_low: ");
//      Serial.println(db[kk::yellow_low]);
//      Serial.print("yellow_high: ");
//      Serial.println(db[kk::yellow_high]); 
    }

    // Если порог 2 сахар - красные показания
//    if(BG < atof(user_settings.red_low) || BG > atof(user_settings.red_high)) {  // Лучше сюда добавить исключение - проверку внесены ли параметры в пороги сигнализации ГК
//    if(bgLevel < string(db[kk::red_low]) || bgLevel > string(db[kk::red_high])) {  // Лучше сюда добавить исключение - проверку внесены ли параметры в пороги сигнализации ГК
    if(BG < db[kk::red_low].toFloat() || BG > db[kk::red_high].toFloat()) {  // Лучше сюда добавить исключение - проверку внесены ли параметры в пороги сигнализации ГК
      tft.setTextColor(TFT_WHITE); // alert is RED
      glColor=TFT_WHITE;
      tft.fillRect(0, 110, 240, 95, TFT_RED);  // Закрасить прямоугольник под показаниями ГК (X Y координаты левой верхней точки, ширина, высота, цвет)
//      Serial.println("ГК: красный порог");
//      Serial.print("ГК: ");
//      Serial.println(bgLevel);
//      Serial.print("red_low: ");
//      Serial.println(db[kk::red_low]);
//      Serial.print("red_high: ");
//      Serial.println(db[kk::red_high]);         
    }
    }
    
//  tft.setTextColor(glColor);
  tft.print(bgLevel);  // Отображение значения ГК

  Serial.print("ГК: ");
  Serial.println(BG);

  if(failDataNS) {   // Если нет показаний из NS более 10 мин
    tft.fillRect(10, 158, 200, 5, TFT_DARKGREY);  // Зачеркнуть показания ГК (прямоугольник X Y координаты левой верхней точки, ширина, высота, цвет)    
  }

 ///  ---------------- Отображение стрелок тенденции ГК -------------------------------------------
 
  if(BG < 10) {
    XarrowOffset = 0;
  } else{
    XarrowOffset = 35;
  }

  if (hasDirection) {
//    drawArrow(коорд Х, коорд Y, 20, arrowAngle, 24, 35, цвет);
    drawArrow(170+XarrowOffset, 170+arrowOffset, 20, arrowAngle, 24, 35, glColor);  // первая стрелка  // X = 205 при ГК больше 10
    if(doubleArrow){
      drawArrow(195+XarrowOffset, 170+arrowOffset, 20, arrowAngle, 24, 35, glColor); // вторая стрелка (если есть) // X = 230 при ГК больше 10
    }
  }

///  ---------------- Отображение дельты изменения ГК -----------------------------------------------

  if(levelDelta != "+0") {
    tft.setCursor(140,210);
//    tft.loadFont("timesNewRoman48"); // Загружаем русский шрифт
    tft.loadFont(FONT_RUS_48); // Загружаем русский шрифт
    tft.setTextColor(TFT_WHITE,TFT_BLACK); // TFT_BLACK нужно использовать для загружаемых RUS шрифтов со сглаживанием
//    tft.setTextFont(4);
//    tft.setTextSize(2);
    tft.setTextColor(deltaColor);
    tft.print(levelDelta);
    tft.unloadFont();  // Выгружаем русский шрифт, освобождаем память
  }

  if(failDataNS) {   // Если нет показаний из NS более 10 мин
//    tft.setTextColor(TFT_DARKGREY, TFT_BLACK); 
    tft.fillRect(140, 226, 90, 4, TFT_DARKGREY);  // Зачеркнуть показания Дельты (прямоугольник X Y координаты левой верхней точки, ширина, высота, цвет)   
  }  

  Serial.print("levelDelta: "); /////////////
  Serial.println(levelDelta); //////////////

}

//------------------------------------------------------------------------------------------

// Возвращает время, прошедшее с момента обновления данных, на основе предоставленного epoch time времени эпохи

String getElapsedTime(long long epochTime){
  //setTime(epochTime);
  long long epochSec = epochTime / 1000;
  unsigned long longNow = getEpochTime();

  int seconds = longNow - epochSec;
//  int minutes = (seconds / 60) - (atoi(user_settings.timezone) * 60);  // секунды переводим в минуты, часы часового пояса переводим в минуты     
  int minutes = (seconds / 60) - (db[kk::timezone].toInt() * 60);  // секунды переводим в минуты, часы часового пояса переводим в минуты  
//  int mins = minutes - (atoi(user_settings.timezone) * 60);
//  int mins = minutes;

  Serial.print("minutes: ");
  Serial.println(minutes);

  if(minutes > 10){
    Serial.println("failDataNS = true ");
    failDataNS = true;
  } else { 
    Serial.println("failDataNS = false ");
    failDataNS = false;
  }      

  String retval = "Сейчас";
  if(minutes > 0){
    if(minutes > 60) {
//      int hours = minutes / 60;
//      int hours = (minutes / 60) - atoi(user_settings.timezone);  // часы с учётом часового пояса
      int hours = (minutes / 60); 
      int min = minutes % 60;
//      retval = "As of " + String(hours) + "h " + String(min) + "m";
      retval = String(hours) + " час " + String(min) + " мин назад";
    } else{
      retval = String(minutes) + " мин назад";
    }
  }
  return retval;
}

//------------------------------------------------------------------------------------------

// Возвращает current epoch time текущее время эпохи
unsigned long getEpochTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

//------------------------------------------------------------------------------------------

// Анализирует данные JSON для текущих показаний
// Parsing code Код анализа, адаптированный из кода, сгенерированного ArduinoJson Assistant
// https://arduinojson.org/v6/assistant
void parseReadings(JsonDocument doc){

  JsonObject bgnow = doc["bgnow"];
  int bgnow_mean = bgnow["mean"]; // 79
  int bgnow_last = bgnow["last"]; // 79
  long long bgnow_mills = bgnow["mills"]; // 1678928653920.828

  JsonObject bgnow_sgvs_0 = bgnow["sgvs"][0];
  const char* bgnow_sgvs_0_id = bgnow_sgvs_0["_id"]; // "64126b175fde9abde091352c"
  int bgnow_sgvs_0_mgdl = bgnow_sgvs_0["mgdl"]; // 79
  long long bgnow_sgvs_0_mills = bgnow_sgvs_0["mills"]; // 1678928653920.828
  const char* bgnow_sgvs_0_device = bgnow_sgvs_0["device"]; // "CGMBLEKit Dexcom G6 21.0"
  const char* bgnow_sgvs_0_direction = bgnow_sgvs_0["direction"]; // "Flat"
  const char* bgnow_sgvs_0_type = bgnow_sgvs_0["type"]; // "sgv"
  double bgnow_sgvs_0_scaled = bgnow_sgvs_0["scaled"]; // 79

  const char* delta_display = doc["delta"]["display"]; // "+0"

  JsonObject pump_data = doc["pump"]["data"];
  int pump_data_level = pump_data["level"]; // -3

  JsonObject pump_data_clock = pump_data["clock"];
  const char* pump_data_clock_value = pump_data_clock["value"]; // "2023-03-16T00:59:23.000Z"
  const char* pump_data_clock_label = pump_data_clock["label"]; // "Last Clock"
  const char* pump_data_clock_display = pump_data_clock["display"]; // "6m ago"
  int pump_data_clock_level = pump_data_clock["level"]; // -3

  JsonObject pump_data_reservoir = pump_data["reservoir"];
  float pump_data_reservoir_value = pump_data_reservoir["value"]; // 42.75
  const char* pump_data_reservoir_label = pump_data_reservoir["label"]; // "Reservoir"
  const char* pump_data_reservoir_display = pump_data_reservoir["display"]; // "42.8U"
  int pump_data_reservoir_level = pump_data_reservoir["level"]; // -3

  const char* pump_data_manufacturer = pump_data["manufacturer"]; // "Insulet"
  const char* pump_data_model = pump_data["model"]; // "Eros"

  const char* pump_data_device_label = pump_data["device"]["label"]; // "Device"
  const char* pump_data_device_display = pump_data["device"]["display"]; // "loop://iPhone"

  const char* pump_data_title = pump_data["title"]; // "Pump Status"

  JsonObject pump_uploader = doc["pump"]["uploader"];
  int pump_uploader_battery = pump_uploader["battery"]; // 100
  const char* pump_uploader_timestamp = pump_uploader["timestamp"]; // "2023-03-16T00:59:23Z"
  const char* pump_uploader_name = pump_uploader["name"]; // "iPhone"
  int pump_uploader_value = pump_uploader["value"]; // 100
  const char* pump_uploader_display = pump_uploader["display"]; // "100%"
  int pump_uploader_level = pump_uploader["level"]; // 100


  // Захватываем значения, используемые при обновлении дисплея
  levelDelta = delta_display;
  batteryLevel = pump_uploader_display;
  battery = pump_uploader_battery; ////////////////////////////////////////////
//  lastUpdate = pump_data_clock_display;     // Раскомментировать если lastUpdate нужно брать из pump_data_clock_display
//  timestamp = pump_data_clock_value; /////////////////////////// Для отладки
//  lastUpdate1 = getElapsedTime(bgnow_mills); /////////////////// Для отладки
  tmptime = bgnow_mills;  /////////////////// 

  if(bgnow_sgvs_0_mgdl == bgnow_sgvs_0_scaled){
    bgLevel = String(bgnow_sgvs_0_mgdl);
  } else {
    bgLevel = String(bgnow_sgvs_0_scaled, 1);
  }

  if(bgLevel != "") {    //  Проверка на отсутствие данных bgLevel
  BG = bgLevel.toFloat();
  }

  insLevel = pump_data_reservoir_display;  // Берём значение insLevel из помпы - формат "104U"

  Serial.print("insLevel: "); /////////////
  Serial.println(insLevel); //////////////

  if(insLevel != "") {    
    insLevelPmp = atoi(pump_data_reservoir_display); // Значение insLevel в формате Int // atoi() преобразует char в integer  - формат "104"
//  insLevel = pump_data_reservoir_display;
    insLevel = String(pump_data_reservoir_value, 0); // убираем знак "U" - отделяем целую часть значения инсулина в резервуаре помпы - формат "104"
  }  

  arrowDir = bgnow_sgvs_0_direction;

//  if(lastUpdate == ""){   // Если с помпы отсутствуют данные pump_data_clock_display то 
//    lastUpdate = getElapsedTime(bgnow_mills);  // берём значение bgnow["mills"]
//  }

  lastUpdate = getElapsedTime(bgnow_mills);  // берём значение bgnow["mills"]  // Если нужно вернуть назад то раскомментировать шаг 997

  Serial.print("insLevel: "); /////////////
  Serial.println(insLevel); //////////////

  Serial.print("insLevelPmp: "); /////////////
  Serial.println(insLevelPmp); //////////////

}

//------------------------------------------------------------------------------------------

// Используется для рисования стрелок, указывающих тенденцию ГК
void drawArrow(int x, int y, int asize, int aangle, int pwidth, int plength, uint16_t color){
  aangle = aangle - 2;
  float dx = (asize-10)*cos(aangle-90)*PI/180+x; // calculate X position  
  float dy = (asize-10)*sin(aangle-90)*PI/180+y; // calculate Y position  
  float x1 = 0;         float y1 = plength;
  float x2 = pwidth/2;  float y2 = pwidth/2;
  float x3 = -pwidth/2; float y3 = pwidth/2;
  float angle = aangle*PI/180-135;
  float xx1 = x1*cos(angle)-y1*sin(angle)+dx;
  float yy1 = y1*cos(angle)+x1*sin(angle)+dy;
  float xx2 = x2*cos(angle)-y2*sin(angle)+dx;
  float yy2 = y2*cos(angle)+x2*sin(angle)+dy;
  float xx3 = x3*cos(angle)-y3*sin(angle)+dx;
  float yy3 = y3*cos(angle)+x3*sin(angle)+dy;

  tft.fillTriangle(xx1,yy1,xx3,yy3,xx2,yy2, color);
  tft.drawLine(x, y, xx1, yy1, color);
  tft.drawLine(x+1, y, xx1+1, yy1, color);
  tft.drawLine(x, y+1, xx1, yy1+1, color);
  tft.drawLine(x-1, y, xx1-1, yy1, color);
  tft.drawLine(x, y-1, xx1, yy1-1, color);
  tft.drawLine(x+2, y, xx1+2, yy1, color);
  tft.drawLine(x, y+2, xx1, yy1+2, color);
  tft.drawLine(x-2, y, xx1-2, yy1, color);
  tft.drawLine(x, y-2, xx1, yy1-2, color);
}

//------------------------------------------------------------------------------------------

void display_brightness() // яркость дисплея
{ 
//  if (( timeClient.getHours() >= day_ ) and ( timeClient.getHours() < night_ ))    
  if (( timeClient.getHours() >= db[kk::hour_day].toInt() ) and ( timeClient.getHours() < db[kk::hour_night].toInt() ))    // db[kk::timezone].toInt()      hour_day,    hour_night,
  {
    dayTime = true;
    Serial.print("Time to Day: ");
    Serial.print(timeClient.getHours());
    Serial.println(" ч.");
//    analogWrite(TFT_BL, slider_day);
    analogWrite(TFT_BL, db[kk::bright_day].toInt());
  }
  else
  {
    dayTime = false;
    Serial.print("Time to Night: ");
    Serial.print(timeClient.getHours());
    Serial.println(" ч.");
    analogWrite(TFT_BL, db[kk::bright_night].toInt());
  }
  Serial.print("Time to seconds: ");
  Serial.println(timeClient.getSeconds());

} // end display_brightness

//------------------------------------------------------------------------------------------

void RestartESP(String msg) {  // перезагрузка ESP
  Serial.println(msg);
  Serial.println("Restarting ESP...");
  ESP.reset();
}

//------------------------------------------------------------------------------------------

void PeriodCheckNS(unsigned long interval){    // Вызов NS API выполняется сразу после запуска или примерно каждую минуту
  static unsigned long prevTime = 0; // храним время последнего запроса
  if(nsFirstUpdate || millis() - prevTime > interval){ //проверяем не прошел ли нужный интервал, если прошел то
    prevTime=millis(); // сохраняем время последнего переключения
  // выполняем действия функции;
    if(apMode == false)
      checkNSapi();
  }
}

//------------------------------------------------------------------------------------------

void touch_calibrate() // калибровка сенсорного экрана
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists // проверить наличие файловой системы
//  if (!SPIFFS.begin()) {
  if (!LittleFS.begin()) {
    Serial.println("formatting file system");
//    SPIFFS.format();
//    SPIFFS.begin();
    LittleFS.format();
    LittleFS.begin();
  }

  // check if calibration file exists and size is correct  // проверьте, существует ли файл калибровки и правильный ли у него размер
//  if (SPIFFS.exists(CALIBRATION_FILE)) {
  if (LittleFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate  // Удалить, если хотим перекалибровать
//      SPIFFS.remove(CALIBRATION_FILE);
      LittleFS.remove(CALIBRATION_FILE);
    }
    else
    {
//      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      File f = LittleFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid  // данные калибровки действительны
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate  // данные недействительны, поэтому выполните повторную калибровку
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(FONT_RUS_24); // Загружаем русский шрифт
    tft.setTextColor(TFT_WHITE,TFT_BLACK); // TFT_BLACK нужно использовать для загружаемых RUS шрифтов со сглаживанием
    tft.setCursor(20, 50);

    tft.println("Необходима");  // Прикоснитесь к углам, как указано
    tft.println("калибровка экрана");
    tft.println("");
    tft.println("Прикоснитесь к углам");  // Прикоснитесь к углам, как указано
    tft.println("экрана по указанным");
    tft.println("стрелкам");    

//    tft.setTextFont(1);
//    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Установите REPEAT_CAL на false, чтобы остановить этот запуск!");  
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK, true); // Доп параметр true - если нужен цветной текст для сглаженных шрифтов
    tft.println("");
    tft.println("Калибровка завершена!");  // Калибровка завершена!    
    tft.unloadFont();  // Выгружаем русский шрифт, освобождаем память
    delay(2000);    

    // store data  // хранить данные
//    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

//------------------------------------------------------------------------------------------
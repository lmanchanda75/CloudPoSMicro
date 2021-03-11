
/*
Purpose - The purpose of the display function is to provide UI to the 
end user to know the status of processing as its proceeding and help to 
do next set of input actions.

Following are set of actions display unit is expected to do-

1. Show the status and action request on LCD screen
2. Show visual queues using LED
3. Sound status and action request on Speaker 

*/


#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Adafruit_GFX.h> //For OLED Displpay
#include <Adafruit_SSD1306.h> //For OLED display
#include <functional> 
#include <Adafruit_NeoPixel.h>
#include "SPIFFS.h"

#include <google-tts.h>
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioFileSourceSPIFFS.h"
#include "AudioOutputI2S.h"
#include "Pins.h"


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_I2C_ADDRESS 0x3C 

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      1
#define  TONE_CHANNEL 0



enum LedPattern{
 RED_BLINK

};

enum AudioPattern{
 FREQ_50HZ_100ms,
 MA_SONIC

};

class Lcd
{
  private:
    
    Adafruit_SSD1306  * oledDisplay;

  public:
   Lcd();
   void initLcd();
   void printText(const char* str);


};

class Led
{
  private:

  Adafruit_NeoPixel * pixels;

  public:
    Led();
    void initLed();
    void showLed(byte red, byte green, byte blue );

};


class Speaker
{
  private:

  AudioGeneratorMP3 *mp3;
  AudioFileSource *file;
  AudioFileSourceBuffer *buff;
  AudioOutputI2S *out;
  TTS * tts; 
  int lastms ;
  void stopPlay();
  uint16_t toneDuration;
  boolean tonePlaying;
  void playMp3(AudioFileSource * file);
  // void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string);
  // void StatusCallback(void *cbData, int code, const char *string);
  public:
  Speaker();

  void initSpeaker();
  void playTTS(const char * str);
  void playFromFile(const char * str);
  void playTone(uint16_t frequency, uint16_t duration,uint8_t repeatCount);
  void speakerLoop();


};


class Display
{
  private:
    bool ledPresent;
    bool lcdPresent;
    bool speakerPresent;
    Lcd * lcd;
    Led * led;
    Speaker * speaker;
    
public:
 
	Display(bool ledPresent,bool lcdPresent, bool speakerPresent);
	void alertUser(const char * textMessage, LedPattern ledPattern, AudioPattern audioPattern);
  void loop();
  void initDisplay();

};

#endif

#ifndef Animator_h
#define Animator_h

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Adafruit_SSD1306.h>

class Animator {
  public:
    Animator();
    void begin(Adafruit_SSD1306*, unsigned int, unsigned int, HardwareSerial*);
    void update();
    void stop();
    int addAnimation(const unsigned char* arr[], unsigned int, unsigned int, unsigned int);
    void removeAnimation(unsigned int);
    void playSequence(unsigned int, bool);
    
    // methods defined here (simple enough)
    void playSequence(unsigned int seq) {playSequence(seq, false);}
    void playLoopedSequence(unsigned int seq){playSequence(seq, true);}
    void play(){_isPlaying = true;}
    void pause(){_isPlaying = false;}
    
  private:
    Stream *_stream;
  
    // display stuff
    Adafruit_SSD1306 *_display;
    int _width;
    int _height;

    // active animation trackers
    unsigned int _currentSeq;
    const unsigned char** _currentAnimation = NULL;
    unsigned int _currentAnimationSize;
    unsigned long _currentFrameDuration; // based on 1000/fps
    unsigned int _currentAnimationColor;

    // state
    unsigned int _currentAnimationFrame;
    bool _isLooping;
    bool _isPlaying;

    // timing management
    long _lastFrameRendered;
    
    // storage of sequences
    int _seqcounter = 0;
    const unsigned char** _sequences[100]; // store up to 100 animation pointers
    unsigned int _seqsizes[100];
    unsigned int _seqfps[100];
    bool _seqcolor[100];
};
#endif
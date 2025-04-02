#include <Arduino.h>
#include "Animator.h"

Animator::Animator(){
  // nothing for now
}

void Animator::begin(Adafruit_SSD1306 *display, unsigned int width, unsigned int height, HardwareSerial* serial){
  _display = display;
  _stream = serial;
  _width = width;
  _height = height;
}

int Animator::addAnimation(const unsigned char* arr[], unsigned int arrSize, unsigned int fps, unsigned int color){
  _sequences[_seqcounter] = arr;
  _seqsizes[_seqcounter] = arrSize;
  _seqfps[_seqcounter] = fps;
  _seqcolor[_seqcounter] = color;
  _seqcounter++;
  return _seqcounter - 1;
}

void Animator::removeAnimation(unsigned int seq){
  if (seq == _currentSeq)
    _isPlaying = false;
    
  _sequences[seq] = NULL;
  _seqsizes[seq] = 0;
  _seqfps[seq] = 0;
  _seqcolor[seq] = 0;
}

void Animator::playSequence(unsigned int seq, bool loop){
  _currentSeq = seq;
  _currentAnimation = _sequences[seq];
  _currentAnimationSize = _seqsizes[seq];
  _currentFrameDuration = round(1000 / _seqfps[seq]); // convert fps to frame duration ms
  _currentAnimationColor = _seqcolor[seq];
  _currentAnimationFrame = 0;
  _isLooping = loop;
  _isPlaying = true;
}

void Animator::update(){
  if (!_isPlaying || _currentAnimation == NULL)
    return;

  // for some odd reason, printing this value seems to make it correct for the calculations in this function
  _stream->print("Frame duration: ");
  _stream->println(_currentFrameDuration);

  // check if enough time has passed to meet fps requirements to draw another frame (or skip frames [future])
  long currentMs = millis();
  long elapsed_time = currentMs - _lastFrameRendered;
  if (elapsed_time > _currentFrameDuration){

    // draw updates 
    _display->clearDisplay(); // clear frame
    // x, y, uint8_t *bitmap, width, height, color (0 or 1 [white])
    // assume animations are full screen for now
    _display->drawBitmap(0, 0, _currentAnimation[_currentAnimationFrame], _width, _height, _currentAnimationColor);
    _display->display();

    _lastFrameRendered = currentMs;

    // figure out advancement
    int frame_jump = (int) elapsed_time / _currentFrameDuration;
    _currentAnimationFrame += frame_jump; // jump X frames forward
    // if over figure out if we loop or stop
    if (_currentAnimationFrame >= _currentAnimationSize){
      if (_isLooping){
        _currentAnimationFrame -= _currentAnimationSize;
      }else{
        _currentAnimationFrame = 0;
        _isPlaying = false;
      }
    }
  }
}

void Animator::stop(){
  _isPlaying = false;
  _currentAnimationFrame = 0;
  _currentAnimationColor = 0;
  _display->clearDisplay();
}
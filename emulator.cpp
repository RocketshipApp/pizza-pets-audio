#include "dx7.h"
#include "speech_synthesizer.h"
#include "nsf_player.h"
#include "synth_unit.h"

#include <stdio.h>
#include <iostream>
#include <emscripten/bind.h>

using std::vector;
using namespace emscripten;

int main() {
  SynthUnit::Init(48000);
  return 0;
}

class SpeechSynthesizerWrapper : public SpeechSynthesizer {
public:
 SpeechSynthesizerWrapper() : SpeechSynthesizer() {}
  void render(uintptr_t output_ptr, int32_t numFrames) {
    float *output_array = reinterpret_cast<float*>(output_ptr);
    SpeechSynthesizer::process(output_array, numFrames);
  }
  void speak(uintptr_t output_ptr, int length) {
    uint8_t *lpc = reinterpret_cast<uint8_t *>(output_ptr);
    vector<uint8_t> lpcVector(&lpc[0], &lpc[length]);
    vector< vector<uint8_t> > concatenations;
    concatenations.push_back(lpcVector);
    SpeechSynthesizer::speak(concatenations);
  }
};

class NSFPlayerWrapper : public NSFPlayer {
public:
 NSFPlayerWrapper() : NSFPlayer() {}
  void render(uintptr_t output_ptr, int32_t numFrames) {
    float *output_array = reinterpret_cast<float*>(output_ptr);
    NSFPlayer::process(output_array, numFrames);
  }
  void load(uintptr_t output_ptr, int length, int callback) {
    uint8_t *nsfData = reinterpret_cast<uint8_t *>(output_ptr);
    NSFPlayer::load(nsfData, length, callback);
  }
};

class DX7Wrapper : public DX7 {
public:
  DX7Wrapper() : DX7() {}
  void render(uintptr_t output_ptr, int32_t frames) {
    float *output_array = reinterpret_cast<float*>(output_ptr);
    DX7::process(output_array, frames);
  }
  void writeData(uintptr_t output_ptr, int size) {
    uint8_t *data = reinterpret_cast<uint8_t*>(output_ptr);
    DX7::writeData(data, size);
  }
};

EMSCRIPTEN_BINDINGS(CLASS_DX7) {
  class_<DX7>("DX7Base")
    .constructor()
    .function("reset", &DX7::reset)
    .function("setVolume", &DX7::setVolume);

  class_<DX7Wrapper, base<DX7>>("DX7")
    .constructor()
    .function("render", &DX7Wrapper::render, allow_raw_pointers())
    .function("writeData", &DX7Wrapper::writeData, allow_raw_pointers());
}

EMSCRIPTEN_BINDINGS(CLASS_SpeechSynthesizer) {
  class_<SpeechSynthesizer>("SpeechSynthesizerBase")
      .constructor()
      .function("stopSpeaking", &SpeechSynthesizer::stopSpeaking)
      .property("complete", &SpeechSynthesizer::getComplete);

  class_<SpeechSynthesizerWrapper, base<SpeechSynthesizer>>("SpeechSynthesizer")
      .constructor()
      .function("render", &SpeechSynthesizerWrapper::render, allow_raw_pointers())
      .function("speak", &SpeechSynthesizerWrapper::speak, allow_raw_pointers());
}

EMSCRIPTEN_BINDINGS(CLASS_NSFPlayer) {
  class_<NSFPlayer>("NSFPlayerBase")
      .constructor()
      .function("stop", &NSFPlayer::stop)
      .function("play", &NSFPlayer::play)
      .property("complete", &NSFPlayer::getComplete);

  class_<NSFPlayerWrapper, base<NSFPlayer>>("NSFPlayer")
      .constructor()
      .function("render", &NSFPlayerWrapper::render, allow_raw_pointers())
      .function("load", &NSFPlayerWrapper::load, allow_raw_pointers());
}


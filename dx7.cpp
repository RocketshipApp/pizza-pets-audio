#include "dx7.h"
#include "synth_unit.h"
#include <iostream>

using std::vector;

DX7::DX7() {
  synthUnit = new SynthUnit(&ringBuffer);
  synthUnit->SetVolume(1);
}

DX7::~DX7() {
}

const char *DX7::processorName() {
  return "dx7-processor";
}

void DX7::setComplete(bool value) {
  complete.store(value);
}

void DX7::reset() {
  synthUnit->Reset();
}

bool DX7::getComplete() const {
  return complete.load();
}

void DX7::process(float *buffer, int length) {
  int16_t *rendered = renderBuffer.get();
  synthUnit->GetSamples(length, rendered);

  for(int i = 0; i < length; i++) {
    buffer[i] = rendered[i] / 32768.0f;
  }
}

void DX7::writeData(uint8_t *data, uint32_t size) {
  ringBuffer.Write(data, size);
}

void DX7::setVolume(float volume) {
  synthUnit->SetVolume(volume);
}

#include "speech_synthesizer.h"

using std::vector;

static uint8_t const RATE_SET_NORMAL = 0x00;
static uint8_t const SPEAK_EXTERNAL_COMMAND = 0x60;
static uint8_t const RESET_COMMAND = 0xFF;
static uint8_t const STATUS_TS_MASK = 0x80;
static uint8_t const STATUS_BL_MASK = 0x40;

SpeechSynthesizer::SpeechSynthesizer() {
  _tms5220.device_start();
  _tms5220.device_reset();
  writeData(RATE_SET_NORMAL);
}

SpeechSynthesizer::~SpeechSynthesizer() {
}

const char *SpeechSynthesizer::processorName() {
  return "tms-5220-speech-synthesizer-processor";
}

void SpeechSynthesizer::writeData(int data) {
  _tms5220.data_w(data);
}

int SpeechSynthesizer::readStatus() {
  return _tms5220.status_r();
}

bool SpeechSynthesizer::chipIsTalking() {
  return !!(readStatus() & STATUS_TS_MASK);
}

bool SpeechSynthesizer::chipFifoIsLow() {
  return !!(readStatus() & STATUS_BL_MASK);
}

void SpeechSynthesizer::setComplete(bool value) {
  complete.store(value);
}

bool SpeechSynthesizer::getComplete() const {
  return complete.load();
}

void SpeechSynthesizer::process(float *buffer, int length) {
  for(int i = 0; i < length; i++) {
    if (_shouldFetchSamples && _sampleIndex == 0) {
      _shouldFetchSamples = false;
      speakFragment();
    }

    _sampleRateCounter += 1;
    buffer[i] = _tmsBuffer[_sampleIndex] / 32768.0f;
    if (_sampleRateCounter == 6) {
        _sampleRateCounter = 0;
        _sampleIndex += 1;
        if (_sampleIndex >= SAMPLES_PER_2_INTERP_PERIODS) {
            _sampleIndex = 0;
            _shouldFetchSamples = true;
        }
    }
  }
}

void SpeechSynthesizer::speak(vector< vector<uint8_t> > concatenations) {
  _lpcIndex = 0;
  _concatenationIndex = 0;
  _concatenations = concatenations;
  _currentConcatenation = _concatenations.front();
  _sampleIndex = 0;
  _shouldFetchSamples = false;
  _sampleRateCounter = 0;
  _tms5220.set_use_no_leading_silence_hack(false);
  _tms5220.device_reset();
  writeData(RESET_COMMAND);
  writeData(SPEAK_EXTERNAL_COMMAND);
  setComplete(false);
}

void SpeechSynthesizer::stopSpeaking() {
  _tms5220.device_reset();
  writeData(RESET_COMMAND);
  _haltWritingData = false;
  reset();
}

void SpeechSynthesizer::reset() {
  _currentConcatenation.clear();
  _concatenations.clear();
  _concatenationIndex = 0;
}

void SpeechSynthesizer::advance() {
  _lpcIndex += 1;
  if (_lpcIndex == _currentConcatenation.size()) {
    _lpcIndex = 0;
    _concatenationIndex += 1;
    if (_concatenationIndex == _concatenations.size()) {
      reset();
      setComplete(true);
      return;
    }
    _currentConcatenation = _concatenations[_concatenationIndex];
    _haltWritingData = true;
  }
}

void SpeechSynthesizer::speakFragment() {
  if (_currentConcatenation.size() > 0) {
    if (_haltWritingData && !chipIsTalking()) {
      _haltWritingData = false;
      writeData(RESET_COMMAND);
      _tms5220.device_reset();
      writeData(SPEAK_EXTERNAL_COMMAND);
    }

    while (chipFifoIsLow() && !_haltWritingData && _currentConcatenation.size() != 0) {
      writeData(_currentConcatenation[_lpcIndex]);
      advance();

      if (_haltWritingData || !chipIsTalking()) { break; }
    }
  }

  _tms5220.process(_tmsBuffer, SAMPLES_PER_2_INTERP_PERIODS);
}

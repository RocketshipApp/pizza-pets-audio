#include <stdint.h>
#include <vector>
#include "sample_streamable.h"
#include "tms5220/tms5220.h"

static int const SAMPLES_PER_2_INTERP_PERIODS = 50;

class SpeechSynthesizer : public SampleStreamable {
public:
  float volume;
  virtual ~SpeechSynthesizer() override;

  const char *processorName() override;
  explicit SpeechSynthesizer();
  void speak(std::vector< std::vector<uint8_t> > concatenations);
  void stopSpeaking();
  void process(float *buffer, int length) override;
  void setComplete(bool value) override;
  bool getComplete() const override;

private:
  std::atomic<bool> complete{false};
  tms5220_device _tms5220;
  bool _shouldFetchSamples;
  int _concatenationIndex;
  std::vector< std::vector<uint8_t> > _concatenations;
  std::vector<uint8_t> _currentConcatenation;
  int _lpcIndex;
  int16_t _tmsBuffer[SAMPLES_PER_2_INTERP_PERIODS];
  uint8_t _sampleIndex;
  uint8_t _sampleRateCounter;
  bool _haltWritingData;
  bool chipFifoIsLow();
  bool chipIsTalking();
  int readStatus();
  void advance();
  void reset();
  void speakFragment();
  void writeData(int data);
 };

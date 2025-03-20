#include <stdint.h>
#include <vector>
#include "sample_streamable.h"
#include "ringbuffer.h"

class SynthUnit;

class DX7 : public SampleStreamable {
public:
  virtual ~DX7() override;
  const char *processorName() override;
  explicit DX7();
  void setComplete(bool value) override;
  bool getComplete() const override;
  void process(float *buffer, int length) override;
  void writeData(uint8_t *data, uint32_t size);
  void reset();
  void setVolume(float volume);
private:
  std::atomic<bool> complete{false};
  RingBuffer ringBuffer;
  std::unique_ptr<int16_t[]> renderBuffer;
  SynthUnit *synthUnit;
 };

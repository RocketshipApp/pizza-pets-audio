#include <stdint.h>
#include <memory>
#include "sample_streamable.h"

class Music_Emu;
class NSFPlayer : public SampleStreamable {
public:
  virtual ~NSFPlayer() override;
  NSFPlayer();

  const char *processorName() override;
  uint8_t songs();
  void load(uint8_t *nsfData, uint32_t size, int callback);
  void play(int trk, float vol);
  void stop();
  void process(float *buffer, int length) override;
  void setComplete(bool value) override;
  bool getComplete() const override;

private:
  std::atomic<bool> complete{false};
  Music_Emu *emu;
  bool playing;
  float volume;
  int track;
  std::unique_ptr<short[]> renderBuffer;
 };

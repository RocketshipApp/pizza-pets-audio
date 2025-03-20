#include "nsf_player.h"
#include "game-music-emu/gme/gme.h"
#include "game-music-emu/gme/Music_Emu.h"
#include <algorithm>

#define FRAMES 128
#define GME_DISABLE_STEREO_DEPTH 1

NSFPlayer::NSFPlayer() : renderBuffer(std::make_unique<short[]>(FRAMES * 2)) {
}

NSFPlayer::~NSFPlayer() {
}

const char *NSFPlayer::processorName() {
  return "nsf-player-processor";
}

void NSFPlayer::setComplete(bool value) {
  complete.store(value);
}

bool NSFPlayer::getComplete() const {
  return complete.load();
}

uint8_t NSFPlayer::songs() {
  return gme_track_count(emu);
}

void NSFPlayer::process(float *buffer, int length) {
  short *rendered = renderBuffer.get();
  if (playing) {
    gme_play(emu, FRAMES * 2, rendered);
    if (gme_track_ended(emu)) {
      setComplete(true);
    }
  }

  int index = 0;
  for(int i = 0; i < length; i++) {
    buffer[i] = rendered[index] / 32768.0f * volume;
    index += 2;
  }
}

void NSFPlayer::load(uint8_t *nsfData, uint32_t size, int callback) {
  gme_open_data(nsfData, size, &emu, 48000);
  void (*cb)(int) = reinterpret_cast<void (*)(int)>(callback);
  cb(songs());
}

void NSFPlayer::play(int trk, float vol) {
  setComplete(false);
  playing = true;
  volume = vol;
  gme_start_track(emu, trk);
}

void NSFPlayer::stop() {
  playing = false;
  std::fill(renderBuffer.get(), renderBuffer.get() + FRAMES * 2, 0);
}

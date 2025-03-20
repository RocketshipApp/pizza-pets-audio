import { BaseProcessor } from './base-processor.js';

class NSFPlayerProcessor extends BaseProcessor {
  renderableClass = 'NSFPlayer';

  processEvent(event) {
    if (event.data.load) {
      this.load(event);
    } else if (event.data.play) {
      this.play(event);
    } else {
      super.processEvent(event);
    }
  }

  load(event) {
    const binary = atob(event.data.load);
    const length = binary.length;
    const bytes = new Uint8Array(length);
    for (var i = 0; i < length; i++) {
      bytes[i] = binary.charCodeAt(i);
    }

    const ptr = this.module._malloc(length);
    const copy = new Uint8Array(this.module.HEAPU8.buffer, ptr, length);
    copy.set(bytes);
    const callbackPtr = this.module.addFunction((tracks) => {
      this.port.postMessage({ nsfLoaded: true, tracks });
      this.module.removeFunction(callbackPtr);
    }, 'vi');
    this.renderable.load(ptr, length, callbackPtr);
    this.module._free(ptr);
  }

  onComplete() {
    this.playing.index = (this.playing.index + 1) % this.playing.tracks.length;
    this.playCurrentTrack();
  }

  play(event) {
    const { tracks, volume } = event.data.play;
    this.playing = {
      tracks,
      volume,
      index: 0,
    };
    this.playCurrentTrack();
  }

  playCurrentTrack() {
    this.resetPostedCompleteMessage();
    const { tracks, index, volume } = this.playing;
    this.renderable.play(tracks[index], volume ?? 1.0);
  }
}

registerProcessor('nsf-player', NSFPlayerProcessor);

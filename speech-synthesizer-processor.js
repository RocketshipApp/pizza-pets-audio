import { BaseProcessor } from './base-processor.js';

export class SpeechSynthesizerProcessor extends BaseProcessor {
  renderableClass = 'SpeechSynthesizer';

  processEvent(event) {
    if (event.data.speak) {
      this.speak(event.data.speak);
    } else {
      super.processEvent(event);
    }
  }

  speak(lpc) {
    this.resetPostedCompleteMessage();
    const ptr = this.module._malloc(lpc.length * Uint8Array.BYTES_PER_ELEMENT);
    const copy = new Uint8Array(this.module.HEAPU8.buffer, ptr, lpc.length);
    copy.set(lpc);
    this.renderable.speak(ptr, lpc.length);
    this.module._free(ptr);
  }

  stop() {
    this.renderable.stopSpeaking();
  }
}

registerProcessor('speech-synthesizer', SpeechSynthesizerProcessor);

import Module from './emulator.js';

const NUM_FRAMES = 128;
const states = { RUNNING: 'running', WAITING: 'waiting', TERMINATING: 'terminating', TERMINATED: 'terminated' };

export class BaseProcessor extends AudioWorkletProcessor {
  state = states.WAITING;
  postedCompleteMessage = false;

  constructor() {
    super();
    Module({}).then((module) => {
      this.module         = module;
      this.renderable     = new module[this.renderableClass]();
      this.bufferPtr      = this.module._malloc(NUM_FRAMES * Float32Array.BYTES_PER_ELEMENT);
      this.buffer         = new Float32Array(this.module.HEAPF32.buffer, this.bufferPtr, NUM_FRAMES);
      this.port.onmessage = this.processEvent.bind(this);
      this.state          = states.RUNNING;
      this.onInit();
    });
  }

  onInit() {}

  process(_inputs, outputs) {
    if (this.state === states.WAITING) {
      return true;
    }

    if (this.state !== states.RUNNING) {
      if (this.state === states.TERMINATING) {
        this.module._free(this.bufferPtr);
        this.state = states.TERMINATED;
      }
      return false;
    }

    this.renderable.render(this.bufferPtr, NUM_FRAMES);
    outputs[0][0].set(this.buffer);

    if (!this.postedCompleteMessage && this.isComplete()) {
      this.postedCompleteMessage = true;
      this.onComplete();
      this.port.postMessage({ complete: this.constructor.name });
    }

    return true;
  }

  stop() {
    this.renderable.stop();
  }

  isComplete() {
    return this.renderable.complete;
  }

  onComplete() {}

  processEvent(event) {
    if (event.data.stop) {
      this.stop();
      this.port.postMessage({ stopped: this.constructor.name });
    } else if (event.data.terminate) {
      this.state = states.TERMINATING;
      this.port.postMessage({ terminated: this.constructor.name });
    } else {
      this.port.postMessage(`unknown event! ${JSON.stringify(event)}`);
    }
  }

  resetPostedCompleteMessage() {
    this.postedCompleteMessage = false;
  }
}

import { BaseProcessor } from './base-processor.js';

const DX7_PATCH_BYTES             = 128;
const DX7_PATCH_NAME_OFFSET       = 4;
const DX7_PATCH_NAME_CHARACTERS   = 10;
const MIDI_COMMAND_NOTE_ON        = 0x90;
const MIDI_COMMAND_NOTE_OFF       = 0x80;
const MIDI_COMMAND_PITCH_BEND     = 0xe0;
const MIDI_COMMAND_CONTROL_CHANGE = 0xb0;
const MIDI_COMMAND_PROGRAM_CHANGE = 0xc0;

class DX7Processor extends BaseProcessor {
  renderableClass = 'DX7';
  static number = 0;

  onInit() {
    this.channel = ++this.constructor.number;
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
    this.renderable.writeData(ptr, length);
    this.module._free(ptr);
    const patchCount = Math.floor(bytes.length / DX7_PATCH_BYTES);
    const patches = [];

    for (let i = 1; i <= patchCount; i++) {
      const offset = (i * DX7_PATCH_BYTES) - DX7_PATCH_NAME_OFFSET;
      const nameBuffer = bytes.slice(offset, offset + DX7_PATCH_NAME_CHARACTERS);
      const name = String.fromCharCode.apply(null, nameBuffer).trim();
      if (name.length) {
        patches.push(name);
      }
    }

    this.renderable.reset();
    this.port.postMessage({ sysexLoaded: true, channel: this.channel, patches });
  }

  play(event) {
    const { note, velocity } = event.data;
    this.writePacket(MIDI_COMMAND_NOTE_ON, [ note, velocity ]);
  }

  stop(event) {
    const { note } = event.data;
    this.writePacket(MIDI_COMMAND_NOTE_OFF, [ note, null ]);
  }

  reset(_event) {
    this.renderable.reset();
  }

  patchChange(event) {
    const { patch } = event.data;
    this.writePacket(MIDI_COMMAND_PROGRAM_CHANGE, [ patch ]);
  }

  controlChange(event) {
    const { number, value } = event.data;
    this.writePacket(MIDI_COMMAND_CONTROL_CHANGE, [ number, value ]);
  }

  pitchBend(event) {
    const { lsb, msb } = event.data;
    this.writePacket(MIDI_COMMAND_PITCH_BEND, [ lsb, msb ]);
  }

  setVolume(event) {
    const { value } = event.data;
    this.renderable.setVolume(value);
  }

  writePacket(command, data) {
    const length = data.length + 1;
    const ptr = this.module._malloc(length);
    const bytes = new Uint8Array(length);
    bytes[0] = command;
    for (let i = 0; i < data.length; i++) {
      bytes[i + 1] = data[i];
    }
    const copy = new Uint8Array(this.module.HEAPU8.buffer, ptr, length);
    copy.set(bytes);
    this.renderable.writeData(ptr, length);
    this.module._free(ptr);
  }

  isComplete() {
    return false;
  }

  processEvent(event) {
    if (event.data.load) {
      this.load(event);
    } else if (event.data.patchChange) {
      this.patchChange(event);
    } else if (event.data.play) {
      this.stop(event);
      this.play(event);
    } else if (event.data.stop) {
      this.stop(event);
    } else if (event.data.pitchBend) {
      this.pitchBend(event);
    } else if (event.data.setVolume) {
      this.setVolume(event);
    } else if (event.data.reset) {
      this.reset(event);
    } else if (event.data.controlChange) {
      this.controlChange(event);
    } else {
      super.processEvent(event);
    }
  }
}

registerProcessor('dx7', DX7Processor);

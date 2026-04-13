# Sound operations: play and record

The [miniaudio](https://miniaud.io) header handles audio playback and
capture with ease: you can seamlessly integrate audio functionality
into your C code, creating immersive and interactive experiences for
your users.

Miniaudio provides two main sources of documentation:

- [The miniaudio Programmer's manual](https://miniaud.io/docs/manual/index.html)
- [Various miniaudio examples](https://miniaud.io/docs/examples/index.html)

Miniaudio includes both low level and high level APIs. The low level
API is good for those who want to do all of their mixing themselves
and only require a light weight interface to the underlying audio
device. The high level API is good for those who have complex mixing
and effect requirements.

In miniaudio, objects are transparent structures. Unlike many other
libraries, there are no handles to opaque objects which means you need
to allocate memory for objects yourself.

A config/init pattern is used throughout the entire library. The idea
is that you set up a config object and pass that into the
initialization routine. The advantage to this system is that the
config object can be initialized with logical defaults and new
properties added to it without breaking the API.

Below and in
[examples/miniaudio.c](https://github.com/dyne/cjit/blob/main/examples/miniaudio.c)
is an example of sin wave synthesis that runs smoothly in CJIT:

```c
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <stdio.h>
#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000

void data_callback(ma_device* pDevice,
                   void* pOutput,
                   const void* pInput,
                   ma_uint32 frameCount) {
    ma_waveform* pSineWave;
    MA_ASSERT(pDevice->playback.channels == DEVICE_CHANNELS);
    pSineWave = (ma_waveform*)pDevice->pUserData;
    MA_ASSERT(pSineWave != NULL);
    ma_waveform_read_pcm_frames(pSineWave, pOutput, frameCount, NULL);
    (void)pInput;   /* Unused in this example. */
}

int main(int argc, char** argv) {
    ma_waveform sineWave;
    ma_device_config deviceConfig;
    ma_device device;
    ma_waveform_config sineWaveConfig;
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = DEVICE_FORMAT;
    deviceConfig.playback.channels = DEVICE_CHANNELS;
    deviceConfig.sampleRate        = DEVICE_SAMPLE_RATE;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &sineWave;
    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return -4;
    }
    printf("Device Name: %s\n", device.playback.name);
    sineWaveConfig = ma_waveform_config_init(device.playback.format,
                                             device.playback.channels,
                                             device.sampleRate,
                                             ma_waveform_type_sine,
                                             0.2, 220);
    ma_waveform_init(&sineWaveConfig, &sineWave);
    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return -5;
    }
    printf("Press Enter to quit...\n");
    getchar();
    ma_device_uninit(&device);
    (void)argc;
    (void)argv;
    return 0;
}
```

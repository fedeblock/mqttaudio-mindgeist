# MQTT Audio Player

A lightweight yet feature-rich MQTT-controlled audio player using SDL2 and SDL2_mixer libraries. Based on bandrews/mqttaudio, this player listens to MQTT messages and plays audio files accordingly, supporting various audio formats like OGG, MOD, MP3, and WAV. Despite its lightweight design, it offers extensive functionality, allowing control over individual channels, volume settings, looping, and more.

## Features

- **MQTT Integration**: Listens to specified MQTT topics and processes commands to control audio playback.
- **Multiple Audio Formats**: Supports OGG, MOD, MP3, and WAV formats.
- **Channel Management**: Play sounds on specific channels, pause, resume, and stop playback.
- **Volume Control**:
  - **Master Volume**: Control the overall volume affecting all channels proportionally.
  - **Channel Volume**: Set individual volumes for each channel.
- **Looping and Exclusive Playback**: Supports looping sounds and exclusive playback that stops all other sounds.
- **Sample Caching**: Preload and cache audio samples for faster playback.
- **No-Cache Playback**: Option to play audio without caching, reloading the file each time.
- **Command Processing**: Handles various commands like play, stop, fade out, set volume, etc.
- **Verbose Logging**: Optional verbose mode for detailed logging.

## Requirements

- **C++ Compiler**: Compatible with GCC.
- **SDL2**: Simple DirectMedia Layer library.
- **SDL2_mixer**: SDL audio mixer library.
- **libmosquitto**: MQTT client library.
- **RapidJSON**: Fast JSON parser and generator for C++.
- **ALSA**: Advanced Linux Sound Architecture for audio device handling.
- **CURL**: Used for HTTP support in SDL.

## Installation

1. **Install Dependencies**:

   Make sure you have the following libraries installed:

   - SDL2
   - SDL2_mixer
   - libmosquitto
   - RapidJSON
   - ALSA
   - CURL

   On Debian-based systems, you can install them using:

   ```bash
   sudo apt-get install libsdl2-dev libsdl2-mixer-dev libmosquitto-dev rapidjson-dev libasound2-dev libcurl4-openssl-dev
   ```

2. **Clone the Repository**:

   ```bash
   git clone https://github.com/yourusername/mqtt-audio-player.git
   cd mqtt-audio-player
   ```

3. **Compile the Source Code**:

   Use the provided `Makefile` to compile the program:

   ```bash
   make mqttaudio
   ```

   This will generate the executable `mqttaudio`.

## Usage

```bash
./mqttaudio [options]
```

### Command-Line Options

- `-s, --server`: The MQTT server to connect to (default `localhost`).
- `-p, --port`: The MQTT server port (default `1883`).
- `-t, --topic`: The MQTT topic to subscribe to (wildcards allowed).
- `-d, --alsa-device`: The ALSA PCM device to use (overrides `SDL_AUDIODRIVER` and `AUDIODEV` environment variables).
- `-l, --list-devices`: Lists available ALSA PCM devices for the `-d` option.
- `-v, --verbose`: Enables verbose logging.
- `-f, --frequency`: Sets the frequency for the sound output (in Hz).
- `-u, --uri-prefix`: Sets a prefix to be prepended to all sound file locations.
- `--preload`: Preloads a sound sample on startup.

### Examples

- **Start the player with default settings and verbose mode**:

  ```bash
  ./mqttaudio -v -t "audio/commands"
  ```

- **Connect to a specific MQTT server and topic**:

  ```bash
  ./mqttaudio -s mqtt.example.com -p 1883 -t "home/audio"
  ```

- **Set a custom ALSA PCM device**:

  ```bash
  ./mqttaudio -d "hw:0,0" -t "audio/commands"
  ```

- **List available ALSA PCM devices**:

  ```bash
  ./mqttaudio -l
  ```

## MQTT Commands

The player listens to MQTT messages in JSON format and processes commands accordingly.

### Command Structure

```json
{
  "command": "commandName",
  "message": {
    // parameters
  }
}
```

### Supported Commands

#### Play Sound

**Command**: `play` or `soundPlay`

**Description**: Plays an audio file with specified parameters.

**Parameters**:

- `file` (string, required): Path or URL to the audio file.
- `channel` (int, optional): Channel number to play the sound on (default `0`).
- `loop` (bool, optional): Whether to loop the sound (default `false`).
- `volume` (float, optional): Volume level (0.0 to 1.0, default `1.0`).
- `exclusive` (bool, optional): If `true`, stops all other sounds before playing (default `false`).
- `bgm` (bool, optional): Background music flag (not currently used).
- `maxPlayLength` (int, optional): Maximum play length in milliseconds (default `-1`, play to the end).
- `nocache` (bool, optional): If `true`, does not cache the sample (default `false`).

**Example**:

```json
{
  "command": "play",
  "message": {
    "file": "/path/to/sound.mp3",
    "channel": 1,
    "loop": false,
    "volume": 0.8,
    "exclusive": false,
    "maxPlayLength": 60000,
    "nocache": true
  }
}
```

#### Stop All Sounds

**Command**: `stopall` or `soundStopAll`

**Description**: Stops all currently playing sounds.

**Example**:

```json
{
  "command": "stopall"
}
```

#### Fade Out Sound

**Command**: `fadeout` or `soundFadeOut`

**Description**: Fades out the sound on a specific channel or all channels over a specified time.

**Parameters**:

- `time` (int, required): Fade out time in milliseconds.
- `channel` (int, optional): Channel to fade out (default `-1` for all channels).

**Example**:

```json
{
  "command": "fadeout",
  "message": {
    "time": 5000,
    "channel": -1
  }
}
```

#### Preload Sample

**Command**: `precache` or `soundPrecache`

**Description**: Preloads an audio sample into the cache.

**Parameters**:

- `file` (string, required): Path or URL to the audio file.

**Example**:

```json
{
  "command": "precache",
  "message": {
    "file": "/path/to/sound.mp3"
  }
}
```

#### Set Channel Volume

**Command**: `soundSetVolume`

**Description**: Sets the volume for a specific channel.

**Parameters**:

- `channel` (int, required): Channel number.
- `volume` (float, required): Volume level (0.0 to 1.0).

**Example**:

```json
{
  "command": "soundSetVolume",
  "message": {
    "channel": 1,
    "volume": 0.5
  }
}
```

#### Pause Channel

**Command**: `soundPause`

**Description**: Pauses playback on a specific channel.

**Parameters**:

- `channel` (int, required): Channel number.

**Example**:

```json
{
  "command": "soundPause",
  "message": {
    "channel": 1
  }
}
```

#### Resume Channel

**Command**: `soundResume`

**Description**: Resumes playback on a specific channel.

**Parameters**:

- `channel` (int, required): Channel number.

**Example**:

```json
{
  "command": "soundResume",
  "message": {
    "channel": 1
  }
}
```

#### Set Master Volume

**Command**: `setMasterVolume`

**Description**: Sets the master volume, affecting all channels proportionally.

**Parameters**:

- `volume` (float, required): Master volume level (0.0 to 1.0).

**Example**:

```json
{
  "command": "setMasterVolume",
  "message": {
    "volume": 0.7
  }
}
```

## How It Works

- The player initializes SDL and SDL_mixer for audio playback.
- Connects to the specified MQTT server and subscribes to the given topic.
- Listens for MQTT messages and parses them using RapidJSON.
- Processes commands to control audio playback, volume, and other functionalities.
- Manages audio samples using `SampleManager`, supporting caching and preloading.

## Customization

- **URI Prefix**: Use the `-u` or `--uri-prefix` option to set a prefix for all audio file paths. This is useful if all your audio files are in a specific directory or URL.
- **Preloading Samples**: Use the `--preload` option to preload samples on startup, reducing latency when playing them later.

## Verbose Logging

Enable verbose logging with the `-v` or `--verbose` option to get detailed output of the player's operations, including:

- Commands received and their parameters.
- Volume levels and calculations.
- Cache operations (loading, removing samples).
- Playback actions (playing, stopping, pausing, resuming).

## Error Handling

- The player outputs error messages if it encounters issues with commands, such as missing parameters or invalid formats.
- SDL and SDL_mixer error messages are displayed if audio initialization or playback fails.

## License

This project is licensed under the MIT License.

## Contributing

Contributions are welcome! Please submit pull requests or open issues to suggest improvements or report bugs.

## Contact

For questions or support, please contact [contact@mindgeist.com](mailto:contact@mindgeist.com).

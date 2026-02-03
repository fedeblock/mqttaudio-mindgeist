#ifndef ELEVENLABS_STREAM_H
#define ELEVENLABS_STREAM_H

#include <string>

// Versión síncrona de TTS ElevenLabs
bool play_elevenlabs_stream(const std::string& voice_id,
                             const std::string& api_key,
                             const std::string& text,
                             const std::string& format,
                             int channel,
                             float volume = 1.0f,
                             int loops = 0);

// Versión asíncrona (detached thread)
void play_elevenlabs_stream_async(const std::string& voice_id,
                                  const std::string& api_key,
                                  const std::string& text,
                                  const std::string& format,
                                  int channel,
                                  float volume = 1.0f,
                                  int loops = 0);

#endif // ELEVENLABS_STREAM_H
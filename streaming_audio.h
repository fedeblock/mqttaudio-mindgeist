#ifndef STREAMING_AUDIO_H
#define STREAMING_AUDIO_H

#include <string>

// Versión síncrona de reproducción por URL
bool play_streaming_url(const std::string& url, int channel, float volume = 1.0f, int loops = 0);

// Versión asíncrona (detached thread)
void play_streaming_url_async(const std::string& url, int channel, float volume = 1.0f, int loops = 0);

#endif // STREAMING_AUDIO_H
#include "elevenlabs_stream.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <curl/curl.h>
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <sstream>
#include <cstring>
#include <cstdio>


extern bool run;

struct MemBuffer {
    std::vector<unsigned char> data;
    std::atomic<bool> error{false};
};

// Callback CURL para acumular datos
static size_t mem_write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t real_size = size * nmemb;
    auto* buf = static_cast<MemBuffer*>(userdata);
    buf->data.insert(buf->data.end(), ptr, ptr + real_size);
    return real_size;
}

// Función síncrona de TTS
bool play_elevenlabs_stream(const std::string& voice_id,
                             const std::string& api_key,
                             const std::string& text,
                             const std::string& format,
                             int channel,
                             float volume,
                             int loops) {
    // Construye URL de streaming
    std::string url = "https://api.elevenlabs.io/v1/text-to-speech/" + voice_id + "/stream";

    // Headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("xi-api-key: " + api_key).c_str());
    headers = curl_slist_append(headers, ("Accept: audio/" + format).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Payload JSON simple
    std::ostringstream oss;
    oss << "{\"text\":\"" << text << "\"}";
    std::string postdata = oss.str();

    MemBuffer buf;
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mem_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    if (res != CURLE_OK) buf.error = true;
    curl_easy_cleanup(curl);

    if (buf.error || buf.data.empty()) {
        return false;
    }
    SDL_RWops* rw = SDL_RWFromMem(buf.data.data(), buf.data.size());
    Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 0);
    if (!chunk) {
        printf("Error al cargar TTS de ElevenLabs: %s\n", Mix_GetError());
        SDL_FreeRW(rw);
        return false;
    }

    int sdlVol = static_cast<int>(volume * MIX_MAX_VOLUME);
    Mix_Volume(channel, sdlVol);
    Mix_PlayChannel(channel, chunk, loops);

    while (Mix_Playing(channel) && run) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (!run) Mix_HaltChannel(channel);

    Mix_FreeChunk(chunk);
    SDL_FreeRW(rw);
    return true;
}

// Función asíncrona de TTS
void play_elevenlabs_stream_async(const std::string& voice_id,
                                  const std::string& api_key,
                                  const std::string& text,
                                  const std::string& format,
                                  int channel,
                                  float volume,
                                  int loops) {
    std::thread([=]() {
        play_elevenlabs_stream(voice_id, api_key, text, format, channel, volume, loops);
    }).detach();
}
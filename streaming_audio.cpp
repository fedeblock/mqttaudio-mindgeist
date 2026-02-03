#include "streaming_audio.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <curl/curl.h>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <cstdio>

extern bool run; // variable global de control de señales

struct StreamBuffer {
    std::vector<unsigned char> data;
    std::atomic<bool> finished{false};
    std::atomic<bool> error{false};
    std::mutex mutex;
    std::condition_variable cv;
};

// Callback de CURL para acumular datos en el buffer
static size_t curl_write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t real_size = size * nmemb;
    StreamBuffer* sbuf = static_cast<StreamBuffer*>(userdata);
    {
        std::lock_guard<std::mutex> lock(sbuf->mutex);
        sbuf->data.insert(sbuf->data.end(), ptr, ptr + real_size);
    }
    return real_size;
}

// Versión síncrona: descarga completa y reproduce
bool play_streaming_url(const std::string& url, int channel, float volume, int loops) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    StreamBuffer* sbuf = new StreamBuffer();

    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, sbuf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) sbuf->error = true;
    sbuf->finished = true;

    // Verifica descarga exitosa
    if (sbuf->data.empty() || sbuf->error) {
        delete sbuf;
        curl_easy_cleanup(curl);
        return false;
    }

    // Crea RWops en memoria y carga Mix_Chunk
    SDL_RWops* rw = SDL_RWFromMem(sbuf->data.data(), sbuf->data.size());
    Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 0);
    if (!chunk) {
        printf("Error al cargar stream: %s\n", Mix_GetError());
        SDL_FreeRW(rw);
        delete sbuf;
        curl_easy_cleanup(curl);
        return false;
    }

    // Ajusta volumen y reproduce en el canal
    int sdlVolume = static_cast<int>(volume * MIX_MAX_VOLUME);
    Mix_Volume(channel, sdlVolume);
    Mix_PlayChannel(channel, chunk, loops);

    // Espera hasta fin de reproducción o señal stop
    while (Mix_Playing(channel) && run) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (!run) {
        Mix_HaltChannel(channel);
    }

    // Limpieza
    Mix_FreeChunk(chunk);
    SDL_FreeRW(rw);
    delete sbuf;
    curl_easy_cleanup(curl);
    return true;
}

// Versión asíncrona: lanza un hilo detached
void play_streaming_url_async(const std::string& url, int channel, float volume, int loops) {
    std::thread([url, channel, volume, loops]() {
        play_streaming_url(url, channel, volume, loops);
    }).detach();
}
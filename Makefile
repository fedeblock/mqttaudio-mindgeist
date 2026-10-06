all:  mqttaudio

CXX := g++
CXXFLAGS := -I/usr/include/SDL2 -I/usr/include/alsa -L/usr/lib/ -g -pthread
LDLIBS := -lSDL2 -lSDL2_mixer -lrt -lmosquitto -lasound -lcurl

SRCS := mqttaudio.cpp sample.cpp samplemanager.cpp SDL_rwhttp.c streaming_audio.cpp elevenlabs_stream.cpp

mqttaudio: $(SRCS)
	$(CXX) -o $@ $(SRCS) $(CXXFLAGS) $(LDLIBS)

test_redact: test_redact.cpp redact.h
	$(CXX) -std=c++11 -Wall -Wextra -o $@ test_redact.cpp

test_tts_request: test_tts_request.cpp tts_request.h
	$(CXX) -std=c++11 -Wall -Wextra -Wno-class-memaccess -I. -o $@ test_tts_request.cpp

# Pruebas sin SDL ni MQTT: redacción de claves en los registros y construcción
# segura de la petición a ElevenLabs (escapado del texto, voice_id y format)
test: test_redact test_tts_request
	./test_redact
	./test_tts_request

.PHONY: all clean test install-dependencies

clean:
	rm -f mqttaudio test_redact test_tts_request

install-dependencies:
	apt-get install libsdl2-dev libsdl2-mixer-dev libsdl2-net-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-gfx-dev mosquitto libmosquitto-dev mosquitto-clients libcurl4-openssl-dev
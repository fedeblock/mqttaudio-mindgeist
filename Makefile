all:  mqttaudio

CXX := g++
CXXFLAGS := -I/usr/include/SDL2 -I/usr/include/alsa -L/usr/lib/ -g -pthread
LDLIBS := -lSDL2 -lSDL2_mixer -lrt -lmosquitto -lasound -lcurl

SRCS := mqttaudio.cpp sample.cpp samplemanager.cpp SDL_rwhttp.c streaming_audio.cpp elevenlabs_stream.cpp

mqttaudio: $(SRCS)
	$(CXX) -o $@ $(SRCS) $(CXXFLAGS) $(LDLIBS)

test_redact: test_redact.cpp redact.h
	$(CXX) -std=c++11 -Wall -Wextra -o $@ test_redact.cpp

# Prueba la redacción de claves en los registros (no necesita SDL ni MQTT)
test: test_redact
	./test_redact

.PHONY: all clean test install-dependencies

clean:
	rm -f mqttaudio test_redact

install-dependencies:
	apt-get install libsdl2-dev libsdl2-mixer-dev libsdl2-net-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-gfx-dev mosquitto libmosquitto-dev mosquitto-clients libcurl4-openssl-dev
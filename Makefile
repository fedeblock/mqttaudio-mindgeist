all:  mqttaudio

CXX := g++
CXXFLAGS := -I/usr/include/SDL2 -I/usr/include/alsa -L/usr/lib/ -g -pthread
LDLIBS := -lSDL2 -lSDL2_mixer -lrt -lmosquitto -lasound -lcurl

SRCS := mqttaudio.cpp sample.cpp samplemanager.cpp SDL_rwhttp.c streaming_audio.cpp elevenlabs_stream.cpp

mqttaudio: $(SRCS)
	$(CXX) -o $@ $(SRCS) $(CXXFLAGS) $(LDLIBS)

clean:
	rm -f mqttaudio

install-dependencies:
	apt-get install libsdl2-dev libsdl2-mixer-dev libsdl2-net-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-gfx-dev mosquitto libmosquitto-dev mosquitto-clients libcurl4-openssl-dev
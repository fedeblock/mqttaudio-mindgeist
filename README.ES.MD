# MQTT Audio Player para Asistentes de Voz

Un reproductor de audio controlado por MQTT diseñado específicamente para asistentes de voz, con soporte avanzado para Text-to-Speech, streaming de audio y gestión inteligente de muestras. Basado en bandrews/mqttaudio pero expandido significativamente con funcionalidades profesionales para aplicaciones de voz.

## Características Principales

### 🎵 **Motor de Audio Avanzado**
- **Múltiples Formatos**: Soporte nativo para OGG, MOD, MP3, WAV
- **16 Canales Simultáneos**: Reproducción concurrente con control individual
- **Gestión Inteligente de Cache**: Sistema de cache automático con opción nocache
- **Streaming de Audio**: Reproducción directa desde URLs HTTP/HTTPS
- **Fade In/Out**: Transiciones suaves entre pistas

### 🗣️ **Integración Text-to-Speech**
- **ElevenLabs TTS**: Integración nativa con la API de ElevenLabs
- **Reproducción Asíncrona**: TTS no bloqueante en hilos separados
- **Control de Calidad**: Soporte para múltiples formatos de salida (MP3, WAV)
- **Gestión de Voz**: Selección de voice_id personalizada

### 🎛️ **Control de Audio Profesional**
- **Volumen Maestro**: Control global que afecta proporcionalmente todos los canales
- **Volumen por Canal**: Control individual de cada uno de los 16 canales
- **Cálculo de Volumen Efectivo**: `volumen_final = volumen_muestra × volumen_canal × volumen_maestro`
- **Pausar/Reanudar**: Control granular de reproducción por canal o global

### 🌐 **Conectividad y Protocolos**
- **MQTT Integration**: Escucha comandos en tiempo real vía MQTT
- **HTTP/HTTPS Support**: Descarga automática de archivos remotos
- **ALSA Integration**: Soporte nativo para dispositivos de audio Linux
- **Reconexión Automática**: Manejo robusto de desconexiones MQTT

## Arquitectura del Sistema

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   MQTT Broker   │───▶│  MQTT Audio      │───▶│  Audio Device   │
│                 │    │  Player          │    │  (ALSA/SDL)     │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                              │
                              ▼
                       ┌──────────────────┐
                       │   Sample Cache   │
                       │   Manager        │
                       └──────────────────┘
                              │
                              ▼
                       ┌──────────────────┐
                       │  HTTP Downloader │
                       │  ElevenLabs TTS  │
                       │  Streaming       │
                       └──────────────────┘
```

## Instalación

### Dependencias del Sistema

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-mixer-dev libmosquitto-dev \
                     rapidjson-dev libasound2-dev libcurl4-openssl-dev \
                     build-essential pkg-config

# CentOS/RHEL/Fedora
sudo dnf install SDL2-devel SDL2_mixer-devel libmosquitto-devel \
                 rapidjson-devel alsa-lib-devel libcurl-devel \
                 gcc-c++ make
```

### Compilación

```bash
git clone https://github.com/yourusername/mqtt-audio-player.git
cd mqtt-audio-player
make mqttaudio
```

### Instalación de Dependencias Automática

```bash
make install-dependencies  # Solo Ubuntu/Debian
```

## Uso Básico

### Inicio Rápido

```bash
# Configuración básica para asistente de voz
./mqttaudio -s mqtt.tu-servidor.com -t "voice/audio" -v

# Con dispositivo ALSA específico
./mqttaudio -s localhost -t "home/assistant" -d "hw:1,0" -v

# Con preloading de samples comunes
./mqttaudio -t "audio/commands" --preload "sounds/beep.wav" \
           --preload "sounds/error.mp3" -v
```

### Opciones de Línea de Comandos

| Opción | Descripción | Ejemplo |
|--------|-------------|---------|
| `-s, --server` | Servidor MQTT (default: localhost) | `-s mqtt.example.com` |
| `-p, --port` | Puerto MQTT (default: 1883) | `-p 8883` |
| `-t, --topic` | Tópico MQTT (wildcards permitidos) | `-t "voice/+/audio"` |
| `-d, --alsa-device` | Dispositivo ALSA PCM | `-d "hw:0,0"` |
| `-l, --list-devices` | Lista dispositivos ALSA disponibles | `-l` |
| `-v, --verbose` | Logging detallado | `-v` |
| `-f, --frequency` | Frecuencia de audio en Hz | `-f 48000` |
| `-u, --uri-prefix` | Prefijo para rutas de archivos | `-u "https://cdn.example.com/"` |
| `--preload` | Precargar sample al inicio | `--preload "alert.wav"` |

## Comandos MQTT

### Estructura de Mensajes

```json
{
  "command": "nombre_comando",
  "message": {
    // parámetros específicos del comando
  }
}
```

### 🎵 Comandos de Reproducción

#### Reproducir Audio Local/Remoto

```json
{
  "command": "play",
  "message": {
    "file": "/path/to/sound.mp3",
    "channel": 1,
    "loop": false,
    "volume": 0.8,
    "exclusive": false,
    "maxPlayLength": 30000,
    "nocache": false
  }
}
```

**Parámetros**:
- `file` (string, **required**): Ruta local o URL del archivo
- `channel` (int): Canal de reproducción (0-15, default: 0)
- `loop` (bool): Reproducción en bucle (default: false)
- `volume` (float): Volumen de la muestra (0.0-1.0, default: 1.0)
- `exclusive` (bool): Detener otros sonidos antes de reproducir (default: false)
- `maxPlayLength` (int): Duración máxima en ms (-1 = completo)
- `nocache` (bool): No guardar en cache (default: false)

#### Streaming de Audio

```json
{
  "command": "stream",
  "message": {
    "url": "https://example.com/stream.mp3",
    "channel": 9,
    "volume": 0.7,
    "loops": 0
  }
}
```

### 🗣️ Text-to-Speech con ElevenLabs

```json
{
  "command": "elevenlabs_tts",
  "message": {
    "voice_id": "21m00Tcm4TlvDq8ikWAM",
    "api_key": "tu_api_key_elevenlabs",
    "text": "Hola, soy tu asistente de voz",
    "format": "mp3",
    "channel": 10,
    "volume": 0.9,
    "loops": 0
  }
}
```

**Parámetros**:
- `voice_id` (string, **required**): ID de voz de ElevenLabs
- `api_key` (string, **required**): API key de ElevenLabs
- `text` (string, **required**): Texto a sintetizar
- `format` (string): Formato de audio ("mp3", "wav", default: "mp3")
- `channel` (int): Canal de reproducción (default: 10)
- `volume` (float): Volumen (0.0-1.0, default: 1.0)
- `loops` (int): Repeticiones (default: 0)

### 🎛️ Comandos de Control

#### Control de Volumen

```json
// Volumen maestro (afecta todos los canales)
{
  "command": "setMasterVolume",
  "message": {
    "volume": 0.5
  }
}

// Volumen de canal específico
{
  "command": "soundSetVolume",
  "message": {
    "channel": 1,
    "volume": 0.8
  }
}
```

#### Control de Reproducción

```json
// Detener canal específico
{
  "command": "stop",
  "message": {
    "channel": 1
  }
}

// Detener todos los canales
{
  "command": "stopall"
}

// Fade out gradual
{
  "command": "fadeout",
  "message": {
    "time": 3000,
    "channel": -1
  }
}

// Pausar/Reanudar canales
{
  "command": "pause-all"
}

{
  "command": "resume-all"
}

// Pausar/Reanudar canal específico
{
  "command": "soundPause",
  "message": {
    "channel": 1
  }
}

{
  "command": "soundResume",
  "message": {
    "channel": 1
  }
}
```

### 🚀 Optimización de Rendimiento

#### Precarga de Samples

```json
{
  "command": "precache",
  "message": {
    "file": "/sounds/frequently_used.wav"
  }
}
```

## Casos de Uso para Asistentes de Voz

### 1. **Sistema de Alertas Multi-nivel**

```json
// Alerta informativa (canal 0)
{"command": "play", "message": {"file": "info.wav", "channel": 0, "volume": 0.6}}

// Alerta de advertencia (canal 1, exclusiva)
{"command": "play", "message": {"file": "warning.wav", "channel": 1, "volume": 0.8, "exclusive": true}}

// Alerta crítica (canal 2, exclusiva, bucle)
{"command": "play", "message": {"file": "critical.wav", "channel": 2, "volume": 1.0, "exclusive": true, "loop": true}}
```

### 2. **Respuestas de Voz Dinámicas**

```json
// TTS para respuesta personalizada
{
  "command": "elevenlabs_tts",
  "message": {
    "voice_id": "21m00Tcm4TlvDq8ikWAM",
    "api_key": "sk-...",
    "text": "La temperatura actual es de 23 grados centígrados",
    "channel": 5,
    "volume": 0.8
  }
}
```

### 3. **Audio Ambiental de Fondo**

```json
// Música de fondo en bucle
{"command": "stream", "message": {"url": "https://radio.example.com/ambient", "channel": 15, "volume": 0.3, "loops": -1}}

// Sonidos ambientales
{"command": "play", "message": {"file": "rain.ogg", "channel": 14, "volume": 0.4, "loop": true}}
```

### 4. **Sistema de Notificaciones Secuenciales**

```bash
# Script de ejemplo para notificación compleja
mosquitto_pub -h localhost -t "voice/audio" -m '{"command": "play", "message": {"file": "notification_start.wav", "channel": 0}}'
sleep 1
mosquitto_pub -h localhost -t "voice/audio" -m '{"command": "elevenlabs_tts", "message": {"voice_id": "21m00Tcm4TlvDq8ikWAM", "api_key": "sk-...", "text": "Tienes 3 mensajes nuevos", "channel": 1}}'
sleep 3
mosquitto_pub -h localhost -t "voice/audio" -m '{"command": "play", "message": {"file": "notification_end.wav", "channel": 0}}'
```

## Configuración Avanzada

### Variables de Entorno

```bash
export SDL_AUDIODRIVER=alsa
export AUDIODEV=/dev/dsp1
export SDL_RWHTTP_USERAGENT="VoiceAssistant/1.0"
export SDL_RWHTTP_TIMEOUT=10
```

### Optimización para Producción

```bash
# Alta calidad de audio
./mqttaudio -f 48000 -t "production/audio" -v \
           --preload "sounds/boot.wav" \
           --preload "sounds/shutdown.wav" \
           --preload "sounds/error.wav" \
           --preload "sounds/success.wav"

# Configuración de baja latencia
./mqttaudio -f 44100 -t "realtime/audio" \
           -u "file:///opt/audio/" -v
```

### Manejo de Errores y Logging

Con el modo verbose (`-v`), el sistema proporciona logging detallado:

```
[INFO] Playing sound alert.wav, on channel 1, once, at effective volume 0.64 (sample volume: 0.80, channel volume: 1.00, master volume: 0.80)
[INFO] ELEVENLABS: voice=21m00Tcm4TlvDq8ikWAM, text='Hello world', channel=10, volume=0.72
[INFO] STREAM: url=https://example.com/music.mp3, channel=9, loops=0, volume=0.50
[INFO] Set volume of channel 2 to 0.90 (effective volume: 0.72)
```

## Integración con Sistemas Existentes

### Home Assistant

```yaml
# configuration.yaml
mqtt:
  broker: localhost
  port: 1883

script:
  voice_alert:
    sequence:
      - service: mqtt.publish
        data:
          topic: "home/audio"
          payload: '{"command": "play", "message": {"file": "alert.wav", "channel": 1, "volume": 0.8}}'
```

### Node-RED

```javascript
// Nodo de función para TTS dinámico
var msg = {
    topic: "voice/audio",
    payload: {
        command: "elevenlabs_tts",
        message: {
            voice_id: "21m00Tcm4TlvDq8ikWAM",
            api_key: flow.get("elevenlabs_api_key"),
            text: msg.payload.text,
            channel: 5,
            volume: 0.8
        }
    }
};
return msg;
```

## Troubleshooting

### Problemas Comunes

**No se reproduce audio:**
```bash
# Verificar dispositivos ALSA
./mqttaudio -l

# Probar con dispositivo específico
./mqttaudio -d "default" -t "test" -v
```

**Conexión MQTT fallida:**
```bash
# Verificar conectividad
mosquitto_pub -h tu-servidor -t "test" -m "hello"

# Usar puerto alternativo
./mqttaudio -s tu-servidor -p 8883 -t "audio" -v
```

**Problemas de TTS:**
- Verificar API key de ElevenLabs válida
- Comprobar conectividad a internet
- Validar voice_id existente

### Logs de Debug

```bash
# Logging completo
./mqttaudio -v -t "debug/audio" 2>&1 | tee audio_debug.log

# Monitoreo MQTT
mosquitto_sub -h localhost -t "audio/+" -v
```

## Rendimiento y Escalabilidad

### Capacidades del Sistema

- **Canales Simultáneos**: 16 canales independientes
- **Formatos Soportados**: WAV, MP3, OGG, MOD
- **Cache de Samples**: Ilimitado (limitado por memoria RAM)
- **Streaming Concurrente**: Múltiples streams simultáneos
- **Latencia TTS**: ~200-500ms (dependiente de ElevenLabs)

### Optimización de Memoria

```bash
# Usar nocache para archivos grandes de un solo uso
{"command": "play", "message": {"file": "large_file.wav", "nocache": true}}

# Precargar samples frecuentes
./mqttaudio --preload "beep.wav" --preload "error.wav" -t "audio"
```

## Desarrollo y Contribuciones

### Estructura del Código

```
├── mqttaudio.cpp           # Aplicación principal y manejo MQTT
├── sample.cpp/h            # Gestión de muestras individuales
├── samplemanager.cpp/h     # Cache y gestión de memoria
├── SDL_rwhttp.c/h          # Soporte HTTP/HTTPS
├── streaming_audio.cpp/h   # Streaming de audio
├── elevenlabs_stream.cpp/h # Integración ElevenLabs TTS
├── alsautil.h              # Utilidades ALSA
└── Makefile               # Configuración de compilación
```

### Extensibilidad

El sistema está diseñado para ser extensible. Para agregar nuevas funcionalidades:

1. **Nuevos Comandos**: Añadir en `processCommand()` en `mqttaudio.cpp`
2. **Nuevos Formatos**: Extender `Sample` class
3. **Nuevos Protocolos**: Crear módulos siguiendo el patrón de `SDL_rwhttp`

## Licencia

MIT License - Ver archivo `LICENSE` para detalles completos.

## Soporte y Contacto

- **Email**: [contact@mindgeist.com](mailto:contact@mindgeist.com)
- **Issues**: GitHub Issues para reportes de bugs
- **Documentación**: Este README y comentarios en código

---

**Versión**: 0.1.2  
**Compatibilidad**: Linux (Ubuntu 18.04+, CentOS 7+, Debian 9+)  
**Dependencias Mínimas**: SDL2, SDL2_mixer, libmosquitto, CURL
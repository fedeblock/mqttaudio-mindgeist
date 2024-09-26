# **MQTT Audio Player Mindgeist**

## **Descripción**
`mqttaudio` es un reproductor de audio controlado mediante mensajes MQTT, diseñado para reproducir sonidos, ajustar el volumen, pausar, y reanudar la reproducción de audio mediante comandos MQTT. Está basado en SDL2 y `mosquitto` para el control de audio y la comunicación MQTT, respectivamente.

Este proyecto es ideal para aplicaciones de automatización y domótica, donde los eventos o comandos se transmiten a través de un servidor MQTT y requieren la reproducción o manipulación de audio en respuesta a dichos comandos.

## **Características**

- **Reproducción de audio**: Permite reproducir archivos de audio desde diferentes fuentes.
- **Control de volumen**: Ajuste de volumen por canal.
- **Comandos de pausa y reanudación**: Puede pausar y reanudar la reproducción en canales específicos.
- **Pre-carga de muestras de audio**: Soporta la precarga de muestras para una reproducción rápida.
- **Fade Out**: Soporta desvanecimiento gradual del audio.
- **Control de exclusividad**: Detiene automáticamente otros sonidos cuando se reproduce un audio exclusivo.

## **Requisitos**

Para compilar y ejecutar `mqttaudio`, asegúrate de tener instalados los siguientes paquetes:

- **SDL2**: Para la reproducción de audio.
- **SDL2_mixer**: Para manejar diferentes formatos de audio.
- **mosquitto**: Para las comunicaciones MQTT.
- **rapidjson**: Para el procesamiento de mensajes JSON.
- **ALSA**: Para la reproducción de audio en Linux.

### **Instalación de dependencias en Linux**

```bash
sudo apt-get install libsdl2-dev libsdl2-mixer-dev libmosquitto-dev libasound2-dev rapidjson-dev libcurl4-openssl-dev
```

## **Compilación**

Para compilar el proyecto, usa el comando `make` en el directorio raíz del proyecto:

```bash
make mqttaudio
```

Esto generará el binario `mqttaudio`.

## **Ejecución**

Ejecuta el programa proporcionando los parámetros necesarios, como el servidor MQTT, el puerto y el tema de suscripción:

```bash
./mqttaudio -s localhost -p 1883 -t "audio/command"
```

Los comandos MQTT se envían a través del tema especificado (por ejemplo, `audio/command`), y el programa responderá a dichos comandos.

## **Comandos MQTT Disponibles**

### 1. **soundPlay**
Reproduce un archivo de audio en un canal específico, con opciones para repetir, ajustar el volumen, establecer exclusividad y reproducir como música de fondo.

#### **Parámetros:**
- `file`: Ruta al archivo de audio que se reproducirá.
- `channel`: Canal donde se reproducirá el audio. Los canales son virtuales y manejados por SDL_mixer.
- `volume`: Nivel de volumen (0.0 a 1.0).
- `loop`: Indica si el audio debe repetirse (`true`) o no (`false`).
- `exclusive`: Si se establece en `true`, detendrá otros sonidos antes de reproducir este.
- `bgm`: Si es `true`, se reproducirá como música de fondo.
- `maxPlayLength`: Tiempo máximo de reproducción (en milisegundos). Si es `-1`, reproduce indefinidamente.

#### **Ejemplo de mensaje MQTT**:

```json
{
  "command": "soundPlay",
  "message": {
    "file": "path/to/audio/file.mp3",
    "channel": 1,
    "volume": 0.8,
    "loop": false,
    "exclusive": true,
    "bgm": false,
    "maxPlayLength": 10000
  }
}
```

### 2. **soundStopAll**
Detiene todos los sonidos que se están reproduciendo actualmente, incluida la música de fondo.

#### **Ejemplo de mensaje MQTT**:

```json
{
  "command": "soundStopAll"
}
```

### 3. **soundFadeOut**
Realiza un fade out (desvanecimiento gradual) en todos los canales activos en un tiempo especificado.

#### **Parámetros:**
- `time`: Tiempo de desvanecimiento en milisegundos.

#### **Ejemplo de mensaje MQTT**:

```json
{
  "command": "soundFadeOut",
  "message": {
    "time": 5000
  }
}
```

### 4. **soundPrecache**
Pre-carga un archivo de audio para una reproducción más rápida en el futuro.

#### **Parámetros:**
- `file`: Ruta del archivo de audio a pre-cargar.

#### **Ejemplo de mensaje MQTT**:

```json
{
  "command": "soundPrecache",
  "message": {
    "file": "path/to/audio/file.mp3"
  }
}
```

### 5. **soundSetVolume**
Ajusta el volumen de un canal específico.

#### **Parámetros:**
- `channel`: Canal cuyo volumen se desea ajustar.
- `volume`: Volumen entre 0.0 y 1.0.

#### **Ejemplo de mensaje MQTT**:

```json
{
  "command": "soundSetVolume",
  "message": {
    "channel": 1,
    "volume": 0.5
  }
}
```

### 6. **soundPause**
Pausa la reproducción de audio en un canal específico.

#### **Parámetros:**
- `channel`: El canal que se desea pausar.

#### **Ejemplo de mensaje MQTT**:

```json
{
  "command": "soundPause",
  "message": {
    "channel": 1
  }
}
```

### 7. **soundResume**
Reanuda la reproducción de audio en un canal previamente pausado.

#### **Parámetros:**
- `channel`: El canal que se desea reanudar.

#### **Ejemplo de mensaje MQTT**:

```json
{
  "command": "soundResume",
  "message": {
    "channel": 1
  }
}
```

## **Escenario de uso**

En un escenario en el que `mqttaudio` está reproduciendo un cuento de fondo y se detecta una palabra clave (wakeword), puedes:

1. **Bajar el volumen del canal del cuento** cuando se detecta la palabra clave para dar prioridad al audio de notificación.

```json
{
  "command": "soundSetVolume",
  "message": {
    "channel": 1,
    "volume": 0.2
  }
}
```

2. **Reproducir una alerta** en otro canal para indicar que se ha detectado la palabra clave.

```json
{
  "command": "soundPlay",
  "message": {
    "file": "path/to/wakeword_alert.mp3",
    "channel": 2,
    "volume": 1.0,
    "loop": false
  }
}
```

3. **Volver a subir el volumen del cuento** una vez que la alerta ha sido procesada.

```json
{
  "command": "soundSetVolume",
  "message": {
    "channel": 1,
    "volume": 0.8
  }
}
```

Este escenario demuestra cómo puedes controlar múltiples sonidos y efectos de audio simultáneamente en diferentes canales y manejar eventos prioritarios como el reconocimiento de palabras clave.

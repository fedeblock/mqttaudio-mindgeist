#!/bin/bash
# Script para probar y diagnosticar problemas de audio
# Guarda como test_audio.sh y ejecuta: chmod +x test_audio.sh && ./test_audio.sh

echo "=== Diagnóstico de Audio ALSA ==="

echo "1. Dispositivos ALSA disponibles:"
aplay -l

echo -e "\n2. Dispositivos PCM configurados:"
aplay -L | grep -E "(default|dmix|mqtt)"

echo -e "\n3. Probando dispositivo mqttaudio:"
speaker-test -D mqttaudio -t sine -f 440 -l 1 -s 1 2>/dev/null && echo "✅ mqttaudio funciona" || echo "❌ Error en mqttaudio"

echo -e "\n4. Probando dmix principal:"
speaker-test -D dmixed -t sine -f 440 -l 1 -s 1 2>/dev/null && echo "✅ dmixed funciona" || echo "❌ Error en dmixed"

echo -e "\n5. Variables de entorno SDL recomendadas:"
echo "export SDL_AUDIODRIVER=alsa"
echo "export AUDIODEV=mqttaudio"
echo "export SDL_AUDIO_DEVICE_NAME=mqttaudio"

echo -e "\n6. Comando para probar mqttaudio con configuración optimizada:"
echo "./mqttaudio --server localhost --port 12183 --topic audio/commands --alsa-device mqttaudio --verbose --frequency 44100"

echo -e "\n7. Si persisten problemas, prueba con device plug:"
echo "./mqttaudio --server localhost --port 12183 --topic audio/commands --alsa-device 'plug:dmixed_mqtt' --verbose"

echo -e "\n8. Verificar estado de ALSA:"
cat /proc/asound/cards

echo -e "\n9. Procesos usando audio:"
sudo fuser -v /dev/snd/* 2>/dev/null || echo "Ningún proceso detectado"

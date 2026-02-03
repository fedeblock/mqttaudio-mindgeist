#!/bin/bash
# Diagnósticos completos de ALSA para mqttaudio

echo "🔍 DIAGNÓSTICO DE ALSA"
echo "====================="

echo "1. Verificar dispositivos ALSA disponibles:"
aplay -L | grep -E "(mqttaudio|dmix|default|seeed)" | head -10

echo ""
echo "2. Probar dispositivo mqttaudio:"
if aplay -L | grep -q "mqttaudio"; then
    echo "✅ Dispositivo mqttaudio encontrado"
    speaker-test -D mqttaudio -t sine -f 440 -l 1 -s 1 2>/dev/null && echo "✅ mqttaudio funciona" || echo "❌ Error en mqttaudio"
else
    echo "❌ Dispositivo mqttaudio NO encontrado"
    echo "Necesitas aplicar la configuración /etc/asound.conf optimizada"
fi

echo ""
echo "3. Configuración actual de /etc/asound.conf:"
if [ -f /etc/asound.conf ]; then
    echo "✅ /etc/asound.conf existe"
    if grep -q "mqttaudio" /etc/asound.conf; then
        echo "✅ Configuración mqttaudio encontrada"
    else
        echo "❌ Configuración mqttaudio NO encontrada"
    fi
else
    echo "❌ /etc/asound.conf NO existe"
fi

echo ""
echo "4. Procesos usando audio:"
sudo fuser -v /dev/snd/* 2>/dev/null || echo "Ningún proceso detectado usando /dev/snd"

echo ""
echo "5. Estado de tarjetas de sonido:"
cat /proc/asound/cards

echo ""
echo "6. Información de PCM:"
cat /proc/asound/pcm

echo ""
echo "7. Probar dispositivos alternativos:"
echo "   - default:"
speaker-test -D default -t sine -f 440 -l 1 -s 1 2>/dev/null && echo "✅ default funciona" || echo "❌ Error en default"

echo "   - sysdefault:CARD=seeed2micvoicec:"
speaker-test -D "sysdefault:CARD=seeed2micvoicec" -t sine -f 440 -l 1 -s 1 2>/dev/null && echo "✅ sysdefault funciona" || echo "❌ Error en sysdefault"

echo "   - hw:seeed2micvoicec:"
speaker-test -D "hw:seeed2micvoicec" -t sine -f 440 -l 1 -s 1 2>/dev/null && echo "✅ hw directo funciona" || echo "❌ Error en hw directo"

echo ""
echo "8. Logs recientes de mqttaudio:"
sudo journalctl -u mqttaudio.service --since "5 minutes ago" --no-pager

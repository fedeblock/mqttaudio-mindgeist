#!/bin/bash

# Script para actualizar mqttaudio de forma segura
# Uso: ./update_mqttaudio.sh [ruta_al_nuevo_binario]

set -e

SERVICE_NAME="mqttaudio"
NEW_BINARY_PATH="${1:-/home/pi/mqttaudio-mindgeist/mqttaudio}"
CURRENT_BINARY_PATH="/usr/local/bin/mqttaudio"
BACKUP_DIR="/usr/local/bin/backups"

# Configuración específica para ReSpeaker
MQTT_SERVER="localhost"
MQTT_PORT="12183"
MQTT_TOPIC="audio/commands"
ALSA_DEVICE="sysdefault:CARD=seeed2micvoicec"

echo "🔄 Iniciando actualización de mqttaudio..."

# Verificar si se ejecuta como root
if [[ $EUID -ne 0 ]]; then
   echo "❌ Este script debe ejecutarse como root (sudo)"
   exit 1
fi

# Crear directorio de backups si no existe
mkdir -p "$BACKUP_DIR"

# Usar la ubicación conocida del binario
echo "🔍 Verificando binario actual en: $CURRENT_BINARY_PATH"

if [ ! -f "$CURRENT_BINARY_PATH" ]; then
    echo "❌ No se encontró el binario actual en: $CURRENT_BINARY_PATH"
    exit 1
fi

echo "📍 Binario actual confirmado en: $CURRENT_BINARY_PATH"

# Verificar que el nuevo binario existe
if [ ! -f "$NEW_BINARY_PATH" ]; then
    echo "❌ El nuevo binario no existe en: $NEW_BINARY_PATH"
    exit 1
fi

# Verificar que el nuevo binario es ejecutable
if [ ! -x "$NEW_BINARY_PATH" ]; then
    echo "❌ El nuevo binario no es ejecutable: $NEW_BINARY_PATH"
    exit 1
fi

# Probar que el nuevo binario funciona
echo "🧪 Probando el nuevo binario..."
if ! "$NEW_BINARY_PATH" --help &>/dev/null; then
    echo "❌ El nuevo binario no parece funcionar correctamente"
    exit 1
fi

# Verificar estado del servicio
echo "📊 Verificando estado del servicio..."
if systemctl is-active --quiet "$SERVICE_NAME"; then
    SERVICE_WAS_RUNNING=true
    echo "✅ Servicio $SERVICE_NAME está ejecutándose"
else
    SERVICE_WAS_RUNNING=false
    echo "ℹ️  Servicio $SERVICE_NAME no está ejecutándose"
fi

# Detener el servicio si está corriendo
if [ "$SERVICE_WAS_RUNNING" = true ]; then
    echo "⏹️  Deteniendo servicio $SERVICE_NAME..."
    systemctl stop "$SERVICE_NAME"
    
    # Esperar a que se detenga completamente
    sleep 2
    
    if systemctl is-active --quiet "$SERVICE_NAME"; then
        echo "❌ No se pudo detener el servicio $SERVICE_NAME"
        exit 1
    fi
    echo "✅ Servicio detenido correctamente"
fi

# Hacer backup del binario actual
BACKUP_NAME="mqttaudio.backup.$(date +%Y%m%d_%H%M%S)"
BACKUP_PATH="$BACKUP_DIR/$BACKUP_NAME"

echo "💾 Creando backup en: $BACKUP_PATH"
cp "$CURRENT_BINARY_PATH" "$BACKUP_PATH"
echo "✅ Backup creado exitosamente"

# Copiar el nuevo binario
echo "📦 Instalando nuevo binario..."
cp "$NEW_BINARY_PATH" "$CURRENT_BINARY_PATH"
chmod +x "$CURRENT_BINARY_PATH"
chown root:root "$CURRENT_BINARY_PATH"
echo "✅ Nuevo binario instalado"

# Recargar systemd
echo "🔄 Recargando systemd..."
systemctl daemon-reload

# Reiniciar el servicio si estaba corriendo
if [ "$SERVICE_WAS_RUNNING" = true ]; then
    echo "▶️  Iniciando servicio $SERVICE_NAME..."
    systemctl start "$SERVICE_NAME"
    
    # Esperar un momento y verificar estado
    sleep 3
    
    if systemctl is-active --quiet "$SERVICE_NAME"; then
        echo "✅ Servicio $SERVICE_NAME iniciado correctamente"
    else
        echo "❌ Error al iniciar el servicio $SERVICE_NAME"
        echo "📋 Estado del servicio:"
        systemctl status "$SERVICE_NAME" --no-pager
        echo ""
        echo "🔙 Restaurando backup..."
        cp "$BACKUP_PATH" "$CURRENT_BINARY_PATH"
        systemctl start "$SERVICE_NAME"
        echo "❌ Actualización fallida, backup restaurado"
        exit 1
    fi
fi

# Mostrar información de la versión
echo ""
echo "🎉 Actualización completada exitosamente!"
echo "📍 Binario actualizado: $CURRENT_BINARY_PATH"
echo "💾 Backup disponible en: $BACKUP_PATH"
echo "🔧 Configuración MQTT: $MQTT_SERVER:$MQTT_PORT -> $MQTT_TOPIC"
echo "🔊 Dispositivo Audio: $ALSA_DEVICE"
echo "🏠 Sistema detectado: ReSpeaker + Rhasspy"
echo ""
echo "📊 Estado del servicio:"
systemctl status "$SERVICE_NAME" --no-pager -l

echo ""
echo "📝 Para ver los logs en tiempo real:"
echo "   sudo journalctl -u $SERVICE_NAME -f"
echo ""
echo "🧪 Para probar el servicio:"
echo "   mosquitto_pub -h $MQTT_SERVER -p $MQTT_PORT -t '$MQTT_TOPIC' -m '{\"command\": \"precache\", \"message\": {\"file\": \"test.wav\"}}'"
echo ""
echo "🗑️  Para limpiar backups antiguos:"
echo "   sudo find $BACKUP_DIR -name 'mqttaudio.backup.*' -mtime +30 -delete"
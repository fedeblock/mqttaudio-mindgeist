#!/bin/bash
# Prueba de extremo a extremo de la autenticación MQTT de mqttaudio.
# Levanta un mosquitto local que exige usuario y contraseña y aplica un ACL, y comprueba que:
#   1. con credenciales correctas mqttaudio conecta y recibe órdenes de quien puede enviarlas;
#   2. una orden de un usuario sin permiso no llega;
#   3. con contraseña errónea o sin credenciales sale con un mensaje claro y un código distinto de 0;
#   4. la contraseña no aparece nunca en el registro ni en la lista de procesos.
# Necesita mosquitto, mosquitto_passwd, mosquitto_pub y el binario compilado (make).
# Uso: ./test_auth.sh
set -u
cd "$(dirname "$0")"
for b in mosquitto mosquitto_passwd mosquitto_pub ./mqttaudio; do
  command -v "$b" >/dev/null 2>&1 || [ -x "$b" ] || { echo "falta $b"; exit 2; }
done

PORT=18850
DIR=$(mktemp -d)
FAIL=0
ok()   { echo "ok    $1"; }
fail() { echo "FALLA $1"; FAIL=$((FAIL+1)); }
cleanup() { kill $MOSQ $PID 2>/dev/null; wait 2>/dev/null; rm -rf "$DIR"; }
trap cleanup EXIT

# Contraseñas de usar y tirar (no son claves reales)
PW_AUDIO=$(head -c 6 /dev/urandom | od -An -tx1 | tr -d ' \n')
PW_NR=$(head -c 6 /dev/urandom | od -An -tx1 | tr -d ' \n')
mosquitto_passwd -c -b "$DIR/passwd" mqttaudio "$PW_AUDIO" >/dev/null 2>&1
mosquitto_passwd -b "$DIR/passwd" node-red "$PW_NR" >/dev/null 2>&1
mosquitto_passwd -b "$DIR/passwd" blk-hw "x$PW_NR" >/dev/null 2>&1
cat > "$DIR/acl" <<ACL
user mqttaudio
topic read audio/commands
user node-red
topic write audio/commands
user blk-hw
topic write blk/v1/hw/+/event
ACL
RUN_AS=""; [ "$(id -u)" = "0" ] && RUN_AS="user root"
printf '%s\nlistener %s 127.0.0.1\nallow_anonymous false\npassword_file %s/passwd\nacl_file %s/acl\n' \
  "$RUN_AS" "$PORT" "$DIR" "$DIR" > "$DIR/m.conf"
mosquitto -c "$DIR/m.conf" >/dev/null 2>&1 & MOSQ=$!
sleep 1

start() {  # $1 = usuario ("" = sin), $2 = contraseña, $3 = registro
  env ${1:+MQTT_USERNAME=$1} ${2:+MQTT_PASSWORD=$2} SDL_AUDIODRIVER=dummy \
    ./mqttaudio -s localhost -p $PORT -t audio/commands -v >"$3" 2>&1 &
  PID=$!
}

# 1 y 2 y 4: credenciales correctas
start mqttaudio "$PW_AUDIO" "$DIR/ok.log"; sleep 2
PS_LINE=$(ps -o args= -p $PID)
mosquitto_pub -h localhost -p $PORT -u node-red -P "$PW_NR" -t audio/commands -m '{"command":"soundPause","message":{"channel":7}}'
sleep 1
grep -q "Authenticating to the MQTT server as 'mqttaudio'" "$DIR/ok.log" && ok "informa del usuario con el que se autentica" || fail "no informa del usuario"
grep -q "Paused channel 7" "$DIR/ok.log" && ok "recibe la orden de quien tiene permiso" || fail "no recibió la orden permitida"
mosquitto_pub -h localhost -p $PORT -u blk-hw -P "x$PW_NR" -t audio/commands -m '{"command":"soundPause","message":{"channel":9}}' 2>/dev/null
sleep 1
grep -q "Paused channel 9" "$DIR/ok.log" && fail "recibió una orden de un usuario SIN permiso" || ok "no recibe la orden de un usuario sin permiso (ACL)"
if grep -q "$PW_AUDIO" "$DIR/ok.log" || echo "$PS_LINE" | grep -q "$PW_AUDIO"; then fail "la contraseña aparece en el registro o en ps"; else ok "la contraseña no aparece en el registro ni en ps"; fi
kill $PID; wait $PID 2>/dev/null

# 3: contraseña errónea
start mqttaudio "contraseña-incorrecta" "$DIR/bad.log"; sleep 2
wait $PID 2>/dev/null; CODE=$?
grep -q "bad user name or password\|not authorized" "$DIR/bad.log" && ok "contraseña errónea: mensaje claro" || fail "contraseña errónea: mensaje poco claro ($(tail -1 "$DIR/bad.log"))"
[ "$CODE" != "0" ] && ok "contraseña errónea: código de salida $CODE" || fail "contraseña errónea: salió con 0"
grep -q "contraseña-incorrecta" "$DIR/bad.log" && fail "la contraseña errónea aparece en el registro" || ok "la contraseña errónea no aparece en el registro"

# 3: sin credenciales
start "" "" "$DIR/anon.log"; sleep 2
wait $PID 2>/dev/null; CODE=$?
grep -q "not authorized\|bad user name" "$DIR/anon.log" && ok "sin credenciales: mensaje claro" || fail "sin credenciales: mensaje poco claro ($(tail -1 "$DIR/anon.log"))"
[ "$CODE" != "0" ] && ok "sin credenciales: código de salida $CODE" || fail "sin credenciales: salió con 0"

echo; [ $FAIL -eq 0 ] && echo "Todas las pruebas pasan." || echo "$FAIL prueba(s) fallan."
exit $FAIL

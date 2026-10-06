// Prueba de redact.h. No necesita SDL, MQTT ni red.
// Uso: make test
#include "redact.h"

#include <cstdio>
#include <cstring>
#include <string>

static int failures = 0;

static void check(bool ok, const char *name)
{
    printf("%s %s\n", ok ? "ok  " : "FALLA", name);
    if (!ok)
        ++failures;
}

static std::string redact(const std::string &s)
{
    return redactSecrets(s.c_str(), s.size());
}

static bool hides(const std::string &out, const char *secret)
{
    return out.find(secret) == std::string::npos && out.find("***") != std::string::npos;
}

int main()
{
    // Las claves de estos casos son inventadas.
    const char *secret = "CLAVE_DE_PRUEBA_123456";

    {
        std::string out = redact(std::string("{\"command\":\"elevenlabs_tts\",\"message\":{\"api_key\":\"") + secret + "\",\"text\":\"hola\"}}");
        check(hides(out, secret), "oculta api_key en un mensaje elevenlabs_tts");
        check(out.find("\"text\":\"hola\"") != std::string::npos, "conserva el resto del mensaje");
        check(out.find("\"command\":\"elevenlabs_tts\"") != std::string::npos, "conserva el comando");
    }
    {
        std::string out = redact(std::string("{\"api_key\" :   \"") + secret + "\"}");
        check(hides(out, secret), "oculta con espacios alrededor de los dos puntos");
    }
    {
        std::string out = redact("{\"api_key\":\"ab\\\"CLAVE_DE_PRUEBA_123456\",\"text\":\"x\"}");
        check(hides(out, "CLAVE_DE_PRUEBA_123456") && out.find("\"text\":\"x\"") != std::string::npos,
              "oculta un valor con comillas escapadas y sigue después");
    }
    {
        std::string out = redact(std::string("{\"message\":{\"api_key\":\"") + secret);
        check(hides(out, secret), "oculta hasta el final si el JSON está truncado");
    }
    {
        std::string out = redact(std::string("{\"a\":{\"api_key\":\"") + secret + "\"},\"b\":{\"xi-api-key\":\"" + secret + "\"}}");
        check(hides(out, secret), "oculta varias apariciones y distintos nombres");
    }
    {
        const std::string in = "{\"command\":\"play\",\"message\":{\"file\":\"a.mp3\",\"channel\":1}}";
        check(redact(in) == in, "no cambia un mensaje sin claves");
    }
    {
        const std::string in = "{\"text\":\"usa \\\"api_key\\\" sin valor\"}";
        check(redact(in) == in, "no toca el texto que solo nombra api_key");
    }
    {
        const std::string in = "{\"api_key\": 123}";
        check(redact(in) == in, "no toca un valor que no es una cadena");
    }
    {
        check(redactSecrets(nullptr, 0).empty(), "payload nulo devuelve cadena vacía");
    }
    {
        // Respeta la longitud: ignora lo que haya después de 'len'.
        const char *buf = "{\"a\":1}XXXXXXXX";
        check(redactSecrets(buf, 7) == "{\"a\":1}", "respeta la longitud del payload");
    }

    printf(failures == 0 ? "\nTodas las pruebas pasan.\n" : "\n%d prueba(s) fallan.\n", failures);
    return failures == 0 ? 0 : 1;
}

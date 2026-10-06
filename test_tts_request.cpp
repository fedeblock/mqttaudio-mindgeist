// Prueba de tts_request.h. No necesita SDL, MQTT ni red.
// Uso: make test
#include "tts_request.h"

#include <cstdio>
#include <string>

#include "rapidjson/document.h"

static int failures = 0;

static void check(bool ok, const std::string &name)
{
    printf("%s %s\n", ok ? "ok  " : "FALLA", name.c_str());
    if (!ok)
        ++failures;
}

// Ida y vuelta: se construye el cuerpo, se parsea y el texto debe volver igual.
static void roundTrip(const std::string &text, const std::string &name)
{
    std::string body;
    if (!buildTtsBody(text, body))
    {
        check(false, name + " (buildTtsBody devolvió false)");
        return;
    }
    rapidjson::Document d;
    d.Parse(body.c_str());
    const bool valid = !d.HasParseError() && d.IsObject() && d.MemberCount() == 1 &&
                       d.HasMember("text") && d["text"].IsString();
    const bool same = valid && std::string(d["text"].GetString(), d["text"].GetStringLength()) == text;
    check(same, name);
}

int main()
{
    // --- buildTtsBody: ida y vuelta ---
    roundTrip("Hola, ¿qué tal?", "texto simple");
    roundTrip("Dijo \"hola\" y se fue", "comillas dobles");
    roundTrip("It's fine", "comilla simple");
    roundTrip("ruta C:\\datos\\nuevo", "barra invertida");
    roundTrip("línea 1\nlínea 2\r\nlínea 3", "saltos de línea");
    roundTrip("col1\tcol2", "tabulador");
    roundTrip(std::string("control \x01\x02\x1f fin"), "caracteres de control");
    roundTrip("¿Qué hora es? áéíóú ñ Ü", "acentos y signos españoles");
    roundTrip("emoji \xF0\x9F\x98\x80 de cuatro bytes", "emoji de 4 bytes");
    roundTrip("", "cadena vacía");
    roundTrip("a/b?c=d&e", "caracteres de URL");
    roundTrip("{\"text\":\"inyectado\"}", "JSON dentro del texto");
    {
        std::string largo;
        for (int i = 0; i < 2000; ++i)
            largo += "frase \"larga\" número " + std::to_string(i) + "\\\n";
        roundTrip(largo, "texto largo (" + std::to_string(largo.size()) + " bytes)");
    }
    {
        // Un texto con NUL no debe romper el JSON.
        roundTrip(std::string("a\0b", 3), "byte NUL dentro del texto");
    }

    // --- buildTtsBody: formato y errores ---
    {
        std::string body;
        buildTtsBody("hola", body);
        check(body == "{\"text\":\"hola\"}", "formato exacto del cuerpo");
    }
    {
        std::string body = "intacto";
        check(!buildTtsBody("mal \xC3\x28 utf8", body), "rechaza UTF-8 inválido");
        check(body == "intacto", "no toca 'out' si falla");
    }
    {
        std::string body = "intacto";
        check(!buildTtsBody("truncado \xE2\x82", body), "rechaza una secuencia UTF-8 incompleta");
    }

    // --- isSafeToken ---
    check(isSafeToken("29vD33N1CtxCmqQRPOHJ", 64), "acepta un voice_id real");
    check(isSafeToken("21m00Tcm4TlvDq8ikWAM", 64), "acepta otro voice_id real");
    check(isSafeToken("mp3", 8), "acepta 'mp3'");
    check(isSafeToken("a_b-C9", 8), "acepta '_' y '-'");
    check(!isSafeToken("", 64), "rechaza la cadena vacía");
    check(!isSafeToken("a/b", 64), "rechaza '/'");
    check(!isSafeToken("a?b", 64), "rechaza '?'");
    check(!isSafeToken("..", 64), "rechaza '..'");
    check(!isSafeToken("a b", 64), "rechaza espacios");
    check(!isSafeToken("mp3\r\nX-Evil: 1", 8), "rechaza saltos de línea (inyección de cabecera)");
    check(!isSafeToken("a%2Fb", 64), "rechaza '%'");
    check(!isSafeToken(std::string(65, 'a'), 64), "rechaza más de 64 caracteres");
    check(isSafeToken(std::string(64, 'a'), 64), "acepta exactamente 64 caracteres");
    check(!isSafeToken("mp3mp3mp3", 8), "rechaza más del máximo indicado");
    check(!isSafeToken("n\xC3\xB1", 64), "rechaza letras no ASCII");

    printf(failures == 0 ? "\nTodas las pruebas pasan.\n" : "\n%d prueba(s) fallan.\n", failures);
    return failures == 0 ? 0 : 1;
}

#ifndef MQTTAUDIO_TTS_REQUEST_H
#define MQTTAUDIO_TTS_REQUEST_H

#include <cstddef>
#include <string>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

// Utilidades para construir la petición a ElevenLabs sin concatenar a mano
// datos que vienen de un mensaje MQTT. No dependen de SDL ni de la red.

// true si 's' tiene entre 1 y maxLen caracteres y todos son letras ASCII,
// números, '_' o '-'. Se usa para los valores que van dentro de la URL
// (voice_id) o de una cabecera HTTP (format): con '/', '?', '..' o un salto de
// línea cambiarían la ruta o inyectarían cabeceras.
inline bool isSafeToken(const std::string &s, size_t maxLen)
{
    if (s.empty() || s.size() > maxLen)
        return false;
    for (std::string::size_type i = 0; i < s.size(); ++i)
    {
        const char c = s[i];
        const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                        (c >= '0' && c <= '9') || c == '_' || c == '-';
        if (!ok)
            return false;
    }
    return true;
}

// Construye el cuerpo JSON {"text": ...} con el Writer de rapidjson, que escapa
// comillas, barras, saltos de línea, caracteres de control y Unicode.
// Devuelve false si el texto no es UTF-8 válido; en ese caso 'out' no se toca.
inline bool buildTtsBody(const std::string &text, std::string &out)
{
    typedef rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>,
                              rapidjson::CrtAllocator, rapidjson::kWriteValidateEncodingFlag>
        ValidatingWriter;

    rapidjson::StringBuffer sb;
    ValidatingWriter w(sb);
    w.StartObject();
    w.Key("text");
    if (!w.String(text.c_str(), static_cast<rapidjson::SizeType>(text.size())))
        return false;
    w.EndObject();
    if (!w.IsComplete())
        return false;

    out.assign(sb.GetString(), sb.GetSize());
    return true;
}

#endif // MQTTAUDIO_TTS_REQUEST_H

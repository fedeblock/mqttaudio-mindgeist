#ifndef MQTTAUDIO_REDACT_H
#define MQTTAUDIO_REDACT_H

#include <cctype>
#include <cstddef>
#include <string>

// Sustituye por "***" el valor de los campos que llevan claves en un JSON en
// texto. No parsea el JSON, así que funciona también con mensajes inválidos o
// truncados. Se usa antes de registrar un payload: una clave de API no debe
// acabar en el journal cuando un mensaje MQTT falla.
//
// Solo cambia el valor de "api_key", "xi-api-key", "apiKey" y "authorization".
inline std::string redactSecrets(const char *payload, size_t len)
{
    std::string s(payload != nullptr ? payload : "", payload != nullptr ? len : 0);
    static const char *const names[] = {"\"api_key\"", "\"xi-api-key\"", "\"apiKey\"", "\"authorization\""};

    for (const char *name : names)
    {
        const std::string key(name);
        size_t pos = 0;
        while ((pos = s.find(key, pos)) != std::string::npos)
        {
            size_t p = pos + key.size();
            while (p < s.size() && std::isspace(static_cast<unsigned char>(s[p])))
                ++p;
            if (p >= s.size() || s[p] != ':')
            {
                pos += key.size();
                continue;
            }
            ++p;
            while (p < s.size() && std::isspace(static_cast<unsigned char>(s[p])))
                ++p;
            if (p >= s.size() || s[p] != '"')
            {
                pos = p;
                continue;
            }

            // Busca la comilla de cierre y respeta los caracteres escapados.
            size_t q = p + 1;
            while (q < s.size())
            {
                if (s[q] == '\\')
                {
                    q += 2;
                    continue;
                }
                if (s[q] == '"')
                    break;
                ++q;
            }
            // Sin comilla de cierre (JSON truncado): se oculta hasta el final.
            const size_t end = q < s.size() ? q : s.size();
            s.replace(p + 1, end - (p + 1), "***");
            pos = p + 4;
        }
    }
    return s;
}

#endif // MQTTAUDIO_REDACT_H

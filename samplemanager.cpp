#include "samplemanager.h"

Sample* SampleManager::GetSample(const char * uri)
{
    if (_database.find(uri) != _database.end())
    {
        return _database[uri];
    }
    else
    {
        Sample* sample = new Sample(uri);
        if (sample->isValid())
        {
            std::string key = uri;
            _database.insert({key, sample});
            return sample;
        }
        else
        {
        return 0;
        }
    }
}

void SampleManager::RemoveSample(const std::string& filename)
{
    auto it = _database.find(filename);
    if (it != _database.end())
    {
        delete it->second;  // Libera la memoria asociada con el sample
        _database.erase(it);  // Elimina la entrada del caché
        if (verbose)
        {
            printf("Sample '%s' removed from cache.\n", filename.c_str());
        }
    }
}



void SampleManager::FreeAll()
{
    for( const auto& s : _database ) {
        s.second->Free();
    }
}
#ifndef SAMPLEMANAGER_H
#define SAMPLEMANAGER_H

#include <string>
#include <unordered_map>

#include "sample.h"

using namespace std;

class SampleManager {
public:
    SampleManager(bool verbose) : verbose(verbose) {}
    Sample* GetSample(const char* uri);
    void FreeAll();
    void RemoveSample(const std::string& filename);
private:
    std::unordered_map<std::string, Sample*> _database;
    bool verbose;  
};


#endif

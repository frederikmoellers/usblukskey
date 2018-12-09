#include <fstream>
#include <string>
#include <unordered_map>

#include "settings.hpp"

static const std::string DEVICE_ID("Device ID");
static const std::string KEY_LENGTH("Key Length");
static const std::string KEY_OFFSET("Key Offset");
static const std::string PASSPHRASE_PROMPT("Passphrase Prompt");
static const std::string POLLING_INTERVAL("Polling Interval");

extern void printd(std::string, bool = false);

/**
 * @brief Reads settings from a file
 */
SettingsContainer::SettingsContainer(std::string filename)
{
    this->defaults();
    std::ifstream file;
    try
    {
        file.open(filename);
        std::string line;
        while(file.good())
        {
            std::getline(file, line);
            for(auto it : this->settings)
            {
                size_t length = it.first.length();
                if(line.compare(0, length, it.first) == 0 && line[length] == '=')
                {
                    setting_t* setting = &(this->settings[it.first]);
                    std::string value_part = line.substr(length + 1);
                    switch(setting->first)
                    {
                        case SETTING_TYPE_SIZET:
                            delete (size_t*) setting->second;
                            setting->second = new size_t(std::stoul(value_part));
                            break;
                        case SETTING_TYPE_STRING:
                            delete (std::string*) setting->second;
                            setting->second = new std::string(value_part);
                            break;
                    }
                }
            }
        }
    }
    catch(std::exception e)
    {
        if(file.is_open())
            file.close();
        printd("Error parsing config file.");
        printd(e.what());
    }
}

std::string SettingsContainer::device_id() const
{
    return *((std::string*) (this->settings.at(DEVICE_ID).second));
}

void SettingsContainer::defaults()
{
    this->settings[DEVICE_ID] = {SETTING_TYPE_STRING, new std::string("there-is-no-good-default")};
    this->settings[KEY_LENGTH] = {SETTING_TYPE_SIZET, new size_t(8192)};
    this->settings[KEY_OFFSET] = {SETTING_TYPE_SIZET, new size_t(0)};
    this->settings[PASSPHRASE_PROMPT] = {SETTING_TYPE_STRING, new std::string("")};
    this->settings[POLLING_INTERVAL] = {SETTING_TYPE_SIZET, new size_t(200)};
}

size_t SettingsContainer::key_length() const
{
    return *((size_t*) (this->settings.at(KEY_LENGTH).second));
}

size_t SettingsContainer::key_offset() const
{
    return *((size_t*) (this->settings.at(KEY_OFFSET).second));
}

std::string SettingsContainer::passphrase_prompt() const
{
    return *((std::string*) (this->settings.at(PASSPHRASE_PROMPT).second));
}

size_t SettingsContainer::polling_interval() const
{
    return *((size_t*) (this->settings.at(POLLING_INTERVAL).second));
}

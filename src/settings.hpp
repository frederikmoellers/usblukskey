#include <string>
#include <unordered_map>

class SettingsContainer
{
    public:
        SettingsContainer(std::string filename);
        ~SettingsContainer();

        std::string device_id() const;
        size_t key_length() const;
        size_t key_offset() const;
        std::string passphrase_prompt() const;
        size_t polling_interval() const;

    private:
        typedef enum {
            SETTING_TYPE_SIZET,
            SETTING_TYPE_STRING,
        } setting_type_t;
        typedef std::pair<setting_type_t, void*> setting_t;

        std::unordered_map<std::string, setting_t> settings;

        void defaults();
};

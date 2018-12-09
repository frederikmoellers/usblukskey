#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/thread.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <string>

#include "settings.hpp"

// enable debug output? (set by "-v" command line switch)
bool debug = false;
// semaphore to signal the main function that a key has been printed and the program can end
boost::interprocess::interprocess_semaphore done(0);
// mutex to make sure only one thread posts its output
boost::interprocess::interprocess_semaphore post_result(1);

/**
 * @brief Prints a debug message
 * If debug is set, it prints the message to stderr. If not, it stays silent.
 * @param message The message to print
 */
void printd(std::string message, bool noeol = false)
{
    if(debug)
    {
        std::cerr << message;
        if(!noeol)
            std::cerr << std::endl;
    }
}

/**
 * @brief Asks the user for a passphrase.
 * This method should be run from a thread. It does not return until the user
 * has entered a passphrase.
 */
void passphrase_key(SettingsContainer const * settings)
{
    // sleep 2 secs to decrease the chance of other output appearing after our prompt
    boost::this_thread::sleep_for(boost::chrono::milliseconds(2000));
    // prompt
    std::string prompt = settings->passphrase_prompt();
    if(prompt.empty())
    {
        std::cerr << "Please unlock disk";
        const char* crypttab_name = std::getenv("CRYPTTAB_NAME");
        if(crypttab_name != NULL)
        {
            std::cerr << " " << crypttab_name;
        }
        std::cerr << ": ";
    }
    else
    {
        std::cerr << prompt;
    }
    // read passphrase
    std::string passphrase;
    // turn off echo
    struct termios tcattr;
    tcgetattr(STDIN_FILENO, &tcattr);
    struct termios tcattr_noecho = tcattr;
    tcattr_noecho.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tcattr_noecho);
    // read passphrase
    std::getline(std::cin, passphrase);
    // turn on echo
    tcsetattr(STDIN_FILENO, TCSANOW, &tcattr);
    printd("\nRead passphrase");
    // if the lock is taken, we can abort
    if(!post_result.try_wait())
    {
        return;
    }
    std::cout << passphrase;
    done.post();
}

/**
 * @brief Searches for the USB drive and outputs the key stored there.
 * This function waits until the USB drive is available, then loads the key
 * stored on it and prints it to stdout.
 * It should be run from a thread.
 * This method does not return until the USB drive has been found!
 */
void usb_key(SettingsContainer const * settings)
{
    // check if the "usb_storage" module is already loaded
    std::ifstream modules("/proc/modules");
    std::string line;
    bool usb_storage_loaded = false;
    while(modules.good())
    {
        std::getline(modules, line);
        if(line.substr(0, 12) == "usb_storage ")
        {
            usb_storage_loaded = true;
            break;
        }
    }
    modules.close();
    if(!usb_storage_loaded)
    {
        printd("Loading usb_storage module.");
        // there's no better way to do this than using system(), see FAQ
        if(system("modprobe usb_storage >/dev/null 2>&1") != 0)
        {
            // modprobe failed, so we have no hope of getting the key
            printd("Failed to load usb_storage module!");
            return;
        }
    }
    printd("usb_storage module is loaded.");
    std::string device_file_name("/dev/disk/by-id/");
    device_file_name.append(settings->device_id());
    // wait for the device to appear
    while(true)
    {
        if(boost::filesystem::status(boost::filesystem::path(device_file_name)).type() == boost::filesystem::block_file)
        {
            printd("Found the device!");
            break;
        }
        printd(".", true);
        boost::this_thread::sleep_for(boost::chrono::milliseconds(settings->polling_interval()));
    }
    // read the key
    std::ifstream device_file(device_file_name, std::ifstream::in | std::ifstream::binary);
    device_file.seekg(settings->key_offset());
    char key[settings->key_length()];
    device_file.read(key, settings->key_length());
    device_file.close();
    // if the lock can't be acquired, we can abort
    if(!post_result.try_wait())
    {
        return;
    }
    std::cout.write(key, settings->key_length());
    done.post();
}

int main(int argc, char* argv[])
{
    if(argc >= 2 && std::string("-v") == argv[1])
    {
        debug = true;
    }
    SettingsContainer* settings = new SettingsContainer("/etc/usblukskey/settings");
    boost::thread usb_thread(usb_key, settings);
    boost::thread passphrase_thread(passphrase_key, settings);
    done.wait();
    return 0;
}

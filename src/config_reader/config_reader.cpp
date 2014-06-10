#include "config_reader.hpp"
#include "errors.hpp"
#include "string_functions.hpp"

#include <algorithm>
#include <cassert>
#include <sstream>

namespace cdnalizerd {

struct ConfigReader {
    std::istream& in;

    strings usernames;
    strings apikeys;
    strings containers;
    std::vector<TempEntry> entries;
    size_t linenum = 0;

    void addEntry(std::string path1, std::string path2) {
        if ((usernames.size() == 0) ||
            (apikeys.size() == 0) ||
            (containers.size() == 0))
            throw ConfigError("Need to have a valid username, apikey and container before adding a path pair");
        entries.push_back(
            {usernames.size()-1,
             apikeys.size()-1,
             containers.size()-1,
             path1, path2});
    }

    /// Reads a single line of config
    void readLine() {
        ++linenum;
        std::string line;
        getline(in, line);
        // Ignore empty lines
        string_fun::trim(line);
        if (line.empty())
            return;
        // See if it starts with a '/'
        if (line.front() == '/')
            processPathLine(line);
        else
            processSettingLine(line);
    }

    using P = std::string::const_iterator;

    /// Process a path .. needs a valid config entry
    void processPathLine(const std::string& line) {
        // Read the first path
        std::string path1;
        P start = readPath(line.begin(), line.end(), path1);
        // Skip any white space in the middle
        start = std::find_if(start, line.end(), [](char x) { return (x != ' ') && (x != '\t'); });
        // Read the second path
        std::string path2;
        readPath(start, line.end(), path2);
        // Store the paths in the configuration
        addEntry(path1, path2);
    }

    /** Reads in a single path. Handles quoteed strings */
    P readPath(P start, P end, std::string& output) {
        P end_of_path;
        if (*start == '/') {
            // If it starts with a / it's a straight forward path with no spaces
            end_of_path = find(start, end, ' ');
            std::copy(start, end_of_path, std::back_inserter(output));
        } else {
            if (*start != '"')
                throw ParseError("Paths should start with a '/' or a '\"'");
            // This is a quoted string
            end_of_path = string_fun::dequoteString(start, end, output);
        }
        return end_of_path;
    }

    /// Processes a setting pair separated by an '='
    void processSettingLine(const std::string& line) {
        // Get the variable and its value
        auto eq = std::find(line.begin(), line.end(), '=');
        std::string variable,
                    value;
        std::copy(line.begin(), eq, std::back_inserter(variable));
        std::copy(++eq, line.end(), std::back_inserter(value));
        // Store the value
        if (variable == "username")
            usernames.push_back(value);
        else if (variable == "apikey")
            apikeys.push_back(value);
        else if (variable == "container")
            containers.push_back(value);
        else {
            std::stringstream msg;
            msg << "Unkown setting '" << variable
                << "' on line " << linenum 
                << " of config file";
            throw ConfigError(msg.str());
        }


    }

    ConfigReader(std::istream& in) : in(in) {
        while (in.good())
            readLine();
    }
};

Config read_config(std::istream& in) {
    ConfigReader reader(in);
    return {std::move(reader.usernames), std::move(reader.apikeys), std::move(reader.containers), std::move(reader.entries)};
}

}

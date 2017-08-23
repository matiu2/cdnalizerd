#include "config_reader.hpp"
#include "errors.hpp"
#include "../logging.hpp"

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <cassert>
#include <iostream>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

/* Now supports boost.property_tree, so it can import json/xml/info/ini files.
 *
 * To make things simple, we'll have one config file per pair.
 *
 * It'll load all config files from:
 *
 *   /etc/cdnalizerd/
 *
 * File format:
 * username = matiu
 * apikey = blah-blah
 * region = DFW
 * container = cdn.supa.ws
 * snet = false
 * local_path = /var/www/vhosts/supa.ws/images/
 * remote_path = /images/
 *
 * -----
 *
 * File Format if it was .info
 *
 * username matiu
 * apikey blah-blah
 * region DFW
 * container cdn.supa.ws
 * local_path /var/www/vhosts/supa.ws/images/
 * remote_path /images/
 *
 * ----
 *
 * File format for .json
 *
 * {
 *   "name" : "matiu-dfw-cdn",
 *   "username" : "matiu",
 *   "apikey" : "blah-blah",
 *   "region" : "DFW",
 *   "container" : "cdn.supa.ws",
 *   "local_path" : "/var/www/vhosts/supa.ws/images/",
 *   "remote_path" : "/images/"
 * }
 *
 * ----
 *
 * File format for .xml ??
 *
 */

namespace cdnalizerd {

using boost::property_tree::ptree;

struct ConfigReader {

  Config config;

  void readFile(const std::string &filename) {
    try {
      ptree pt;
      // We can read any kind of config file from here:
      // http://www.boost.org/doc/libs/1_61_0/doc/html/property_tree/parsers.html
      if (boost::iends_with(filename, ".ini"))
        boost::property_tree::ini_parser::read_ini(filename, pt);
      else if (boost::iends_with(filename, ".json"))
        boost::property_tree::json_parser::read_json(filename, pt);
      else if (boost::iends_with(filename, ".xml"))
        boost::property_tree::xml_parser::read_xml(filename, pt);
      else if (boost::iends_with(filename, ".info"))
        boost::property_tree::info_parser::read_info(filename, pt);
      parseTree(pt);
    } catch (...) {
      LOG_S(ERROR) << "Unable to load config file: '" << filename << "': "
                   << boost::current_exception_diagnostic_information(true)
                   << std::endl;
    }
  }

  void parseTree(const ptree& pt) {
    config.addUsername(pt.get<std::string>("username"));
    config.addApiKey(pt.get<std::string>("apikey"));
    config.addRegion(pt.get<std::string>("region"));
    config.addContainer(pt.get<std::string>("container"));
    config.setSNet(pt.get("snet", false));
    config.addEntry(pt.get<std::string>("local_dir"), pt.get<std::string>("remote_dir"));
  }

  Config &&getConfig() {
    return std::move(config);
  }
};

/// Reads a configuration directory (eg. /etc/cdnalizerd)
Config read_config(const std::string &dir_name) {
  namespace fs = boost::filesystem;

  ConfigReader reader;

  try {
    fs::path dir(dir_name);
    if (fs::is_directory(dir)) {
      fs::directory_iterator end;
      for (auto f = fs::directory_iterator(dir); f != end; ++f)
        reader.readFile(f->path().native());
    } else {
      // This isn't a directory name, it's a filename
      reader.readFile(dir_name);
    }

    Config result = reader.getConfig();
    return result;
  } catch (...) {
    LOG_S(ERROR) << "Unable to load config directory: '"
                 << boost::current_exception_diagnostic_information(true)
                 << std::endl;
    throw;
  }
}
}

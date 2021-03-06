/* # Copyright 2014-2017 Peter Vrabel(kybu@kybu.org)
#
# This file is part of 'Headless Selenium for Win'.
#
# 'Headless Selenium for Win' is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# 'Headless Selenium for Win' is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with 'Headless Selenium for Win'.If not, see <http://www.gnu.org/licenses/>. */

//
// Following http://utf8everywhere.org/ convention.

#include "stdafx.h"

#include "../version.h"

int version[] = {PRODUCTVER};

using namespace std;
namespace po = boost::program_options;
namespace bo = boost;

string headlessCmd,
       desktopName = "HeadlessDesktop",
       switchToDesktop,

       header = (bo::format(
         "Desktop utils v%1%.%2%, Peter Vrabel (c) 2014-2017")
         % version[0] % version[1]).str();
bool noExplorer = false;

enum class ParseStatus {
  OK = 1, FAILED, HELP_SHOWN, EXIT_0
};

ParseStatus parseCommandLine(int argc, _TCHAR *argv[]) try {
  po::options_description cmdOptions(header);

  cmdOptions.add_options()
      ("help,h", "Feeling desperate?")
      ("run,r", po::value(&headlessCmd), "Command to run headlessly.")
      ("desktop,n", po::value(&desktopName), "Set the headless desktop name. Used with '--run'. Optional, default = HeadlessDesktop")
      ("list,l", "List available desktops of current Window station.")
      ("switch-to,s", po::value(&switchToDesktop), "Switch to a desktop. Takes a desktop name from the list of desktops.")
      ("switch-to-default,t", "Switch to the default desktop. Can be used if you are being stranded.")
	    ("no-explorer,x", po::bool_switch(&noExplorer), "Don't run explorer in the created desktop");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, cmdOptions), vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << cmdOptions << endl;

    return ParseStatus::HELP_SHOWN;
  }

  if (vm.count("switch-to-default")) {
    Desktop::switchToDefault();

    return ParseStatus::EXIT_0;
  }

  if (vm.count("switch-to")) {
    Desktop::switchTo(switchToDesktop);

    return ParseStatus::EXIT_0;
  }

  if (vm.count("list")) {
    cout << header << endl << endl
         << "* Available desktops:" << endl;

    for (auto desktop : Desktop::desktops())
    cout << desktop << endl;
      
    return ParseStatus::EXIT_0;
  }

  if (!vm.count("run"))
    throw runtime_error("--run option must be specified!");

  return ParseStatus::OK;
}
catch (po::error &e) {
  LOGF << e.what();

  return ParseStatus::FAILED;
}

void setupLogger() {
  using namespace boost::log;

  add_console_log(
    cout,
    keywords::format = "%Message%");

#ifdef _DEBUG
  core::get()->set_filter(
    trivial::severity >= trivial::trace);
#else
  core::get()->set_filter(
    trivial::severity >= trivial::info);
#endif
}

int _tmain(int argc, _TCHAR* argv[]) {
  try {
    setupLogger();

    switch (parseCommandLine(argc, argv)) {
    case ParseStatus::FAILED:
      return 1;
      break;
    case ParseStatus::EXIT_0:
    case ParseStatus::HELP_SHOWN:
      return 0;
      break;
    };

    HDESK desktopHandle = Desktop::create(desktopName);

    if (noExplorer) {
      Desktop::createProcess(
        desktopName,
        "",
        headlessCmd);

      Sleep(2000);
    }
    else {
      Desktop::createProcess(
        desktopName,
        "C:\\Windows\\explorer.exe");

      Sleep(2000);

      Desktop::createProcess(
        desktopName,
        "",
        headlessCmd);
    }
  }
  catch (runtime_error &e) {
    LOGF << e.what();
    return 1;
  }
  catch (...) {
    LOGF << "A bummer, unknown exception caught!";

    return 1;
  }

  return 0;
}


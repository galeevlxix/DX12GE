#pragma once

#include <QStandardPaths>

#include "json.hpp"
#include <fstream>
#include <iostream>
#include <string>
using json = nlohmann::json;

namespace BianLauncherHelpers
{
    struct BianLauncherSettings
    {
        std::string ProjectsDirectory;
        std::string EditorPath;
    };

    static std::string GetUserPath() 
    {
        QString userPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        return userPath.toStdString();
    }

    static void ReadSettingsFile(BianLauncherSettings* result)
    {
        std::ifstream in;
        in.open("settings.json");

        if (!in.is_open())
        {
            std::cout << "Error! Could not open scene file settings. json" << std::endl;
            RestoreSettingsFile();
            return;
        }

        json settings;
        in >> settings;

        if (settings.contains("should_be_restored"))
        {
            bool should_be_restored = settings.at("should_be_restored");
            if (should_be_restored)
            {
                RestoreSettingsFile();
                return;
            }
        }
        else
        {
            RestoreSettingsFile();
            return;
        }

        if (settings.contains("projects_directory"))
        {
            std::string projects_directory = settings.at("projects_directory");
        }
        else
        {
            RestoreSettingsFile();
            return;
        }
    }

    

}
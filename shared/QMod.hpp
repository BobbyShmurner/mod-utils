#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdlib.h>

#include "modloader-utils/shared/Dependency.hpp"
#include "modloader-utils/shared/FileCopy.hpp"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"

#define ASSERT(condition) if (!condition) return nullptr;
#define ASSERT_VOID(condition) if (!condition) return;

#define GET_STRING(value, parentObject) (parentObject.HasMember(value) && parentObject[value].IsString()) ? parentObject[value].GetString() : ""
#define GET_BOOL(value, parentObject) (parentObject.HasMember(value) && parentObject[value].IsBool()) ? parentObject[value].GetBool() : false

#define GET_ARRAY(value, array, type) \
if (value.IsArray()) { \
	for (rapidjson::SizeType i = 0; i < value.Size(); i++) { \
		array->push_back(value[i].Get##type()); \
	} \
}

#define GET_DEPENDENCIES(value, dependencies) \
if (value.IsArray()) { \
	for (rapidjson::SizeType i = 0; i < value.Size(); i++) { \
		if (value[i].IsObject()) { \
			const rapidjson::Value& dependencyValue = value[i]; \
			\
			std::string id = GET_STRING("id", dependencyValue); \
			std::string version = GET_STRING("version", dependencyValue); \
			std::string downloadIfMissing = GET_STRING("downloadIfMissing", dependencyValue); \
			\
			dependencies->push_back({id, version, downloadIfMissing}); \
		} \
	} \
}

#define GET_FILE_COPIES(value, fileCopies) \
if (value.IsArray()) { \
	for (rapidjson::SizeType i = 0; i < value.Size(); i++) { \
		if (value[i].IsObject()) { \
			const rapidjson::Value& fileCopyValue = value[i]; \
			\
			std::string name = GET_STRING("name", fileCopyValue); \
			std::string destination = GET_STRING("destination", fileCopyValue); \
			\
			fileCopies->push_back({name, destination}); \
		} \
	} \
}

namespace ModloaderUtils {
	class QMod {
	public:
		static QMod* LoadQMod(std::string fileDir) {
			std::system("rm -r \"/sdcard/BMBFData/Mods/Temp/\"");
			std::system("mkdir -p \"/sdcard/BMBFData/Mods/Temp/\"");
			std::system(string_format("unzip %s mod.json -d \"/sdcard/BMBFData/Mods/Temp/\"", fileDir.c_str()).c_str());
			
			std::ifstream qmodFile("/sdcard/BMBFData/Mods/Temp/mod.json");
			ASSERT(qmodFile.good());

			std::stringstream qmodJson;
			qmodJson << qmodFile.rdbuf();

			std::system("rm -r \"/sdcard/BMBFData/Mods/Temp/\"");

			rapidjson::Document document;

			ASSERT(!document.Parse(qmodJson.str().c_str()).HasParseError());
			ASSERT(document.IsObject());

			std::string name = GET_STRING("name", document);
			std::string id = GET_STRING("id", document);
			std::string description = GET_STRING("description", document);
			std::string author = GET_STRING("author", document);
			std::string porter = GET_STRING("porter", document);
			std::string version = GET_STRING("version", document);
			std::string coverImage = GET_STRING("coverImage", document);
			std::string packageId = GET_STRING("packageId", document);
			std::string packageVersion = GET_STRING("packageVersion", document);

			std::vector<std::string>* modFiles = new std::vector<std::string>();
			GET_ARRAY(document["modFiles"], modFiles, String);

			std::vector<std::string>* libraryFiles = new std::vector<std::string>();
			GET_ARRAY(document["libraryFiles"], libraryFiles, String);

			std::vector<Dependency>* dependencies = new std::vector<Dependency>();
			GET_DEPENDENCIES(document["dependencies"], dependencies);

			std::vector<FileCopy>* fileCopies = new std::vector<FileCopy>();
			GET_FILE_COPIES(document["fileCopies"], fileCopies);

			QMod* qmod = new QMod(name, id, description, author, porter, version, coverImage, packageId, packageVersion, modFiles, libraryFiles, dependencies, fileCopies, fileDir);
			return qmod;
		}

		void CollectBMBFData() {
			std::ifstream configFile("/sdcard/BMBFData/config.json");
			ASSERT_VOID(configFile.good());

			std::stringstream configJson;
			configJson << configFile.rdbuf();

			rapidjson::Document document;

			ASSERT_VOID(!document.Parse(configJson.str().c_str()).HasParseError());
			ASSERT_VOID(document.IsObject());

			const rapidjson::Value& mods = document["Mods"].GetArray();

			for (rapidjson::SizeType i = 0; i < mods.Size(); i++) {
				const rapidjson::Value& mod = mods[i];
				std::string id = GET_STRING("Id", mod);

				if (id != m_Id) continue;

				m_Path = GET_STRING("Path", mod);
				m_Installed = GET_BOOL("Installed", mod);
				m_Uninstallable = GET_BOOL("Uninstallable", mod);

				getLogger().info("Found BMBFInfo for \"%s\"", m_Id.c_str());
				getLogger().info("\t- Path: %s", m_Path.c_str());
				getLogger().info("\t- Installed: %s", m_Installed ? "True" : "False");
				getLogger().info("\t- Uninstallable: %s", m_Uninstallable ? "True" : "False");
			}
		}

		const std::string Name() { return m_Name; }
		const std::string Id()  { return m_Id; }
		const std::string Description()  { return m_Description; }
		const std::string Author()  { return m_Author; }
		const std::string Porter()  { return m_Porter; }
		const std::string Version()  { return m_Version; }
		const std::string CoverImage()  { return m_CoverImage; }

		const std::string PackageId()  { return m_PackageId; }
		const std::string PackageVersion()  { return m_PackageVersion; }

		const std::vector<std::string> ModFiles() { return *m_ModFiles; }
		const std::vector<std::string> LibraryFiles() { return *m_LibraryFiles; }
		const std::vector<Dependency> Dependencies() { return *m_Dependencies; }
		const std::vector<FileCopy> FileCopies() { return *m_FileCopies; }

		const std::string Path() { return m_Path; }
		const std::string CoverImageFilename() { return m_CoverImageFilename; }

		const bool Installed() { return m_Installed; }
		const bool Uninstallable() { return m_Uninstallable; }
	private:
		QMod (std::string name, std::string id, std::string description, std::string author, std::string porter, std::string version, std::string coverImage, std::string packageId, std::string packageVersion, std::vector<std::string>* modFiles, std::vector<std::string>* libraryFiles, std::vector<Dependency>* dependencies, std::vector<FileCopy>* fileCopies, std::string path = "", std::string coverImageFilename = "", bool installed = false, bool uninstallable = true)
		: m_Name(name), m_Id(id), m_Description(description), m_Author(author), m_Porter(porter), m_Version(version), m_CoverImage(coverImage), m_PackageId(packageId), m_PackageVersion(packageVersion), m_ModFiles(modFiles), m_LibraryFiles(libraryFiles), m_Dependencies(dependencies), m_FileCopies(fileCopies), m_Path(path), m_CoverImageFilename(coverImageFilename), m_Installed(installed), m_Uninstallable(uninstallable) {}

		std::string m_Name;
		std::string m_Id;
		std::string m_Description;
		std::string m_Author;
		std::string m_Porter;
		std::string m_Version;
		std::string m_CoverImage;

		std::string m_PackageId;
		std::string m_PackageVersion;

		std::vector<std::string>* m_ModFiles;
		std::vector<std::string>* m_LibraryFiles;
		std::vector<Dependency>* m_Dependencies;
		std::vector<FileCopy>* m_FileCopies;

		// BMBF Stuff

		std::string m_Path;
		std::string m_CoverImageFilename;

		bool m_Installed;
		bool m_Uninstallable;
	};
}
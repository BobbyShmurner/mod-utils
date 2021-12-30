#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <memory>

#include "modloader-utils/shared/Dependency.hpp"
#include "modloader-utils/shared/FileCopy.hpp"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/writer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filewritestream.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/error/error.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/error/en.h"

#define ASSERT(condition)                                                                               \
	if (!(condition))                                                                                   \
	{                                                                                                   \
		getLogger().info("ERROR [%s:%i]: Condition \"%s\" Failed!", __FILE__, __LINE__, "" #condition); \
		return nullptr;                                                                                 \
	}
#define ASSERT_VOID(condition)                                                                          \
	if (!(condition))                                                                                   \
	{                                                                                                   \
		getLogger().info("ERROR [%s:%i]: Condition \"%s\" Failed!", __FILE__, __LINE__, "" #condition); \
		return;                                                                                         \
	}

#define GET_STRING(value, parentObject) (parentObject.HasMember(value) && parentObject[value].IsString()) ? parentObject[value].GetString() : ""
#define GET_BOOL(value, parentObject) (parentObject.HasMember(value) && parentObject[value].IsBool()) ? parentObject[value].GetBool() : false

#define CREATE_STRING_VALUE(string)

#define ADD_MEMBER(name, value, object, allocator) \
	if (object.HasMember(name))                    \
	{                                              \
		object.RemoveMember(name);                 \
	}                                              \
	object.AddMember(name, value, allocator)

#define ADD_MEMBER_NULL(name, object, allocator) ADD_MEMBER(name, rapidjson::Value(rapidjson::Type::kNullType), object, allocator);

#define ADD_STRING_MEMBER(name, value, object, allocator)     \
	if (value == "")                                          \
	{                                                         \
		ADD_MEMBER_NULL(name, object, allocator);             \
	}                                                         \
	else                                                      \
	{                                                         \
		rapidjson::Value str;                                 \
		str.SetString(value.data(), value.size(), allocator); \
		ADD_MEMBER(name, str, object, allocator);             \
	}

#define GET_ARRAY(value, array, type)                          \
	if (value.IsArray())                                       \
	{                                                          \
		for (rapidjson::SizeType i = 0; i < value.Size(); i++) \
		{                                                      \
			array->push_back(value[i].Get##type());            \
		}                                                      \
	}

#define GET_DEPENDENCIES(value, dependencies)                                                     \
	if (value.IsArray())                                                                          \
	{                                                                                             \
		for (rapidjson::SizeType i = 0; i < value.Size(); i++)                                    \
		{                                                                                         \
			if (value[i].IsObject())                                                              \
			{                                                                                     \
				const rapidjson::Value &dependencyValue = value[i];                               \
                                                                                                  \
				std::string id = GET_STRING("id", dependencyValue);                               \
				std::string version = GET_STRING("version", dependencyValue);                     \
				std::string downloadIfMissing = GET_STRING("downloadIfMissing", dependencyValue); \
                                                                                                  \
				dependencies->push_back({id, version, downloadIfMissing});                        \
			}                                                                                     \
		}                                                                                         \
	}

#define GET_FILE_COPIES(value, fileCopies)                                          \
	if (value.IsArray())                                                            \
	{                                                                               \
		for (rapidjson::SizeType i = 0; i < value.Size(); i++)                      \
		{                                                                           \
			if (value[i].IsObject())                                                \
			{                                                                       \
				const rapidjson::Value &fileCopyValue = value[i];                   \
                                                                                    \
				std::string name = GET_STRING("name", fileCopyValue);               \
				std::string destination = GET_STRING("destination", fileCopyValue); \
                                                                                    \
				fileCopies->push_back({name, destination});                         \
			}                                                                       \
		}                                                                           \
	}

namespace ModloaderUtils
{
	class QMod
	{
	public:
		static QMod *LoadQMod(std::string fileDir)
		{
			std::string fileName = fileDir.substr(fileDir.find_last_of("/\\") + 1);
			size_t lastindex = fileName.find_last_of("."); 
			if (lastindex != std::string::npos) fileName = fileName.substr(0, lastindex); 

			// Create Temp Dir To Read the mod.json

			std::system(string_format("rm -r \"/sdcard/BMBFData/Mods/Temp/%s/\"", fileName.c_str()).c_str());
			std::system(string_format("mkdir -p \"/sdcard/BMBFData/Mods/Temp/%s/\"", fileName.c_str()).c_str());
			std::system(string_format("unzip \"%s\" mod.json -d \"/sdcard/BMBFData/Mods/Temp/%s/\"", fileDir.c_str(), fileName.c_str()).c_str());

			// Read the mod.json

			std::ifstream qmodFile(string_format("/sdcard/BMBFData/Mods/Temp/%s/mod.json", fileName.c_str()).c_str());
			ASSERT(qmodFile.good());

			std::stringstream qmodJson;
			qmodJson << qmodFile.rdbuf();

			rapidjson::Document document;
			ASSERT(!document.Parse(qmodJson.str().c_str()).HasParseError());

			// Remove the Temp Dir after reading
			std::system(string_format("rm -r \"/sdcard/BMBFData/Mods/Temp/%s/\"", fileName.c_str()).c_str());

			// Get Values

			std::string name = GET_STRING("name", document);
			std::string id = GET_STRING("id", document);
			std::string description = GET_STRING("description", document);
			std::string author = GET_STRING("author", document);
			std::string porter = GET_STRING("porter", document);
			std::string version = GET_STRING("version", document);
			std::string coverImage = GET_STRING("coverImage", document);
			std::string packageId = GET_STRING("packageId", document);
			std::string packageVersion = GET_STRING("packageVersion", document);

			std::vector<std::string> *modFiles = new std::vector<std::string>();
			GET_ARRAY(document["modFiles"], modFiles, String);

			std::vector<std::string> *libraryFiles = new std::vector<std::string>();
			GET_ARRAY(document["libraryFiles"], libraryFiles, String);

			std::vector<Dependency> *dependencies = new std::vector<Dependency>();
			GET_DEPENDENCIES(document["dependencies"], dependencies);

			std::vector<FileCopy> *fileCopies = new std::vector<FileCopy>();
			GET_FILE_COPIES(document["fileCopies"], fileCopies);

			// Create The QMod using this data
			QMod *qmod = new QMod(name, id, description, author, porter, version, coverImage, packageId, packageVersion, modFiles, libraryFiles, dependencies, fileCopies, fileDir);

			// Attempt to load BMBF Specific Data
			qmod->CollectBMBFData();

			return qmod;
		}

		void CollectBMBFData()
		{
			if (strcmp(m_PackageId.c_str(), "com.beatgames.beatsaber"))
			{
				getLogger().info("Failed to collect BMBF Data, QMod isn't for Beat Saber! (PackageId: %s)", m_PackageId.c_str());
				return;
			}

			// Read the config.json file

			std::ifstream configFile("/sdcard/BMBFData/config.json");
			ASSERT_VOID(configFile.good());

			std::stringstream configJson;
			configJson << configFile.rdbuf();

			rapidjson::Document document;

			document.Parse(configJson.str().c_str());

			const auto &mods = document["Mods"].GetArray();

			bool foundMod = false;
			for (rapidjson::SizeType i = 0; i < mods.Size(); i++)
			{
				// Find our mod id, then read the data

				auto &mod = mods[i];
				std::string id = GET_STRING("Id", mod);

				if (id != m_Id)
					continue;
				foundMod = true;

				m_Path = GET_STRING("Path", mod);
				m_Installed = GET_BOOL("Installed", mod);
				m_Uninstallable = GET_BOOL("Uninstallable", mod);
			}

			// Couldnt Find existing BMBF Data, So just set default values;
			if (!foundMod)
			{
				m_Installed = false;
				m_Uninstallable = true;
			}
		}

		void ExtractQMod()
		{
			std::string extractionPath = GetExtractionDir();
			std::string modsExtractionPath = extractionPath + "Mods/";
			std::string libsExtractionPath = extractionPath + "Libs/";
			std::string fileCopiesExtractionPath = extractionPath + "FileCopies/";

			// Remove existing files
			std::system(string_format("rm -r \"%s\"", extractionPath.c_str()).c_str());

			// Create dirs
			std::system(string_format("mkdir -p \"%s\"", modsExtractionPath.c_str()).c_str());
			std::system(string_format("mkdir -p \"%s\"", libsExtractionPath.c_str()).c_str());
			std::system(string_format("mkdir -p \"%s\"", fileCopiesExtractionPath.c_str()).c_str());

			// Extract Mods
			for (std::string mod : *m_ModFiles)
			{
				std::system(string_format("unzip \"%s\" %s -d \"%s\"", m_Path.c_str(), mod.c_str(), modsExtractionPath.c_str()).c_str());
			}

			// Extract Libs
			for (std::string lib : *m_LibraryFiles)
			{
				std::system(string_format("unzip \"%s\" %s -d \"%s\"", m_Path.c_str(), lib.c_str(), libsExtractionPath.c_str()).c_str());
			}

			// Extract File Copies
			for (FileCopy fileCopy : *m_FileCopies)
			{
				std::system(string_format("unzip \"%s\" %s -d \"%s\"", m_Path.c_str(), fileCopy.name.c_str(), fileCopiesExtractionPath.c_str()).c_str());
			}
		}

		void Install(std::vector<std::string> installedInBranch = std::vector<std::string>())
		{
			if (m_Installed)
			{
				getLogger().info("Mod \"%s\" Already Installed!", m_Id.c_str());
				return;
			}

			getLogger().info("Installing mod \"%s\"", m_Id.c_str());
			installedInBranch.push_back(m_Id); // Add to the installed tree so that dependencies further down on us will trigger a recursive install error

			// for (Dependency dependency : *m_Dependencies)
			// {
			//     PrepareDependency(dependency, installedInBranch);
			// }

			// Extract QMod so we can move the files
			ExtractQMod();

			std::string extractionPath = GetExtractionDir();
			std::string modsExtractionPath = extractionPath + "Mods/";
			std::string libsExtractionPath = extractionPath + "Libs/";
			std::string fileCopiesExtractionPath = extractionPath + "FileCopies/";

			// Copy the Mods files to the Mods folder
			std::system(string_format("cp -a \"%s.\" \"%s\"", modsExtractionPath.c_str(), "/sdcard/Android/data/com.beatgames.beatsaber/files/mods/").c_str());

			// Copy the Libs files to the Libs folder
			std::system(string_format("cp -a \"%s.\" \"%s\"", libsExtractionPath.c_str(), "/sdcard/Android/data/com.beatgames.beatsaber/files/libs/").c_str());

			// Copy the File Copies to their respective destination folders
			for (FileCopy fileCopy : *m_FileCopies)
			{
				std::system(string_format("mkdir -p \"%s\"", fileCopy.destination.c_str()).c_str());
				std::system(string_format("cp -a \"%s/%s\" \"%s\"", fileCopiesExtractionPath.c_str(), fileCopy.name.c_str(), fileCopy.destination.c_str()).c_str());
			}

			m_Installed = true;
			installedInBranch.erase(std::remove(installedInBranch.begin(), installedInBranch.end(), m_Id), installedInBranch.end());

			// If QMod is for Beat Saber, then Update its BMBF Data
			if (!strcmp(m_PackageId.c_str(), "com.beatgames.beatsaber")) {
				UpdateBMBFData();
			}
		}

		const std::string Name() { return m_Name; }
		const std::string Id() { return m_Id; }
		const std::string Description() { return m_Description; }
		const std::string Author() { return m_Author; }
		const std::string Porter() { return m_Porter; }
		const std::string Version() { return m_Version; }
		const std::string CoverImage() { return m_CoverImage; }

		const std::string PackageId() { return m_PackageId; }
		const std::string PackageVersion() { return m_PackageVersion; }

		const std::vector<std::string> ModFiles() { return *m_ModFiles; }
		const std::vector<std::string> LibraryFiles() { return *m_LibraryFiles; }
		const std::vector<Dependency> Dependencies() { return *m_Dependencies; }
		const std::vector<FileCopy> FileCopies() { return *m_FileCopies; }

		const std::string Path() { return m_Path; }
		const std::string CoverImageFilename() { return m_CoverImageFilename; }

		const bool Installed() { return m_Installed; }
		const bool Uninstallable() { return m_Uninstallable; }

		const std::string GetExtractionDir() { return string_format("/sdcard/BMBFData/Mods/Temp/%s/", m_Id.c_str()); }

		const std::string GetDisplayName() {
			std::string fileName = m_Path.substr(m_Path.find_last_of("/\\") + 1);
			size_t lastindex = fileName.find_last_of("."); 
			if (lastindex != std::string::npos) fileName = fileName.substr(0, lastindex); 

			return fileName;
		}

	private:
		QMod(std::string name, std::string id, std::string description, std::string author, std::string porter, std::string version, std::string coverImage, std::string packageId, std::string packageVersion, std::vector<std::string> *modFiles, std::vector<std::string> *libraryFiles, std::vector<Dependency> *dependencies, std::vector<FileCopy> *fileCopies, std::string path = "", std::string coverImageFilename = "", bool installed = false, bool uninstallable = true)
			: m_Name(name), m_Id(id), m_Description(description), m_Author(author), m_Porter(porter), m_Version(version), m_CoverImage(coverImage), m_PackageId(packageId), m_PackageVersion(packageVersion), m_ModFiles(modFiles), m_LibraryFiles(libraryFiles), m_Dependencies(dependencies), m_FileCopies(fileCopies), m_Path(path), m_CoverImageFilename(coverImageFilename), m_Installed(installed), m_Uninstallable(uninstallable) {}

		void UpdateBMBFJSONData(auto &mod, auto &allocator)
		{
			ADD_STRING_MEMBER("Id", m_Id, mod, allocator);
			ADD_STRING_MEMBER("Path", m_Path, mod, allocator);
			ADD_MEMBER("Installed", m_Installed, mod, allocator);
			ADD_MEMBER("TogglingOnSync", false, mod, allocator);
			ADD_MEMBER("RemovingOnSync", false, mod, allocator);
			ADD_STRING_MEMBER("Version", m_Version, mod, allocator);
			ADD_MEMBER("Uninstallable", true, mod, allocator);
			ADD_STRING_MEMBER("CoverImageFilename", m_CoverImageFilename, mod, allocator);
			ADD_STRING_MEMBER("TargetBeatsaberVersion", m_PackageVersion, mod, allocator);
			ADD_STRING_MEMBER("Author", m_Author, mod, allocator);
			ADD_STRING_MEMBER("Porter", m_Porter, mod, allocator);
			ADD_STRING_MEMBER("Name", m_Name, mod, allocator);
			ADD_STRING_MEMBER("Description", m_Description, mod, allocator);
		}

		void UpdateBMBFData()
		{
			getLogger().info("Updating BMBF Info");
			// Read the config.json file

			std::ifstream configFile("/sdcard/BMBFData/config.json");
			ASSERT_VOID(configFile.good());

			std::stringstream configJson;
			configJson << configFile.rdbuf();

			rapidjson::Document document;

			document.Parse(configJson.str().c_str());

			const auto &mods = document["Mods"].GetArray();
			bool foundMod = false;

			std::string fileName = m_Path.substr(m_Path.find_last_of("/\\") + 1);
			std::string displayName = GetDisplayName();

			// Move QMod

			std::system(string_format("mv \"%s\" \"/sdcard/BMBFData/Mods/%s\"", m_Path.c_str(), fileName.c_str()).c_str());
			m_Path = string_format("/sdcard/BMBFData/Mods/%s", fileName.c_str()).c_str();

			// Attempt To Install The Cover

			if (m_CoverImage != "") {
				std::string tmpDir = string_format("/sdcard/BMBFData/Mods/Temp/%s/", displayName.c_str());

				std::system(string_format("rm -r \"%s\"", tmpDir.c_str()).c_str());
				std::system(string_format("mkdir -p \"%s\"", tmpDir.c_str()).c_str());

				std::system(string_format("unzip \"%s\" \"%s\" -d \"%s\"", m_Path.c_str(), m_CoverImage.c_str(), tmpDir.c_str()).c_str());

				std::system(string_format("mv \"%s/%s\" \"sdcard/BMBFData/Mods/%s_%s\"", tmpDir.c_str(), m_CoverImage.c_str(), displayName.c_str(), m_CoverImage.c_str()).c_str());

				m_CoverImageFilename = string_format("%s_%s", displayName.c_str(), m_CoverImage.c_str());
			}

			// Try Find Out Mod in the BMBF Data
			for (auto &mod : mods)
			{
				// Find our mod id, then read the data

				std::string id = GET_STRING("Id", mod);

				if (id != m_Id)
					continue;
				foundMod = true;

				getLogger().info("Found existing Data, Updating It...");

				mod.SetObject();
				UpdateBMBFJSONData(mod, document.GetAllocator());
			}

			// BMBF Data could not be found, create the object first
			if (!foundMod)
			{
				getLogger().info("No BMBF Data Found! Creating It Now...");

				rapidjson::Value modDataObject = rapidjson::Value(rapidjson::Type::kObjectType);

				UpdateBMBFJSONData(modDataObject, document.GetAllocator());

				std::string id = GET_STRING("Id", modDataObject);
				getLogger().info("Id: %s, Equals: %s", id.c_str(), id == m_Id ? "True" : "False");

				getLogger().info("Size Before: %i", mods.Capacity());
				mods.PushBack(modDataObject, document.GetAllocator());
				getLogger().info("Size After: %i", mods.Capacity());
			}

			getLogger().info("Updated Data, Saving...");

			for (auto &mod : mods)
			{
				// Find our mod id, then read the data

				std::string id = GET_STRING("Id", mod);

				if (id != m_Id) continue;

				getLogger().info("Found Mod In Mods!");
				getLogger().info("Uninstallable: %s", mod["Uninstallable"].GetBool() ? "True" : "False");
			}

			// Save To Buffer

			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

			document.Accept(writer);

			// Write To File

			std::ofstream out("/sdcard/BMBFData/config.json");
			out << buffer.GetString();
			out.close();
		}

		std::string m_Name;
		std::string m_Id;
		std::string m_Description;
		std::string m_Author;
		std::string m_Porter;
		std::string m_Version;
		std::string m_CoverImage;

		std::string m_PackageId;
		std::string m_PackageVersion;

		std::vector<std::string> *m_ModFiles;
		std::vector<std::string> *m_LibraryFiles;
		std::vector<Dependency> *m_Dependencies;
		std::vector<FileCopy> *m_FileCopies;

		// BMBF Stuff

		std::string m_Path;
		std::string m_CoverImageFilename;

		bool m_Installed;
		bool m_Uninstallable;
	};
}
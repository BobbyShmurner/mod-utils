#pragma once

#include "modloader-utils/shared/Types/QMod.hpp"

#include "modloader/shared/modloader.hpp"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/rapidjson.h"

#include "jni-utils/shared/JNIUtils.hpp"

#include <list>
#include <dirent.h>
#include <jni.h>
#include <unordered_map>
#include <sstream>
#include <fstream>

Logger& getLogger();

namespace ModloaderUtils {
	// Declerations

	inline bool m_HasInitialized;
 
	inline const char* m_ModPath;
	inline const char* m_LibPath;
	inline const char* m_QModPath;
 
	inline std::string m_GameVersion;
	inline std::string m_PackageName;
 
	inline std::list<std::string>* m_OddLibNames;
	inline std::list<std::string>* m_CoreMods;
	inline std::list<std::string>* m_LoadedMods;
 
	inline std::unordered_map<std::string, std::string>* m_ModVersions;

	/**
	 * @brief Get all the files that are contained in a specified directory
	 * 
	 * @param dirPath The Path to get the contents of
	 * @return A list of all the files in the directory
	 */
	inline std::list<std::string> GetDirContents(std::string dirPath);

	/**
	 * @brief Sets the activity of a specific mod
	 * 
	 * @param name The mod to enable or disable
	 * @param active Whether to enable or disable the mod
	 */
	inline void SetModActive(std::string name, bool active);

	/**
	 * @brief Sets the activity of a list of mods
	 * 
	 * @param mods The list of mods to enable or disable
	 * @param active Whether to enable or disable the mods
	 */
	inline void SetModsActive(std::list<std::string>* mods, bool active);

	/**
	 * @brief Toggles the activity of a specific mod to either enabled or diabled
	 * 
	 * @param name The mod to toggle
	 */
	inline void ToggleMod(std::string name);

	/**
	 * @brief Toggles a list of mods on or off
	 * 
	 * @param mods The list of mods to be toggled
	 */
	inline void ToggleMods(std::list<std::string>* mods);

	/**
	 * @brief Checks if a mod is disabled for not
	 * 
	 * @param name The mod to check
	 * @return Returns true if disabled
	 */
	inline bool IsDisabled(std::string name);

	/**
	 * @brief Checks if a mod is an odd lib or not
	 * @details An "Odd Lib" is a mod that doesnt start with "lib"
	 * @details This just makes it easy to test if a name is a lib
	 * 
	 * @param name The mod to check
	 * @return Returns true if mod is an odd lib
	 */
	inline bool IsOddLibName(std::string name);

	/**
	 * @brief Checks if a mod is loaded
	 * @details A mod is considered loaded if it has been dlopened by ModLoader
	 * 
	 * @param name The mod to check
	 * @return Returns true if the mod is loaded
	 */
	inline bool IsModLoaded(std::string name);

	/**
	 * @brief Checks if a mod is a core mod
	 * 
	 * @param name The mod to check
	 * @return Returns true if the mod is a core mod
	 */
	inline bool IsCoreMod(std::string name);

	/**
	 * @brief Checks if a mod is a "Library" file or a "Mod" file
	 * @details A mod is considered a "Library" file if its found in the lib folder, and vice versa
	 * 
	 * @param name The mod to check
	 * @return Returns true if the mod is a "Library" File. Returns false if the mod is a "Mod" File
	 */
	inline bool IsModALibrary(std::string name);

    // Mod Id = Mod Name
    // Lib Name = libmodname
    // File Name = libmodname.so / libmodname.disabled

    // Mod Names, Lib Names and File Names can all convert between eachother

    // Name Tests

	/**
	 * @brief Checks if a mod name is a Mod ID
	 * 
	 * @param name The name to check
	 * @return Returns true if the name is a Mod ID
	 */
    inline bool IsModID(std::string name);

	/**
	 * @brief Checks if a mod name is a Lib Name
	 * 
	 * @param name The name to check
	 * @return Returns true if the name is a Lib Name
	 */
    inline bool IsLibName(std::string name);

	/**
	 * @brief Checks if a mod name is a File Name
	 * 
	 * @param name The name to check
	 * @return Returns true if the name is a File Name
	 */
    inline bool IsFileName(std::string name);

    // Name Conversions

	/**
	 * @brief Gets the Mod ID of a mod.
	 * NOTE: You cannot get the Mod ID of a library
	 * 
	 * @param name The mod to get the id of
	 * @return The mod's Mod ID. If the mod isn't loaded or the mod is a lib, the mod's Lib Name will be returned instead
	 */
	inline std::string GetModID(std::string name);

	/**
	 * @brief Gets the Lib Name of a mod
	 * @details The File Name is just the file name without the file extention
	 * 
	 * @param name The mod to get the lib name of
	 * @return The mod's Lib Name.
	 */
    inline std::string GetLibName(std::string name);

	/**
	 * @brief Gets the File Name of a mod
	 * @details The File Name is just the lib name with the file extention
	 * 
	 * @param name The mod to get the file name of
	 * @return The mod's File Name.
	 */
    inline std::string GetFileName(std::string name);

	/**
	 * @brief Gets the Version of a loaded mod
	 * 
	 * @param name The mod to get the version of
	 * @return The mod's version. Returns "Unknown" if failed to get the mod version
	 */
    inline std::string GetModVersion(std::string name);

	/**
	 * @brief Get a list of all the loaded mods
	 * 
	 * @return Returns a list of loaded mod file names
	 */
	inline std::list<std::string> GetLoadedModsFileNames();

	/**
	 * @brief Get a list of all the core mods for this version
	 * 
	 * @return Returns a list of Mod IDs for all the core mods
	 */
	inline std::list<std::string> GetCoreMods();

	/**
	 * @brief Get a list of all the "Odd Libs"
	 * 
	 * @return Returns a list of Odd Lib Names
	 */
	inline std::list<std::string> GetOddLibNames();

	/**
	 * @brief Get's the error for a mod
	 * @details The mod is dlopened, and then then the error is fetched using dlerror
	 * 
	 * @param name The name of the mod to test for an error
	 * @return Returns the error if there was one, else returns null
	 */
	inline std::optional<std::string> GetModError(std::string name);

	/**
	 * @brief Gets the location of the Mods folder
	 * 
	 * @return The location of the Mods folder
	 */
    inline std::string GetModsFolder();

	/**
	 * @brief Gets the location of the Libs folder
	 * 
	 * @return The location of the Libs folder
	 */
    inline std::string GetLibsFolder();

	/**
	 * @brief Get the version of the app that's currently running
	 * 
	 * @return A string that contains the game version
	 */
	inline std::string GetGameVersion();

	/**
	 * @brief Gets the package name for the current app, for example "com.beatgames.beatsaber"
	 * 
	 * @return The current app's package name
	 */
	inline std::string GetPackageName();

	/**
	 * @brief Restarts The Current Game
	 */
	inline void RestartGame();

	/**
	 * @brief Removes any .disabled files if a .so version of the file is found
	 * @details This has a big impact on performance, and will stall if called on the main thread
	 * 
	 * @return Returns true if a duplicate mod was found
	 */
	inline bool RemoveDuplicateMods();

	// Private shit dont use >:(

	inline void Init();
	inline void CacheJVM();

	inline void CollectCoreMods();
	inline void CollectLoadedMods();
	inline void CollectModVersions();
	inline void CollectOddLibs();
	inline void CollectGameVersion();
	inline void CollectPackageName();
	inline void CollectDownloadedQMods();

	inline std::string GetFileNameFromDir(std::string libName, bool guessLibName = false);
	inline std::string GetFileNameFromModID(std::string modID);

	// Definitions

	std::list<std::string> GetDirContents(std::string dirPath) {
		DIR* dir = opendir(dirPath.c_str());
		dirent* dp;
		std::list<std::string> files; 

		if (dir == nullptr) return files;

		while ((dp = readdir(dir)) != NULL) {
			if (dp->d_type != DT_DIR) {
				files.emplace_front(std::string(dp->d_name));
			}
		}

		return files;
	}

	void SetModActive(std::string name, bool active) {
		getLogger().info("%s mod \"%s\"", active ? "Enabling" : "Disabling", GetLibName(name).c_str());

		std::string path;
		if (IsModALibrary(name)) path = m_LibPath;
		else path = m_ModPath;

		std::list<std::string> fileNames = GetDirContents(path);

		for (std::string fileName : fileNames) {
			if (!IsFileName(fileName)) continue;
			if (strcmp(fileName.c_str(), GetFileName(name).c_str())) continue;

			if (active) rename(string_format("%s/%s", path.c_str(), fileName.c_str()).c_str(), string_format("%s/%s.so", path.c_str(), GetLibName(name).c_str()).c_str());
			else rename(string_format("%s/%s", path.c_str(), fileName.c_str()).c_str(), string_format("%s/%s.disabled", path.c_str(), GetLibName(name).c_str()).c_str());

			return;
		}
	}

	void SetModsActive(std::list<std::string>* mods, bool active) {
		getLogger().info("%s a list of mods", active ? "Enabling" : "Disabling");

		for (std::string modFileName : *mods) {
			SetModActive(modFileName, active);
		}
	}

	void ToggleMod(std::string name) {
		SetModActive(name, IsDisabled(name));
	}

	void ToggleMods(std::list<std::string>* mods) {
		getLogger().info("Toggling a list of mods");

		for (std::string modFileName : *mods) {
			ToggleMod(modFileName);
		}
	}

	bool IsDisabled(std::string name) {
		std::string fileName = GetFileName(name);

		return fileName.length() > 9 && !strcmp(fileName.substr(fileName.size() - 9).c_str(), ".disabled");
	}

	bool IsOddLibName(std::string name) {
		Init();
		return (std::find(m_OddLibNames->begin(), m_OddLibNames->end(), name) != m_OddLibNames->end());
	}

	bool IsModLoaded(std::string name) {
		Init();
		return (std::find(m_LoadedMods->begin(), m_LoadedMods->end(), GetFileName(name)) != m_LoadedMods->end());;
	}

	bool IsCoreMod(std::string name) {
		Init();
		return (std::find(m_CoreMods->begin(), m_CoreMods->end(), GetFileName(name)) != m_CoreMods->end());
	}

	bool IsModALibrary(std::string name) {
		std::string fileName = GetFileName(name);
		std::list<std::string> libFiles = GetDirContents(m_LibPath);

		return (std::find(libFiles.begin(), libFiles.end(), fileName) != libFiles.end());
	}

	// Mod Name = Mod Name
	// Lib Name = libmodname
	// File Name = libmodname.so / libmodname.disabled

	// Mod Names, Lib Names and File Names can all convert between eachother

	// Name Tests

	bool IsModID(std::string name) {
		std::string fileName = GetFileName(name);
		std::unordered_map<std::string, const Mod> mods = Modloader::getMods();

		for (std::pair<std::string, const Mod> modPair : mods) {
			if (!strcmp(fileName.c_str(), modPair.second.name.c_str())) return true;
		}

		return false;
	}

	bool IsLibName(std::string name) {
		return ((!IsFileName(name)) && (name.length() > 3 && !strcmp(name.substr(0, 3).c_str(), "lib"))) || IsOddLibName(name);
	}

	bool IsFileName(std::string name) {
		return (name.length() > 9 && !strcmp(name.substr(name.size() - 9).c_str(), ".disabled")) || (name.length() > 3 && !strcmp(name.substr(name.size() - 3).c_str(), ".so"));
	}

	// Name Conversions

	std::string GetModID(std::string name) {
		std::string fileName = GetFileName(name);
		std::unordered_map<std::string, const Mod> mods = Modloader::getMods();
		
		for (std::pair<std::string, const Mod> modPair : mods) {
			if (!strcmp(fileName.c_str(), modPair.second.name.c_str())) return modPair.first;
		}

		return GetLibName(name);
	}

	std::string GetLibName(std::string name) {
		if (IsLibName(name)) return name;

		std::string fileName;

		if (IsFileName(name)) fileName = name;
		else fileName = GetFileNameFromModID(name);

		if (fileName == "Null") fileName = GetFileNameFromDir(name);
		if (fileName == "Null") return "Null";

		if (IsDisabled(fileName)) return fileName.substr(0, fileName.size() - 9);
		else return fileName.substr(0, fileName.size() - 3);
	}

	std::string GetFileName(std::string name) {
		if (IsFileName(name)) return name;
		std::string libName;

		if (IsLibName(name)) libName = name;
		else libName = GetLibName(name);

		return GetFileNameFromDir(libName);
	}

	std::string GetModVersion(std::string name) {
		Init();

		std::string fileName = GetFileName(name);
		if (m_ModVersions->find(fileName) == m_ModVersions->end()) return "Unknown";

		return m_ModVersions->at(fileName);
	}

	std::list<std::string> GetLoadedModsFileNames() {
		Init();
		return *m_LoadedMods;
	}

	std::list<std::string> GetCoreMods() {
		Init();
		return *m_CoreMods;
	}

	std::list<std::string> GetOddLibNames() {
		Init();
		return *m_OddLibNames;
	}

	// Thanks for Laurie for the original code snippet: 
	std::optional<std::string> GetModError(std::string name) {
		std::string fileName = GetFileName(name);
		std::string filePath = Modloader::getDestinationPath() + fileName;
		
		dlerror(); // Clear Existing Errors
		dlopen(filePath.c_str(), RTLD_LOCAL | RTLD_NOW);

		char* error = dlerror();
		return error ? std::optional(std::string(error).substr(15)) : std::nullopt;
	}

	std::string GetModsFolder() {
		return m_ModPath;
	}

	std::string GetLibsFolder() {
		return m_LibPath;
	}

	std::string GetGameVersion() {
		return m_GameVersion;
	}

	std::string GetPackageName() {
		return m_PackageName;
	}

	void RestartGame() {
		Init();
		getLogger().info("-- STARTING RESTART --");

		JNIEnv* env = JNIUtils::GetJNIEnv();

		jstring packageName = JNIUtils::GetPackageName(env);

		// Get Activity
		jobject appActivity = JNIUtils::GetAppActivity(env);

		// Get Package Manager
		CALL_JOBJECT_METHOD(env, packageManager, appActivity, "getPackageManager", "()Landroid/content/pm/PackageManager;");

		// Get Intent
		CALL_JOBJECT_METHOD(env, intent, packageManager, "getLaunchIntentForPackage", "(Ljava/lang/String;)Landroid/content/Intent;", packageName);

		// Set Intent Flags
		CALL_JOBJECT_METHOD(env, setFlagsSuccess, intent, "setFlags", "(I)Landroid/content/Intent;", 536870912);

		// Get Component Name
		CALL_JOBJECT_METHOD(env, componentName, intent, "getComponent", "()Landroid/content/ComponentName;");

		// Create Restart Intent
		GET_JCLASS(env, intentClass, "android/content/Intent");
		CALL_STATIC_JOBJECT_METHOD(env, restartIntent, intentClass, "makeRestartActivityTask", "(Landroid/content/ComponentName;)Landroid/content/Intent;", componentName);

		// Restart Game
		CALL_VOID_METHOD(env, appActivity, "startActivity", "(Landroid/content/Intent;)V", restartIntent);

		GET_JCLASS(env, processClass, "android/os/Process");

		CALL_STATIC_JINT_METHOD(env, pid, processClass, "myPid", "()I");
		CALL_STATIC_VOID_METHOD(env, processClass, "killProcess", "(I)V", pid);
	}

	bool RemoveDuplicateMods() {
		std::list<std::string> modFileNames = GetDirContents(m_ModPath);
		std::list<std::string> libFileNames = GetDirContents(m_LibPath);

		std::list<std::string> fileNames = modFileNames;
		fileNames.merge(libFileNames);

		bool removedDuplicate = false;

		for (std::string file : fileNames) {
			if (!IsDisabled(file)) continue;

			std::string enabledName = GetLibName(file) + ".so";
			if (std::find(fileNames.begin(), fileNames.end(), enabledName) != fileNames.end()) {
				std::string path = m_ModPath;
				if (IsModALibrary(file)) path = m_LibPath;

				remove(string_format("%s/%s", path.c_str(), file.c_str()).c_str());
				getLogger().info("Removed Duplicated File \"%s\"", file.c_str());

				removedDuplicate = true;
			}
		}

		return removedDuplicate;
	}

	void CollectCoreMods() {
		std::ifstream coreModsFile("/sdcard/BMBFData/core-mods.json");
		std::stringstream coreModsSS;
		coreModsSS << coreModsFile.rdbuf();

		rapidjson::Document coreModsDoc;
		coreModsDoc.Parse(coreModsSS.str().c_str());

		getLogger().info("Collecting Core Mods...");

		if (coreModsDoc.HasMember(m_GameVersion)) {
			const rapidjson::Value& versionInfo = coreModsDoc[m_GameVersion];
			const rapidjson::Value& coreModsList = versionInfo["mods"];

			for (rapidjson::SizeType i = 0; i < coreModsList.Size(); i++) { // rapidjson uses SizeType instead of size_t.
				const rapidjson::Value& coreModInfo = coreModsList[i];

				std::string fileName = GetFileName(coreModInfo["id"].GetString());

				m_CoreMods->emplace_front(fileName);
				getLogger().info("Found Core mod %s", fileName.c_str());
			}
		} else {
			getLogger().info("ERROR! No Core Mods Found For This Version!");
		}

		getLogger().info("Finished Collecting Core Mods!");
	}

	void CollectLoadedMods() {
		for (std::pair<std::string, const Mod> modPair : Modloader::getMods()) {
			m_LoadedMods->emplace_front(modPair.second.name);
		}

		// As Modloader only keeps track of loaded mods, not libs, we have to collect the ourself
		for (std::string fileName : GetDirContents(m_LibPath)) {
			if (GetModError(fileName) == std::nullopt) m_LoadedMods->emplace_front(fileName);
		}
	}

	void CollectModVersions() {
		for (std::pair<std::string, const Mod> modPair : Modloader::getMods()) {
			m_ModVersions->emplace(modPair.second.name, modPair.second.info.version);
		}
	}

	void CollectOddLibs() {
		m_OddLibNames->clear();
		std::list<std::string> modFileNames = GetDirContents(m_ModPath);
		std::list<std::string> libFileNames = GetDirContents(m_LibPath);

		std::list<std::string> fileNames = modFileNames;
		fileNames.merge(libFileNames);

		for (std::string fileName : fileNames) {
			if (!IsFileName(fileName)) continue;

			if (strcmp(fileName.substr(0, 3).c_str(), "lib")) {
				getLogger().info("Mod \"%s\" does not start with \"lib\"! Adding to m_OddLibNames", fileName.c_str());

				if (IsDisabled(fileName)) m_OddLibNames->emplace_front(fileName.substr(0, fileName.size() - 9));
				else m_OddLibNames->emplace_front(fileName.substr(0, fileName.size() - 3));
			}
		}
	}

	void CollectPackageName() {
		getLogger().info("Collecting Package Name...");
		JNIEnv* env = JNIUtils::GetJNIEnv();

		jstring packageName = JNIUtils::GetPackageName(env);
		m_PackageName = JNIUtils::ToString(env, packageName);

		getLogger().info("Got Package Name \"%s\"!", m_PackageName.c_str());
	}

	void CollectGameVersion() {
		getLogger().info("Collecting Game Version...");
		JNIEnv* env = JNIUtils::GetJNIEnv();

		jstring gameVersion = JNIUtils::GetGameVersion(env);
		m_GameVersion = JNIUtils::ToString(env, gameVersion);

		getLogger().info("Got Game Version \"%s\"!", m_GameVersion.c_str());
	}

	void CollectDownloadedQMods() {
		getLogger().info("Collecting Downloaded QMods...");

		QMod::DownloadedQMods->clear();
		QMod::DownloadedQModIds->clear();
		std::list<std::string> fileNames = GetDirContents(m_QModPath);

		for (std::string file : fileNames) {
			std::string filePath = m_QModPath + file;
			QMod* qmod = new QMod(filePath, false);

			if (qmod != nullptr) {
				getLogger().info("Found QMod File \"%s\"", file.c_str());
				QMod::DownloadedQMods->push_back(qmod);
				QMod::DownloadedQModIds->push_back(qmod->Id());
			}
		}

		getLogger().info("Finished Collecting Downloaded QMods!");
	}

	std::string GetFileNameFromDir(std::string libName, bool guessLibName) {
		std::list<std::string> modFileNames = GetDirContents(m_ModPath);
		std::list<std::string> libFileNames = GetDirContents(m_LibPath);

		std::list<std::string> fileNames = modFileNames;
		fileNames.merge(libFileNames);

		if (guessLibName) libName = "lib" + libName;
			
		for (std::string fileName : fileNames) {
			if (!strcmp(GetLibName(fileName).c_str(), libName.c_str())) return fileName;
		}

		if (!guessLibName) {
			return GetFileNameFromDir(libName, true); // Just try gussing it xD
		}

		getLogger().info("Failed to get file name for \"%s\"!", libName.c_str());
		return {"Null"};
	}

	std::string GetFileNameFromModID(std::string modID) {
		if (!Modloader::getMods().contains(modID)) return {"Null"};

		return Modloader::getMods().at(modID).name;
	}

	void Init() {
		if (m_HasInitialized) return;
		m_HasInitialized = true;

		m_ModPath = "/sdcard/Android/data/com.beatgames.beatsaber/files/mods/";
		m_LibPath = "/sdcard/Android/data/com.beatgames.beatsaber/files/libs/";
		m_QModPath = "/sdcard/BMBFData/Mods/";

		m_CoreMods = new std::list<std::string>();
		m_OddLibNames = new std::list<std::string>();
		m_LoadedMods = new std::list<std::string>();
		m_ModVersions = new std::unordered_map<std::string, std::string>();

		CollectPackageName();
		CollectGameVersion();
		CollectLoadedMods();
		CollectModVersions();
		CollectOddLibs();
		CollectCoreMods();

		CollectDownloadedQMods();
	}
};
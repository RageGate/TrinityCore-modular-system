/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINITY_PLUGIN_CONFIG_H
#define TRINITY_PLUGIN_CONFIG_H

#include "Define.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <any>

class TC_GAME_API PluginConfig
{
public:
    PluginConfig() = default;
    explicit PluginConfig(std::string const& configFile);
    ~PluginConfig() = default;
    
    // Configuration loading
    bool LoadFromFile(std::string const& configFile);
    bool LoadFromString(std::string const& configData);
    bool SaveToFile(std::string const& configFile) const;
    
    // Value getters with type safety
    template<typename T>
    T GetValue(std::string const& key, T const& defaultValue = T{}) const;
    
    std::string GetString(std::string const& key, std::string const& defaultValue = "") const;
    int32 GetInt(std::string const& key, int32 defaultValue = 0) const;
    uint32 GetUInt(std::string const& key, uint32 defaultValue = 0) const;
    float GetFloat(std::string const& key, float defaultValue = 0.0f) const;
    bool GetBool(std::string const& key, bool defaultValue = false) const;
    
    // Value setters
    template<typename T>
    void SetValue(std::string const& key, T const& value);
    
    void SetString(std::string const& key, std::string const& value);
    void SetInt(std::string const& key, int32 value);
    void SetUInt(std::string const& key, uint32 value);
    void SetFloat(std::string const& key, float value);
    void SetBool(std::string const& key, bool value);
    
    // Array/List support
    std::vector<std::string> GetStringArray(std::string const& key) const;
    std::vector<int32> GetIntArray(std::string const& key) const;
    std::vector<uint32> GetUIntArray(std::string const& key) const;
    std::vector<float> GetFloatArray(std::string const& key) const;
    
    void SetStringArray(std::string const& key, std::vector<std::string> const& values);
    void SetIntArray(std::string const& key, std::vector<int32> const& values);
    void SetUIntArray(std::string const& key, std::vector<uint32> const& values);
    void SetFloatArray(std::string const& key, std::vector<float> const& values);
    
    // Configuration management
    bool HasKey(std::string const& key) const;
    void RemoveKey(std::string const& key);
    void Clear();
    
    // Section support
    PluginConfig GetSection(std::string const& sectionName) const;
    void SetSection(std::string const& sectionName, PluginConfig const& section);
    std::vector<std::string> GetSectionNames() const;
    
    // Validation
    bool Validate(std::string const& schema) const;
    std::vector<std::string> GetValidationErrors() const;
    
    // Utility
    std::string ToString() const;
    size_t GetKeyCount() const { return _values.size(); }
    std::vector<std::string> GetAllKeys() const;
    
private:
    std::unordered_map<std::string, std::any> _values;
    mutable std::vector<std::string> _validationErrors;
    
    // Helper methods
    std::string NormalizeKey(std::string const& key) const;
    std::vector<std::string> ParseArray(std::string const& value) const;
    std::string SerializeArray(std::vector<std::string> const& values) const;
};

struct PluginDependency
{
    std::string name;
    std::string version;
    bool optional;
    std::string reason;
    
    PluginDependency() : optional(false) { }
    PluginDependency(std::string const& n, std::string const& v, bool opt = false, std::string const& r = "")
        : name(n), version(v), optional(opt), reason(r) { }
};

class TC_GAME_API PluginConfigManager
{
public:
    static PluginConfigManager* Instance();
    
    // Plugin configuration management
    bool LoadPluginConfig(std::string const& pluginName, std::string const& configFile);
    bool SavePluginConfig(std::string const& pluginName, std::string const& configFile);
    
    PluginConfig* GetPluginConfig(std::string const& pluginName);
    bool HasPluginConfig(std::string const& pluginName) const;
    void RemovePluginConfig(std::string const& pluginName);
    
    // Global plugin settings
    void SetGlobalSetting(std::string const& key, std::string const& value);
    std::string GetGlobalSetting(std::string const& key, std::string const& defaultValue = "") const;
    
    // Plugin dependency management
    bool RegisterPluginDependency(std::string const& pluginName, PluginDependency const& dependency);
    std::vector<PluginDependency> GetPluginDependencies(std::string const& pluginName) const;
    bool CheckDependency(std::string const& pluginName, std::string const& dependencyName) const;
    bool ValidateAllDependencies() const;
    
    // Configuration validation
    bool ValidatePluginConfig(std::string const& pluginName, std::string const& schema);
    std::vector<std::string> GetConfigValidationErrors(std::string const& pluginName) const;
    
    // Configuration templates
    void RegisterConfigTemplate(std::string const& templateName, PluginConfig const& templateConfig);
    PluginConfig GetConfigTemplate(std::string const& templateName) const;
    bool HasConfigTemplate(std::string const& templateName) const;
    
    // Hot reload support
    void EnableHotReload(std::string const& pluginName, bool enable = true);
    bool IsHotReloadEnabled(std::string const& pluginName) const;
    void ReloadPluginConfig(std::string const& pluginName);
    void ReloadAllConfigs();
    
    // Configuration backup and restore
    bool BackupPluginConfig(std::string const& pluginName);
    bool RestorePluginConfig(std::string const& pluginName);
    void ClearConfigBackups();
    
    // Configuration export/import
    bool ExportPluginConfigs(std::string const& exportFile) const;
    bool ImportPluginConfigs(std::string const& importFile);
    
    // Configuration monitoring
    void StartConfigMonitoring();
    void StopConfigMonitoring();
    bool IsConfigMonitoringActive() const { return _monitoringActive; }
    
private:
    PluginConfigManager();
    ~PluginConfigManager();
    
    // Internal storage
    std::unordered_map<std::string, std::unique_ptr<PluginConfig>> _pluginConfigs;
    std::unordered_map<std::string, std::vector<PluginDependency>> _pluginDependencies;
    std::unordered_map<std::string, PluginConfig> _configTemplates;
    std::unordered_map<std::string, std::unique_ptr<PluginConfig>> _configBackups;
    std::unordered_map<std::string, std::string> _globalSettings;
    std::unordered_map<std::string, bool> _hotReloadEnabled;
    
    // Configuration monitoring
    bool _monitoringActive;
    std::unordered_map<std::string, std::string> _configFilePaths;
    std::unordered_map<std::string, uint64> _configFileTimestamps;
    
    // Thread safety
    mutable std::mutex _configMutex;
    
    // Singleton
    static std::unique_ptr<PluginConfigManager> _instance;
    static std::mutex _instanceMutex;
    
    // Helper methods
    bool CheckConfigFileChanged(std::string const& pluginName);
    uint64 GetFileTimestamp(std::string const& filePath) const;
    void ProcessConfigChange(std::string const& pluginName);
};

#define sPluginConfigManager PluginConfigManager::Instance()

// Template implementations
template<typename T>
T PluginConfig::GetValue(std::string const& key, T const& defaultValue) const
{
    std::string normalizedKey = NormalizeKey(key);
    auto it = _values.find(normalizedKey);
    
    if (it != _values.end())
    {
        try
        {
            return std::any_cast<T>(it->second);
        }
        catch (std::bad_any_cast const&)
        {
            // Type mismatch, return default
        }
    }
    
    return defaultValue;
}

template<typename T>
void PluginConfig::SetValue(std::string const& key, T const& value)
{
    std::string normalizedKey = NormalizeKey(key);
    _values[normalizedKey] = value;
}

#endif // TRINITY_PLUGIN_CONFIG_H
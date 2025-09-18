# TrinityCore Custom Plugin Architecture

This document describes the custom plugin architecture designed for TrinityCore, providing a modular system for extending server functionality without modifying core code.

## Overview

The TrinityCore Plugin Architecture is a comprehensive system that allows developers to create modular extensions for TrinityCore servers. It provides:

- **Dynamic Plugin Loading**: Load and unload plugins at runtime
- **Event-Driven Architecture**: Hook into core server events
- **Configuration Management**: Per-plugin configuration with hot-reload support
- **Dependency Management**: Handle plugin dependencies automatically
- **Thread Safety**: Safe multi-threaded plugin operations
- **Build System Integration**: Seamless CMake integration

## Architecture Components

### Core Components

1. **IPlugin.h** - Base plugin interface and registration macros
2. **PluginManager.h/.cpp** - Central plugin management system
3. **PluginHooks.h** - Event hooks for core system integration
4. **PluginConfig.h** - Configuration management system

### Plugin Lifecycle

```
Unloaded → Loading → Loaded → Initializing → Initialized → Running → Stopping → Loaded → Unloaded
```

## Quick Start Guide

### 1. Creating Your First Plugin

```cpp
// MyPlugin.h
#include "IPlugin.h"

class MyPlugin : public IPlugin
{
public:
    MyPlugin();
    virtual ~MyPlugin();
    
    // Lifecycle methods
    bool Load() override;
    bool Initialize() override;
    void Start() override;
    void Stop() override;
    void Unload() override;
    
    // Configuration
    bool LoadConfig(std::string const& configPath) override;
    void ReloadConfig() override;
    
    // Dependencies
    bool CheckDependencies() const override;
    
private:
    // Your plugin implementation
};

// Register the plugin
REGISTER_PLUGIN(MyPlugin)
```

### 2. Plugin Implementation

```cpp
// MyPlugin.cpp
#include "MyPlugin.h"
#include "Log.h"

MyPlugin::MyPlugin()
{
    _info.name = "MyPlugin";
    _info.version = "1.0.0";
    _info.author = "Your Name";
    _info.description = "My custom plugin";
    _info.priority = PluginPriority::NORMAL;
}

bool MyPlugin::Load()
{
    TC_LOG_INFO("plugins.myplugin", "Loading MyPlugin...");
    _state = PluginState::LOADED;
    return true;
}

bool MyPlugin::Initialize()
{
    TC_LOG_INFO("plugins.myplugin", "Initializing MyPlugin...");
    _state = PluginState::INITIALIZED;
    return true;
}

void MyPlugin::Start()
{
    TC_LOG_INFO("plugins.myplugin", "Starting MyPlugin...");
    _state = PluginState::RUNNING;
}

void MyPlugin::Stop()
{
    TC_LOG_INFO("plugins.myplugin", "Stopping MyPlugin...");
    _state = PluginState::LOADED;
}

void MyPlugin::Unload()
{
    TC_LOG_INFO("plugins.myplugin", "Unloading MyPlugin...");
    _state = PluginState::UNLOADED;
}
```

### 3. Event Handling

```cpp
// In your plugin class
class MyEventHandler : public IPluginEventHandler
{
public:
    void OnPlayerLogin(Player* player) override
    {
        // Handle player login
    }
    
    void OnPlayerLevelChanged(Player* player, uint8 oldLevel) override
    {
        // Handle level changes
    }
};
```

### 4. Configuration Management

```cpp
// MyPlugin.conf
MyPlugin.Enabled = 1
MyPlugin.WelcomeMessage = "Welcome to the server!"
MyPlugin.MaxLevel = 80

// In your plugin
bool MyPlugin::LoadConfig(std::string const& configPath)
{
    if (!_config->LoadFromFile(configPath))
        return false;
    
    bool enabled = _config->GetBool("MyPlugin.Enabled", true);
    std::string message = _config->GetString("MyPlugin.WelcomeMessage", "Welcome!");
    uint32 maxLevel = _config->GetUInt("MyPlugin.MaxLevel", 80);
    
    return true;
}
```

## Available Event Hooks

### Player Events
- `OnPlayerLogin(Player* player)`
- `OnPlayerLogout(Player* player)`
- `OnPlayerLevelChanged(Player* player, uint8 oldLevel)`
- `OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg)`
- `OnPlayerEnterCombat(Player* player, Unit* enemy)`
- `OnPlayerLeaveCombat(Player* player)`

### World Events
- `OnWorldUpdate(uint32 diff)`
- `OnServerStart()`
- `OnServerStop()`
- `OnConfigReload()`

### Creature Events
- `OnCreatureCreate(Creature* creature)`
- `OnCreatureDeath(Creature* creature, Unit* killer)`
- `OnCreatureRespawn(Creature* creature)`

### GameObject Events
- `OnGameObjectCreate(GameObject* go)`
- `OnGameObjectUse(GameObject* go, Player* player)`
- `OnGameObjectDestroy(GameObject* go)`

### Extended Events
- Spell events (cast, hit, effect)
- Item events (equip, use, enchant)
- Quest events (accept, complete, abandon)
- Group/Guild events
- Battleground/Arena events
- Instance events
- And many more...

## Plugin Manager Usage

### Loading Plugins

```cpp
// Get plugin manager instance
PluginManager* manager = PluginManager::Instance();

// Initialize the plugin system
manager->Initialize();

// Load a specific plugin
if (manager->LoadPlugin("plugins/MyPlugin.dll"))
{
    TC_LOG_INFO("server.loading", "Plugin loaded successfully");
}

// Load all plugins from directory
manager->LoadPluginsFromDirectory("plugins/");

// Start all loaded plugins
manager->StartAllPlugins();
```

### Runtime Management

```cpp
// Reload a plugin
manager->ReloadPlugin("MyPlugin");

// Unload a plugin
manager->UnloadPlugin("MyPlugin");

// Get plugin information
PluginInfo info = manager->GetPluginInfo("MyPlugin");

// Check plugin status
PluginState state = manager->GetPluginState("MyPlugin");
```

## Build System Integration

### CMake Configuration

The plugin system integrates with TrinityCore's CMake build system:

```cmake
# Add plugin to build
add_trinitycore_plugin(MyPlugin
    SOURCES
        MyPlugin.h
        MyPlugin.cpp
    DEPENDENCIES
        SomeOtherPlugin
    VERSION "1.0.0"
    AUTHOR "Your Name"
    DESCRIPTION "My custom plugin"
)
```

### Building Plugins

1. **In-tree Build**: Place your plugin in `src/server/game/Plugins/plugins/MyPlugin/`
2. **Out-of-tree Build**: Build as separate project linking to plugin-system library

```bash
# Configure with plugin support
cmake -DBUILD_EXAMPLE_PLUGIN=ON ..

# Build
make -j$(nproc)

# Plugins will be built to build/plugins/
```

## Configuration System

### Plugin Configuration Files

Place configuration files in `configs/` directory:

```
configs/
├── MyPlugin.conf
├── AnotherPlugin.conf
└── GlobalPlugins.conf
```

### Configuration Format

```ini
# Comments start with #
MyPlugin.Enabled = 1
MyPlugin.StringValue = "Hello World"
MyPlugin.IntValue = 42
MyPlugin.FloatValue = 3.14
MyPlugin.BoolValue = true

# Array values
MyPlugin.ArrayValue = [1, 2, 3, 4, 5]

# Section support
[Database]
Host = "localhost"
Port = 3306
User = "trinity"
Password = "trinity"
```

### Hot Reload Support

Configurations can be reloaded at runtime:

```cpp
// Reload specific plugin config
manager->ReloadPluginConfig("MyPlugin");

// Reload all plugin configs
manager->ReloadAllConfigs();
```

## Dependency Management

### Declaring Dependencies

```cpp
MyPlugin::MyPlugin()
{
    // Add dependencies
    _dependencies.push_back("CorePlugin");
    _dependencies.push_back("DatabasePlugin");
}
```

### Dependency Resolution

The plugin manager automatically:
- Resolves dependency order
- Loads dependencies before dependents
- Prevents circular dependencies
- Handles missing dependencies gracefully

## Thread Safety

The plugin system is designed to be thread-safe:

- Plugin manager operations are protected by mutexes
- Event dispatching is thread-safe
- Configuration access is synchronized
- Plugin state changes are atomic

## Best Practices

### 1. Plugin Design
- Keep plugins focused on single functionality
- Use proper error handling and logging
- Implement all lifecycle methods
- Handle configuration changes gracefully

### 2. Performance
- Minimize work in event handlers
- Use caching for expensive operations
- Avoid blocking operations in main thread
- Profile plugin performance impact

### 3. Compatibility
- Test with different TrinityCore versions
- Handle API changes gracefully
- Document plugin requirements
- Provide migration guides for updates

### 4. Security
- Validate all input data
- Use secure coding practices
- Avoid exposing sensitive information
- Implement proper access controls

## Debugging and Troubleshooting

### Logging

Use TrinityCore's logging system:

```cpp
TC_LOG_ERROR("plugins.myplugin", "Error message");
TC_LOG_WARN("plugins.myplugin", "Warning message");
TC_LOG_INFO("plugins.myplugin", "Info message");
TC_LOG_DEBUG("plugins.myplugin", "Debug message");
TC_LOG_TRACE("plugins.myplugin", "Trace message");
```

### Common Issues

1. **Plugin Not Loading**
   - Check file permissions
   - Verify plugin registration macro
   - Check dependencies
   - Review error logs

2. **Configuration Not Working**
   - Verify config file path
   - Check file format
   - Ensure proper key names
   - Test with default values

3. **Events Not Firing**
   - Verify event handler registration
   - Check plugin state (must be RUNNING)
   - Ensure proper inheritance
   - Review hook integration

## Example Plugins

### ExamplePlugin

A comprehensive example plugin demonstrating:
- Basic plugin structure
- Event handling
- Configuration management
- Chat commands
- Statistics tracking
- Player data management

See `Examples/ExamplePlugin.h` and `Examples/ExamplePlugin.cpp` for implementation details.

## API Reference

### IPlugin Interface

```cpp
class IPlugin
{
public:
    virtual ~IPlugin() = default;
    
    // Lifecycle
    virtual bool Load() = 0;
    virtual bool Initialize() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Unload() = 0;
    
    // Configuration
    virtual bool LoadConfig(std::string const& configPath) = 0;
    virtual void ReloadConfig() = 0;
    
    // Dependencies
    virtual bool CheckDependencies() const = 0;
    
    // Information
    PluginInfo const& GetInfo() const { return _info; }
    PluginState GetState() const { return _state; }
    std::vector<std::string> const& GetDependencies() const { return _dependencies; }
    
protected:
    PluginInfo _info;
    PluginState _state = PluginState::UNLOADED;
    std::vector<std::string> _dependencies;
};
```

### PluginManager Interface

```cpp
class PluginManager
{
public:
    static PluginManager* Instance();
    
    // Lifecycle
    bool Initialize();
    void Shutdown();
    
    // Plugin Management
    bool LoadPlugin(std::string const& path);
    bool UnloadPlugin(std::string const& name);
    bool ReloadPlugin(std::string const& name);
    
    // Bulk Operations
    void LoadPluginsFromDirectory(std::string const& directory);
    void StartAllPlugins();
    void StopAllPlugins();
    void UnloadAllPlugins();
    
    // Information
    std::vector<std::string> GetLoadedPlugins() const;
    PluginInfo GetPluginInfo(std::string const& name) const;
    PluginState GetPluginState(std::string const& name) const;
    
    // Configuration
    bool ReloadPluginConfig(std::string const& name);
    void ReloadAllConfigs();
    
    // Event Dispatching
    template<typename... Args>
    void DispatchEvent(std::string const& eventName, Args&&... args);
};
```

## Contributing

To contribute to the plugin architecture:

1. Follow TrinityCore coding standards
2. Add comprehensive documentation
3. Include unit tests for new features
4. Update this README for API changes
5. Test with multiple plugin scenarios

## License

This plugin architecture is part of TrinityCore and follows the same GPL v2 license.

## Support

For support and questions:
- TrinityCore Discord: [Discord Link]
- TrinityCore Forums: [Forum Link]
- GitHub Issues: [Issues Link]

---

*This plugin architecture provides a solid foundation for extending TrinityCore servers while maintaining code organization and system stability.*
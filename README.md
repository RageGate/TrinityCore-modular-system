# TrinityCore Modular System Integration Guide

This guide explains how to integrate and use the custom modular system with your TrinityCore server, similar to AzerothCore's module system.

## Overview

The modular system provides:
- **Dynamic Plugin Loading**: Load/unload modules at runtime
- **Event-Driven Architecture**: Hook into game events seamlessly
- **Configuration Management**: Per-module configuration files
- **Dependency Management**: Handle module dependencies automatically
- **Hot Reload Support**: Update modules without server restart
- **Build Integration**: Seamless CMake integration

## Directory Structure

```
eclipse/
├── CMakeLists.txt              # Main project configuration
├── src/
│   └── server/
│       └── game/
│           └── Plugins/        # Core plugin system
│               ├── IPlugin.h
│               ├── PluginManager.h/.cpp
│               ├── PluginHooks.h
│               ├── PluginConfig.h
│               └── Examples/
└── modules/                    # Modules directory (like AzerothCore)
    ├── CMakeLists.txt         # Module build configuration
    ├── README.md              # Module system documentation
    ├── create_module.py       # Module creation script (Python)
    ├── create_module.ps1      # Module creation script (PowerShell)
    ├── list_modules.cmake     # List available modules
    ├── validate_modules.cmake # Validate module configurations
    └── mod-example/           # Example module
        ├── module.json        # Module metadata
        ├── CMakeLists.txt     # Module build config
        ├── conf/              # Configuration files
        └── src/               # Source code
```

## Quick Start

### 1. Enable Modules in Build

```bash
# Configure with modules enabled
cmake -DBUILD_MODULES=ON -DBUILD_EXAMPLE_MODULES=ON ..

# Build the project
cmake --build .
```

### 2. Create a New Module

**Using Python:**
```bash
cd modules
python create_module.py --name "MyAwesome" --author "YourName" --description "My awesome module"
```

**Using PowerShell:**
```powershell
cd modules
.\create_module.ps1 -ModuleName "MyAwesome" -Author "YourName" -Description "My awesome module"
```

### 3. Implement Your Module

Edit `modules/mod-myawesome/src/MyAwesomeModule.h`:

```cpp
#pragma once
#include "IPlugin.h"
#include "PluginConfig.h"
#include <memory>

class MyAwesomeModule : public IPlugin
{
public:
    MyAwesomeModule() = default;
    ~MyAwesomeModule() override = default;

    // Plugin lifecycle
    bool OnInitialize() override;
    void OnShutdown() override;
    void OnUpdate(uint32 diff) override;

    // Plugin info
    std::string GetName() const override { return "MyAwesome"; }
    std::string GetVersion() const override { return "1.0.0"; }
    std::string GetDescription() const override { return "My awesome module"; }

    // Event handlers
    void OnPlayerLogin(Player* player);
    void OnPlayerLogout(Player* player);

private:
    std::shared_ptr<PluginConfig> config;
};

// Register the module
REGISTER_PLUGIN(MyAwesomeModule);
```

### 4. Configure Your Module

Edit `modules/mod-myawesome/conf/MyAwesome.conf`:

```ini
[MyAwesome]
# Enable/disable the module
Enabled = 1

# Debug mode
DebugMode = 0

# Custom settings
WelcomeMessage = "Welcome to my awesome server!"
MaxLevel = 80
```

### 5. Build and Test

```bash
# Build with your new module
cmake --build .

# List available modules
cmake --build . --target list-modules

# Validate module configurations
cmake --build . --target validate-modules
```

## Core Integration Points

### 1. Server Startup Integration

Add to your `Main.cpp` or server initialization:

```cpp
#include "PluginManager.h"

// During server startup
bool InitializeServer()
{
    // ... existing initialization ...
    
    // Initialize plugin system
    std::string configPath = "./etc/modules";
    std::string modulesPath = "./modules";
    
    if (!PluginManager::Instance().Initialize(configPath, modulesPath))
    {
        LOG_ERROR("server", "Failed to initialize plugin system");
        return false;
    }
    
    LOG_INFO("server", "Plugin system initialized successfully");
    return true;
}

// During server shutdown
void ShutdownServer()
{
    PluginManager::Instance().Shutdown();
    // ... existing shutdown ...
}
```

### 2. Event Hook Integration

Add hooks to existing game events:

**Player Login (in `WorldSession.cpp`):**
```cpp
void WorldSession::HandlePlayerLogin(LoginQueryHolder* holder)
{
    // ... existing login logic ...
    
    // Trigger plugin hooks
    PluginManager::Instance().GetHooks().OnPlayerLogin(player);
}
```

**Player Logout:**
```cpp
void WorldSession::LogoutPlayer(bool save)
{
    if (Player* player = GetPlayer())
    {
        // Trigger plugin hooks before logout
        PluginManager::Instance().GetHooks().OnPlayerLogout(player);
    }
    
    // ... existing logout logic ...
}
```

**Chat Commands:**
```cpp
bool ChatHandler::HandleCustomCommand(const char* args)
{
    // Let plugins handle custom commands first
    if (PluginManager::Instance().GetHooks().OnChatCommand(GetSession()->GetPlayer(), args))
    {
        return true; // Plugin handled the command
    }
    
    // ... existing command handling ...
}
```

### 3. Configuration Integration

Add module configuration loading to your config system:

```cpp
// In your configuration loading code
void LoadConfigurations()
{
    // ... load existing configs ...
    
    // Load module configurations
    std::filesystem::path moduleConfigDir = "./etc/modules";
    if (std::filesystem::exists(moduleConfigDir))
    {
        for (const auto& entry : std::filesystem::directory_iterator(moduleConfigDir))
        {
            if (entry.path().extension() == ".conf")
            {
                // Module configs are handled by PluginManager
                LOG_INFO("config", "Found module config: {}", entry.path().filename().string());
            }
        }
    }
}
```

## Advanced Features

### Hot Reload Support

```cpp
// Enable hot reload during development
#ifdef TRINITY_DEBUG
    PluginManager::Instance().EnableHotReload(true);
#endif

// Reload a specific module
PluginManager::Instance().ReloadPlugin("MyAwesome");
```

### Dependency Management

In your `module.json`:

```json
{
    "dependencies": {
        "required": ["mod-database", "mod-logging"],
        "optional": ["mod-statistics"]
    }
}
```

### Thread Safety

The plugin system is thread-safe by default:

```cpp
// Safe to call from any thread
PluginManager::Instance().GetPlugin("MyAwesome");

// Plugin hooks are automatically synchronized
PluginManager::Instance().GetHooks().OnPlayerLogin(player);
```

## Build System Integration

### CMake Options

```bash
# Available build options
-DBUILD_MODULES=ON/OFF           # Enable/disable module system
-DBUILD_EXAMPLE_MODULES=ON/OFF   # Build example modules
-DMODULE_HOT_RELOAD=ON/OFF       # Enable hot reload support
```

### Custom Module CMakeLists.txt

```cmake
# modules/mod-mymodule/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Read module metadata
include(${CMAKE_CURRENT_SOURCE_DIR}/../ModuleUtils.cmake)
read_module_json(${CMAKE_CURRENT_SOURCE_DIR}/module.json MODULE_INFO)

# Create module library
file(GLOB_RECURSE MODULE_SOURCES "src/*.cpp" "src/*.h")
add_library(${MODULE_INFO_NAME} SHARED ${MODULE_SOURCES})

# Link with core libraries
target_link_libraries(${MODULE_INFO_NAME}
    game
    shared
    ${TRINITY_GLOBAL_LIBRARIES}
)

# Install module
install(TARGETS ${MODULE_INFO_NAME}
    DESTINATION lib/modules
)

install(FILES conf/${MODULE_INFO_NAME}.conf
    DESTINATION etc/modules
    OPTIONAL
)
```

## Debugging and Development

### Debug Mode

Enable debug logging in your module config:

```ini
[MyModule]
DebugMode = 1
LogLevel = DEBUG
```

### Development Tools

```bash
# List all modules
cmake --build . --target list-modules

# Validate module configurations
cmake --build . --target validate-modules

# Clean and rebuild modules only
cmake --build . --target clean-modules
cmake --build . --target build-modules
```

### Logging

```cpp
// In your module code
#include "Log.h"

void MyAwesomeModule::OnPlayerLogin(Player* player)
{
    LOG_INFO("modules.myawesome", "Player {} logged in", player->GetName());
    LOG_DEBUG("modules.myawesome", "Player GUID: {}", player->GetGUID().ToString());
}
```

## Best Practices

1. **Module Naming**: Use `mod-` prefix for module directories
2. **Configuration**: Always provide default configuration files
3. **Error Handling**: Handle exceptions gracefully in event handlers
4. **Performance**: Avoid heavy operations in frequently called hooks
5. **Dependencies**: Minimize dependencies between modules
6. **Documentation**: Include README.md with usage instructions
7. **Testing**: Test module loading/unloading thoroughly

## Troubleshooting

### Common Issues

1. **Module Not Loading**:
   - Check `module.json` syntax
   - Verify CMakeLists.txt configuration
   - Check server logs for error messages

2. **Build Errors**:
   - Ensure all dependencies are available
   - Check include paths in CMakeLists.txt
   - Validate module structure with `validate-modules`

3. **Runtime Errors**:
   - Enable debug mode in module configuration
   - Check plugin manager initialization
   - Verify event hook registration

### Getting Help

- Check the example module in `modules/mod-example/`
- Review plugin system documentation in `src/server/game/Plugins/README.md`
- Use the validation tools to check module configuration
- Enable debug logging for detailed information

## Migration from Other Systems

If you're migrating from other modular systems:

1. **From AzerothCore Modules**: The structure is very similar, main differences are in the plugin interface
2. **From Custom Scripts**: Convert scripts to modules using the provided templates
3. **From Static Linking**: Modules are dynamically loaded, update your build configuration

This modular system provides the flexibility and power similar to AzerothCore's module system while being tailored for TrinityCore's architecture.

# TrinityCore Modules Directory

This directory contains modular extensions for TrinityCore, inspired by AzerothCore's module system. Each module is a self-contained plugin that extends server functionality.

## Directory Structure

```
modules/
├── README.md                    # This file
├── mod-example/                 # Example module
│   ├── CMakeLists.txt          # Module build configuration
│   ├── module.json             # Module metadata
│   ├── conf/                   # Configuration files
│   │   └── ExampleModule.conf  # Module configuration
│   ├── src/                    # Source code
│   │   ├── ExampleModule.h     # Module header
│   │   └── ExampleModule.cpp   # Module implementation
│   └── README.md               # Module documentation
├── mod-player-tools/           # Player utility module
├── mod-custom-commands/        # Custom chat commands
├── mod-world-events/           # World event system
└── mod-database-tools/         # Database utilities
```

## Module Standards

Each module should follow these conventions:

### 1. Naming Convention
- Module directories: `mod-{module-name}/`
- Module classes: `{ModuleName}Module`
- Configuration files: `{ModuleName}.conf`

### 2. Required Files
- `CMakeLists.txt` - Build configuration
- `module.json` - Module metadata
- `src/` directory with source files
- `README.md` - Module documentation

### 3. Optional Files
- `conf/` directory for configuration files
- `sql/` directory for database scripts
- `data/` directory for data files
- `scripts/` directory for utility scripts

## Module Metadata (module.json)

```json
{
  "name": "ExampleModule",
  "version": "1.0.0",
  "description": "Example module for TrinityCore",
  "author": "Your Name",
  "website": "https://example.com",
  "repository": "https://github.com/user/mod-example",
  "license": "GPL-2.0",
  "trinitycore_version": "3.3.5",
  "dependencies": [],
  "tags": ["example", "tutorial"],
  "enabled_by_default": false,
  "requires_database": false,
  "database_version": "2024010100"
}
```

## Creating a New Module

1. Create module directory: `modules/mod-{your-module}/`
2. Copy template files from `mod-example/`
3. Update `module.json` with your module information
4. Implement your module in `src/` directory
5. Add configuration in `conf/` directory
6. Update CMakeLists.txt for your module
7. Write documentation in README.md

## Building Modules

Modules are automatically discovered and built with TrinityCore:

```bash
# Configure with modules
cmake -DBUILD_MODULES=ON ..

# Build specific module
make mod-example

# Build all modules
make modules
```

## Module Installation

Built modules are installed to:
- **Binaries**: `bin/modules/`
- **Configurations**: `etc/modules/`
- **Data**: `share/modules/`

## Module Management

### Loading Modules
```cpp
// Load specific module
ModuleManager::Instance()->LoadModule("ExampleModule");

// Load all modules from directory
ModuleManager::Instance()->LoadModulesFromDirectory("modules/");
```

### Runtime Management
```cpp
// Enable/disable module
ModuleManager::Instance()->EnableModule("ExampleModule");
ModuleManager::Instance()->DisableModule("ExampleModule");

// Reload module configuration
ModuleManager::Instance()->ReloadModuleConfig("ExampleModule");
```

## Available Modules

### Core Modules
- **mod-example** - Example module demonstrating basic functionality
- **mod-player-tools** - Player utility commands and features
- **mod-custom-commands** - Custom chat command system
- **mod-world-events** - Custom world event management

### Community Modules
Community-contributed modules can be found at:
- [TrinityCore Module Catalogue](https://github.com/trinitycore/modules)
- [Community Forum](https://community.trinitycore.org/forum/133-custom-modules/)

## Module Development Guidelines

### Best Practices
1. **Single Responsibility** - Each module should have one clear purpose
2. **Minimal Dependencies** - Avoid unnecessary dependencies on other modules
3. **Configuration Driven** - Make features configurable rather than hardcoded
4. **Error Handling** - Implement proper error handling and logging
5. **Documentation** - Provide clear documentation and examples

### Code Standards
- Follow TrinityCore coding standards
- Use proper namespacing: `TC::Modules::{ModuleName}`
- Implement all required interface methods
- Use TrinityCore's logging system
- Handle database operations safely

### Testing
- Test module loading/unloading
- Test configuration changes
- Test with multiple players
- Test database operations
- Test error conditions

## Module API

### Core Interfaces
```cpp
class IModule
{
public:
    virtual bool Load() = 0;
    virtual bool Initialize() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Unload() = 0;
    virtual bool LoadConfig(std::string const& path) = 0;
    virtual void ReloadConfig() = 0;
};
```

### Event System
```cpp
// Register for events
ModuleManager::Instance()->RegisterEventHandler("OnPlayerLogin", this);

// Handle events
void OnPlayerLogin(Player* player) override
{
    // Your event handling code
}
```

### Configuration Access
```cpp
// Get configuration values
bool enabled = GetConfigBool("Module.Enabled", true);
std::string message = GetConfigString("Module.Message", "Default");
uint32 value = GetConfigUInt("Module.Value", 100);
```

## Contributing

To contribute a module:

1. Fork the repository
2. Create your module in `modules/mod-{name}/`
3. Follow the module standards
4. Test thoroughly
5. Submit a pull request
6. Update module catalogue

## Support

For module development support:
- [TrinityCore Discord](https://discord.gg/trinitycore)
- [Development Forum](https://community.trinitycore.org/forum/30-development/)
- [GitHub Issues](https://github.com/trinitycore/trinitycore/issues)

## License

Modules inherit TrinityCore's GPL-2.0 license unless otherwise specified.
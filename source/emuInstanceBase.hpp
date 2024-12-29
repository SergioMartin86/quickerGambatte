#pragma once

#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/file.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include "inputParser.hpp"
#include <SDL.h>
#include <gambatte.h>

struct MemoryAreas 
{
	void* vram;
	void* rom;
	void* wram;
  void* cartram;
  void* oam;
  void* hram;
};

struct MemorySizes
{
	int vram;
	int rom;
	int wram;
  int cartram;
  int oam;
  int hram;
};

#define GB_VIDEO_HORIZONTAL_PIXELS 160
#define	GB_VIDEO_VERTICAL_PIXELS 144

#define _AUDIO_SAMPLE_COUNT 35112
#define _AUDIO_EXTRA_SAMPLE_COUNT 2064

namespace jaffar
{

class EmuInstanceBase
{
  public:

  EmuInstanceBase(const nlohmann::json &config)
  {
    _emu =  new gambatte::GB();
    _romFilePath = jaffarCommon::json::getString(config, "Rom File Path");
    _biosFilePath = jaffarCommon::json::getString(config, "Bios File Path");
    _systemType = jaffarCommon::json::getString(config, "System Type");
    _inputParser = std::make_unique<jaffar::InputParser>(config);
  }

  virtual ~EmuInstanceBase() = default;

  virtual void advanceState(const jaffar::input_t &input)
  {
    if (input.power) JAFFAR_THROW_RUNTIME("Power button pressed, but not supported");

    // Setting input
    _inputValue = input.port;

    size_t samples = _AUDIO_SAMPLE_COUNT;
    if (_renderingEnabled == true)  _emu->runFor(_videoBuffer, GB_VIDEO_HORIZONTAL_PIXELS, _audioBuffer, samples);
    if (_renderingEnabled == false) _emu->runFor(nullptr, 0, _audioBuffer, samples); 
    
    // if (status < 0) JAFFAR_THROW_LOGIC("Could not advance state");
    // printf("Status: %d\n", status);
  }

  inline jaffarCommon::hash::hash_t getStateHash() const
  {
    MetroHash128 hash;
    
    //  Getting RAM pointer and size
    //hash.Update(_memoryAreas.wram, _memorySizes.wram);
    hash.Update(_memoryAreas.vram, _memorySizes.vram);

    jaffarCommon::hash::hash_t result;
    hash.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  void initialize()
  {
    // Setting speedup flag
    _emu->setSpeedupFlags(gambatte::GB::SpeedupFlag::NO_SOUND);

    // Rom loading flags
    int romLoadFlags = 0;

    // Getting system type
    bool systemTypeRecognized = false;
    if (_systemType == "Gameboy") { systemTypeRecognized = true; }
    if (_systemType == "Gameboy Color") { romLoadFlags |= gambatte::GB::LoadFlag::CGB_MODE; systemTypeRecognized = true; }
    if (_systemType == "Gameboy Advance") { romLoadFlags |= gambatte::GB::LoadFlag::GBA_FLAG; systemTypeRecognized = true; }
    if (systemTypeRecognized == false) JAFFAR_THROW_LOGIC("Could not recognize system type: %s\n", _systemType.c_str());

    // Set input getter
    _emu->setInputGetter(InputGetter, &_inputValue);

    // Reading from Rom file
    std::string romFileData;
    bool        status = jaffarCommon::file::loadStringFromFile(romFileData, _romFilePath.c_str());
    if (status == false) JAFFAR_THROW_LOGIC("Could not find/read from Rom file: %s\n", _romFilePath.c_str());

    // Reading from Bios file, if defined
    if (_biosFilePath != "")
    {
      std::string biosFileData;
      bool        status = jaffarCommon::file::loadStringFromFile(biosFileData, _biosFilePath.c_str());
      if (status == false) JAFFAR_THROW_LOGIC("Could not find/read from BIOS file: %s\n", _biosFilePath.c_str());

      // Loading bios
      {
        int status = _emu->loadBios(biosFileData.data(), biosFileData.size());
      if (status != 0) JAFFAR_THROW_LOGIC("Could not load BIOS file: '%s' into the emulator\n", _biosFilePath.c_str());
      }
    }
    // If bios not loaded, indicate that to the emu
    else
    {
      romLoadFlags |= gambatte::GB::LoadFlag::NO_BIOS;
    }

    // Load rom file
    {
      int status = _emu->load(romFileData.c_str(), romFileData.length(), romLoadFlags);
      if (status != 0) JAFFAR_THROW_LOGIC("Could not load Rom file: '%s' into the emulator\n", _romFilePath.c_str());
    }

    _stateSize = getEmulatorStateSize();
    // printf("State Size: %lu\n", _stateSize);
    _dummyStateData = (uint8_t*) malloc(_stateSize);

    _videoBufferSize = GB_VIDEO_HORIZONTAL_PIXELS * GB_VIDEO_VERTICAL_PIXELS * sizeof(uint32_t);
    _videoBuffer = (uint32_t*) malloc (_videoBufferSize);
    _audioBuffer = (gambatte::uint_least32_t*) malloc (sizeof(gambatte::uint_least32_t) * (_AUDIO_SAMPLE_COUNT + _AUDIO_EXTRA_SAMPLE_COUNT));

    // printf("Game Title: %s\n", _emu->romTitle().c_str());

    //// Getting memory areas
    /** 0 = vram, 1 = rom, 2 = wram, 3 = cartram, 4 = oam, 5 = hram */

    // VRAM
    {
     bool status = _emu->getMemoryArea(0, (unsigned char**)&_memoryAreas.vram, &_memorySizes.vram);
     if (status == false) JAFFAR_THROW_LOGIC("Could not get memory area: 'vram' from the emulator\n");
    }

    // ROM
    {
     bool status = _emu->getMemoryArea(1, (unsigned char**)&_memoryAreas.rom, &_memorySizes.rom);
     if (status == false) JAFFAR_THROW_LOGIC("Could not get memory area: 'rom' from the emulator\n");
    }

    // WRAM
    {
     bool status = _emu->getMemoryArea(2, (unsigned char**)&_memoryAreas.wram, &_memorySizes.wram);
     if (status == false) JAFFAR_THROW_LOGIC("Could not get memory area: 'wram' from the emulator\n");
    }

    // CARTRAM
    {
     bool status = _emu->getMemoryArea(3, (unsigned char**)&_memoryAreas.cartram, &_memorySizes.cartram);
     if (status == false) JAFFAR_THROW_LOGIC("Could not get memory area: 'cartram' from the emulator\n");
    }

    // OAM
    {
     bool status = _emu->getMemoryArea(4, (unsigned char**)&_memoryAreas.oam, &_memorySizes.oam);
     if (status == false) JAFFAR_THROW_LOGIC("Could not get memory area: 'oam' from the emulator\n");
    }

    // HRAM
    {
     bool status = _emu->getMemoryArea(5, (unsigned char**)&_memoryAreas.hram, &_memorySizes.hram);
     if (status == false) JAFFAR_THROW_LOGIC("Could not get memory area: 'hram' from the emulator\n");
    }
  }

  void initializeVideoOutput()
  {
    SDL_Init(SDL_INIT_VIDEO);
    _renderWindow = SDL_CreateWindow("QuickerMGBA",  SDL_WINDOWPOS_UNDEFINED,  SDL_WINDOWPOS_UNDEFINED, GB_VIDEO_HORIZONTAL_PIXELS, GB_VIDEO_VERTICAL_PIXELS, 0);
    _renderer = SDL_CreateRenderer(_renderWindow, -1, SDL_RENDERER_ACCELERATED);
    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, GB_VIDEO_HORIZONTAL_PIXELS, GB_VIDEO_VERTICAL_PIXELS);
  }

  void finalizeVideoOutput()
  {
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_renderWindow);
    SDL_Quit();
  }

  void enableRendering()
  {
    _renderingEnabled = true;
    _emu->setSpeedupFlags(gambatte::GB::SpeedupFlag::NO_SOUND);
  }

  void disableRendering()
  {
    _renderingEnabled = false;
    _emu->setSpeedupFlags(gambatte::GB::SpeedupFlag::NO_SOUND | gambatte::GB::SpeedupFlag::NO_VIDEO | gambatte::GB::SpeedupFlag::NO_PPU_CALL);
  }

  void updateRenderer()
  {
    void *pixels = nullptr;
    int pitch = 0;

    SDL_Rect srcRect  = { 0, 0, GB_VIDEO_HORIZONTAL_PIXELS, GB_VIDEO_VERTICAL_PIXELS };
    SDL_Rect destRect = { 0, 0, GB_VIDEO_HORIZONTAL_PIXELS, GB_VIDEO_VERTICAL_PIXELS };

    if (SDL_LockTexture(_texture, nullptr, &pixels, &pitch) < 0) return;
    memcpy(pixels, _videoBuffer, sizeof(uint32_t) * GB_VIDEO_VERTICAL_PIXELS * GB_VIDEO_HORIZONTAL_PIXELS);
    // memset(pixels, (32 << 24) + (32 << 16) + (32 << 8) + 32, sizeof(uint32_t) * GB_VIDEO_VERTICAL_PIXELS * GB_VIDEO_HORIZONTAL_PIXELS);
    SDL_UnlockTexture(_texture);
    SDL_RenderClear(_renderer);
    SDL_RenderCopy(_renderer, _texture, &srcRect, &destRect);
    SDL_RenderPresent(_renderer);
  }

  size_t getEmulatorStateSize()
  {
    return (size_t)_emu->saveState(nullptr, 0, nullptr);
  }

  void enableStateBlock(const std::string& block) 
  {
    enableStateBlockImpl(block);
  }

  void disableStateBlock(const std::string& block)
  {
     disableStateBlockImpl(block);
    _stateSize = getEmulatorStateSize();
  }

  virtual void setWorkRamSerializationSize(const size_t size)
  {
    setWorkRamSerializationSizeImpl(size);
    _stateSize = getEmulatorStateSize();
  }

  inline size_t getStateSize() const 
  {
    return _stateSize;
  }

  inline jaffar::InputParser *getInputParser() const { return _inputParser.get(); }
  
  void serializeState(jaffarCommon::serializer::Base& s) const
  {
    // VFile* vf;
    _emu->saveState(nullptr, 0, (char*)_dummyStateData);
    s.push(_dummyStateData, _stateSize);
  }

  void deserializeState(jaffarCommon::deserializer::Base& d) 
  {
    d.pop(_dummyStateData, _stateSize);
    _emu->loadState((char*)_dummyStateData, _stateSize);
  }

  size_t getVideoBufferSize() const { return _videoBufferSize; }
  uint8_t* getVideoBufferPtr() const { return (uint8_t*)_videoBuffer; }

  MemoryAreas getMemoryAreas() const { return _memoryAreas; }
  MemorySizes getMemorySizes() const { return _memorySizes; }

  // Virtual functions

  virtual void doSoftReset() = 0;
  virtual void doHardReset() = 0;
  virtual std::string getCoreName() const = 0;

  protected:


  virtual void setWorkRamSerializationSizeImpl(const size_t size) {};
  virtual void enableStateBlockImpl(const std::string& block) {};
  virtual void disableStateBlockImpl(const std::string& block) {};

  // State size
  size_t _stateSize;

  private:

  static uint32_t InputGetter(void* inputValue) { return *(uint32_t*)inputValue; }

  std::string _systemType;

  gambatte::GB* _emu;
  MemoryAreas _memoryAreas;
  MemorySizes _memorySizes;

  // Dummy storage for state load/save
  uint8_t* _dummyStateData;
  std::string _romFilePath;
  std::string _biosFilePath;

  // Input parser instance
  uint32_t _inputValue;
  std::unique_ptr<jaffar::InputParser> _inputParser;

  // Rendering stuff
  SDL_Window* _renderWindow;
  SDL_Renderer* _renderer;
  SDL_Texture* _texture;
  uint32_t* _videoBuffer;
  size_t _videoBufferSize;
  bool _renderingEnabled = false;
  gambatte::uint_least32_t* _audioBuffer;
};

} // namespace jaffar
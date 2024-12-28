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
// #include <mgba-util/vfs.h>

struct MemoryAreas 
{
	void* bios;
	void* wram;
	void* iwram;
	void* mmio;
	void* palram;
	void* vram;
	void* oam;
	void* rom;
	void* sram;
};

struct MemorySizes
{
	size_t bios;
	size_t wram;
	size_t iwram;
	size_t mmio;
	size_t palram;
	size_t vram;
	size_t oam;
	size_t rom;
	size_t sram;
};

// extern "C" {
//  void* BizCreate(const void* bios, const void* data, uint32_t length, bool skipbios);
//  bool BizStartGetState(void* ctx, struct VFile** file, uint32_t* size);
//  void BizFinishGetState(struct VFile* file, void* data, uint32_t size);
//  bool BizPutState(void* ctx, const void* data, uint32_t size);
//  bool BizAdvance(void* ctx, uint16_t keys, int64_t time, int16_t gyrox, int16_t gyroy, int16_t gyroz, uint8_t luma, uint32_t* videoBuffer);
//  void BizGetMemoryAreas(void* ctx, struct MemoryAreas* dst);
//  void BizGetMemorySizes(void* ctx, struct MemorySizes* dst);
// }

#define GBA_VIDEO_HORIZONTAL_PIXELS 240
#define	GBA_VIDEO_VERTICAL_PIXELS 160

namespace jaffar
{

class EmuInstanceBase
{
  public:

  EmuInstanceBase(const nlohmann::json &config)
  {
    _romFilePath = jaffarCommon::json::getString(config, "Rom File Path");
    _biosFilePath = jaffarCommon::json::getString(config, "Bios File Path");
    _inputParser = std::make_unique<jaffar::InputParser>(config);
  }

  virtual ~EmuInstanceBase() = default;

  virtual void advanceState(const jaffar::input_t &input)
  {
    if (input.power) JAFFAR_THROW_RUNTIME("Power button pressed, but not supported");

    int64_t time = 1729787535;
    time += (_emulationStep * 4389L) >> 18;

    // if (_renderingEnabled == true) BizAdvance(_emu, input.port, time, 0, 0, 0, 255, _videoBuffer);
    // if (_renderingEnabled == false) BizAdvance(_emu, input.port, time, 0, 0, 0, 255, NULL);

    _emulationStep++;
  }

  inline jaffarCommon::hash::hash_t getStateHash() const
  {
    MetroHash128 hash;
    
    //  Getting RAM pointer and size
    // hash.Update(_memoryAreas.wram, _memorySizes.wram);
    // hash.Update(_memoryAreas.iwram, _memorySizes.iwram);
    // hash.Update(getVideoBufferPtr(), getVideoBufferSize());

    jaffarCommon::hash::hash_t result;
    hash.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  void initialize()
  {
    // Reading from Rom file
    std::string romFileData;
    bool        status = jaffarCommon::file::loadStringFromFile(romFileData, _romFilePath.c_str());
    if (status == false) JAFFAR_THROW_LOGIC("Could not find/read from Rom file: %s\n", _romFilePath.c_str());

    // Reading from Bios file, if defined
    std::string biosFileData;
    void* biosFileDataPtr = nullptr;
    bool skipBios = true;
    if (_biosFilePath != "")
    {
      bool        status = jaffarCommon::file::loadStringFromFile(biosFileData, _biosFilePath.c_str());
      if (status == false) JAFFAR_THROW_LOGIC("Could not find/read from BIOS file: %s\n", _biosFilePath.c_str());
      biosFileDataPtr = biosFileData.data();
      skipBios = false;
    }

    // _emu = BizCreate(biosFileDataPtr, romFileData.data(), romFileData.size(), skipBios);
    
    // _emuStateSize = getEmulatorStateSize();
    // _stateSize = _emuStateSize + sizeof(_emulationStep);

    _dummyStateData = (uint8_t*) malloc(_stateSize);

    _videoBufferSize = GBA_VIDEO_HORIZONTAL_PIXELS * GBA_VIDEO_VERTICAL_PIXELS * sizeof(uint32_t);
    _videoBuffer = (uint32_t*) malloc (_videoBufferSize);

    // BizGetMemoryAreas(_emu, &_memoryAreas);
    // BizGetMemorySizes(_emu, &_memorySizes);
  }

  void initializeVideoOutput()
  {
    SDL_Init(SDL_INIT_VIDEO);
    _renderWindow = SDL_CreateWindow("QuickerMGBA",  SDL_WINDOWPOS_UNDEFINED,  SDL_WINDOWPOS_UNDEFINED, GBA_VIDEO_HORIZONTAL_PIXELS, GBA_VIDEO_VERTICAL_PIXELS, 0);
    _renderer = SDL_CreateRenderer(_renderWindow, -1, SDL_RENDERER_ACCELERATED);
    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, GBA_VIDEO_HORIZONTAL_PIXELS, GBA_VIDEO_VERTICAL_PIXELS);
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
  }

  void disableRendering()
  {
    _renderingEnabled = false;
  }

  void updateRenderer()
  {
    void *pixels = nullptr;
    int pitch = 0;

    SDL_Rect srcRect  = { 0, 0, GBA_VIDEO_HORIZONTAL_PIXELS, GBA_VIDEO_VERTICAL_PIXELS };
    SDL_Rect destRect = { 0, 0, GBA_VIDEO_HORIZONTAL_PIXELS, GBA_VIDEO_VERTICAL_PIXELS };

    if (SDL_LockTexture(_texture, nullptr, &pixels, &pitch) < 0) return;
    memcpy(pixels, _videoBuffer, sizeof(uint32_t) * GBA_VIDEO_VERTICAL_PIXELS * GBA_VIDEO_HORIZONTAL_PIXELS);
    // memset(pixels, (32 << 24) + (32 << 16) + (32 << 8) + 32, sizeof(uint32_t) * GBA_VIDEO_VERTICAL_PIXELS * GBA_VIDEO_HORIZONTAL_PIXELS);
    SDL_UnlockTexture(_texture);
    SDL_RenderClear(_renderer);
    SDL_RenderCopy(_renderer, _texture, &srcRect, &destRect);
    SDL_RenderPresent(_renderer);
  }

  size_t getEmulatorStateSize()
  {
    VFile* vf;
    uint32_t size; 
    // BizStartGetState(_emu, &vf, &size);
    return (size_t) size;
  }

  void enableStateBlock(const std::string& block) 
  {
    enableStateBlockImpl(block);
    _emuStateSize = getEmulatorStateSize();
    _stateSize = _emuStateSize + sizeof(_emulationStep);
    _differentialStateSize = getDifferentialStateSizeImpl();
  }

  void disableStateBlock(const std::string& block)
  {
     disableStateBlockImpl(block);
    _emuStateSize = getEmulatorStateSize();
    _stateSize = _emuStateSize + sizeof(_emulationStep);
    _differentialStateSize = getDifferentialStateSizeImpl();
  }

  virtual void setWorkRamSerializationSize(const size_t size)
  {
    setWorkRamSerializationSizeImpl(size);
    _emuStateSize = getEmulatorStateSize();
    _stateSize = _emuStateSize + sizeof(_emulationStep);
    _differentialStateSize = getDifferentialStateSizeImpl();
  }

  inline size_t getStateSize() const 
  {
    return _stateSize;
  }

  inline size_t getDifferentialStateSize() const
  {
    return _differentialStateSize;
  }
  
  inline jaffar::InputParser *getInputParser() const { return _inputParser.get(); }
  
  void serializeState(jaffarCommon::serializer::Base& s) const
  {
    VFile* vf;
    uint32_t size; 
    // BizStartGetState(_emu, &vf, &size);
    // BizFinishGetState(vf, _dummyStateData, _emuStateSize);
    s.push(_dummyStateData, _emuStateSize);
    s.push(&_emulationStep, sizeof(_emulationStep));
  }

  void deserializeState(jaffarCommon::deserializer::Base& d) 
  {
    d.pop(_dummyStateData, _emuStateSize);
    // BizPutState(_emu, _dummyStateData, _emuStateSize);
    d.pop(&_emulationStep, sizeof(_emulationStep));
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
  virtual size_t getDifferentialStateSizeImpl() const = 0;

  // State size
  size_t _emuStateSize;
  size_t _stateSize;

  private:

  void* _emu;
  MemoryAreas _memoryAreas;
  MemorySizes _memorySizes;

  // Dummy storage for state load/save
  uint8_t* _dummyStateData;
  std::string _romFilePath;
  std::string _biosFilePath;

  // Differential state size
  size_t _differentialStateSize;

  // Input parser instance
  std::unique_ptr<jaffar::InputParser> _inputParser;

  // Rendering stuff
  SDL_Window* _renderWindow;
  SDL_Renderer* _renderer;
  SDL_Texture* _texture;
  uint32_t* _videoBuffer;
  size_t _videoBufferSize;
  bool _renderingEnabled = false;

  // current emulation step
  size_t _emulationStep = 0;

};

} // namespace jaffar
# Gambatte Core sources

gambatteSrc =  [
			'core/libgambatte/src/bitmap_font.cpp',
			'core/libgambatte/src/cpu.cpp',
			'core/libgambatte/src/gambatte.cpp',
			'core/libgambatte/src/initstate.cpp',
			'core/libgambatte/src/interrupter.cpp',
			'core/libgambatte/src/interruptrequester.cpp',
			'core/libgambatte/src/loadres.cpp',
			'core/libgambatte/src/memory.cpp',
			'core/libgambatte/src/newstate.cpp',
			'core/libgambatte/src/sound.cpp',
			'core/libgambatte/src/state_osd_elements.cpp',
			'core/libgambatte/src/statesaver.cpp',
			'core/libgambatte/src/tima.cpp',
			'core/libgambatte/src/video.cpp',
			'core/libgambatte/src/mem/camera.cpp',
			'core/libgambatte/src/mem/cartridge.cpp',
			'core/libgambatte/src/mem/huc3_chip.cpp',
			'core/libgambatte/src/mem/infrared.cpp',
			'core/libgambatte/src/mem/memptrs.cpp',
			'core/libgambatte/src/mem/pakinfo.cpp',
			'core/libgambatte/src/mem/remote.cpp',
			'core/libgambatte/src/mem/rtc.cpp',
			'core/libgambatte/src/mem/sgb.cpp',
			'core/libgambatte/src/mem/time.cpp',
			'core/libgambatte/src/mem/mbc/huc1.cpp',
			'core/libgambatte/src/mem/mbc/huc3.cpp',
			'core/libgambatte/src/mem/mbc/m161.cpp',
			'core/libgambatte/src/mem/mbc/mbc0.cpp',
			'core/libgambatte/src/mem/mbc/mbc1.cpp',
			'core/libgambatte/src/mem/mbc/mbc2.cpp',
			'core/libgambatte/src/mem/mbc/mbc3.cpp',
			'core/libgambatte/src/mem/mbc/mbc5.cpp',
			'core/libgambatte/src/mem/mbc/mmm01.cpp',
			'core/libgambatte/src/mem/mbc/pocket_camera.cpp',
			'core/libgambatte/src/mem/mbc/wisdom_tree.cpp',
			'core/libgambatte/src/mem/snes_spc/dsp.cpp',
			'core/libgambatte/src/mem/snes_spc/SNES_SPC.cpp',
			'core/libgambatte/src/mem/snes_spc/SNES_SPC_misc.cpp',
			'core/libgambatte/src/mem/snes_spc/SNES_SPC_state.cpp',
			'core/libgambatte/src/mem/snes_spc/spc.cpp',
			'core/libgambatte/src/mem/snes_spc/SPC_DSP.cpp',
			'core/libgambatte/src/mem/snes_spc/SPC_Filter.cpp',
			'core/libgambatte/src/sound/channel1.cpp',
			'core/libgambatte/src/sound/channel2.cpp',
			'core/libgambatte/src/sound/channel3.cpp',
			'core/libgambatte/src/sound/channel4.cpp',
			'core/libgambatte/src/sound/duty_unit.cpp',
			'core/libgambatte/src/sound/envelope_unit.cpp',
			'core/libgambatte/src/sound/length_counter.cpp',
			'core/libgambatte/src/video/ly_counter.cpp',
			'core/libgambatte/src/video/lyc_irq.cpp',
			'core/libgambatte/src/video/next_m0_time.cpp',
			'core/libgambatte/src/video/ppu.cpp',
			'core/libgambatte/src/video/sprite_mapper.cpp',
			'core/libgambatte/src/file/crc32.cpp',
			'core/libgambatte/src/file/file.cpp',
]

gambatteIncludeDirs = [
  '.',
  '..',
		'core/common',
		'core/libgambatte/include',
		'core/libgambatte/src',
		'core/libgambatte/src/file',
		'core/libgambatte/src/mem/mbc',
		'core/libgambatte/src/mem/snes_spc',
		'core/libgambatte/src/sound',
		'core/libgambatte/src/video',
]

gambatteCompileArgs = [
  '-DHAVE_STDINT_H'
]

# gambatte Core Configuration

 baseDependency = declare_dependency(
  compile_args        : [  gambatteCompileArgs],
		link_args           : [ '-lz' ],
  include_directories : include_directories(gambatteIncludeDirs),
  sources             : [ gambatteSrc ],
  dependencies        : [  
                          dependency('sdl2',  required : true),
                        ]
 )

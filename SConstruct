import os

tooldir=os.getenv('PATH') 

if not ARGUMENTS.has_key('CROX_TOOLS_PATH'):
    #ARGUMENTS['CROX_TOOLS_PATH'] = '/opt/hisi-linux/x86-arm/arm-hisiv100-linux/target/bin'
    ARGUMENTS['CROX_TOOLS_PATH'] = '/opt/hisi-linux-nptl/arm-hisiv100-linux/target/bin/'
if not ARGUMENTS.has_key('CROX_TOOLS_PREFIX'):
    #ARGUMENTS['CROX_TOOLS_PREFIX'] = 'arm-hisiv100-linux-'
    ARGUMENTS['CROX_TOOLS_PREFIX'] = 'arm-hisiv100nptl-linux-'

#_path, _prefix =  ('/opt/hisi-linux/x86-arm/arm-hisiv100-linux/target/bin', 'arm-hisiv100-linux-')
#_path, _prefix =  ('/opt/hisi-linux-nptl/arm-hisiv100-linux/target/bin/', 'arm-hisiv100nptl-linux-')

_path, _prefix =  (tooldir, '')

targets = set(COMMAND_LINE_TARGETS)

if 0 == len(targets):
	targets = [
	'webrtc/',
	'webrtc/base/',
	'webrtc/voice_engine/',	
	'webrtc/audio/',
	'webrtc/video/',
	'webrtc/system_wrappers',
	'webrtc/common_audio/',
	'webrtc/common_audio/resampler',
	'webrtc/common_audio/signal_processing',
	'webrtc/common_audio/vad',
	'webrtc/modules/utility',
	'webrtc/modules/media_file',
	'third_party/depends/libyuv',
	'webrtc/modules/audio_conference_mixer',
	'webrtc/modules/audio_processing',
	'webrtc/modules/audio_coding/codecs',
	'webrtc/modules/audio_coding/codecs/cng',
	'webrtc/modules/audio_coding/codecs/g711',
	'webrtc/modules/audio_coding/codecs/g722',
	'webrtc/modules/audio_coding/codecs/g7221',
	'webrtc/modules/audio_coding/codecs/g726',
	'webrtc/modules/audio_coding/codecs/g729',
	'webrtc/modules/audio_coding/codecs/ilbc',
	'webrtc/modules/audio_coding/codecs/isac/fix/source',
	'webrtc/modules/audio_coding/codecs/isac/',
	'webrtc/modules/audio_coding/codecs/pcm16b',
	'webrtc/modules/audio_coding/codecs/opus',
	'webrtc/modules/audio_coding/acm2',
	'webrtc/modules/audio_coding/neteq',
	'webrtc/common_video/',
	'webrtc/modules/video_processing',
	'webrtc/modules/video_coding/codecs/vp8',
	'webrtc/modules/video_coding/codecs/vp9',
	'webrtc/modules/video_coding/codecs/h264',
	'webrtc/modules/pacing',
	'webrtc/modules/video_capture',
	'webrtc/modules/rtp_rtcp',
	#'webrtc/modules/rtp_rtcp/test/mytest',
	'webrtc/modules/video_coding',
	'webrtc/modules/bitrate_controller',
	'webrtc/modules/remote_bitrate_estimator',
	'webrtc/modules/congestion_controller',
	'webrtc/test',
	'webrtc/call/',
	]



for _target in targets:
    if 'install' == _target:
       continue
    #env = Environment(tools=['default', 'croxpile'], **ARGUMENTS)
    env = Environment(CC = _prefix + 'gcc', 
            			CXX = _prefix + 'g++', 
			            STRIP = _prefix + 'strip', 
			            AR = _prefix + 'ar', 
			            RANLIB = _prefix + 'ranlib',
			            AS=_prefix + 'gcc')

    env['ARFLAGS']='crsP'

    _ASFLAGS='-c -fno-exceptions -Wno-multichar -msoft-float -fpic -fPIE -ffunction-sections -fdata-sections -funwind-tables -fstack-protector -Wa,--noexecstack -Werror=format-security -D_FORTIFY_SOURCE=1 -fno-short-enums -march=armv7-a -fno-builtin-sin -Wno-psabi -mthumb-interwork -DANDROID -fmessage-length=0 -W -Wall -Wno-unused -Winit-self -Wpointer-arith -Werror=return-type -Werror=non-virtual-dtor -Werror=address -Werror=sequence-point -DNDEBUG -g -Wstrict-aliasing=2 -fgcse-after-reload -frerun-cse-after-loop -frename-registers'
#-march=armv7-a -mfloat-abi=softfp -mfpu=vfp 

  #  env.Append(ASFLAGS = _ASFLAGS.split())  #-mcpu=cortex-a9 -mtune=cortex-a9 -O3 -mfloat-abi=softfp -mfpu=vfp -ffast-math 
    
    '''-fno-exceptions -Wno-multichar -msoft-float -fpic -fPIE -ffunction-sections -fdata-sections -funwind-tables -fstack-protector -Wa,--noexecstack -Werror=format-security -D_FORTIFY_SOURCE=1 -fno-short-enums -O3 -mcpu=cortex-a9 -mfpu=vfpv3-d16 -mfloat-abi=softfp -ffast-math -fno-builtin-sin -Wno-psabi -mthumb-interwork -DANDROID -fmessage-length=0 -W -Wall -Wno-unused -Winit-self -Wpointer-arith -Werror=return-type -Werror=non-virtual-dtor -Werror=address -Werror=sequence-point -DNDEBUG  -Wstrict-aliasing=2 -fgcse-after-reload -frerun-cse-after-loop -frename-registers '''
    env.Append(CPPFLAGS = ' -fno-exceptions -Wno-multichar  -fpic -fPIE -ffunction-sections -fdata-sections -funwind-tables -fstack-protector -Wa,--noexecstack -Werror=format-security -D_FORTIFY_SOURCE=1 -fno-short-enums -std=c++11 -fno-builtin-sin -Wno-psabi -fmessage-length=0 -W -Wall -Wno-unused -Winit-self -Wpointer-arith -Werror=return-type -Werror=address -Werror=sequence-point -DNDEBUG -g -Wstrict-aliasing=2 -fgcse-after-reload -frerun-cse-after-loop -frename-registers -DNDEBUG -UDEBUG -fvisibility-inlines-hidden -DANDROID -fmessage-length=0 -W -Wall -Wno-unused -Winit-self -Wpointer-arith -Wsign-promo -Werror=return-type -Werror=address -Werror=sequence-point -DNDEBUG -UDEBUG -fomit-frame-pointer -fno-strict-aliasing  -fno-rtti  -DLOG_NDEBUG=0 -fno-omit-frame-pointer -Wno-multichar -DWEBRTC_DIRECT_TRACE\
                            -DNDEBUG -UDEBUG\
                            -DWEBRTC_TARGET_PC\
                            -DWEBRTC_LINUX\
                            -DWEBRTC_THREAD_RR\
                            -DWEBRTC_CLOCK_TYPE_REALTIME\
                            -DWEBRTC_ARCH_ARM\
                            -DWEBRTC_POSIX\
                            -DNO_NEON_SUPPORT\
                            ' )
    #-DWEBRTC_ANDROID\
    # -DWEBRTC_ARCH_ARM_NEON -mfloat-abi=softfp -mfpu=neon -DWEBRTC_ARCH_ARM_V7A
    env.PrependENVPath('PATH', _path) 
       
    env.Append(CPPPATH = [os.getcwd()+'/../']) 
    Export('env')
   

    _output = os.sep.join(['out', _target])
    _subcons = os.sep.join([_target, 'SConscript'])
    objs = env.SConscript(_subcons, variant_dir=_output, duplicate=0)
    print objs
# print '-----------',objs,_subcons
    for _ in objs: 
         print str(_)
         env.InstallAs(os.getcwd()+'/release/'+str(_).split('/')[-1],_) 
         #env.InstallAs('/nfsroot/release/process_test/'+str(_).split('/')[-1],_) 
         env.Alias('install', os.getcwd()+'/release')
         #env.Alias('install', '/nfsroot/release/process_test')
    


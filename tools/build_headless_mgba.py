"""Build a pinned mGBA headless frontend with screenshots, SRAM and frame bounds.
The emulator core is unchanged. Source/binaries live only under ignored build/.
"""
import argparse, subprocess
from pathlib import Path
REVISION='cef7dde504af189e47f6074365f2d77e8177ad06'
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=Path('build/tools/mgba-source'));p.add_argument('--output',type=Path,default=Path('build/tools/mgba-headless'));args=p.parse_args()
revision=subprocess.check_output(['git','-C',str(args.source),'rev-parse','HEAD'],text=True).strip()
assert revision==REVISION,'mGBA source must match desktop emulator commit'
# Start from the immutable upstream file, so running setup twice is idempotent.
source=subprocess.check_output(['git','-C',str(args.source),'show','HEAD:src/platform/headless-main.c'],text=True)
source=source.replace('#define HEADLESS_OPTIONS "S:R:"','#define HEADLESS_OPTIONS "S:R:F:"')
source=source.replace('"Additional options:\\n"','"Additional options:\\n"\n\t"  -F FRAMES        Exit cleanly after a bounded video-frame count\\n"')
source=source.replace('struct HeadlessOpts {','struct HeadlessOpts {\n\tuint32_t frameLimit;')
source=source.replace('struct HeadlessOpts headlessOpts = { 3, NULL };','struct HeadlessOpts headlessOpts = { .exitSwiImmediate = 3 };')
source=source.replace('static void _headlessCallback(void* context);','''static void _headlessCallback(void* context);
static uint32_t _frameLimit, _frames;
static void _headlessFrame(void* context) {
 UNUSED(context);
 if (_frameLimit && ++_frames >= _frameLimit) _dispatchExiting = true;
}''')
source=source.replace('struct mCoreCallbacks callbacks = {0};','''struct mCoreCallbacks callbacks = {0};
 _frameLimit = headlessOpts.frameLimit;
 struct mCoreCallbacks frameCallbacks = { .videoFrameEnded = _headlessFrame };
 core->addCoreCallbacks(core, &frameCallbacks);''')
source=source.replace('\tcore->reset(core);',''' unsigned width, height;
 core->baseVideoSize(core, &width, &height);
 mColor* videoBuffer = calloc(width * height, sizeof(mColor));
 if (!videoBuffer) goto loadError;
 core->setVideoBuffer(core, videoBuffer, width);
 mCoreAutoloadSave(core);
 core->reset(core);''',1)
source=source.replace('\tcore->unloadROM(core);\n\n#ifdef ENABLE_SCRIPTING','\tcore->unloadROM(core);\n free(videoBuffer);\n\n#ifdef ENABLE_SCRIPTING',1)
source=source.replace("\tcase 'S':\n\t\treturn _parseSwi",''' case 'F': {
  char* end = NULL;
  errno = 0;
  unsigned long frames = strtoul(arg, &end, 10);
  if (errno || !end || *end || !frames || frames > UINT32_MAX) return false;
  opts->frameLimit = (uint32_t) frames;
  return true;
 }
\tcase 'S':
\t\treturn _parseSwi''',1)
(args.source/'src/platform/headless-main.c').write_text(source)
cmake=Path('.venv/bin/cmake').resolve()
subprocess.run([str(cmake),'-S',str(args.source),'-B',str(args.output),'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_POLICY_VERSION_MINIMUM=3.5','-DCMAKE_MAKE_PROGRAM='+str(Path('.venv/bin/ninja').resolve()),'-DBUILD_QT=OFF','-DBUILD_SDL=OFF','-DBUILD_HEADLESS=ON','-DBUILD_SHARED=OFF','-DBUILD_STATIC=ON','-DBUILD_GL=OFF','-DBUILD_GLES2=OFF','-DBUILD_GLES3=OFF','-DUSE_FFMPEG=OFF','-DUSE_DISCORD_RPC=OFF','-DUSE_LUA=ON','-DUSE_PNG=ON','-DUSE_ZLIB=ON','-DCMAKE_PREFIX_PATH=/opt/homebrew'],check=True)
subprocess.run([str(cmake),'--build',str(args.output),'--target','mgba-headless','--parallel','4'],check=True)
print('Built pinned mGBA core with frontend-only frame bound, pixel buffer and SRAM loading.')

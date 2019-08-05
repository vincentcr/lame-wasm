#!/bin/sh

####################################################################################################
#
# Converts an MP3 file to PCM float32 (little endian), which is the encoding used by the web audio
# api.
#
# Ref:
# https://stackoverflow.com/questions/4854513/can-ffmpeg-convert-audio-to-raw-pcm-if-so-how/4854627
# https://trac.ffmpeg.org/wiki/audio%20types
# https://trac.ffmpeg.org/wiki/AudioChannelManipulation
#
####################################################################################################

set -e

SOURCE_FILE="src.mp3"
DEST_FILE_PFX="__new__"
TS_RUNNER="../../node_modules/.bin/ts-node"

cd "$(dirname "$0")"

ffmpeg -y  \
  -i $SOURCE_FILE  \
  -filter_complex "[0:a]channelsplit=channel_layout=stereo[left][right]" \
  -acodec pcm_f32le -f f32le -ac 1 -ar 44100 -map "[left]" ${DEST_FILE_PFX}input-stereo-left.pcm \
  -acodec pcm_f32le -f f32le -ac 1 -ar 44100 -map "[right]" ${DEST_FILE_PFX}input-stereo-right.pcm

ffmpeg -y  \
  -i $SOURCE_FILE \
  -acodec pcm_f32le -f f32le -ac 1 -ar 44100 ${DEST_FILE_PFX}input-mono.pcm


$TS_RUNNER ./encode.ts ${DEST_FILE_PFX}input-mono.pcm > \
  ${DEST_FILE_PFX}output-mono.mp3
$TS_RUNNER ./encode.ts ${DEST_FILE_PFX}input-stereo-{left,right}.pcm > \
  ${DEST_FILE_PFX}output-stereo.mp3

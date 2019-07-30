// simple reference working example using the lame library directly in c++
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <lame/lame.h>

#define MAX_SAMPLES 16384
#define BUF_SIZE (MAX_SAMPLES * 1.25 + 7200)

const char* INPUT_FILE = "sample.pcm";
const char* OUTPUT_FILE = "sample-out-c.mp3";
const int SAMPLE_RATE = 44100;

size_t fileSize(FILE* pFile);
void die(int exitCode, const char fmt[], ...);
size_t readAllBytes(const char fname[], void** pData);

int main() {
  int ret;

  lame_global_flags* gfp = lame_init();
  lame_set_mode(gfp, MONO);
  lame_set_num_channels(gfp, 1);
  lame_set_in_samplerate(gfp, SAMPLE_RATE);
  lame_set_VBR(gfp, vbr_default);
  lame_set_VBR_quality(gfp, 5);

  ret = lame_init_params(gfp);
  if (ret != 0) {
    die(1, "lame_init_params failed with %d", ret);
  }

  float* pcmSamples;
  size_t inputSize = readAllBytes(INPUT_FILE, (void**)&pcmSamples);
  int numSamplesTotal = inputSize / sizeof(float);
  FILE* pOutputFile = fopen(OUTPUT_FILE, "wb");
  if (pOutputFile == NULL) {
    die(2, "Failed to open %s", OUTPUT_FILE);
  }

  int outBufSize = 1024 * 1024;
  unsigned char* outBuf = (unsigned char*)malloc(outBufSize);

  int chunkOffset = 0;
  while (chunkOffset < numSamplesTotal) {
    float* chunk = pcmSamples + chunkOffset;
    int nSamples = fmin(MAX_SAMPLES, numSamplesTotal - chunkOffset);
    printf("calling encode with offset %d, nSamples %d, total size %ld\n",
          chunkOffset, nSamples, numSamplesTotal);

    int nEncoded = lame_encode_buffer_ieee_float(gfp, chunk, NULL, nSamples, outBuf, outBufSize);
    printf("encoded: numSamplesTotal=%ld; nSamples=%d; chunkOffset=%d; nEncoded=%d\n",
          numSamplesTotal, nSamples, chunkOffset, nEncoded);
    chunkOffset += nSamples;
    if (nEncoded <0) {
      die(4, "Failed to encode %d bytes from offset %d", nSamples, chunkOffset);
    }

    int nWritten = fwrite(outBuf, 1, nEncoded, pOutputFile);
    if (nWritten != nEncoded) {
      die(4, "Failed to write %d bytes to output, fwrite returned %d", nEncoded, nWritten);
    }
  }

  fclose(pOutputFile);
  free(outBuf);
  free (pcmSamples);
}

size_t readAllBytes(const char fname[], void** pData) {
  FILE * pFile = fopen(fname, "rb");
  if (pFile == NULL) {
    die(2, "Failed to open %s", fname);
  }

  long sz = fileSize(pFile);

  // allocate memory to contain the whole file:
  float* data = (float*) malloc(sz);
  if (data == NULL) {
    die(3, "Failed to allocate %d bytes for input", sz);
  }

  // copy the file into the buffer:
  size_t nRead = fread(data, 1, sz, pFile);
  if (nRead != sz) {
    die(4, "reading error. got %d bytes, expected %d", nRead, sz);
  }

  fclose(pFile);

  *pData = data;

  return sz;
}


size_t fileSize(FILE* pFile) {
  fseek (pFile , 0 , SEEK_END);
  long sz = ftell(pFile);
  rewind (pFile);
  return sz;
}

void die(int exitCode, const char fmt[], ...) {

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  if (errno != 0) {
    fprintf(stderr, ". Last system error: %s (%d)", strerror(errno), errno);
  } else {
    fprintf(stderr, ".");
  }
  fprintf(stderr, "\n");
  exit(exitCode);
}

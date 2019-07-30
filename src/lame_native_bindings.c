// bindings to expose lame library as wasm module
// credits to https://github.com/Kagami/vmsg/blob/master/vmsg.c
// modified for streamed encoding

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <lame/lame.h>

#define WASM_EXPORT __attribute__((visibility("default")))
#define MAX_SAMPLES 65536
#define PCM_BUF_SIZE MAX_SAMPLES * sizeof(float)
#define BUF_SIZE (MAX_SAMPLES * 1.25 + 7200)

typedef struct {
  // Public fields.
  float *pcm_left;
  float *pcm_right;
  uint8_t *outbuf;
  bool stereo;
  // Private fields. Should not be touched by API user.
  lame_global_flags *gfp;
} vmsg;

void lamejs_free(vmsg *v);


WASM_EXPORT
vmsg *lamejs_init(int rate, bool stereo, int vbr_quality) {
  vmsg *v = calloc(1, sizeof (vmsg));
  if (!v) {
    goto err;
  }

  v->gfp = lame_init();
  if (!v->gfp) {
    goto err;
  }

  v->outbuf = malloc(BUF_SIZE);
  if (!v->outbuf) {
    goto err;
  }

  v->pcm_left = malloc(PCM_BUF_SIZE);
  if (!v->pcm_left) {
    goto err;
  }

  v->stereo = stereo;
  if (stereo) {
    v->pcm_right = malloc(PCM_BUF_SIZE);
    if (!v->pcm_right) {
      goto err;
    }

    lame_set_mode(v->gfp, STEREO);
    lame_set_num_channels(v->gfp, 2);
  } else {
    v->pcm_right = NULL;

    lame_set_mode(v->gfp, MONO);
    lame_set_num_channels(v->gfp, 1);
  }

  lame_set_in_samplerate(v->gfp, rate);
  lame_set_VBR(v->gfp, vbr_default);
  lame_set_VBR_quality(v->gfp, vbr_quality);

  if (lame_init_params(v->gfp) < 0) {
    goto err;
  }

  return v;

err:
  lamejs_free(v);
  return NULL;
}

WASM_EXPORT
void lamejs_print_debug_info(vmsg *v) {
  fprintf(stderr, "__LAMEJS (native)__ %p; pcm_left: %p (%.4f %.4f %.4f...); pcm_right: %p (%.4f %.4f %.4f...);\n",
    v, v->pcm_left, v->pcm_left[0], v->pcm_left[1], v->pcm_left[2]
    , v->pcm_right, v->pcm_right[0], v->pcm_right[1], v->pcm_right[2]);
}

WASM_EXPORT
int lamejs_max_encode_samples() {
  return MAX_SAMPLES;
}

WASM_EXPORT
int lamejs_encode(vmsg *v, int nsamples) {
  if (nsamples > MAX_SAMPLES)
    return -1;

  int n = lame_encode_buffer_ieee_float(v->gfp, v->pcm_left, v->pcm_right, nsamples, v->outbuf, BUF_SIZE);
  return n;
}

WASM_EXPORT
int lamejs_flush(vmsg *v) {
  int n = lame_encode_flush(v->gfp, v->outbuf, BUF_SIZE);
  return n;
}


WASM_EXPORT
int lamejs_tag_frame(vmsg *v) {
  int n = lame_get_lametag_frame(v->gfp, v->outbuf, BUF_SIZE);
  return n;
}

WASM_EXPORT
void lamejs_free(vmsg *v) {
  if (v) {
    lame_close(v->gfp);
    free(v->pcm_left);
    free(v->outbuf);
    free(v);
  }
}

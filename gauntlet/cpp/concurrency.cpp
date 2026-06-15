#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int64_t chunk;
  int64_t rounds;
  int64_t offset;
  int64_t out;
} worker_arg_t;

static int64_t now_us(void) {
  LARGE_INTEGER f, c;
  QueryPerformanceFrequency(&f);
  QueryPerformanceCounter(&c);
  return (int64_t)((c.QuadPart * 1000000LL) / f.QuadPart);
}

DWORD WINAPI worker_fn(LPVOID ptr) {
  worker_arg_t *a = (worker_arg_t *)ptr;
  int64_t *buf = (int64_t *)malloc((size_t)a->chunk * sizeof(int64_t));
  if (!buf) { a->out = -1; return 0; }
  int64_t local = 0;
  for (int64_t r = 0; r < a->rounds; ++r) {
    int64_t base = (r * 17 + a->offset * 13 + 97) % 1000003;
    for (int64_t i = 0; i < a->chunk; ++i) buf[i] = (i * 19 + base + 31) % 1000003;
    local += buf[(r * 31 + a->offset) % a->chunk];
  }
  free(buf);
  a->out = local;
  return 0;
}

int main(void) {
  int64_t seed = 0, workers = 1, chunk = 40000, rounds = 8;
  scanf("%lld", &seed);
  scanf("%lld", &workers);
  scanf("%lld", &chunk);
  scanf("%lld", &rounds);
  if (workers < 1) workers = 1;
  if (workers > 4) workers = 4;
  if (chunk < 1) chunk = 1;
  if (rounds < 1) rounds = 1;

  HANDLE hs[4] = {0};
  worker_arg_t args[4];
  int64_t t0 = now_us();
  for (int t = 0; t < workers; ++t) {
    args[t].chunk = chunk;
    args[t].rounds = rounds;
    args[t].offset = t * 7 + 3;
    args[t].out = 0;
    hs[t] = CreateThread(NULL, 0, worker_fn, &args[t], 0, NULL);
  }
  for (int t = 0; t < workers; ++t) {
    if (hs[t]) { WaitForSingleObject(hs[t], INFINITE); CloseHandle(hs[t]); }
  }
  int64_t t1 = now_us();
  int64_t checksum = (seed % 1000003) + workers + chunk + rounds;
  printf("%lld\n", (long long)checksum);
  printf("%lld\n", (long long)(t1 - t0));
  return 0;
}

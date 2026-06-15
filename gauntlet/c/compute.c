#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

static int64_t now_us(void) {
  LARGE_INTEGER f, c;
  QueryPerformanceFrequency(&f);
  QueryPerformanceCounter(&c);
  return (int64_t)((c.QuadPart * 1000000LL) / f.QuadPart);
}

int main(void) {
  int64_t seed = 0, mode = 0, n = 1, p1 = 0, p2 = 0, p3 = 0, p4 = 2;
  scanf("%lld", &seed);
  scanf("%lld", &mode);
  scanf("%lld", &n);
  scanf("%lld", &p1);
  scanf("%lld", &p2);
  scanf("%lld", &p3);
  scanf("%lld", &p4);
  if (n < 1) n = 1;

  int64_t checksum = 0;
  int64_t t0 = now_us();

  if (mode == 0) {
    int64_t modv = (p4 < 2) ? 2 : p4;
    if (p1 == 0) {
      for (int64_t i = 0; i < n; ++i) checksum += ((i * p2 + p3 + seed) % modv);
    } else if (p1 == 1) {
      for (int64_t i = 0; i < n; ++i) {
        if (((i + seed) % 2) == 0) checksum += ((i * p2 + 7 + seed) % modv);
        else if (((i + seed) % 3) == 0) checksum += ((i * p3 + 11 + seed) % modv);
        else checksum += ((i * (p2 + p3 + 1) + 13 + seed) % modv);
      }
    } else {
      int64_t a = 3, b = 5, c = 7;
      for (int64_t i = 0; i < n; ++i) {
        int64_t nxt = (a * 13 + b * 17 + c * 19 + i + seed + p2) % modv;
        a = b; b = c; c = nxt;
        checksum += nxt;
      }
    }
  } else if (mode == 1) {
    int64_t modv = (p4 < 2) ? 2 : p4;
    int64_t *buf = (int64_t *)malloc((size_t)n * sizeof(int64_t));
    if (!buf) {
      printf("-1\n0\n");
      return 0;
    }
    if (p1 == 0) {
      for (int64_t i = 0; i < n; ++i) buf[i] = (i * p2 + p3 + seed) % modv;
      for (int64_t i = 0; i < n; ++i) checksum += buf[i];
    } else {
      int64_t stride = (p2 < 1) ? 1 : p2;
      int64_t passes = (p3 < 1) ? 1 : p3;
      for (int64_t i = 0; i < n; ++i) buf[i] = (i + seed) % modv;
      for (int64_t p = 0; p < passes; ++p) {
        for (int64_t i = 0; i < n; i += stride) {
          int64_t nxt = (buf[i] * (p4 + 3) + p + seed) % modv;
          buf[i] = nxt;
          checksum += nxt;
        }
      }
    }
    free(buf);
  } else {
    int64_t modv = (p4 < 2) ? 2 : p4;
    if (p1 == 0) {
      for (int64_t i = 0; i < n; ++i) {
        checksum += ((i * 7 + seed) % modv);
        checksum += ((i * 11 + p2) % modv);
      }
    } else if (p1 == 1) {
      int64_t outer = (p2 < 1) ? 1 : p2;
      int64_t inner = (p3 < 1) ? 1 : p3;
      for (int64_t i = 0; i < outer; ++i) {
        int64_t base = (i * 13 + seed) % modv;
        for (int64_t j = 0; j < inner; ++j) checksum += ((base + j * 17) % modv);
      }
    } else {
      int64_t *buf = (int64_t *)malloc((size_t)n * sizeof(int64_t));
      if (!buf) {
        printf("-1\n0\n");
        return 0;
      }
      for (int64_t i = 0; i < n; ++i) buf[i] = (i + seed) % modv;
      for (int64_t p = 0; p < 8; ++p) {
        for (int64_t i = 0; i < n; i += 3) {
          int64_t nxt = (buf[i] * 19 + p + seed) % modv;
          buf[i] = nxt;
          checksum += nxt;
        }
      }
      free(buf);
    }
  }

  int64_t t1 = now_us();
  printf("%lld\n", (long long)checksum);
  printf("%lld\n", (long long)(t1 - t0));
  return 0;
}

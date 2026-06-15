#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int64_t now_us(void) {
  LARGE_INTEGER f, c;
  QueryPerformanceFrequency(&f);
  QueryPerformanceCounter(&c);
  return (int64_t)((c.QuadPart * 1000000LL) / f.QuadPart);
}

int main(void) {
  long long port_ll = 19080, req_ll = 1;
  scanf("%lld", &port_ll);
  scanf("%lld", &req_ll);
  if (req_ll < 1) req_ll = 1;
  int port = (int)port_ll;
  int requests = (int)req_ll;

  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { printf("-1\n0\n"); return 0; }
  SOCKET srv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (srv == INVALID_SOCKET) { WSACleanup(); printf("-1\n0\n"); return 0; }
  int opt = 1;
  setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons((unsigned short)port);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) != 0 || listen(srv, 64) != 0) {
    closesocket(srv); WSACleanup(); printf("-1\n0\n"); return 0;
  }

  printf("READY\n");
  fflush(stdout);

  const char *resp = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nok";
  int64_t handled = 0;
  int64_t req_bytes = 0;
  int64_t t0 = now_us();
  while (handled < requests) {
    SOCKET c = accept(srv, NULL, NULL);
    if (c == INVALID_SOCKET) continue;
    char buf[2048];
    int n = recv(c, buf, (int)sizeof(buf), 0);
    if (n > 0) req_bytes += n;
    send(c, resp, (int)strlen(resp), 0);
    closesocket(c);
    handled += 1;
  }
  int64_t t1 = now_us();
  closesocket(srv);
  WSACleanup();

  printf("%lld\n", (long long)handled);
  printf("%lld\n", (long long)(t1 - t0));
  return 0;
}

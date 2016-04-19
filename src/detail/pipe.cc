#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <utility>

#include "broker/detail/pipe.hh"

namespace broker {
namespace detail {

static void pipe_fail(int eno) {
  char tmp[256];
  strerror_r(eno, tmp, sizeof(tmp));
  fprintf(stderr, "pipe failure: %s\n", tmp);
  abort();
}

static void set_flags(int fd, int flags) {
  if (flags)
    fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | flags);
}

static void set_status_flags(int fd, int flags) {
  if (flags)
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | flags);
}

static int dup_or_fail(int fd, int flags) {
  int rval = dup(fd);
  if (rval < 0)
    pipe_fail(errno);
  set_flags(fd, flags);
  return rval;
}

pipe::pipe(int flags0, int flags1, int status_flags0,
                         int status_flags1) {
  // pipe2 can set flags atomically, but not yet available everywhere.
  if (::pipe(fds))
    pipe_fail(errno);
  flags[0] = flags0;
  flags[1] = flags1;
  set_flags(fds[0], flags[0]);
  set_flags(fds[1], flags[1]);
  set_status_flags(fds[0], status_flags0);
  set_status_flags(fds[1], status_flags1);
}

pipe::~pipe() {
  close();
}

pipe::pipe(const pipe& other) {
  copy(other);
}

pipe::pipe(pipe&& other) {
  steal(std::move(other));
}

pipe& pipe::operator=(const pipe& other) {
  if (this == &other)
    return *this;
  close();
  copy(other);
  return *this;
}

pipe& pipe::operator=(pipe&& other) {
  close();
  steal(std::move(other));
  return *this;
}

void pipe::close() {
  if (fds[0] != -1)
    ::close(fds[0]);
  if (fds[1] != -1)
    ::close(fds[1]);
}

void pipe::copy(const pipe& other) {
  fds[0] = dup_or_fail(other.fds[0], other.flags[0]);
  fds[1] = dup_or_fail(other.fds[1], other.flags[1]);
  flags[0] = other.flags[0];
  flags[1] = other.flags[1];
}

void pipe::steal(pipe&& other) {
  fds[0] = other.fds[0];
  fds[1] = other.fds[1];
  flags[0] = other.flags[0];
  flags[1] = other.flags[1];
  other.fds[0] = other.fds[1] = -1;
}

} // namespace detail
} // namespace broker

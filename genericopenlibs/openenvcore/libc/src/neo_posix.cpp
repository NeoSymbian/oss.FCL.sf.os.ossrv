// SPDX-License-Identifier: Apache-2.0
// NeoSymbian (browser WB2c, docs browser-std.md): POSIX calls Rust's std, mio and the browser's C libraries need that
// P.I.P.S. lacks: poll() over select(), pread() and pwrite() over lseek().

#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <e32def.h>
#include <pthread.h>

// = struct pollfd and the POLL* values of stdapis' poll.h (mw.appsupport), which libc's own headers lack
struct pollfd { int fd; short events; short revents; };
typedef unsigned int nfds_t;
enum { KPollIn = 0x1, KPollPri = 0x2, KPollOut = 0x4, KPollErr = 0x8, KPollNval = 0x20 };

extern "C" {

// shortcut: no POLLHUP/POLLERR from select's exceptfds (end of file and a failed connect show as readable/writable,
// as BSD reports them; callers read 0 or ask SO_ERROR), fds >= FD_SETSIZE are POLLNVAL; a native backend poll if
// a caller needs more.
EXPORT_C int poll(struct pollfd* fds, nfds_t nfds, int timeout)
{
	fd_set r, w;
	FD_ZERO(&r);
	FD_ZERO(&w);
	int maxfd = -1, ready = 0;
	for (nfds_t i = 0; i < nfds; i++)
	{
		struct pollfd& p = fds[i];
		p.revents = 0;
		if (p.fd < 0)
			continue;
		if (p.fd >= FD_SETSIZE || fcntl(p.fd, F_GETFD) < 0)
		{
			p.revents = KPollNval;
			ready++;
			continue;
		}
		if (p.events & (KPollIn | KPollPri))
			FD_SET(p.fd, &r);
		if (p.events & KPollOut)
			FD_SET(p.fd, &w);
		if (p.fd > maxfd)
			maxfd = p.fd;
	}
	if (ready)
		timeout = 0;	// report the bad fds now, with whatever else is ready
	struct timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };
	if (maxfd < 0)
	{
		if (timeout > 0)
		{
			struct timespec ts = { timeout / 1000, (timeout % 1000) * 1000000L };
			nanosleep(&ts, 0);
		}
		return ready;
	}
	int n = select(maxfd + 1, &r, &w, 0, timeout < 0 ? 0 : &tv);
	if (n < 0)
		return -1;
	for (nfds_t i = 0; i < nfds; i++)
	{
		struct pollfd& p = fds[i];
		if (p.fd < 0 || p.revents)
			continue;
		if (FD_ISSET(p.fd, &r))
			p.revents |= p.events & (KPollIn | KPollPri);
		if (FD_ISSET(p.fd, &w))
			p.revents |= KPollOut;
		if (p.revents)
			ready++;
	}
	return ready;
}

// One lock for every positioned call: the seek, the transfer and the seek back can't interleave with another
// thread's pread/pwrite on the same fd (US1: 8 threads on one fd read each other's data).
// shortcut: plain read/write/lseek on the same fd from another thread still see the moved offset (P.I.P.S. has no
// positioned I/O in its backend); move into the backend's file descriptors (RFile::Read/Write at a position) if a
// caller mixes them.
static pthread_mutex_t PositionedLock = PTHREAD_MUTEX_INITIALIZER;

static ssize_t Positioned(int fd, void* buf, size_t n, off_t off, int write)
{
	pthread_mutex_lock(&PositionedLock);
	ssize_t k = -1;
	off_t cur = lseek(fd, 0, SEEK_CUR);
	if (cur >= 0 && lseek(fd, off, SEEK_SET) >= 0)
	{
		k = write ? ::write(fd, buf, n) : ::read(fd, buf, n);
		int e = errno;
		lseek(fd, cur, SEEK_SET);
		errno = e;
	}
	pthread_mutex_unlock(&PositionedLock);
	return k;
}

EXPORT_C ssize_t pread(int fd, void* buf, size_t n, off_t off)
{
	return Positioned(fd, buf, n, off, 0);
}

EXPORT_C ssize_t pwrite(int fd, const void* buf, size_t n, off_t off)
{
	return Positioned(fd, const_cast<void*>(buf), n, off, 1);
}

} // extern "C"

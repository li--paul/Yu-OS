#include <inc/lib.h>

const char *msg = "This is the NEW message of the day!\n\n";

#define FVA ((struct Fd*)0xCCCCC000)

static int
xopen(const char *path, int mode)
{
	extern union Fsipc fsipcbuf;
	envid_t fsenv;

	strcpy(fsipcbuf.open.req_path, path);
	fsipcbuf.open.req_omode = mode;

	fsenv = ipc_find_env(ENV_TYPE_FS);
	ipc_send(fsenv, FSREQ_OPEN, &fsipcbuf, PTE_P | PTE_W | PTE_U);
	return ipc_recv(NULL, FVA, NULL);
}

void
umain(int argc, char **argv)
{
	int r, f, i;
	struct Stat st;
	struct Fd fdcopy;
	char buf[512];

	// We open files manually first, to avoid the FD layer
	if ((r = xopen("/not-found", O_RDONLY)) < 0 && r != -E_NOT_FOUND) {
		panic("serve_open /not-found failed: %e", r);
	} else if (r >= 0) {
		panic("serve_open /not-found succeeded!\n");
	}

	if ((r = xopen("/newmotd", O_RDONLY)) < 0) {
		panic("serve_open /newmotd failed: %e", r);
	}
	if (FVA->fd_dev_id != 'f' || FVA->fd_offset != 0 || FVA->fd_omode != O_RDONLY) {
		panic("serve_open did not fill struct Fd correctly\n");
	}
	cprintf("serve_open is good\n");

	if ((r = devfile.dev_stat(FVA, &st)) < 0) {
		panic("file_stat failed: %e", r);
	}
	if (strlen(msg) != st.st_size) {
		panic("file_stat returned size %d wanted %d\n", st.st_size, strlen(msg));
	}
	cprintf("file_stat is good\n");

	memset(buf, 0, sizeof(buf));
	if ((r = devfile.dev_read(FVA, buf, sizeof(buf))) < 0) {
		panic("file_read failed: %e", r);
	}
	if (strcmp(buf, msg) != 0) {
		panic("file_read returned wrong data");
	}
	cprintf("file_read is good\n");

	if ((r = devfile.dev_close(FVA)) < 0) {
		panic("file_close failed: %e", r);
	}
	cprintf("file_close is good\n");

	// We're about to unmap the FD, but still need a way to get
	// the stale filenum to serve_read, so we make a local copy.
	// The file server won't think it's stale until we unmap the
	// FD page.
	fdcopy = *FVA;
	sys_page_unmap(0, FVA);

	if ((r = devfile.dev_read(&fdcopy, buf, sizeof(buf))) != -E_INVAL) {
		panic("serve_read does not handle stale fileids correctly: %e", r);
	}
	cprintf("stale fileid is good\n");

	// Try writing
	if ((r = xopen("/new-file", O_RDWR|O_CREAT)) < 0) {
		panic("serve_open /new-file: %e", r);
	}

	if ((r = devfile.dev_write(FVA, msg, strlen(msg))) != strlen(msg)) {
		panic("file_write: %e", r);
	}
	cprintf("file_write is good\n");

	FVA->fd_offset = 0;
	memset(buf, 0, sizeof(buf));
	if ((r = devfile.dev_read(FVA, buf, sizeof(buf))) < 0) {
		panic("file_read after file_write failed: %e", r);
	}
	if (r != strlen(msg)) {
		panic("file_read after file_write returned wrong length: %d", r);
	}
	if (strcmp(buf, msg) != 0) {
		panic("file_read after file_write returned wrong data");
	}
	cprintf("file_read after file_write is good\n");

}

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
}
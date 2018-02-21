#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#if defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <unistd.h>
#endif // defined(__APPLE__)

#if defined(_WIN32)
#include <Windows.h>
#include <sys/types.h>
#endif // defined(_WIN32)

#include "gecko_vr.h"

using namespace mozilla::gfx;

volatile VRExternalShmem* api_shmem = NULL;

void OpenShmem();
void CloseShmem();

bool gecko_vr_init()
{
  OpenShmem();
  if (api_shmem == NULL) {
  	return false;
  }
  return true;
}

void gecko_vr_shutdown()
{
  CloseShmem();
}

#if defined(__APPLE__)
// This must be less that 31 characters
static const char* kShmemName = "/moz.gecko.vr_ext.0.0.1";
int api_shmem_fd = 0;
#elif defined(_WIN32)
static const char* kShmemName = "moz.gecko.vr_ext.0.0.1";
HANDLE api_shmem_file = NULL;
#endif

void OpenShmem()
{
  if (api_shmem) {
    return;
  }
  off_t length = sizeof(VRExternalShmem);
#if defined(__APPLE__)
  fprintf(stderr, "Creating Shmem...\n");
  shm_unlink(kShmemName);
  if (api_shmem_fd == 0) {
    api_shmem_fd = shm_open(kShmemName, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
  }
  if (api_shmem_fd <= 0) {
    fprintf(stderr, "shm_open error: %i\n", errno); // findme! kip! hack!  Need to properly log this...
    api_shmem_fd = 0;
    return;
  }
  int r = ftruncate(api_shmem_fd, length);
  if (r == -1) {
    fprintf(stderr, "ftruncate error: %i\n", errno); // findme! kip! hack!  Need to properly log this...
    CloseShmem();
    return;
  }
  api_shmem = (VRExternalShmem*)mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, api_shmem_fd, 0);
  if (api_shmem == MAP_FAILED) {
    api_shmem = NULL;
    CloseShmem();
    return;
  }
  fprintf(stderr, "Created Shmem\n");

#elif defined(_WIN32)
  if (api_shmem_file == NULL) {
    api_shmem_file = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 8192 /*length*/, kShmemName);
    if (api_shmem_file == NULL) {
      DWORD err = GetLastError();
      fprintf(stderr, "Could not open file mapping object (%d).\n", err);
      CloseShmem();
      return;
    }
  }

  api_shmem = (VRExternalShmem*)MapViewOfFile(api_shmem_file, // handle to map object
    FILE_MAP_ALL_ACCESS,  // read/write permission
    0,
    0,
    length);

  if (api_shmem == NULL) {
    fprintf(stderr, "Could not map view of file (%d).\n",
      GetLastError());
    CloseShmem();
    return;
  }
#endif
}

void
CloseShmem()
{
#if defined(__APPLE__)
  fprintf(stderr, "Closing Shmem...\n");
  shm_unlink(kShmemName);
  if (api_shmem) {
    munmap((void *)api_shmem, sizeof(VRExternalShmem));
    api_shmem = NULL;
  }
  if (api_shmem_fd) {
    close(api_shmem_fd);
  }
  api_shmem_fd = 0;
  fprintf(stderr, "Closed Shmem\n");
#elif defined(_WIN32)
  if (api_shmem) {
    UnmapViewOfFile((LPCVOID)api_shmem);
    api_shmem = NULL;
  }
  if (api_shmem_file) {
    CloseHandle(api_shmem_file);
    api_shmem_file = NULL;
  }
#else
#error Platform not supported
#endif
}

void
gecko_vr_push_state(const mozilla::gfx::VRSystemState& aState)
{
  // FINDME!! TODO!! HACK!! Need proper synchronization
  // or memory guard to preserve write order
  api_shmem->generationA++;
  memcpy((void *)&api_shmem->state, &aState, sizeof(VRSystemState));
  api_shmem->generationB++;
}

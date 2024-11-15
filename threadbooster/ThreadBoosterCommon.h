#define THREAD_BOOSTER_DEVICE 0x8000
#define IOCTL_THREAD_BOOSTER_SET_PRIORITY CTL_CODE(THREAD_BOOSTER_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)

typedef struct ThreadData
{
	ULONG ThreadId;
	int Priority;
} THREAD_DATA, *PTHREAD_DATA;
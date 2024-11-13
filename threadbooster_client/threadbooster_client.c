#include <stdio.h>
#include <Windows.h>
#include "..\threadbooster\ThreadBoosterCommon.h"

int Error(const char* message)
{
    printf("[!] %s | CODE %ld\n", message, GetLastError());
    return 1;
}

int main(int argc, const char* argv[])
{
    if (argc < 3)
    {
        printf("Usage: threadbooster_client.exe <ThreadId> <Priority>\n");
        exit(1);
    }

    // Setup IO struct
    THREAD_DATA threadData;
    threadData.ThreadId = atoi(argv[1]);
    threadData.Priority = atoi(argv[2]);

    // Open handle to symlink device
    HANDLE hDevice = NULL;
    hDevice = CreateFile(L"\\\\.\\ThreadBooster", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == NULL)
    {
        Error("Failed to get device handle.");
        exit(1);
    }

    // Communicate to driver
    DWORD returned;
    BOOL success = DeviceIoControl(
        hDevice,
        IOCTL_THREAD_BOOSTER_SET_PRIORITY,
        &threadData,
        sizeof(THREAD_DATA),
        NULL,
        0,
        &returned,
        NULL
    );
    if (success)
    {
        printf("[+] Updated thread priority!\n");
    }
    else {
        Error("Failed to update priority. Driver communication error.");
    }
    return 0;
}
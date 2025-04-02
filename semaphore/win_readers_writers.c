#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <process.h>
#include <signal.h>
 
#define WRITERS_CNT 4
#define READERS_CNT 5
 
HANDLE can_read, can_write, mutex;
 
DWORD active_readers = 0,
      active_writer = 0,
      writers_queue = 0,
      readers_queue = 0;
 
char shared_letter = 'z';
 
int flag =1;
 
void start_read()
{
    InterlockedIncrement(&readers_queue);
    if(active_writer || (WaitForSingleObject(can_write, 0) == WAIT_OBJECT_0 && writers_queue))
        WaitForSingleObject(can_read, INFINITE);
    WaitForSingleObject(mutex, INFINITE);
    InterlockedDecrement(&readers_queue);
    InterlockedIncrement(&active_readers);
    SetEvent(can_read);
    ReleaseMutex(mutex);
}
 
void stop_read()
{
    InterlockedDecrement(&active_readers);
    if(!active_readers)
    {
        ResetEvent(can_read);
        SetEvent(can_write);
    }
}
 
void start_write()
{
    InterlockedIncrement(&writers_queue);
    if(active_readers || active_writer)
        WaitForSingleObject(can_write, INFINITE);
    InterlockedDecrement(&writers_queue);
    InterlockedIncrement(&active_writer);
}
 
void stop_write()
{
    InterlockedDecrement(&active_writer);
    if(readers_queue > 0)
        SetEvent(can_read);
    else
        SetEvent(can_write);
}
 
PVOID WINAPI writer(const LPVOID param)
{
    srand(GetCurrentThreadId());
    while(flag)
    {
        Sleep((rand()%3)*1000);
        start_write();
        shared_letter = (shared_letter == 'z' ? shared_letter = 'a' : ++shared_letter);
        printf("Writer %d write %c\n", GetCurrentThreadId(), shared_letter);
        stop_write();
    }
    return 0;
}
 
PVOID WINAPI reader(CONST LPVOID param)
{
    srand(GetCurrentThreadId());
    while(flag)
    {
        Sleep((rand()%2)*1000);
        start_read();
        printf("Reader %d read %c\n", GetCurrentThreadId(), shared_letter);
        stop_read();
    }
    return 0;
}
 
void sigint_handler(int sig_num)
{
    flag = 0;
    printf("Thread %d received signal %d\n", GetCurrentThreadId(), sig_num);
}
 
int main()
{
    if(signal(SIGINT, sigint_handler)==SIG_ERR)
    {
        perror("Error: set new SIGINT handler error");
        exit(1);
    }
    HANDLE p_threads[READERS_CNT+WRITERS_CNT];
    if((can_read = CreateEvent(NULL, 0, 0, NULL))==NULL)
    {
        perror("Error: read event create error\n");
        exit(1);
    }
    if((can_write=CreateEvent(NULL, 1, 0, NULL)) == NULL)
    {
 
        perror("Error: write event create error\n");
        exit(1);
    }
    if((mutex = CreateMutex(NULL, 0, NULL)) == NULL)
    {
        perror("Error: create mutex error");
        exit(1);
    }
 
    for(int i = 0; i < WRITERS_CNT; ++i)
    {
        p_threads[i] = _beginthreadex(NULL, 0, writer, NULL, 0, NULL);
        if (p_threads[i]==NULL)
        {
            perror("Error: thread create error");
            exit(1);
        }
    }
    for(int i = 0; i < READERS_CNT; ++i)
    {
        p_threads[WRITERS_CNT + i] = _beginthreadex(NULL, 0, reader, NULL, 0, NULL);
        if (p_threads[WRITERS_CNT + i]==NULL)
        {
            perror("Error: thread create error");
            exit(1);
        }
    }
    int running_threads_cnt=WRITERS_CNT+READERS_CNT;
    while(running_threads_cnt)
    {
        DWORD id = WaitForMultipleObjects(WRITERS_CNT+READERS_CNT, p_threads, 0, INFINITE);
        if (id == WAIT_FAILED)
        {
            perror("Error: wait error");
            exit(1);
        }
        else if(id == WAIT_TIMEOUT)
        {
            continue;
        }
        else{
            printf("Thread %d ended\n", id - WAIT_OBJECT_0);
            --running_threads_cnt;
        }
    }
    CloseHandle(can_read);
    CloseHandle(can_write);
    CloseHandle(mutex);
    return 0;
}

#pragma once

#include <windows.h>

typedef void (*JobFunc)(int, void*);

struct JobSystem {
    HANDLE startSignal;      // Manual-reset event to wake all threads
    HANDLE completionSignal; // Manual-reset event to signal the main thread

    JobFunc workerFunc;      // The function to call (your ray tracer)
    void* sharedData;        // Shared data (scene, buffer)

    alignas(16) volatile LONG nextJobIndex;   // The "Atomic Counter"
    volatile LONG activeJobs;      // Tracks how many jobs are left
    int totalJobs;
    bool quit = false;

    void Initialize(int numThreads) {
        startSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
        completionSignal = CreateEvent(NULL, TRUE, TRUE, NULL);

        for (int i = 0; i < numThreads; ++i) {
            CreateThread(NULL, 0, StaticWorkerEntry, this, 0, NULL);
        }
    }

    // Call this in your Tick/Draw loop
    void Execute(JobFunc func, void* data, int jobCount) {
        workerFunc = func;
        sharedData = data;
        totalJobs = jobCount;
        nextJobIndex = 0;
        activeJobs = jobCount;

        ResetEvent(completionSignal);
        SetEvent(startSignal); // Release all 64 threads simultaneously

        // Wait for threads to finish all jobs
        WaitForSingleObject(completionSignal, INFINITE);
        ResetEvent(startSignal); // Put threads back to sleep
    }

    static DWORD WINAPI StaticWorkerEntry(LPVOID param) {
        JobSystem* js = (JobSystem*)param;
        while (!js->quit) {
            // Sleep until work is available
            WaitForSingleObject(js->startSignal, INFINITE);
            if (js->quit) break;

            while (true) {
                // LOCK-FREE: Atomically grab the next index and increment
                int jobIdx = InterlockedIncrement(&js->nextJobIndex) - 1;

                if (jobIdx >= js->totalJobs) break;

                // Do the actual work
                js->workerFunc(jobIdx, js->sharedData);

                // Atomically decrement remaining jobs
                if (InterlockedDecrement(&js->activeJobs) == 0) {
                    SetEvent(js->completionSignal);
                }
            }
        }
        return 0;
    }
};
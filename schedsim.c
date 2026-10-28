// C program for implementation of Scheduling Simulation
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "process.h"
#include "util.h"

/*
 * First Come First Serve (FCFS)
 */
void findWaitingTime(ProcessType plist[], int n)
{
    if (n <= 0)
        return;

    int current_time = 0;

    for (int i = 0; i < n; i++) {
        if (current_time < plist[i].art)
            current_time = plist[i].art;

        plist[i].wt = current_time - plist[i].art;
        current_time += plist[i].bt;
    }
}

/*
 * Shortest Job First (SJF) / Shortest Remaining Time First
 */
void findWaitingTimeSJF(ProcessType plist[], int n)
{
    if (n <= 0)
        return;

    int *remaining = malloc((size_t)n * sizeof(int));

    if (remaining == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        remaining[i] = plist[i].bt;
        plist[i].wt = 0;
    }

    int completed = 0;
    int current_time = 0;

    while (completed < n) {
        int shortest = -1;
        int shortest_remaining = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (remaining[i] > 0 &&
                plist[i].art <= current_time &&
                remaining[i] < shortest_remaining) {
                shortest_remaining = remaining[i];
                shortest = i;
            }
        }

        if (shortest == -1) {
            current_time++;
            continue;
        }

        remaining[shortest]--;
        current_time++;

        if (remaining[shortest] == 0) {
            int waiting = current_time
                        - plist[shortest].art
                        - plist[shortest].bt;

            plist[shortest].wt = waiting > 0 ? waiting : 0;
            completed++;
        }
    }

    free(remaining);
}

/*
 * Round Robin
 */
void findWaitingTimeRR(ProcessType plist[], int n, int quantum)
{
    int *remaining = malloc(n * sizeof(int));
    int *queue = malloc(n * sizeof(int));
    int *in_queue = calloc(n, sizeof(int));

    if (remaining == NULL || queue == NULL || in_queue == NULL)
    {
        free(remaining);
        free(queue);
        free(in_queue);
        return;
    }

    for (int i = 0; i < n; i++)
    {
        remaining[i] = plist[i].bt;
        plist[i].wt = 0;
    }

    int head = 0;
    int tail = 0;
    int count = 0;
    int completed = 0;
    int time = 0;

    while (completed < n)
    {
        /* Add processes that have arrived */
        for (int i = 0; i < n; i++)
        {
            if (remaining[i] > 0 &&
                !in_queue[i] &&
                plist[i].art <= time)
            {
                queue[tail] = i;
                tail = (tail + 1) % n;
                count++;
                in_queue[i] = 1;
            }
        }

        /* Advance time if no process is ready */
        if (count == 0)
        {
            time++;
            continue;
        }

        /* Select the next ready process */
        int index = queue[head];
        head = (head + 1) % n;
        count--;
        in_queue[index] = 0;

        int run_time = remaining[index] < quantum
                           ? remaining[index]
                           : quantum;

        /*
         * Execute one time unit at a time so processes
         * arriving during the quantum enter the ready queue.
         */
        for (int t = 0; t < run_time; t++)
        {
            remaining[index]--;
            time++;

            for (int i = 0; i < n; i++)
            {
                if (i != index &&
                    remaining[i] > 0 &&
                    !in_queue[i] &&
                    plist[i].art <= time)
                {
                    queue[tail] = i;
                    tail = (tail + 1) % n;
                    count++;
                    in_queue[i] = 1;
                }
            }
        }

        /* Requeue the running process only if it has work left */
        if (remaining[index] > 0)
        {
            queue[tail] = index;
            tail = (tail + 1) % n;
            count++;
            in_queue[index] = 1;
        }
        else
        {
            plist[index].wt =
                time - plist[index].art - plist[index].bt;

            if (plist[index].wt < 0)
                plist[index].wt = 0;

            completed++;
        }
    }

    free(remaining);
    free(queue);
    free(in_queue);
}

/*
 * Calculate turnaround time.
 */
void findTurnAroundTime(ProcessType plist[], int n)
{
    for (int i = 0; i < n; i++)
        plist[i].tat = plist[i].bt + plist[i].wt;
}

/*
 * Priority comparator: higher numerical priority first.
 */
int my_comparer(const void *a, const void *b)
{
    const ProcessType *p1 = (const ProcessType *)a;
    const ProcessType *p2 = (const ProcessType *)b;

    /* Higher priority first */
    if (p1->pri != p2->pri)
        return p2->pri - p1->pri;

    /* Earlier arrival first when priorities are equal */
    if (p1->art != p2->art)
        return p1->art - p2->art;

    /* Lower PID first when priority and arrival are equal */
    return p1->pid - p2->pid;
}

/*
 * FCFS results.
 */
void findavgTimeFCFS(ProcessType plist[], int n)
{
    findWaitingTime(plist, n);
    findTurnAroundTime(plist, n);

    printf("\n*********\nFCFS\n");
}

/*
 * SJF results.
 */
void findavgTimeSJF(ProcessType plist[], int n)
{
    findWaitingTimeSJF(plist, n);
    findTurnAroundTime(plist, n);

    printf("\n*********\nSJF\n");
}

/*
 * Round Robin results.
 */
void findavgTimeRR(ProcessType plist[], int n, int quantum)
{
    findWaitingTimeRR(plist, n, quantum);
    findTurnAroundTime(plist, n);

    printf("\n*********\nRR Quantum = %d\n", quantum);
}

/*
 * Priority scheduling results.
 */
void findavgTimePriority(ProcessType plist[], int n)
{
    qsort(plist, (size_t)n, sizeof(ProcessType), my_comparer);

    findWaitingTime(plist, n);
    findTurnAroundTime(plist, n);

    printf("\n*********\nPriority\n");
}

/*
 * Print process metrics and averages.
 */
void printMetrics(ProcessType plist[], int n)
{
    if (n <= 0) {
        printf("No processes to schedule.\n");
        return;
    }

    int total_wt = 0;
    int total_tat = 0;

    printf("\tProcesses\tBurst time\tWaiting time\tTurn around time\n");

    for (int i = 0; i < n; i++) {
        total_wt += plist[i].wt;
        total_tat += plist[i].tat;

        printf("\t%d\t\t%d\t\t%d\t\t%d\n",
               plist[i].pid,
               plist[i].bt,
               plist[i].wt,
               plist[i].tat);
    }

    printf("\nAverage waiting time = %.2f",
           (double)total_wt / n);

    printf("\nAverage turn around time = %.2f\n",
           (double)total_tat / n);
}

/*
 * Read process data from the input file.
 */
ProcessType *initProc(char *filename, int *n)
{
    FILE *input_file = fopen(filename, "r");

    if (input_file == NULL) {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    ProcessType *plist = parse_file(input_file, n);

    fclose(input_file);

    if (plist == NULL) {
        fprintf(stderr, "Error: Could not read process data.\n");
        exit(EXIT_FAILURE);
    }

    return plist;
}

/*
 * Main program.
 */
int main(int argc, char *argv[])
{
    int n;
    const int quantum = 2;
    ProcessType *proc_list;

    if (argc < 2) {
        fprintf(stderr, "Usage: ./schedsim <input-file-path>\n");
        return EXIT_FAILURE;
    }

    /* FCFS */
    n = 0;
    proc_list = initProc(argv[1], &n);
    findavgTimeFCFS(proc_list, n);
    printMetrics(proc_list, n);
    free(proc_list);

    /* SJF */
    n = 0;
    proc_list = initProc(argv[1], &n);
    findavgTimeSJF(proc_list, n);
    printMetrics(proc_list, n);
    free(proc_list);

    /* Priority */
    n = 0;
    proc_list = initProc(argv[1], &n);
    findavgTimePriority(proc_list, n);
    printMetrics(proc_list, n);
    free(proc_list);

    /* Round Robin */
    n = 0;
    proc_list = initProc(argv[1], &n);
    findavgTimeRR(proc_list, n, quantum);
    printMetrics(proc_list, n);
    free(proc_list);

    return EXIT_SUCCESS;
}

/* ============================================================
 * Sorting Algorithms: implementation, measurement, comparison
 * Author : Abdelmola Abdallah
 * Language: C (gcc)
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define RUNS            3       /* each measurement is repeated 3 times (average)   */
#define SEED            42      /* fixed seed: same data at every execution         */
#define MAX_SIZE        100000  /* biggest array the program can handle             */
#define BIG_SIZE        100000  /* size used in the "big array" example             */
#define SLOW_LIMIT      50000   /* Bubble/Selection/Insertion are not run above it  */
#define BIG_SLOW_SIZE   10000   /* size used for these 3 sorts in the big example   */

/* The same original data is copied into workArray before every sort */
int originalArray[MAX_SIZE];
int workArray[MAX_SIZE];

const char *algoNames[5] = {"Bubble Sort", "Selection Sort", "Insertion Sort",
                            "Merge Sort", "Quick Sort"};
const char *typeNames[4] = {"random", "sorted", "reverse", "duplicates"};

/* ------------------------------------------------------------
 * Memory tracking: the only extra memory used by the sorts is
 * the temporary array of Merge Sort. We count every byte asked
 * with trackedMalloc and keep the highest value (the peak).
 * ------------------------------------------------------------ */
size_t currentBytes = 0;
size_t peakBytes = 0;

int *trackedMalloc(int count)
{
    int *p = malloc((size_t)count * sizeof(int));
    if (p == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    currentBytes += (size_t)count * sizeof(int);
    if (currentBytes > peakBytes) {
        peakBytes = currentBytes;
    }
    return p;
}

void trackedFree(int *p, int count)
{
    currentBytes -= (size_t)count * sizeof(int);
    free(p);
}

/* ============================================================
 * 1. THE FIVE SORTING ALGORITHMS (no menu, no display, no timing)
 * ============================================================ */

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

/* Bubble Sort: swap neighbours; stop if a full pass made no swap */
void bubbleSort(int array[], int size)
{
    for (int pass = 0; pass < size - 1; pass++) {
        bool swapped = false;
        for (int j = 0; j < size - 1 - pass; j++) {
            if (array[j] > array[j + 1]) {
                swap(&array[j], &array[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) {
            break;                    /* already sorted */
        }
    }
}

/* Selection Sort: put the minimum of the unsorted part in place */
void selectionSort(int array[], int size)
{
    for (int i = 0; i < size - 1; i++) {
        int minIndex = i;
        for (int j = i + 1; j < size; j++) {
            if (array[j] < array[minIndex]) {
                minIndex = j;
            }
        }
        if (minIndex != i) {
            swap(&array[i], &array[minIndex]);
        }
    }
}

/* Insertion Sort: insert each element in the sorted left part */
void insertionSort(int array[], int size)
{
    for (int i = 1; i < size; i++) {
        int key = array[i];
        int j = i - 1;
        while (j >= 0 && array[j] > key) {
            array[j + 1] = array[j];  /* shift to the right */
            j--;
        }
        array[j + 1] = key;
    }
}

/* Merge Sort: merge the two sorted halves [left..middle] and [middle+1..right] */
void merge(int array[], int temp[], int left, int middle, int right)
{
    int i = left;
    int j = middle + 1;
    int k = left;

    while (i <= middle && j <= right) {
        if (array[i] <= array[j]) {
            temp[k++] = array[i++];
        } else {
            temp[k++] = array[j++];
        }
    }
    while (i <= middle) {
        temp[k++] = array[i++];
    }
    while (j <= right) {
        temp[k++] = array[j++];
    }
    for (k = left; k <= right; k++) {
        array[k] = temp[k];           /* copy back */
    }
}

void mergeSortRec(int array[], int temp[], int left, int right)
{
    if (left >= right) {
        return;                       /* 0 or 1 element: already sorted */
    }
    int middle = left + (right - left) / 2;
    mergeSortRec(array, temp, left, middle);
    mergeSortRec(array, temp, middle + 1, right);
    merge(array, temp, left, middle, right);
}

void mergeSort(int array[], int size)
{
    int *temp = trackedMalloc(size);  /* one temporary array for the whole sort */
    mergeSortRec(array, temp, 0, size - 1);
    trackedFree(temp, size);
}

/* Quick Sort. Pivot strategy: the MIDDLE element (moved to the end).
 * Choosing the middle avoids the worst case on sorted and reverse-sorted data.
 * To keep the recursion depth small, we call the function on the smaller part
 * and loop on the bigger part (depth is at most about log2(n)). */
int partition(int array[], int low, int high)
{
    int middle = low + (high - low) / 2;
    swap(&array[middle], &array[high]);
    int pivot = array[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (array[j] <= pivot) {
            i++;
            swap(&array[i], &array[j]);
        }
    }
    swap(&array[i + 1], &array[high]);
    return i + 1;                     /* final position of the pivot */
}

void quickSortRec(int array[], int low, int high)
{
    while (low < high) {
        int p = partition(array, low, high);
        if (p - low < high - p) {
            quickSortRec(array, low, p - 1);
            low = p + 1;
        } else {
            quickSortRec(array, p + 1, high);
            high = p - 1;
        }
    }
}

void quickSort(int array[], int size)
{
    quickSortRec(array, 0, size - 1);
}

/* Call the algorithm number 0..4 */
void runSort(int algo, int array[], int size)
{
    switch (algo) {
        case 0: bubbleSort(array, size);    break;
        case 1: selectionSort(array, size); break;
        case 2: insertionSort(array, size); break;
        case 3: mergeSort(array, size);     break;
        case 4: quickSort(array, size);     break;
    }
}

/* ============================================================
 * 2. UTILITIES: data generation, checking, display
 * ============================================================ */

/* rand() only goes up to 32767 on Windows, so we combine two calls */
int randomValue(void)
{
    return (int)(((long)rand() * 32768L + rand()) % 1000000L);
}

/* Fill originalArray with data of the chosen type (same seed every time) */
void generateData(int type, int size)
{
    srand(SEED);
    for (int i = 0; i < size; i++) {
        if (type == 0) {
            originalArray[i] = randomValue();        /* random            */
        } else if (type == 1) {
            originalArray[i] = i;                    /* already sorted    */
        } else if (type == 2) {
            originalArray[i] = size - i;             /* reverse sorted    */
        } else {
            originalArray[i] = rand() % 100;         /* many duplicates   */
        }
    }
}

bool isSorted(const int array[], int size)
{
    for (int i = 1; i < size; i++) {
        if (array[i - 1] > array[i]) {
            return false;
        }
    }
    return true;
}

void printArray(const int array[], int size)
{
    printf("[ ");
    for (int i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    printf("]\n");
}

/* Read an integer between min and max. Asks again if the input is invalid. */
int readInt(const char *message, int min, int max)
{
    char line[100];
    char extra;
    int value;

    while (true) {
        printf("%s", message);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            exit(0);                  /* end of input */
        }
        if (sscanf(line, "%d %c", &value, &extra) == 1 && value >= min && value <= max) {
            return value;
        }
        printf("Invalid input. Enter a number between %d and %d.\n", min, max);
    }
}

int askSize(void)
{
    const int sizes[5] = {1000, 5000, 10000, 50000, 100000};

    printf("\nArray size:\n");
    printf("1. 1 000   2. 5 000   3. 10 000   4. 50 000   5. 100 000   6. Other\n");
    int choice = readInt("Your choice: ", 1, 6);
    if (choice == 6) {
        return readInt("Enter the size (1 to 100000): ", 1, MAX_SIZE);
    }
    return sizes[choice - 1];
}

int askType(void)
{
    printf("\nType of data:\n");
    printf("1. Random   2. Already sorted   3. Reverse sorted   4. Many duplicates\n");
    return readInt("Your choice: ", 1, 4) - 1;
}

/* ============================================================
 * 3. MEASUREMENT (time with clock(), memory with the tracker)
 * ============================================================ */

typedef struct {
    double timeMs;       /* average time of the sort, in milliseconds */
    double memoryKB;     /* peak extra memory, in KB                  */
    bool   sorted;       /* is the result really sorted?              */
} Result;

/* Sort a COPY of originalArray RUNS times. Only the sort is timed. */
Result measure(int algo, int size)
{
    Result r;
    double totalMs = 0.0;
    r.memoryKB = 0.0;
    r.sorted = true;

    for (int run = 0; run < RUNS; run++) {
        memcpy(workArray, originalArray, (size_t)size * sizeof(int));  /* not timed */
        currentBytes = 0;
        peakBytes = 0;

        clock_t start = clock();
        runSort(algo, workArray, size);
        clock_t end = clock();

        totalMs += 1000.0 * (double)(end - start) / CLOCKS_PER_SEC;
        if ((double)peakBytes / 1024.0 > r.memoryKB) {
            r.memoryKB = (double)peakBytes / 1024.0;
        }
        if (!isSorted(workArray, size)) {
            r.sorted = false;
        }
    }
    r.timeMs = totalMs / RUNS;
    return r;
}

/* ============================================================
 * 4. MENU OPTIONS
 * ============================================================ */

/* Options 1 to 5: one algorithm */
void runOneAlgorithm(int algo)
{
    int size = askSize();
    int type = askType();

    generateData(type, size);
    printf("\n%s | size = %d | data = %s\n", algoNames[algo], size, typeNames[type]);
    if (algo < 3 && size > SLOW_LIMIT) {
        printf("This algorithm is slow on a big array, please wait...\n");
    }
    if (size <= 20) {
        printf("Before: ");
        printArray(originalArray, size);
    }

    Result r = measure(algo, size);

    if (size <= 20) {
        printf("After : ");
        printArray(workArray, size);
    }
    printf("Average time (%d runs) : %.3f ms\n", RUNS, r.timeMs);
    printf("Extra memory (peak)    : %.2f KB\n", r.memoryKB);
    printf("Result sorted          : %s\n", r.sorted ? "YES" : "NO (error!)");
}

/* Option 6: all five algorithms, five sizes, results in a table + CSV file */
void compareAll(void)
{
    const int sizes[5] = {1000, 5000, 10000, 50000, 100000};
    int type = askType();

    FILE *csv = fopen("comparison_results.csv", "w");
    if (csv != NULL) {
        fprintf(csv, "algorithm,data_type,size,time_ms,memory_kb,rank\n");
    }

    printf("\nData: %s | seed %d | %d runs per measurement\n", typeNames[type], SEED, RUNS);
    printf("Bubble, Selection and Insertion are skipped above %d elements (too slow).\n", SLOW_LIMIT);
    printf("Please wait, this can take about one minute...\n");

    for (int s = 0; s < 5; s++) {
        int size = sizes[s];
        Result res[5];
        bool skipped[5];

        generateData(type, size);
        for (int a = 0; a < 5; a++) {
            skipped[a] = (a < 3 && size > SLOW_LIMIT);
            if (!skipped[a]) {
                res[a] = measure(a, size);
            }
        }

        printf("\n%-16s %10s %12s %16s %6s\n",
               "Algorithm", "Size", "Time (ms)", "Memory (KB)", "Rank");
        for (int a = 0; a < 5; a++) {
            if (skipped[a]) {
                printf("%-16s %10d %12s %16s %6s\n", algoNames[a], size, "skipped", "-", "-");
                continue;
            }
            int rank = 1;                         /* rank = 1 + number of faster algorithms */
            for (int b = 0; b < 5; b++) {
                if (!skipped[b] && res[b].timeMs < res[a].timeMs) {
                    rank++;
                }
            }
            printf("%-16s %10d %12.3f %16.2f %6d\n",
                   algoNames[a], size, res[a].timeMs, res[a].memoryKB, rank);
            if (csv != NULL) {
                fprintf(csv, "%s,%s,%d,%.3f,%.2f,%d\n", algoNames[a], typeNames[type],
                        size, res[a].timeMs, res[a].memoryKB, rank);
            }
        }
    }

    if (csv != NULL) {
        fclose(csv);
        printf("\nResults saved in comparison_results.csv\n");
    }
}

/* Option 7: big array example */
void bigArrayExample(void)
{
    generateData(0, BIG_SIZE);
    printf("\nBig array: %d random integers, seed = %d\n", BIG_SIZE, SEED);
    printf("Bubble, Selection and Insertion are too slow here: they use %d elements.\n\n", BIG_SLOW_SIZE);
    printf("%-16s %10s %12s %16s %8s\n", "Algorithm", "Size", "Time (ms)", "Memory (KB)", "Sorted");

    for (int a = 0; a < 5; a++) {
        int size = (a < 3) ? BIG_SLOW_SIZE : BIG_SIZE;
        Result r = measure(a, size);
        printf("%-16s %10d %12.3f %16.2f %8s\n",
               algoNames[a], size, r.timeMs, r.memoryKB, r.sorted ? "YES" : "NO");
    }
}

int main(void)
{
    while (true) {
        printf("\n=========== SORTING ALGORITHMS ===========\n");
        printf("1. Bubble Sort\n");
        printf("2. Selection Sort\n");
        printf("3. Insertion Sort\n");
        printf("4. Merge Sort\n");
        printf("5. Quick Sort\n");
        printf("6. Compare all algorithms\n");
        printf("7. Big array example (100 000 elements)\n");
        printf("0. Exit\n");

        int choice = readInt("Your choice: ", 0, 7);

        if (choice == 0) {
            printf("Goodbye!\n");
            break;
        } else if (choice <= 5) {
            runOneAlgorithm(choice - 1);
        } else if (choice == 6) {
            compareAll();
        } else {
            bigArrayExample();
        }
    }
    return 0;
}

// This c file is standalone and will test the speed of the device to write to a csv file.
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    FILE *file;
    const char *filename = "test_speed.csv";
    const int num_entries = 1000000;
    clock_t start, end;

    file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    start = clock();
    for (int i = 1000000; i < num_entries + 1000000; i++)
    {
        fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", i, i * 2, i * 3, i * 4, i * 5, i * 6, i * 7);
    }
    end = clock();

    fclose(file);

    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Time taken to write %d entries: %.2f seconds\n", num_entries, time_taken);

    return EXIT_SUCCESS;
}
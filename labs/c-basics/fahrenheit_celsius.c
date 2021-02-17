#include <stdio.h>
#include <stdlib.h>

/* print Fahrenheit-Celsius conversion or table */

int main(int argc, char **argv)
{

    if (argc == 2)
    {
        printf("Farenheit: %d, Celsuis: %6.1f\n", atoi(argv[1]), (5.0 / 9.0) * (atoi(argv[1]) - 32));
    }
    else if (argc == 4)
    {
        int fahr;
        int lower = atoi(argv[1]);
        int upper = atoi(argv[2]);
        int step = atoi(argv[3]);
        for (fahr = lower; fahr <= upper; fahr = fahr + step)
            printf("Fahrenheit: %3d, Celsius: %6.1f\n", fahr, (5.0 / 9.0) * (fahr - 32));
    }
    else
    {
        printf("Invalid input\n");
    }

    return 0;
}
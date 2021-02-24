#include <stdio.h>

void reverse(int length, char word[])
{
    char tmp;
    for (int i = 0; i < length; i++)
    {
        tmp = word[i];
        word[i] = word[length - i - 1];
        word[length - i - 1] = tmp;
    }
}

void printReversed(int length, char word[]) {
    for (int i = 0; i < length; i++)
    {
        printf("%c", word[length - i - 1]);
    }
    printf("\n");
}


int main(int argc, char **argv)
{
    int length = 0;
    char c, word[1000];

	while ((c = getchar()) != EOF)
	{
        if (c == '\n')
        {
            reverse(length, word);
            printReversed(length, word);
            length = 0;
        }
		else
		{
            word[length] = c;
            length++;
		}
	}

	return 0;
}
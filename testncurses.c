#include <ncurses.h>
#include <stdio.h>
int main()
{
    char buffer[10] = {'s', 'j', 'l', 'n', 't', '\0'};
    char peakabo[10];
    int i = 5;
    while(i >= 0)
    {
        peakabo[i] = buffer[i];
        i--;
    }
    printf("%s", peakabo);
    return 0;
}

#include <stdio.h>
#include <ncurses.h>

int main()
{
    if(has_colors()){
        printf("It has colors");
    }
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void move_cursor_up();
void move_cursor_down();
void move_cursor_right();
void move_cursor_left();
void clear_terminal();
void set_cursor_position(int r, int c);

int main(int argc, char *argv[]){
    // clear_terminal();
    system("clear");
    if(argc != 2){
        printf("Usage: ./chroma filename\n");
        return 1;
    }
    
    FILE *input = fopen(argv[1], "r");
    if(input == NULL){
        printf("Error opening the file\n");
        return 2;
    }
    
    char *buffer = (char*)malloc(1024 * sizeof(char));
    while(fgets(buffer, sizeof(buffer), input) != NULL){
        printf("%s", buffer);
    }
    
    // Navigation
    char keystroke;
    int r = 1;
    int c = 1;
    // clear_terminal();

    while(1){
        set_cursor_position(r,c);
        keystroke = getchar();
        if(keystroke == 'k'){
            move_cursor_up();
            r--;
        }
        else if(keystroke == 'j'){
            move_cursor_down();
            r++;
        }
        else if(keystroke == 'l'){
            move_cursor_right();
            c++;
        }
        else if(keystroke == 'h'){
            move_cursor_left();
            c--;
        }
        else if(keystroke == 's'){
            scanf(" %d", &r);
            scanf(" %d", &c);
            // set_cursor_position(r,c);
        }
        else if(keystroke == 'q'){
            break;
        }
    }
    fclose(input);
    return 0;
}

void move_cursor_up(){
    printf("\033[A");
}

void move_cursor_down(){
    printf("\033[B");
}

void move_cursor_right(){
    printf("\033[C");
}

void move_cursor_left(){
    printf("\033[D");
}

void clear_terminal(){
    printf("\033[2J");
}

void set_cursor_position(int r, int c){
    printf("\033[%d;%dH", r, c);
}

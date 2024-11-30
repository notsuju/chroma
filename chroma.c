#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void display_file(int no_of_lines, char *buffer[], int r, int c);
void exit_ncurses_environment(char *buffer[]);
#define MAX_LINES 1024
#define MAX_LINE_LENGTH 1200
#define ESCAPE_KEY 27
#define KEY_TAB '\t'
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Use './chroma help' for more information\n");
        return 1;
    }
    // Help prompt
    if (strcmp(argv[1], "help") == 0)
    {
        printf(" Usage: './chroma file_name'\n\n");
        printf(" Command Mode: \n\n");
        printf(" k --move up\n j --move down\n l --move right\n h --move left\n s --save file\n q --quit the program\n i --edit mode\n x <line_number> enter <column_number> enter --jump to a specific line\n\n");
        printf(" Edit Mode: \n\n");
        printf(" esc --quit edit mode and enter command mode\n");
        return -1;
    }

    // Opening File
    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        FILE *new = fopen(argv[1], "w");
        fclose(new);
    }

    // Allocating memory
    char **buffer = (char **)malloc(MAX_LINES * sizeof(char *));
    for (int i = 0; i < MAX_LINES; i++)
    {
        buffer[i] = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
    }
    if (buffer == NULL)
    {
        printf("Error allocating memory\n");
        return 2;
    }

    // Copying into Buffer
    int no_of_lines = 0;
    if (input != NULL)
    {
        while (fgets(buffer[no_of_lines], MAX_LINE_LENGTH, input))
        {
            no_of_lines++;
        }
        fclose(input);
    }

    // Initialising screen
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    putp("\033[1 q");
    fflush(stdout);
    // timeout(100); // Set a timeout for getch() to handle ESC key better

    // Actually algorithm
    int r = 0;
    int c = 0;
    int editing = 0;
    int keystroke;
    while (1)
    {
        display_file(no_of_lines, buffer, r, c);
        keystroke = getch();
        switch (keystroke)
        {
        case KEY_UP:
            if (r > 0)
            {
                r--;
            }
            continue;
        case KEY_DOWN:
            if (r < (no_of_lines - 1))
            {
                r++;
            }
            continue;
        case KEY_RIGHT:
            if (c < (strlen(buffer[r]) - 1))
            {
                c++;
            }
            continue;
        case KEY_LEFT:
            if (c > 0)
            {
                c--;
            }
            continue;
        case ESCAPE_KEY:
            editing = 0;
            putp("\033[1 q");
            fflush(stdout);
            continue;
        default:
            break;
        }
        if (!editing)
        {
            if (keystroke == 'q')
            {
                putp("\033[1 q");
                fflush(stdout);
                exit_ncurses_environment(buffer);
                printf("Quit the program successfully\n");
                return 0;
            }
            else if (keystroke == 'k' && r > 0)
            {
                r--;
            }
            else if (keystroke == 'j' && r < no_of_lines - 1)
            {
                r++;
            }
            else if (keystroke == 'h' && c > 0)
            {
                c--;
            }
            else if (keystroke == 'l' && c < (strlen(buffer[r]) - 1))
            {
                c++;
            }
            else if (keystroke == 'i')
            {
                editing = 1;
                putp("\033[5 q");
                fflush(stdout);
                continue;
            }
            else if (keystroke == 's')
            {
                FILE *output = fopen(argv[1], "w");
                if (output == NULL)
                {
                    exit_ncurses_environment(buffer);
                    printf("Error saving file\n");
                    return 3;
                }
                for (int i = 0; i < no_of_lines; i++)
                {
                    fputs(buffer[i], output);
                }
                fclose(output);
            }
        }
        if (editing)
        {
            // TAB
            if (keystroke == KEY_TAB && c < MAX_LINE_LENGTH)
            {
                char temp[MAX_LINE_LENGTH];
                strcpy(temp, &buffer[r][c]);
                for(int i = c; i < c + 4; i++)
                {
                    buffer[r][i] = ' ';
                }
                buffer[r][c + 4] = '\0';
                strcat(buffer[r], temp);
                c += 4;
            }
            // DELETE CHARACTER
            else if (keystroke == KEY_BACKSPACE)
            {
                if (c == 0 && r == 0)
                {
                    continue;
                }
                else if (c > 0)
                {
                    for (int i = c; i < MAX_LINE_LENGTH; i++)
                    {
                        buffer[r][i - 1] = buffer[r][i];
                    }
                    c--;
                }
                else
                {
                    int line_length = strlen(buffer[r - 1]);
                    // Setting the cursor to the end of the previous line
                    r--;
                    c = strlen(buffer[r]);

                    // Removing the endline character from the previous line
                    buffer[r][line_length - 1] = '\0';

                    // Concatenating the previous line with the current line
                    strcat(buffer[r], buffer[r + 1]);

                    // Shifting all the lines up
                    for (int i = r + 1; i < no_of_lines; i++)
                    {
                        memset(buffer[i], '\0', MAX_LINE_LENGTH);
                        strcpy(buffer[i], buffer[i + 1]);
                    }
                    no_of_lines--;
                }
            }
            // NEW LINE
            else if (keystroke == KEY_ENTER || keystroke == 10 || keystroke == 13)
            {
                // Shift all lines down
                for (int i = no_of_lines; i > r; i--)
                {
                    strcpy(buffer[i], buffer[i - 1]);
                }

                // Create new line buffer
                char temp[MAX_LINE_LENGTH];
                strcpy(temp, &buffer[r][c]);
                buffer[r][c] = '\n';
                buffer[r][c + 1] = '\0';

                // Set up next line
                memset(buffer[r + 1], '\0', MAX_LINE_LENGTH);
                strcpy(buffer[r + 1], temp);

                r++;
                c = 0;
                no_of_lines++;
            }
            // INSERT CHARACTER
            else if (c < MAX_LINE_LENGTH)
            {
                int line_length = strlen(buffer[r]);
                // For handling empty first line
                if (c == 0 && r == 0 && line_length == 0)
                {
                    buffer[0][0] = '\n';
                    buffer[0][1] = '\0';
                    no_of_lines++;
                    line_length++;
                }
                for (int i = line_length; i >= c; i--)
                {
                    buffer[r][i] = buffer[r][i - 1];
                }
                buffer[r][c] = keystroke;
                c++;
            }
        } // if editing
    } // while loop
    exit_ncurses_environment(buffer);
    return 0;
}

void display_file(int no_of_lines, char *buffer[], int r, int c)
{
    clear();
    for (int i = 0; i < no_of_lines; i++)
    {
        mvprintw(i, 0, "%s", buffer[i]);
    }
    move(r, c);
    refresh();
    return;
}

void exit_ncurses_environment(char *buffer[])
{
    endwin();
    for (int i = 0; i < MAX_LINES; i++)
    {
        free(buffer[i]);
    }
    free(buffer);
    buffer = NULL;
    return;
}

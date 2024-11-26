#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void display_file(int no_of_lines, char *buffer[], int r, int c);
#define MAX_LINES 1024
#define MAX_LINE_LENGTH 1200
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./chroma filename\n");
        return 1;
    }

    // Opening File
    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        printf("Error opening the file\n");
        return 1;
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
    while (fgets(buffer[no_of_lines], MAX_LINE_LENGTH, input))
    {
        no_of_lines++;
    }

    // Initialising screen
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Actually algorithm
    int r = 0;
    int c = 0;
    int editing = 0;
    int keystroke;
    int insert;
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
        case 27:
            editing = 0;
            break;
        default:
            break;
        }
        if (!editing)
        {
            insert = 0;
            if (keystroke == 'q')
            {
                endwin();
                return 0;
            }
            else if (keystroke == 'i')
            {
                editing = 1;
            }
            else if (keystroke == 's')
            {
                FILE *output = fopen(argv[1], "w");
                if (output == NULL)
                {
                    printf("Error saving file");
                    endwin();
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
            // DELETE CHARACTER
            if (keystroke == KEY_BACKSPACE)
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
            else if (keystroke == KEY_ENTER)
            {

                // Shift all lines down
                for (int i = no_of_lines; i > r + 1; i--)
                {
                    // Clearing the line to avoid overlapping
                    memset(buffer[i], '\0', MAX_LINE_LENGTH);
                    strcpy(buffer[i], buffer[i - 1]);
                }

                memset(buffer[r], '\0', MAX_LINE_LENGTH);
                int line_length = strlen(buffer[r]);
                if (c < line_length)
                {
                    strcpy(buffer[r + 1], &buffer[r][c]);
                    buffer[r][c] = '\n';
                    buffer[r][c + 1] = '\0';
                }
                else
                {
                    buffer[r][line_length - 1] = '\n';
                    buffer[r][line_length] = '\0';
                    buffer[r + 1][0] = '\n';
                    buffer[r + 1][0] = '\0';
                }

                r++;
                c = 0;
                no_of_lines++;
            }
            // INSERT CHARACTER
            else if (c < MAX_LINE_LENGTH)
            {
                if (insert)
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
                insert = 1;
            }
        } // if editing
    } // while loop

    // Important
    endwin();
    for (int i = 0; i < MAX_LINES; i++)
    {
        free(buffer[i]);
    }
    free(buffer);
    buffer = NULL;
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
}

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINES 1024
#define MAX_LINE_LENGTH 1200
#define MAX_CLIPBOARD 400
#define ESCAPE_KEY 27
#define KEY_TAB '\t'

char **clipboard()
{
    char **buffer = (char **)malloc(MAX_CLIPBOARD * sizeof(char *));
    for (int i = 0; i < MAX_CLIPBOARD; i++)
    {
        buffer[i] = (char *)malloc(MAX_CLIPBOARD * sizeof(char));
    }
    return buffer;
}
void display_file(int no_of_lines, char *buffer[], int r, int c);
void exit_ncurses_environment(char *buffer[], char *clipboard_buffer[]);
void help_prompt(const char *command);
void copy_partial_string(char *clipboard_buffer[], char *buffer[], int end_c, int end_r, int starting_c, int starting_r, int written_lines);
void swap(int *a, int *b);
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Use './chroma --help' for more information\n");
        return 1;
    }

    // Help promp
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "--h") == 0)
    {
        help_prompt(argv[1]);
        return 0;
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
    int starting_r = 0;
    int starting_c = 0;
    int end_r = 0;
    int end_c = 0;
    char **clipboard_buffer = clipboard();
    int normal = 1;
    int insert = 0;
    int visual = 0;
    int written_lines = 0;
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
            normal = 1;
            insert = 0;
            visual = 0;
            putp("\033[1 q");
            fflush(stdout);
            continue;
        default:
            break;
        }
        if (normal)
        {
            if (keystroke == 'q')
            {
                putp("\033[1 q");
                fflush(stdout);
                exit_ncurses_environment(buffer, clipboard_buffer);
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
            else if (keystroke == 'x')
            {
                int x, y;
                scanf("%d %d", &x, &y);
                if (x <= no_of_lines && y <= strlen(buffer[x - 1]))
                {
                    r = x - 1;
                    c = y - 1;
                }
            }
            else if (keystroke == 'i')
            {
                insert = 1;
                normal = 0;
                visual = 0;
                putp("\033[5 q");
                fflush(stdout);
            }
            else if (keystroke == 'v')
            {
                starting_r = r;
                starting_c = c;
                visual = 1;
                normal = 0;
                insert = 0;
            }
            // else if (keystroke == 'p')
            // {
            //     int temp_r = r;
            //     int temp_c = c;
            //     for(int i = 0; i < written_lines; i++)
            //     {
            //         if(i == 0)
            //         {
            //             //mvprintw(temp_r, temp_c + 1, "%s", clipboard_buffer[i]);
            //             endwin();
            //             printf("%s", clipboard_buffer[i]);
            //             return 0;
            //         }
            //         else
            //         {
            //             mvprintw(temp_r, 0, "%s", clipboard_buffer[i]);
            //         }
            //         temp_r++;
            //         no_of_lines++;
            //     }
            // }
            else if (keystroke == 's')
            {
                FILE *output = fopen(argv[1], "w");
                if (output == NULL)
                {
                    exit_ncurses_environment(buffer, clipboard_buffer);
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
        else if (visual)
        {
            if (keystroke == 'y')
            {
                end_r = r;
                end_c = c;
                if (end_r != starting_r)
                {
                    if(end_r < starting_r)
                    {
                        swap(&end_r, &starting_r);
                        swap(&end_c, &starting_c);
                    }
                    written_lines = 0;
                    for (int i = starting_r; i < end_r; i++)
                    {
                        if (i == starting_r)
                        {
                            strcpy(clipboard_buffer[written_lines], &buffer[i][starting_c]);
                        }
                        else
                        {
                            strcpy(clipboard_buffer[written_lines], buffer[i]);
                        }
                        written_lines++;
                    }
                    copy_partial_string(clipboard_buffer, buffer, end_c, end_r, starting_c, starting_r, written_lines);
                }
                else
                {
                    if (end_c < starting_c)
                    {
                        swap(&end_c, &starting_c);
                    }
                    written_lines = 0;
                    copy_partial_string(clipboard_buffer, buffer, end_c, end_r, starting_c, starting_r, written_lines);
                }
                // endwin();
                // printf("%s", clipboard_buffer[0]);
                // printf("%s", clipboard_buffer[1]);
                // return 0;
                visual = 0;
                normal = 1;
            }
        }
        else if (insert)
        {
            // TAB
            if (keystroke == KEY_TAB && c < MAX_LINE_LENGTH)
            {
                char temp[MAX_LINE_LENGTH];
                strcpy(temp, &buffer[r][c]);
                for (int i = c; i < c + 4; i++)
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
                    c = strlen(buffer[r]) - 1;

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
    exit_ncurses_environment(buffer, clipboard_buffer);
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

void swap(int *a, int *b)
{
    *a = *a ^ *b;
    *b = *a ^ *b;
    *a = *a ^ *b;
    return;
}

void exit_ncurses_environment(char *buffer[], char *clipboard_buffer[])
{
    endwin();
    for (int i = 0; i < MAX_LINES; i++)
    {
        free(buffer[i]);
    }
    free(buffer);
    for (int i = 0; i < MAX_CLIPBOARD; i++)
    {
        free(clipboard_buffer[i]);
    }
    free(clipboard_buffer);
    clipboard_buffer = NULL;
    buffer = NULL;
    return;
}

void copy_partial_string(char *clipboard_buffer[], char *buffer[], int end_c, int end_r, int starting_c, int starting_r, int written_lines)
{
    int temp = (end_r == starting_r) ? starting_c : 0;
    int k = 0;
    for (int i = temp; i <= end_c && buffer[end_r][i] != '\0'; i++)
    {
        clipboard_buffer[written_lines][k] = buffer[end_r][i];
        k++;
    }
    clipboard_buffer[end_r][k] = '\0';
    return;
}

void help_prompt(const char *command)
{
    printf("Usage: './chroma %s'\n\n", command);
    printf("Normal Mode:\n");
    printf("k   --move cursor up\n");
    printf("written_lines   --move cursor down\n");
    printf("l   --move cursor right\n");
    printf("h   --move cursor left\n");
    printf("i   --insert mode\n");
    printf("x   -x<line_number>enter<column_number>enter    --jumps to a specified line and column\n");
    printf("s   --save file\n");
    printf("\n");
    printf("Insert Mode:\n");
    printf("esc --enter normal mode\n");
}

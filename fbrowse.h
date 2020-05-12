#ifndef __BROWSE_H__
#define __BROWSE_H__

#include <stdlib.h>
#include <curses.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

#include "fbutils.h"

#define FILE_PAIR	1
#define DIR_PAIR	2
#define HL_COLOR    15

#define MAX_LINE    512
#define MAX_FILES   256


void init_curses();
void init_colors();
void print_entries(WINDOW *win, char *wd, struct dirent **entries, int *size, int hl);
void print_info(WINDOW *win, char *fname);
void print_archive_preview(WINDOW *win, char *fname, int rows, int cols, suffix suf);
void print_text_preview(WINDOW *win, char *fnamefile, int rows, int cols);

int run_command(WINDOW *win, char *cmd);


void print_entries(WINDOW *win, char* wd, struct dirent **entries, int *size, int hl)
{
    int max_x, max_y;
    getmaxyx(win, max_y, max_x);

    int start_index = 0;
    
    if(hl != -1 && *size > max_y){
        if(hl > max_y/2){
            start_index = hl - max_y/2;
        }
    }

    for(int i = start_index; i < *size; i++){

        int line_color = 0;
        if(entries[i]->d_type == DT_DIR){
            line_color = DIR_PAIR;
        }else if(entries[i]->d_type == DT_REG){
            line_color = FILE_PAIR;
        }

        if(i == hl){
            short fg, bg;//, pair = 0;
            pair_content(line_color, &fg, &bg);
            init_pair(HL_COLOR, fg, COLOR_GREEN);
            line_color = HL_COLOR;
        }
        mvwaddch(win, i-start_index, 0, ACS_VLINE);

        wattron(win, COLOR_PAIR(line_color));

        mvwprintw(win, i-start_index, 1, "%s\t\t\t\t\t\t\t\t", entries[i]->d_name);

        wattroff(win, COLOR_PAIR(line_color));
     }
}

void print_info(WINDOW *win, char *wd)
{
    int max_x, max_y;
    getmaxyx(win, max_y, max_x);
    int rpane_x = max_x /2;
    int mpane_x = max_x/4;

    mvwhline(win, 1, 0, ACS_HLINE, max_x);
    struct stat info;
    if(stat(wd, &info) == -1){
        return;
    }
    mvwprintw(win, 0, 1, "%s", wd);
    mvwprintw(win, 2, 1, "inode: %d", info.st_ino);

    mvwprintw(win, 2, mpane_x, "bytes: %d", info.st_size);
    mvwprintw(win, 3, mpane_x, "blocks: %d", info.st_blocks);
    mvwprintw(win, 4, mpane_x, "blksize: %d", info.st_blksize);

    mvwprintw(win, 2, rpane_x, "last access: %s" , ctime(&info.st_atim.tv_sec));
    mvwprintw(win, 3, rpane_x, "last modification: %s" , ctime(&info.st_mtim.tv_sec));
}

void print_file_preview(WINDOW *win, char* fname)
{
    int rows, cols;
    getmaxyx(win, rows, cols);

    int sufind = suffindx(fname);

    // preview tarballs
    if(!strcmp(&fname[sufind], ".tgz") 
            || !strcmp(&fname[sufind], "tar.gz") 
            || !strcmp(&fname[sufind], ".tar")){
        print_archive_preview(win, fname, rows, cols, TAR);
    // preview text files
    }else{
        print_text_preview(win, fname, rows, cols);
        
    }
}

void print_text_preview(WINDOW *win, char *fname, int rows, int cols)
{
    FILE* file;
    if((file = fopen(fname, "r") ) == NULL){
        return;
    }else{
        char line[MAX_LINE];
        int lno = 0;

        while(fgets(line, MAX_LINE, file) != NULL && lno < rows){
            mvwaddch(win, lno, 0, ACS_VLINE);
            mvwprintw(win, lno, 1, "%s", line);
            lno++;
        }
        fclose(file);
    }
}

void print_archive_preview(WINDOW *win, char *fname, int rows, int cols, suffix suf)
{
    char cmd[2*PATH_MAX]; //NULL;
    switch(suf){
        case TAR:
            sprintf(cmd, "/usr/bin/tar -tf %s", fname);
            break;
    }

    FILE *output = popen(cmd, "r"); 

    char *line = malloc(cols+1);
    int ln = 0;
    while((fgets(line, cols, output)) > 0){
       mvwaddnstr(win, ln, 0, line, cols); 
       ln++;
    }
    pclose(output);

    if(line)
        free(line);
}

void init_curses()
{
	initscr();

    start_color(); 
    noecho();
    cbreak();
    //nodelay(info_win, true);
    curs_set(0);
    keypad(stdscr, true);

    init_colors();
    refresh();
}


void init_colors()
{	
    	init_color(COLOR_BLUE, 0, 0, 1000);

        init_pair(FILE_PAIR, COLOR_WHITE, COLOR_BLACK);
        init_pair(DIR_PAIR, COLOR_BLUE, COLOR_BLACK);
}


int run_command(WINDOW *win, char *cmd)
{
    if(strlen(cmd) > 1){

        endwin();
        pid_t chi = fork();
        if(chi == 0){
            endwin();
            system(cmd);
            exit(EXIT_SUCCESS);

        }else{

            int ret;
            wait(&ret);

            init_curses();
        }
        
    }
    noecho();
}


#endif

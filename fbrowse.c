#include "fbrowse.h"
#include "fbutils.h"

#define position_cur_x(w) (w/6)
#define position_chi_x(w) ((3*w)/7)

#define INFO_PANE_HEI 12

#define CMD_MAX 128

//TODO: permission check on files and directories 

 
int main(int argc, char **argv)
{
    init_curses();
    WINDOW* par_win = newwin(LINES-8, COLS, 0, 0);  // leftmost pane
    WINDOW* cur_win = newwin(LINES-8, COLS, 0, position_cur_x(COLS)); // center pane
    WINDOW* chi_win = newwin(LINES-8, COLS, 0, position_chi_x(COLS)); // rightmost pane
    WINDOW* info_win = newwin(INFO_PANE_HEI, COLS, LINES-6, 0);// bottom pane 
    WINDOW* cmd_win = newwin(1, COLS, LINES-1, 0);

    int selected = 0; // selected directory entry

    char wd_str[PATH_MAX]; // path to fbrowse's current directory
    char start_wd_str[PATH_MAX];

    getcwd(wd_str, PATH_MAX);
    strncpy(start_wd_str, wd_str, PATH_MAX);

    char par_str[PATH_MAX];
    strncpy(par_str, wd_str, PATH_MAX);
    pop_dir(par_str);

    char chi_str[PATH_MAX];

    // file entries in the central window 
    struct dirent **cur_ents; 
    int ncur_ents = 0;

    // file entries in the leftmost window 
    struct dirent **par_ents;
    int npar_ents = 0;

    // file entries in the rightmost window (shown when highlighting directory)
    struct dirent ** chi_ents;
    int nchi_ents = 0;

    ncur_ents = scandir(wd_str, &cur_ents, exclude_dotdirs, alphasort);
    if(ncur_ents == -1){
        perror("failed to stat");
        exit(-1);
    }
    npar_ents = scandir(par_str, &par_ents, exclude_dotdirs, alphasort);
    if(npar_ents == -1){
            perror("failed to stat");
            exit(-1);
    }

    strncpy(chi_str, wd_str, PATH_MAX);
    if(ncur_ents > 0){
        push_dir(chi_str, cur_ents[selected]->d_name);
        strncpy(chi_str, wd_str, PATH_MAX);
        if(cur_ents[selected]->d_type == DT_DIR){
            nchi_ents = scandir(chi_str, &chi_ents, exclude_dotdirs, alphasort);
        }
    }

	int ch = ERR; // input character (ERR for no input)

    int moved = true;
    int quit = false;
    do{	
        // handle input
        switch(ch){
            case 'q': // quit
                quit = true;
                free(cur_ents);
                free(par_ents);
                free(chi_ents);

                endwin();
                exit(0);
                break;
            case 'j': // move down) 
                if(selected < ncur_ents-1){
                    selected++;
                    moved = true;
                }
                break;
            case 'k': // move up
                if(selected > 0){
                    selected--;
                    moved = true;
                }
                break;
            case 'h':  // move to parent
                {
                    // move selected index to the previous current directory
                    char rel[FILENAME_MAX];
                    abs_to_rel(rel, wd_str);
                    for(int i = 0; i < npar_ents; i++){
                        if(strcmp(par_ents[i]->d_name, rel) == 0){
                            selected = i;
                        }
                    }

                    // backup parent string in case dir change fails
                    char par_bkp[PATH_MAX];
                    strncpy(par_bkp, par_str, PATH_MAX);
                    pop_dir(par_str);
                    if(chdir(par_str) == 0){
                        strncpy(chi_str, wd_str, PATH_MAX);
                        pop_dir(wd_str);
                        moved = true;
                    }else{
                        wprintw(cmd_win, "failed to change directory");
                        strncpy(par_str, par_bkp, PATH_MAX);
                    }

                    break;
                }
            case 'l': // move to selected
                if(!cur_ents)
                    break;
                if(cur_ents[selected]->d_type == DT_DIR){

                    if(chdir(chi_str) == 0){
                        strncpy(par_str, wd_str, PATH_MAX);
                        strncpy(wd_str, chi_str, PATH_MAX);
                        moved = true;
                    }else{
                        wprintw(cmd_win, "failed to change directory");
                    }

                }
                break;
            case ':':
                {
                    char cmd[CMD_MAX];
                    echo();

                    wprintw(cmd_win, "Command: ");
                    wgetnstr(cmd_win, cmd, CMD_MAX);
                    run_command(cmd_win, cmd);
                    break;
                }
            default:
                break;
        }
        if(moved){
            wclear(par_win);
            wclear(cur_win);
            wclear(chi_win);
            wclear(info_win);

            free(cur_ents);
            free(par_ents);

            ncur_ents = scandir(wd_str, &cur_ents, exclude_dotdirs, alphasort);
            npar_ents = scandir(par_str, &par_ents, exclude_dotdirs, alphasort);

            if(selected > ncur_ents-1)
                selected = ncur_ents-1;
            
            if(ncur_ents > 0){
                strncpy(chi_str, wd_str, PATH_MAX);
                push_dir(chi_str, cur_ents[selected]->d_name); 

                if(S_ISDIR(get_mode(chi_str))){
                    free(chi_ents);
                    nchi_ents = scandir(chi_str, &chi_ents, exclude_dotdirs, alphasort);
                    print_entries(chi_win, chi_str,  chi_ents, &nchi_ents, -1);
                }else if(S_ISREG(get_mode(chi_str))){
                    print_file_preview(chi_win, chi_str);  
                }
            }

            print_entries(cur_win, wd_str, cur_ents, &ncur_ents, selected);
            print_entries(par_win, par_str, par_ents, &npar_ents, -1);
            print_info(info_win, chi_str);

            wrefresh(par_win);
            wrefresh(cur_win);
            wrefresh(info_win);
            wrefresh(chi_win);

            moved = false;
        }
        ch = wgetch(info_win);

    }while(!quit);

    free(cur_ents);
    free(par_ents);
    free(chi_ents);

	endwin();
	exit(0);
}




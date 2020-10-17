#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
CHYBOVE HLASENIA
 2 nedostal som ziadne argumenty na vstupe
 */

typedef enum {SCAN_DELIM_AND_ARGS, RUN, EXIT} RunMode;

typedef enum {SCAN_DELIM, AWAIT_DELIM, DONE} DelimMode;

typedef enum {SCAN_PARAMS, AWAIT_IROW_PARAM, AWAIT_DROW_PARAM, AWAIT_DROWS_PARAMS_1, AWAIT_DROWS_PARAMS_2, AWAIT_ICOL_PARAM, AWAIT_DCOL_PARAM, AWAIT_DCOLS_PARAMS_1, AWAIT_DCOLS_PARAMS_2, AWAIT_CSET_PARAM, PARAM_ERROR, HAVE_ALL_PARAMS} ParamMode;

typedef enum {NONE, IROW, AROW, DROW, DROWS, ICOL, ACOL, DCOL, DCOLS, CSET} ArgsMode;


void find_delim(int argc, char **argv, char *delim, bool *found_delim, char delim_string[]);
void process_args (int argc, char **argv, const bool *found_delim, int *param1, int *param2, char *string_param, ArgsMode *args_mode, ParamMode *param_mode);
int verify_digits_only_in_string(const char string[]);
int verify_unsigned_integer(const char string[]);
void print_string_to_stdout(FILE *file_out, const char string_param[]);
void irow(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string);
int is_delim (const char *delim, char *delim_string[], const bool *multi_character_delim, int znak);

// PROTOTYPY FUNKCII NA UPRAVU TABULKY
void arow(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string);
void drow(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string);
void icol(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string);
void acol(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string);
void dcol(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string);
void dcols(FILE *file_in, FILE *file_out, int param1, int param2, const char *delim, char delim_string[], bool multi_character_string);
void drows(FILE *file_in, FILE *file_out, int param1, int param2, const char *delim, char delim_string[], bool multi_character_string);

// PROTOTYPY FUNKCII NA SPRACOVANIE DAT

void cset(FILE *file_in, FILE *file_out, char string_param[], const char *delim, char delim_string[], bool multi_character_string);

int main(int argc, char *argv[]){
    // neboli zadane ziadne args ani delim
    if (argc == 1) return 2;

    printf("[START]\n");

    // pointer na stdin, stdout IO stream
    FILE *file_in = stdin;
    FILE *file_out = stdout;


    char delim;
    char delim_string[101];
    bool found_delim = false;
    bool multi_character_delim = false;

    RunMode run_mode = SCAN_DELIM_AND_ARGS;
    ParamMode param_mode = SCAN_PARAMS;
    ArgsMode args_mode = NONE;

    // PARAMETRE PRE ARGUMENTY
    int param1 = 0;
    int param2 = 0;
    char string_param[101];

    while (run_mode != EXIT) {
        switch (run_mode) {
            case SCAN_DELIM_AND_ARGS:
                find_delim(argc, argv, &delim, &found_delim, delim_string);

                if (found_delim) {
                    if(strlen(delim_string) != 0){
                        delim = delim_string[0];
                        multi_character_delim = true;
                        printf("[NOTICE] Nasiel sa viacznakovy delim: %s\n", delim_string);
                    }else {
                        printf("[NOTICE] Nasiel sa jednoznakovy delim: %c\n",delim);
                    }
                }else{
                    printf("[NOTICE] Nenasiel sa delim, nastavujem default\n");
                    delim = ' ';
                }

                process_args(argc, argv, &found_delim, &param1, &param2, string_param, &args_mode, &param_mode);

                if (param_mode == HAVE_ALL_PARAMS) {
                    printf("param1: %d\n", param1);
                    printf("param2: %d\n", param2);
                    printf("strprm: %s\n", string_param);

                    run_mode = RUN;
                }
                if (param_mode == PARAM_ERROR) {
                    printf("[ERROR] Parameter error.\n");
                    run_mode = EXIT;
                }
                break;

            case RUN:
                switch (args_mode) {

                    case IROW:
                        irow(file_in, file_out, param1, &delim, delim_string, multi_character_delim);
                        break;

                    case AROW:
                        arow(file_in, file_out, param1, &delim, delim_string, multi_character_delim);
                        break;

                    case DROW:
                        drow(file_in, file_out, param1, &delim, delim_string, multi_character_delim);
                        break;

                    case ACOL:
                        acol(file_in, file_out, param1, &delim, delim_string, multi_character_delim);
                        break;

                    case ICOL:
                        icol(file_in, file_out, param1, &delim, delim_string, multi_character_delim);
                        break;

                    case DCOL:
                        dcol(file_in, file_out, param1, &delim, delim_string, multi_character_delim);
                        break;

                    case DCOLS:
                        dcols(file_in, file_out, param1, param2, &delim, delim_string, multi_character_delim);
                        break;

                    case DROWS:
                        drows(file_in, file_out, param1, param2, &delim, delim_string, multi_character_delim);
                        break;

                    case CSET:
                        cset(file_in, file_out, string_param, &delim, delim_string, multi_character_delim);
                        break;

                    case NONE:
                        printf("[ERROR] Neboli najdene ziadne argumenty na upravu tabulky.\n");
                        break;
                }
                run_mode = EXIT;

            case EXIT:
                printf("[NOTICE] Argumenty vykonane.\n");
                break;

            default:
                break;

        }
    }


    printf("[EXIT]");
}

void process_args (int argc, char **argv, const bool *found_delim,int *param1,int *param2, char *string_param, ArgsMode *args_mode, ParamMode *param_mode){
    // roztriedi argumenty, zaradi RunMode

    // ak uz ma delim tak je arg na 3 pozicii, inak je na prvej
    int init_position =  (*found_delim ? 3:1);

    for (int i=init_position; i<argc; i++) {

        switch (*param_mode) {
            case SCAN_PARAMS:
                if (!strcmp(argv[i], "irow")) {
                    printf("[NOTICE] Nasiel som irow\n");
                    *param_mode = AWAIT_IROW_PARAM;

                } else if (!strcmp(argv[i], "arow")){
                    printf("[NOTICE] Nasiel som arow.\n");
                    *param_mode = HAVE_ALL_PARAMS;

                } else if (!strcmp(argv[i], "drow")){
                    printf("[NOTICE] Nasiel som drow.\n");
                    *param_mode = AWAIT_DROW_PARAM;

                } else if (!strcmp(argv[i], "drows")){
                    printf("[NOTICE] Nasiel som drows.\n");
                    *param_mode = AWAIT_DROWS_PARAMS_1;

                } else if (!strcmp(argv[i], "icol")){
                    printf("[NOTICE] Nasiel icol.\n");
                    *param_mode = AWAIT_ICOL_PARAM;

                } else if (!strcmp(argv[i], "acol")){
                    printf("[NOTICE] Nasiel som acol.\n");
                    *param_mode = HAVE_ALL_PARAMS;

                } else if (!strcmp(argv[i], "dcol")){
                    printf("[NOTICE] Nasiel som dcol.\n");
                    *param_mode = AWAIT_DCOL_PARAM;

                } else if (!strcmp(argv[i], "dcols")) {
                    printf("[NOTICE] Nasiel som dcols.\n");
                    *param_mode = AWAIT_DCOLS_PARAMS_1;
                } else if (!strcmp(argv[i], "cset")) {
                    printf("[NOTICE] Nasiel som cset.\n");
                    *param_mode = AWAIT_CSET_PARAM;
                }else {
                    *param_mode = PARAM_ERROR;
                    printf("[ERROR] Tento argument nepoznam.\n");
                }

                break;

            case AWAIT_IROW_PARAM:
                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if((*param1 = (int)strtol(argv[i], NULL, 10)),*param1 != 0){ // NOLINT(cert-err34-c)

                    // prepnutie modu na IROW
                    *args_mode = IROW;

                    // prepnutie modu, moze sa prepnut main mode na execution
                    *param_mode = HAVE_ALL_PARAMS;
                    break;

                }else{
                    printf("[ERROR] Parameter musi byt vacsi ako nula.\n");
                    break;
                }

            case AWAIT_DROW_PARAM:
                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if ((*param1 = (int)strtol(argv[i], NULL, 10)), *param1 > 0){
                    *args_mode = DROW;
                    *param_mode = HAVE_ALL_PARAMS;
                    break;
                }else{
                    *param_mode = PARAM_ERROR;
                    printf("[ERROR] Parameter musi byt vacsi ako nula.\n");
                    break;
                }


            case AWAIT_DROWS_PARAMS_1:

                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if ((*param1 = (int)strtol(argv[i], NULL, 10)), *param1 != 0){
                    *param_mode = AWAIT_DROWS_PARAMS_2;

                    }else{
                        *param_mode = PARAM_ERROR;
                        printf("[ERROR] Parameter musi byt vacsi ako nula.\n");
                    }
                break;


            case AWAIT_DROWS_PARAMS_2:

                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if ((*param2 = (int)strtol(argv[i], NULL, 10)), *param2 != 0) {
                    if (*param1 == *param2) {
                        *args_mode = DROW;
                        *param_mode = HAVE_ALL_PARAMS;
                    } else if (*param1 > *param2) {
                        *param_mode = PARAM_ERROR;
                        printf("[ERROR] Parameter 1 musi byt mensi alebo rovny parametru 2.\n");
                    } else {
                        *args_mode = DROWS;
                        *param_mode = HAVE_ALL_PARAMS;
                    }
                }
                    break;

            case AWAIT_ICOL_PARAM:
                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if((*param1 = (int)strtol(argv[i], NULL, 10)),*param1 != 0){ // NOLINT(cert-err34-c)

                    // prepnutie modu na IROW
                    *args_mode = ICOL;

                    // prepnutie modu, moze sa prepnut main mode na execution
                    *param_mode = HAVE_ALL_PARAMS;
                }else{
                    *param_mode = PARAM_ERROR;
                    printf("[ERROR] Parameter musi byt vacsi ako nula.\n");
                }
                    break;

            case AWAIT_DCOL_PARAM:
                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if ((*param1 = (int)strtol(argv[i], NULL, 10)), param1 != 0){

                    *args_mode = DCOL;
                    *param_mode = HAVE_ALL_PARAMS;
                    break;
                }else{
                    *param_mode = PARAM_ERROR;
                    printf("[ERROR] Parameter musi byt vacsi ako nula.\n");
                    break;
                }
            case AWAIT_DCOLS_PARAMS_1:
                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if ((*param1 = (int)strtol(argv[i], NULL, 10)), param1 != 0){

                    *args_mode = DCOL;
                    *param_mode = AWAIT_DCOLS_PARAMS_2;
                    break;
                }else{
                    *param_mode = PARAM_ERROR;
                    printf("[ERROR] Parameter musi byt vacsi ako nula.\n");
                    break;
                }

            case AWAIT_DCOLS_PARAMS_2:
                if (verify_unsigned_integer(argv[i]) || verify_digits_only_in_string(argv[i])){
                    break;
                }else if ((*param2 = (int)strtol(argv[i], NULL, 10)), param1 != 0){
                    if (*param2 == *param1) {
                        *args_mode = DCOL;
                        *param_mode = HAVE_ALL_PARAMS;
                    } else if (*param1 > *param2) {
                        *param_mode = PARAM_ERROR;
                        printf("[ERROR] Parameter 1 musi byt mensi alebo rovny parametru 2.\n");
                        break;
                    } else {
                        *args_mode = DCOLS;
                        *param_mode = HAVE_ALL_PARAMS;
                    }
                    break;
                }else{
                    *param_mode = PARAM_ERROR;
                    printf("[ERROR] Parameter musi byt vacsi ako nula.\n");
                    break;
                }

            case AWAIT_CSET_PARAM:

                for (unsigned long int position_in_string = 0, size_of_string_param = strlen(argv[i]); position_in_string < size_of_string_param; position_in_string++){
                    string_param[position_in_string] = (char)argv[i][position_in_string];
                }

                *args_mode = CSET;
                *param_mode = HAVE_ALL_PARAMS;
                break;


            case PARAM_ERROR:
                printf("[ERROR] Parameter error, pozri chybove hlasky.\n");
                break;

            default:
                break;
        }
    }

}



// POMOCNE FUNKCIE

int verify_digits_only_in_string(const char string[]){
    // overenie ci su v argumente v argv[] iba cisla a ziadne ine znaky
    // ak je vsetko v poriadku a je to iba int, vrati 0. inak vrati 1

    for (int position_in_string = 0; string[position_in_string] != '\0'; position_in_string++){
        if(!isdigit((char)string[position_in_string])){
            printf("[ERROR] Nespravne argumenty (objavil sa necislovy znak.)\n");
            return 1;
        }
    }
    return 0;
}

int verify_unsigned_integer(const char string[]){
    // zisti ci je na prvom mieste v parametri -
    // ak ano, tak vrati 1, inak 0

    if (string[0] == '-'){
        printf("[ERROR] Nespravne argumenty (objavilo sa negativne cislo)\n");
        return 1;
    }else{
        return 0;
    }

}

void print_string_to_stdout(FILE *file_out, const char string_param[]) {
    int i = 0;
    while (string_param[i] != '\0') {
        fputc(string_param[i], file_out);
        i++;
    }
}

// FUNKCIE NA DELIM

void find_delim (int argc, char **argv, char *delim, bool *found_delim, char delim_string[]){

    DelimMode arg_mode = SCAN_DELIM;
    unsigned long int size_of_delim;

    if (!*found_delim) {
        for (int i = 1; i < argc; i++) {
            if (!*found_delim) {
                switch (arg_mode) {

                    //hlada delim
                    case SCAN_DELIM:
                        if (!strcmp(argv[i], "-d")) {
                            arg_mode = AWAIT_DELIM;
                        }
                        break;

                        // nasiel sa -d ocakavam char delimu
                    case AWAIT_DELIM:

                        size_of_delim = strlen(argv[i]);
                        if (size_of_delim == 1){
                            *delim = argv[i][0];
                        }else{
                            for (int position_in_delim = 0; position_in_delim < size_of_delim; position_in_delim++){
                                delim_string[position_in_delim] = (char)argv[i][position_in_delim];
                            }
                        }

                        *found_delim = true;
                        arg_mode = DONE;

                        break;

                        // dostal hodnotu, uz nehlada nic
                    default :
                        break;
                }
            }
        }
    }
}

int is_delim (const char *delim, char *delim_string[], const bool *multi_character_delim, int znak) {
    // porovna ci sa znak nachadza v retazci delim
    // ak je znak delim, vrati 1, inak 0

    if (*multi_character_delim) {
        if ( strchr(*delim_string, (char)znak) != NULL )
        {
            return 1;
        }

        return 0;
    }else{
        if(*delim == znak){
            return 1;
        }
        return 0;
    }
}

// FUNKCIE NA UPRAVU TABULKY

void irow(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string){
    // vlozi riadok tabulky pred riadok R > 0
    // TODO spocitat bunky?
    int znak;
    int riadky_counter = 0;


    while (riadky_counter != param-1){
        znak = fgetc(file_in);

        if (is_delim(delim, &delim_string, &multi_character_string, znak)){
            fputc(*delim, file_out);
        }else {
            fputc(znak, file_out);
        }

        if (znak == '\n') {
            riadky_counter ++;
        }
    }

    fputc('\n', file_out);

    znak = fgetc(file_in);

    while (znak != EOF){

        if (is_delim(delim, &delim_string, &multi_character_string, znak)){
            fputc(*delim, file_out);
        }else {
            fputc(znak, file_out);
        }
        znak = fgetc(file_in);
    }

}

void icol(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string){
    // vlozi prazdny stlpec pred stlpec C > 0

    int znak = 0;

    bool printed_empty_column = false;

    while (znak != EOF) {

        int delim_counter = 0;

        znak = fgetc(file_in);

        while (znak != '\n' && znak != EOF){

            if (delim_counter == param && !printed_empty_column){
                fputc(*delim, file_out);
                printed_empty_column = true;
            }

            if (is_delim(delim, &delim_string, &multi_character_string, znak)){
                fputc(*delim, file_out);
                delim_counter++;
            }else {
                fputc(znak, file_out);
            }
            znak = fgetc(file_in);
        }

        if (znak != EOF) {
            fputc(znak, file_out);
        }

        printed_empty_column = false;

    }
}

void arow(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string) {
    // prida prazdny riadok na koniec, pocet buniek ako v prvom riadku

    int znak;
    int bunky_counter = 0;
    bool prvy_riadok = true;

    znak = fgetc(file_in);
    while (znak != EOF) {

        if (znak == '\n' && prvy_riadok){
            prvy_riadok = false;
        }

        if (is_delim(delim, &delim_string, &multi_character_string, znak)){
            fputc(*delim, file_out);
            if (prvy_riadok) bunky_counter++;
        }else {
            fputc(znak, file_out);
        }

        znak = fgetc(file_in);
    }
    fputc('\n', file_out);

    // pridanie poctu delimov ako v prvom riadku
    for (int i = 0; i<bunky_counter; i++) {
        fputc(*delim, file_out);
    }
}

void drow(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string) {
    // vymaze riadok cislo R > 0

    int riadky_counter = 1;
    int znak;
    znak = fgetc(file_in);
    bool printed_delim = false;

    while (znak != EOF){

        if (riadky_counter == param) {
            if (znak == '\n') riadky_counter++;
        }else{
            if (znak == '\n'){
                riadky_counter++;
                fputc(znak, file_out);
            }else{

                if (is_delim(delim, &delim_string, &multi_character_string, znak)){
                    fputc(*delim, file_out);
                }else {
                    fputc(znak, file_out);
                }

            }
        }
        znak = fgetc(file_in);
    }
    fputc('\n', file_out);
}

void acol(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string) {
    // prida prazdny stlpec za posledny stlpec

    int znak = fgetc(file_in);

    while (znak != EOF) {
        while (znak != '\n' && znak != EOF) {

            if (is_delim(delim, &delim_string, &multi_character_string, znak)){
                fputc(*delim, file_out);
            }else {
                fputc(znak, file_out);
            }
            znak = fgetc(file_in);
        }

        if (znak == '\n') {
            fputc(*delim, file_out);
            fputc('\n', file_out);
            znak = fgetc(file_in);
        }
    }
    fputc(*delim, file_out);
    fputc('\n', file_out);
}

void dcol(FILE *file_in, FILE *file_out, int param, const char *delim, char delim_string[], bool multi_character_string) {
    // odstrani stlpec cislo C

    int znak = fgetc(file_in);
    int stlpce_counter = 1;
    bool pass = false;

    while (znak != EOF) {
        if (param == 1) pass = true;
        while (znak != '\n' && znak != EOF) {

            if (is_delim(delim, &delim_string, &multi_character_string, znak)){
                //fputc(*delim, file_out);
                if (!pass) fputc(*delim, file_out);
                stlpce_counter++;

                if (stlpce_counter == param) {
                    pass = true;
                } else {
                    pass = false;
                }

            }else {
                if (!pass) fputc(znak, file_out);

            }
            znak = fgetc(file_in);

        }

        if (znak == '\n') {
            pass = false;
            stlpce_counter = 1;
            fputc('\n', file_out);
            znak = fgetc(file_in);
        }
    }
    fputc('\n', file_out);
}

void dcols(FILE *file_in, FILE *file_out, int param1, int param2, const char *delim, char delim_string[], bool multi_character_string) {
    // odstrani stlpce N az M (N <= M), ak je N==M tak sa spravi DCOL N

    int znak = fgetc(file_in);
    int stlpce_counter = 1;
    bool pass = false;

    while (znak != EOF) {
        while (znak != '\n' && znak != EOF) {

            if (is_delim(delim, &delim_string, &multi_character_string, znak)){
                if (!pass) fputc(*delim, file_out);
                stlpce_counter++;

                if (stlpce_counter >= param1 && stlpce_counter <= param2) {
                    pass = true;
                } else {
                    pass = false;
                }

            }else {
                if (!pass) fputc(znak, file_out);

            }
            znak = fgetc(file_in);

        }

        if (znak == '\n') {
            pass = false;
            stlpce_counter = 1;
            fputc('\n', file_out);
            znak = fgetc(file_in);
        }
    }
    fputc('\n', file_out);
}

void drows(FILE *file_in, FILE *file_out, int param1, int param2, const char *delim, char delim_string[], bool multi_character_string) {
    // odstrani riadky N az M (N <= M), ak je N==M tak sa vykona DROW N

    int riadky_counter = 1;
    int znak;
    znak = fgetc(file_in);
    bool printed_delim = false;

    while (znak != EOF){

        if (riadky_counter >= param1 && riadky_counter <= param2) {
            if (znak == '\n') riadky_counter++;
        }else{
            if (znak == '\n'){
                riadky_counter++;
                fputc(znak, file_out);
            }else{

                if (is_delim(delim, &delim_string, &multi_character_string, znak)){
                    fputc(*delim, file_out);
                }else {
                    fputc(znak, file_out);
                }

            }
        }
        znak = fgetc(file_in);
    }
    fputc('\n', file_out);
}

// FUNKCIE NA SPRACOVANIE DAT

void cset(FILE *file_in, FILE *file_out, char string_param[], const char *delim, char delim_string[], bool multi_character_string) {

}
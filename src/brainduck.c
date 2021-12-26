/*

    --- Lince Programming Language ---

    Inspired by Brainfuck

    -- Commands --
    + increment cell value by one
    - decrement cell value by one
    . print cell ascii character
    , retrieve ascii character from input
    [ if current cell value is zero, jump to matching clsojg bracket
    ] jump back to matching opening bracket

    -- Extensions --
    
    Multi-line comments
        Enclosed in round brackets
        Example: 
            (multi-line comment)

    Line comments
        Line starting in a hash #
        Brackets within must be matched
        Example:
            # line comment

    -- Planned extensions --

    Input numerical value
        Similar to ',' but using semicolon ;
        Integer value sought from stdin.
        Hex values also supported if starting with '0x'.
        
    Print numerical value
        Similar to '.' but using colon :
        Integer value of current cell printed to stdout.

    Macros
        Executes piece of code,
        and returns instruction pointer back to caller.
        Declaration starts with exclamation symbol !
        followed by macro name (only [a-z][A-Z] valid),
        and the code to execute between brackets.
            !name{code}
        Macro call starts with the exxclamation followed
        by the function name:
            !name
        Example:
            !copy{
                (Requires three cells: #1, #2, and #3 (temp).
                Copies the value of cell #1 to cell #2,
                preserving the original value of cell #1.)
                [->+>+<<]
                >>[-<<+>>]<<
            }
            +++++!copy
            (
                Initial state: 5 0 0
                Final state:   5 5 0
            )
    
    Import System
        Imports macros from other scripts.
        Called with at @ symbol followed by script name.
            @script
        Recognises scripts with .bf extension only in current directory.
    
    Cell References
        Allows to name cells and move the stack pointer to the specified cell.
        To name the current cell, use ampersand & followed by cell name.
            &cell
        When the variable is next mentioned, the stackpointer will be moved there.
        Example:
            &one (reference to cell #1)
            >>> (move three cells to cell #4)
            &one (moved back to cell #1)

    


    %
    *
    =

*/




#include <stdio.h>

#define STACK_SIZE 1000
#define LINE_MAX 255

//#define DEBUG 1

typedef enum error_code {
    ERR_OK = 0,
    ERR_UNKNOWN_CHAR,
    ERR_MATCHING_BRACKET,
    ERR_BOUNDS,
    ERR_FILE,
    ERR_UNKNOWN
} Error;


char stack[STACK_SIZE] = {0}; // stack buffer
char* stackptr = stack; // stack pointer
unsigned int depth = 0; // counts how many loops the instruction pointer is enclosed in

/* Own implementation to avoid including string.h */
int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void debug_stack(unsigned int max){
    char* ptr = stack;
    unsigned int i;

    // print stack cell numbers
    for(i=0; i!=max; ++i) printf("%03u ", i);
    printf("\n");

    // print stack values
    for(i=0; i!=max; ++i) {
        if (*ptr >= 33 && *ptr <= 126) printf("'%c' ", *(ptr++));
        else printf("%03d ", *(ptr++));
    }
    printf("\n");
    
    // print stack pointer location
    if( (stackptr-stack) < max) printf("%*c^", (int)(stackptr-stack)*4+2, 0);   
    printf("\n");
}

/* Retrieves line from stdin, return buffer must be freed */
char* get_line(char* line){
    size_t max = LINE_MAX;
    if ( getline(&line, &max, stdin) != -1){
        return line;
    }
    return NULL;
}

/* COMMAND: Retrieves a single byte from stdin */
char input_byte(){
    char c, line[LINE_MAX];
    if ( !(get_line(line)) ) return (char)0;
    c = line[0];
    return c;
}

/* COMMAND: Prints a single byte to stdout */
void print_byte(char c){
    putc(c, stdout);
    fflush(stdout);
}


/* Read one char from file without moving file pointer */
int freadc(FILE* file){
    int i = 0;
    i = fgetc(file);
    fseek(file, -1, SEEK_CUR);
    return i;
}

/* Moves file pointer n bytes forward */
int fmove(FILE* file, int n){
    return fseek(file, n, SEEK_CUR);
}

/* Moves file pointer n bytes backward */
int fmoveback(FILE* file, int n){
    if(n < 0) n = -n;
    return fseek(file, -n, SEEK_CUR);
}

/* Move file pointer until either given byte or EOF is found */
int ffindc(FILE* file, char c){
    int ret = 0;
    // note: simplify loop
    while( 1 ){
        ret = freadc(file);
        if (ret == EOF) break;
        else if (ret == c) break;
        fmove(file, 1);
    }
    return ret;
}


int ffindc_matching(FILE* file, char c1, char c2, int direction){
    int depth = 0;
    int v = 0;
    direction = (direction < 0) ? -1 : 1;
    while ( (v = freadc(file)) != EOF ){
        if (v == (int)c1) depth++;
        else if (v == (int)c2){
            if (depth == 1) break;
            else depth--;
        }
        fmove(file, direction);
    }
    return v;
}


///////////////////////////

/*
Loop opening.
If value at current stack location is zero,
skip to the next matching bracket.
Otherwise, execute instructions within.
*/
void jump_forward(FILE* file){ 
    depth++; // watch out! this executes even when relooping;
    if (*stackptr != (char)0) return;    
    fmove(file, 1);

    char c = 0; // current character
    unsigned int current_depth = depth;
    
    // note: simplify this loop
    while( (c = freadc(file)) != EOF ){
        if (c == '[') current_depth++; // new loop found within
        else if (c == ']'){
            if (current_depth == depth){
                // matching bracket found!
                break;
            }
            else{
                // non-matching inner closing bracket
                current_depth--;
            }
        }
        else if (c == '(') ffindc_matching(file, '(', ')', 1);
        fmove(file, 1); // move one file position
    }
}


void jump_backward(FILE* file){ 
    if (*stackptr == (char)0){
        depth--;
        return;
    }
    
    fmoveback(file, 1);

    char c = 0; // current character
    unsigned int current_depth = depth;
    
    // note: simplify this loop
    while(1){
        c = freadc(file);
        if (c == ']') current_depth++; // new loop found within
        else if (c == '['){
            if (current_depth == depth){
                // matching bracket found!
                break;
            }
            else{
                // non-matching inner opening bracket
                current_depth--;
            }
        }
        else if (c == ')') ffindc_matching(file, ')', '(', -1);
        fmoveback(file, 1); // move one file position backwards
    }
    depth--; // when forward bracket is read, depth will be incremented
    fmoveback(file, 1); // ensure next byte read is opening bracket
}



/* Iterates through every byte of the script and executes each command */
Error interpret_file(FILE* file){
    char c = (char)0;
    while((c = freadc(file)) != EOF){
        /* Bounds checking */
        if (stackptr < stack){
            stackptr = stack;
            return ERR_BOUNDS;
        }
        else if (stackptr >= stack + STACK_SIZE){
            stackptr = stack + STACK_SIZE - 1;
            return ERR_BOUNDS;
        }
        /* Read command */
        switch(c){
            /* instructions */
            case '>': ++stackptr;    break;
            case '<': --stackptr;    break;
            case '+': ++(*stackptr); break; 
            case '-': --(*stackptr); break;
            case '.': print_byte(*stackptr);    break;
            case ',': *stackptr = input_byte(); break;
            case '[': jump_forward(file);   break;
            case ']': jump_backward(file);  break;
            /* extra characters */
            case '\n': case '\0':           break; // end of line/string, ignore
            case '(': ffindc_matching(file, '(', ')', 1); break; // comment opening
            case ')': break; // comment close
            case '#': ffindc(file, '\n');   break; // comment line, skip till next newline
            case ' ': case '\r': case '\t': break; // whitespace, simply ignore
            default: return ERR_UNKNOWN_CHAR;
        }
        fmove(file, 1); // jump to next character
    }
    return ERR_OK;
}

// very similar to ffindc_matching
Error check_matching_brackets(FILE* file, char open, char close, int* where){
    int level = 0;
    char c = 0;
    while((c = freadc(file)) != EOF){
        if (c == open) level++;
        else if (c == close) level--;
        (*where)++;
        fmove(file, 1);
    }
    fseek(file, 0, SEEK_SET); // return to start of file
    if(level != 0) return ERR_MATCHING_BRACKET;
    return ERR_OK;
}

Error manage_error(Error err){
    switch(err){
        case ERR_OK: break;
        case ERR_UNKNOWN_CHAR:
            printf( "Error: unknown character\n");
            break;
        case ERR_MATCHING_BRACKET:
            printf( "Error: missing matching bracket\n");
            break;
        case ERR_BOUNDS:
            printf( "Error: stack pointer out of bounds\n");
            break;
        case ERR_FILE:
            printf( "Error: could not open file\n");
            break;
        case ERR_UNKNOWN: default:
            printf( "Error: unknown error\n");
            break;
    }
    return err;
}


Error readfile(const char* filename){
    /* Open script */
    Error err = ERR_OK;
    FILE* file = NULL;
    file = fopen(filename, "rb");
    if(!file){
        printf( "Error: Unable to open file '%s'\n", filename);
        return ERR_FILE;
    }

    /* Check for unmatched brackets */
    int where_err = 0;
    if ( check_matching_brackets(file, '[', ']', &where_err) != ERR_OK
         || check_matching_brackets(file, '(' , ')', &where_err) != ERR_OK
       ){
        printf( "Error: missing matching bracket at character %d\n", where_err);
        fclose(file);
        return ERR_MATCHING_BRACKET;
    }
    
    /* Read and execute commands */
    err = interpret_file(file);
    manage_error(err);
    fclose(file);
    return err;
}


int main(int argc, char* argv[]){
     
    if (argc < 2){
        printf("Error: no input file specified.\n");
        return ERR_FILE;
    }
    int code = readfile(argv[1]);    
    
    if (argc >= 3 && strcmp(argv[2], "--debug") == 0){
        printf("\n --- Stack debug mode ---\n");
        debug_stack(10);
    }
    return code;
}





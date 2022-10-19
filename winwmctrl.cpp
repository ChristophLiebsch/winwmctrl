
#include <windows.h>
#include <locale.h>
#include <stdio.h>


void Error(const char * const lpszFunction)
{ 
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
    

    printf("%s  - failed with error %d: %s\n", 
        lpszFunction, dw, lpMsgBuf); 

    LocalFree(lpMsgBuf);

}

static char * getWindowTitle(HWND hWnd){
  int windowTitleLenght = 0;
  LPSTR windowTitle;

  windowTitleLenght = GetWindowTextLength(hWnd);
  
  if (windowTitleLenght == 0)  return NULL;

  windowTitle = (LPSTR) malloc((windowTitleLenght+1)*sizeof(char));
  GetWindowText(hWnd,windowTitle, windowTitleLenght+1);

  return windowTitle;

}

static void getWindowsProcessIdWhenOptionSet(HWND hWnd, DWORD * processId){
  DWORD threadID;

  threadID = GetWindowThreadProcessId( hWnd, processId );

}

static BOOL CALLBACK PrintWndHandleCallback(HWND hWnd, LPARAM lparam) {

    LPSTR windowTitle;
    DWORD processId;
    TPrintingOptions * printingOptions = (TPrintingOptions *)lparam;
 
    // List visible windows with a non-empty title
    if (IsWindowVisible(hWnd)) {

      windowTitle = getWindowTitle(hWnd);
      
      if (windowTitle != NULL){ 

        getWindowsProcessIdWhenOptionSet(hWnd, &processId);


        //TODO auslagern in Zeiger-Methoden, die alle ein Stück drucken (wahrscheinlich dann ohne printf)
        wprintf(L"WinHandle:%d\tWinTitle:%s\tprocessID:%d\n",hWnd, windowTitle,processId);
      }
      free(windowTitle);
    }
    return TRUE;
}


/* ToDo:
Array mit Actions
Eine Action ist ein Struct mit Kommandzeilen-Buchstabe, 
  einem Bool-Wert für den Treffer, 
  einem Verweis auf die Methode mit Code, 
  einen Boolwert ob die Option dahinter einen Value braucht (oder am Ende)
  und ein Verweis auf das Option-Array
Arrays mit Options die zu Actionen gehören
Eine Option ist ein Struct mit Kommandozeilen-Buchstabe, 
  einem Bool-Wert für den Treffer,
  einen Boolwert ob die Option dahinter einen Value braucht (oder am Ende)
  und drei Verweis auf Daten oder Methoden, die für die Option notwendig sind (z.B. ein String mit einem Formatkennzeichner oder ein Verweise auf eine Methode)
*/

/* needsValue */
#define NV_NO 0
#define NV_NEXT 1
#define NV_LAST 2

typedef int(*pfunc_optionMethod)(TOption option, LPARAM lparam); 

typedef struct StructOption
{
  char letter;
  BOOL set = FALSE;
  int needsValue = NV_NO;
  pfunc_optionMethod method;
} TOption;

typedef int(*pfunc_actionMethod)(TOption * options); 

typedef struct StructAction
{
  char letter;
  BOOL set = FALSE;
  pfunc_actionMethod method;
  int needsValue = NV_NO;
  TOption * optionArray;
} TAction;






/*********** PROPERTIES ******************/

/* Configuration for interpreting command line options given by users */
/* Actions */
int actionMethod_l(TOption * options);

TAction action_l = {
  'l',
  FALSE,
  &actionMethod_l,
  NV_NO,
  optionsFor_l
};




/* Optoins */
TOption option_p = {
  'p',
  FALSE,
  NV_NO,
  &optionMethod_p
};

typedef struct StructOptionsForAction_l
{
  BOOL printProcessId = false;
  BOOL printGeometry = false;
} TPrintingOptions;

int optionMethod_p(TOPtion option, LPARAM param);

/* Arrays of Actions and Options */
#define NUM_OPTIONS_L 1
TOption optionsFor_l[NUM_OPTIONS_L] = {option_p};


#define NUM_ACTIONS 1
TAction all_actions[NUM_ACTIONS] = {action_l};

#define NUM_OPT_ARRAYS 1
TOption * all_option_arrays[NUM_OPT_ARRAYS] = {optionsFor_l};


/*********** PROPERTIES - ENDE ***********/

BOOL callOptionMethods(TOption * options, LPARAM lparam){

  for(int i = 0; i <= NUM_OPTIONS_L; ++i){
    options[i].method(options[i],lparam);
  }

}

int actionMethod_l(TOption * options){
  TPrintingOptions printingOptions;
  LPARAM lparam;
  lparam = (LPARAM) &printingOptions;

  callOptionMethods(options, lparam);
  printf("in actionMethod_l\n");
  
  EnumWindows(PrintWndHandleCallback, lparam); 
  return 0;
}

int optionMethod_p(TOption option, LPARAM lparam){
  
  TPrintingOptions * printingOptions;
  printingOptions = (TPrintingOptions *) lparam;

  printingOptions->printProcessId = true;
  
  printf("in optenMethod_p\n");
  return 0;
}

char retrieveSingleLetterFromCommandLineArg(char * argument){
  if (argument[0] == 0)  return 0;
  if (argument[0] == '-' && argument[2] == 0)  return argument[1]; else return 0;
}

BOOL markActionLettersAsSet(char letter, TAction * actions){

  for(int i = 0; i <= NUM_ACTIONS; ++i){
    if (letter == actions[i].letter){
      actions[i].set = true;
      return true;
    }
  }
  return false;
}

BOOL markOptionLettersAsSet(char letter, TOption ** all_options){

  for(int i = 0; i <= NUM_OPT_ARRAYS; ++i){
    for(int j = 0; j <= NUM_OPTIONS_L; ++j){
      if (letter == all_options[i][j].letter){
       all_options[i][j].set = true;
        return true;
      }
    }
  }
  return false;
}

void iterateCommandLine(int argc, char ** argv, TAction * actions, TOption ** all_options){

  char singleLetter;

  for(int i = 1; i < argc; ++i) {
    printf("parameter: %s\n",argv[i]);

    singleLetter = retrieveSingleLetterFromCommandLineArg(argv[i]);

    if (markActionLettersAsSet(singleLetter, actions)) continue;
      
    if (markOptionLettersAsSet(singleLetter, all_options)) continue;
      
  }

  callActionMethods(actions);

}

void callActionMethods(TAction * actions){
  
  for(int i = 0; i <= NUM_ACTIONS; ++i){
    if (actions[i].set){
      
      actions[i].method(actions[i].optionArray);
      break;
    }
  }
}



void interateOptionsAndCallMethod(TOption ** all_options, LPARAM lparam){

  for(int i = 0; i <= NUM_OPT_ARRAYS; ++i){
    for(int j = 0; j <= NUM_OPTIONS_L; ++j){
      if (all_options[i][j].set){
        all_options[i][j].method();
        break;
      }
    }
  }
}

int main(int argc, char *argv[]){

  LPARAM lparam;
  setlocale(LC_ALL,"");

  printf("Anfang Function main\n");
  iterateCommandLine(argc, argv, all_actions, all_option_arrays);
  printf("all_actions[0].set: %d optionsFor_p[0]: %d\n\n",all_actions[0].set,all_option_arrays[0][0].set);

  // temporay switched off
  //EnumWindows(PrintWndHandleCallback, lparam);

  return 0;
}









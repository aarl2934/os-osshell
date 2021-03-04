#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <filesystem>
#include <sys/wait.h>

void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);
void spawnProcess(const char* path, char** command_list);
std::vector<std::string> readHistoryFile();
void readHistoryResults(std::vector<std::string> history, int input);
void addToHistoryFile(char command[]);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    std::vector<std::string> os_path_list;
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);

    
    /************************************************************************************
     *   Example code - remove in actual program                                        *
     ************************************************************************************/
    // Shows how to loop over the directories in the PATH environment variable
    int i;
    for (i = 0; i < os_path_list.size(); i++)
    {
        printf("PATH[%2d]: %s\n", i, os_path_list[i].c_str());
    }
    /************************************************************************************
     *   End example code                                                               *
     ************************************************************************************/


    // Welcome message
    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    std::vector<std::string> command_list; // to store command user types in, split into its variour parameters
    char **command_list_exec; // command_list converted to an array of character arrays
    // Repeat:
    
    while(true){
        
    
        std::cout << "osshell>";
        std::cin.sync();
        char command[256];
        std::cin.getline(command, 256);
        //std::cout << std::endl;
        bool found = false;
        splitString(command, ' ', command_list);
        vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);

        if(command_list.size() == 0){

        }else if(command_list[0] == "exit"){
            addToHistoryFile(command);
            freeArrayOfCharArrays(command_list_exec, command_list.size());
            exit(0);
        }
        else if(command_list[0] == "history"){
            
            if(command_list.size() > 1){ //Check for a second argument.
                if(command_list[1] == "clear"){ //if the second argument is clear, use ofstream trunc to wipe the history file.
                    std::ofstream clearfile;
                    clearfile.open("history.txt", std::ofstream::out | std::ofstream::trunc);
                    clearfile.close();

                } else { //otherwise the second argument has to be numeric, so we read the file and print the results to however many lines we need.
                    std::vector<std::string> contents = readHistoryFile();
                    readHistoryResults(contents, std::stoi(command_list[1]));
                }
            } else { //if no second argument exists print the whole file (by passing -1 to readHistoryResults.)
                std::vector<std::string> contents = readHistoryFile();
                readHistoryResults(contents, -1);
            }

            //End of History functionality

        }else{
            for(int i = 0; i < os_path_list.size() && !found; i++){
                for(auto &file : std::filesystem::directory_iterator(os_path_list[i].c_str())){
                    if(file.path().filename().string() == command_list[0]){
                        spawnProcess(file.path().string().c_str(), command_list_exec);
                        found = true; // set the found variable to true so it won't look for more programs.
                        break;
                    }
                    
                }
            }
            
            if(!found){
                printf("%s: Error command not found\n", command_list_exec[0]);
            }
        }
        
        addToHistoryFile(command);

    }
    
    //  Print prompt for user input: "osshell> " (no newline)
    //  Get user input for next command
    //  If command is `exit` exit loop / quit program
    //  If command is `history` print previous N commands
    //  For all other commands, check if an executable by that name is in one of the PATH directories
    //   If yes, execute it
    //   If no, print error statement: "<command_name>: Error command not found" (do include newline)


    /************************************************************************************
     *   Example code - remove in actual program                                        *
     ************************************************************************************/
    // Shows how to split a command and prepare for the execv() function
    std::string example_command = "ls -lh";
    splitString(example_command, ' ', command_list);
    vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
    // use `command_list_exec` in the execv() function rather than looping and printing
    i = 0;
    while (command_list_exec[i] != NULL)
    {
        printf("CMD[%2d]: %s\n", i, command_list_exec[i]);
        i++;
    }
    // free memory for `command_list_exec`
    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    printf("------\n");

    // Second example command - reuse the `command_list` and `command_list_exec` variables
    example_command = "echo \"Hello world\" I am alive!";
    splitString(example_command, ' ', command_list);
    vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
    // use `command_list_exec` in the execv() function rather than looping and printing
    i = 0;
    while (command_list_exec[i] != NULL)
    {
        printf("CMD[%2d]: %s\n", i, command_list_exec[i]);
        i++;
    }
    // free memory for `command_list_exec`
    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    printf("------\n");
    /************************************************************************************
     *   End example code                                                               *
     ************************************************************************************/


    return 0;
}

void spawnProcess(const char* path, char** command_list){
    int pid = fork();
    if(pid == 0){
        execv(path, command_list);
        //perror(0);
        exit(0);
    }else if(pid == -1){
        std::cout << "ERROR" << std::endl;
    }else{
        int status;
        waitpid(pid, &status, 0);
    }
   
}


/*
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string>& result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

/*
   list: vector of strings to convert to an array of character arrays
   result: pointer to an array of character arrays when the vector of strings is copied to
*/
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result)
{
    int i;
    int result_length = list.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < list.size(); i++)
    {
        (*result)[i] = new char[list[i].length() + 1];
        strcpy((*result)[i], list[i].c_str());
    }
    (*result)[list.size()] = NULL;
}

/*
   array: list of strings (array of character arrays) to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        if (array[i] != NULL)
        {
            delete[] array[i];
        }
    }
    delete[] array;
}

//Start of History helper methods (Jonas' stuff)

void addToHistoryFile(char command[]){
        //record command in history file last 
        if(command[0] != NULL && strstr(command, "clear") == NULL){
            std::ofstream history;
            history.open("history.txt", std::ofstream::out | std::ofstream::app);
            if(history.is_open()){
                history << command << std::endl;
            }
            history.close();
        }
}

std::vector<std::string> readHistoryFile(){ //Always returns a vector with a full copy of the history file.
    std::ifstream hs ("history.txt");
    std::vector<std::string> historyfileContents;

    if(hs.is_open()){
        std::string line;
        while(std::getline(hs, line)){
            historyfileContents.insert(historyfileContents.end(), line); //read lines in reverse for proper ordering
        }
    }

    hs.close();

    return historyfileContents;
}

/* 
    Helper method for history. Takes the full file and a numeric input for how many lines to read.
    If input = -1, it reads the entire file. 
*/
void readHistoryResults(std::vector<std::string> history, int input){
    if(input == -1){
        int j = 1;
        for(auto i = history.begin(); i != history.end(); ++i){
            std::cout << "  " << j << ": " << *i << '\n';
            j++;
        }
    } else {
        for(int i = history.size() - (input); i < history.size(); i++){
            std::cout << "  " << i + 1 << ": " << history.at(i) << '\n';
        }
    }  
}

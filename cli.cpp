#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>

using namespace std;

void executeCommand(const string &command, const string &input, const string &option, const string &redirection, const string &backgroundJob);

void parseCommand(const string& commandLine, ofstream& outputFile) {
    outputFile << "----------" << endl;
    cout << "----------" << endl;


    istringstream iss(commandLine);
    vector<string> tokens;
    string token;

   
    while (iss >> token) {
        tokens.push_back(token);
    }

    
    if (!tokens.empty()) {
        string commandName = tokens[0];
        outputFile << "Command : " << commandName << endl;
        cout << "Command : " << commandName << endl;


        
        string inputValue;
        string optionValue;
        string redirectionValue = "-";
        string backgroundJob = "n";

       
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i] == "<" && i - 1 < tokens.size()) {
                
                inputValue = tokens[i - 1];
                redirectionValue = "<";
            } else if (tokens[i] == ">" && i - 1 < tokens.size()) {
                
                inputValue = tokens[i - 1];
                redirectionValue = ">";
            } else if (tokens[i][0] == '-' && i - 1 < tokens.size()) {
            
                optionValue = tokens[i];
            } else if (tokens[i] == "&") {
                
                backgroundJob = "y";
            }
        }

        
        if (!inputValue.empty()) {
            outputFile << "Input : " << inputValue << endl;
            cout << "Input : " << inputValue << endl;
        }
        if (!optionValue.empty()) {
            outputFile << "Option : " << optionValue << endl;
            cout << "Option : " << optionValue << endl;
        }
        outputFile << "Redirection : " << redirectionValue << endl;
        cout << "Redirection : " << redirectionValue << endl;
        outputFile << "Background Job : " << backgroundJob << endl;
        cout << "Background Job : " << backgroundJob << endl;
    
        executeCommand(commandName, inputValue, optionValue, redirectionValue, backgroundJob);
    } 

    
}


void executeCommand(const string &command, const string &input, const string &option, const string &redirection, const string &backgroundJob) {
    pid_t pid = fork();

    if (pid == -1) {
        cerr << "Error forking process" << endl;
    } else if (pid == 0) {
        // Child process
        if (redirection == "<" && !input.empty()) {
            freopen(input.c_str(), "r", stdin);
        }

        // Execute the command
        execlp(command.c_str(), command.c_str(), option.c_str(), (char *)NULL);

       
        cerr << "Error executing command: " << command << endl;
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        if (backgroundJob != "y") {
            // Wait for the child process to complete if it's not a background job
            waitpid(pid, NULL, 0);
        }
    }
}

int main() {
    
    ifstream inputFile("commands.txt");
    if (!inputFile) {
        cerr << "Error: Unable to open commands.txt" << endl;
        return 1;
    }

    ofstream outputFile("parse.txt");
    if (!outputFile) {
        cerr << "Error: Unable to open parse.txt for writing" << endl;
        return 1;
    }

    
    string line;
    while (getline(inputFile, line)) {
        // Parse and print command information for each line
        parseCommand(line, outputFile);

    
    }


    outputFile << "----------" << endl; 
    cout << "----------" << endl;


    outputFile.close();


    return 0;
}

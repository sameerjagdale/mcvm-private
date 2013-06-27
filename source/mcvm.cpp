#include <string>
#include <iostream>
#include "mcvm.h"
#include "configmanager.h"
#include "mcvmstdlib.h"
#include "interpreter.h"
#include "jitcompiler.h"
#include "profiling.h"
#include "filesystem.h"
#include "parser.h"
#include "utility.h"
#include "client.h"

void initialize(int argc, char** argv) 
{
    llvm::InitializeNativeTarget();
    ConfigManager::initialize();
    Interpreter::initialize();
    JITCompiler::initialize();
    Profiler::initialize();
    ConfigManager::parseCmdArgs(argc, argv);
    Client::openSocketStream(Client::FRONTEND_DEFAULT_HOST, Client::FRONTEND_DEFAULT_PORT);
    mcvm::stdlib::loadLibrary();
}

void run() {

    std::cout << "*******************************************************" << std::endl;
    std::cout << "         McVM - The McLab Virtual Machine v1.0         " << std::endl;
    std::cout << "Visit http://www.sable.mcgill.ca for more information. " << std::endl;
    std::cout << "*******************************************************" << std::endl;
    std::cout << std::endl;

    // Change to the starting directory
    bool cdResult = setWorkingDir(ConfigManager::s_startDirVar.getStringValue());

    // If the directory change failed
    if (cdResult == false)
    {
        // Print out a warning message
        std::cout << "WARNING: could not change to specified starting directory" << std::endl;
    }

    // If the target file name was set
    if (ConfigManager::getFileName() != "")
    {
        // Setup a try block to catch errors
        try
        {
            // Attempt to run the target file as an M-file
            Interpreter::callByName(ConfigManager::getFileName());
        }

        // If any error occurs
        catch (RunError error)
        {
            // Print the run-time error
            std::cout << std::endl << "Run-time error: " << std::endl << error.toString() << std::endl;
        }
    }

    // Otherwise, by default
    else
    {
        // Run the read-eval-print loop
        runREPLoop();
    }

}

void shutdown() {
    JITCompiler::shutdown();
    Client::shutdown();
}

void runREPLoop()
{
    // Declare a variable to store the current nesting level
    size_t nestLevel = 0;

    // Declare a variable to store the command string
    std::string commandStr = "";

    // Repeat for each command
    for (;;)
    {
        // Declare a buffer to store the input
        char lineBuffer[2048];

        // If the nesting level is 0
        if (nestLevel == 0)
        {
            // Print the prompt symbol
            std::cout << ">: ";
        }

        // Read one line from the input
        std::cin.getline(lineBuffer, sizeof(lineBuffer));

        // Build a string object from the line buffer
        std::string line = lineBuffer;

        // Add this line to the command string
        commandStr += line + "\n";

        // Tokenize the line
        std::vector<std::string> tokens = tokenize(line, " \t\r\n", false, false);

        // Extract the first token
        std::string firstToken = (tokens.size() != 0)? tokens[0]:"";

        // If this is the start of a new code block
        if (firstToken == "if" ||
                firstToken == "switch" ||
                firstToken == "for" ||
                firstToken == "while" ||
                firstToken == "function")
        {
            // Increase the nesting level
            nestLevel += 1;
        }

        // Otherwise, if this is the end of a code block
        else if (firstToken == "end")
        {
            // Decrease the nesting level
            nestLevel -= 1;
        }

        // If the nesting level is now 0
        if (nestLevel == 0)
        {
            // Setup a try block to catch errors
            try
            {
                // Trim whitespace characters from the command string
                commandStr = trimString(commandStr, " \t\r\n");

                // Attempt to run the command string
                Interpreter::runCommand(commandStr);
            }

            // If any error occurs
            catch (RunError error)
            {
                // Print the run-time error
                std::cout << std::endl << "Run-time error: " << std::endl << error.toString() << std::endl;
            }

            // Clear the command string
            commandStr = "";
        }

        // If eof was encountered
        if (std::cin.eof())
        {
            // Print an end of line
            std::cout << std::endl;

            // Break out of the REP loop
            break;
        }
    }
}

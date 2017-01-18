#ifndef OPENGLWINDOW_CONSOLEAUTOEXECMANAGER_H
#define OPENGLWINDOW_CONSOLEAUTOEXECMANAGER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ConsoleCommandManager;

/**
* Manages automatically executing commands by reading an autoexec file
*
* The autoexec file is a plaintext file where each line is a command
* to execute. Blank lines are allowed and lines can be commented with //
*
* The layout of the autoexec file will be preserved, and any NEW watches
* will be added last
*
* \see AddAutoexecWatch for information regarding watches
*/
class ConsoleAutoexecManager
{
public:
	ConsoleAutoexecManager();
	~ConsoleAutoexecManager() = default;

	void Init(ConsoleCommandManager* commandManager);

	/**
	* Called whenever a function is executed so it can be added to the autoexec file if needed
	*
	* Example: FunctionExecuted("FunctionCall", "arg1, arg2, arg3")
	* 
	* \param function the executed function
	* \param arguments the function's arguments without parameters, but with commas
	*/
	void FunctionExecuted(const std::string& function, const std::string& arguments);

	/**
	* Adds the given command to autoexec watches.
	*
	* After a command is added to autoexec watches it will be added to the autoexec file 
	* with the parameters that were last used when calling it. Add "watch" in front of
	* a command in the autoexec file to automatically start watching it when the program
	* starts. Any commands added through AddAutoexecWatch will be watched until
	* RemoveAutoexecWatch is called - even between sessions
	*
	* Example: 
	* +  `AddAutoexecWatch("SomeVariable", commandManager)`
	* Adds the given ConsoleVariable to watches, which means ConsoleVariable::GetValue 
	* will be called when writing "SomeVariable" to autoexec
	*
	* + `AddAutoexecWatch("SomeFunction", commandManager)`
	* Adds the given ConsoleCommand to watches, which means SomeFunction will
	* be monitored, and the last time it is called, its arguments will be written to autoexec.
	*
	*
	* Example:
	* \code
	* AddAutoexecWatch("SomeFunction", commandManager);
	* ExecuteFunction("SomeFunction(3, 2, 1)");
	* ExecuteFunction("SomeFunction(1, 2, 3)");
	* \endcode
	* Will write "Somefunction(1,2,3)" to autoexec
	*
	* \code
	* ExecuteFunction("SomeFunction(1, 2, 3)");
	* AddAutoexecWatch("SomeFunction", commandManager);
	* \endcode
	* Will write "SomeFunction()" to autoexec
	*
	* \param command command to watch, can be either a ConsoleVariable or a ConsoleCommand
	* \param commandManager the ConsoleCommandManager where the command is located
	* \returns true if the command was successfully added, false if the command either
	* didn't exist, or if it was already added.
	*/
	bool AddAutoexecWatch(const std::string& command);
	/**
	* Removes the given command from autoexec watches
	*
	* Example:
	* 
	* \param command command to remove, can be either a ConsoleVariable or a ConsoleCommand
	* \returns true if the command was removed, false if the command didn't exist
	*/
	bool RemoveAutoexecWatch(const std::string& command);

	/**
	* Returns the names and arguments of watches
	*
	* The vector contains a pair where `pair.first` contains the command, and `pair.second`
	* contains the arguments. `pair.second` does not contain parentheses
	*
	* Example:
	* \code
	* for(auto pair : GetWatches())
	*     std::cout << pair.first << "(" << pair.second << ")\n";
	* \endcode
	* Possible output:
	* \code
	* Foo(1, 2)
	* Bar(3, 4)
	* \endcode
	* 
	* \returns a vector containing an std::pair containing <function, arguments>
	*/
	std::vector<std::pair<std::string, std::string>> GetWatches();
	/**
	* Returns the names and arguments of variables (all ConsoleVariable) watches
	*
	* \see GetWatches for more comments
	* 
	* \returns a vector containing an std::pair containing <function, arguments>
	*/
	std::vector<std::pair<std::string, std::string>> GetVariableWatches();
	/**
	* Returns the names and arguments of function (all watches excluding ConsoleVariable) 
	* watches as a vector
	*
	* \see GetWatches for more comments
	*
	* \returns a vector containing an std::pair containing functions, arguments
	*/
	std::vector<std::pair<std::string, std::string>> GetFunctionWatches();

	/**
	* Parses the autoexec file at \p path and adds all commands into \p commandManager
	* as well as executing them 
	* 
	* \throws std::invalid_argument if ConsoleCommandManager::ExecuteFunction throws
	* \throws std::runtime_error if ConsoleCommandManager::ExecuteFunction throws any exception
	* that isn't an std::invalid_argument
	* \param path path (including extension) to autoexec file
	* \param commandManager ConsoleCommandManager to execute commands and add them too
	* \returns false if the file at \p path couldn't be opened
	*/
	bool ParseAutoexec(const std::string& path, ConsoleCommandManager& commandManager);
	/**
	* Writes all autoexec watches to the file at \p path
	* 
	* \param path path (including extension) to autoexec file
	* \returns false if the file at path cannot be opened, or a file at path + ".tmp"
	* can't be created
	*/
	bool WriteAutoexec(const std::string& path);

	/**
	* Pauses autoexec watches
	* 
	* Watches can be applied with ApplyPausedWatches() or ApplySelectedPausedChanges()
	*/
	void PauseAutoexecWatching();
	/**
	* Resumes autoexec watches
	*
	* \see ApplySelectedPausedChanges() to be able to choose which changes are applied
	*
	* \param applyChanges whether or not to apply \b ALL changed parameters since pausing
	*/
	void ResumeAutoexecWatching(bool applyChanges);

	/**
	* Applies all changes since autoexec watching was paused
	*/
	void ApplyPausedChanges();

	/**
	* Applies the given paused watches
	*
	* \param watches name of which watches to apply
	* \returns a list of which variables \b couldn't be applied (non-existent name)
	*/
	std::vector<std::string> ApplySelectedPausedChanges(const std::vector<std::string>& watches);
private:
	ConsoleCommandManager* commandManager;

	//<function, arguments>
	std::unordered_map<std::string, std::string> autoexecWatchesVariables;
	std::unordered_map<std::string, std::string> autoexecWatchesFunctions;

	std::unordered_map<std::string, std::string> pausedAutoexecWatchesVariables;
	std::unordered_map<std::string, std::string> pausedAutoexecWatchesFunctions;

	std::unordered_set<std::string> removedWatches;

	bool autoexecLoaded;
	bool autoexecFound;

	bool paused;

	/**
	* \see ConsoleCommandManager::TrimTextFrontBack for documentation
	*/
	std::string TrimTextFrontBack(const std::string& text);
};

#endif //OPENGLWINDOW_CONSOLEAUTOEXECMANAGER_H

Name:
Sish - C++ Shell

Synopsis:
sish [-x] [-d <level>] [-f file [arg]...]

Description:
sish is a shell written in C++ to provide basic shell functionality.  Arguements can be specified to provide debugging information or file input/output.  Additionally, unreconized commands specified will be executed in the shell that started sish.  

Arguement List Processing:

-x
The command to be executed is displayed before execution.  If the command contains variable substitution the substitution will be done before execution.

-d <level>
Arguement enables the output of debug messages.  If level 0 is specified, no debug information is provided.  Higher levels will provide more information.

-f file [Arg]
Arguement allows input to be provided from a file instead of the command line.


Startup and shutdown:
The shell can be compiled by running the make command.  Once compiled, the shell can be started by running ./sish.  To shutdown the shell, use the exit command.


Built-In Commands:

show W1 W2 ...:
Displays the words specified.

set W1 W2:
Sets the local variable with the name W1 to the value W2.  If the variable does not exist, it will be created. If it does exist, the current value will be overwritten.

unset W1:
Removes a local variable W1 set by the set command.  If it does not exist, an error message will be displayed.

export W1 W2:
Sets a global variable with the name W1 to the value W2.  If the variable does not exist, it will be created.  If it does exist, the current value will be overwritten.

unexport W1:
Removes a global variable W1 set by the export command.  If it does not exist, an error message will be displayed.

environ:
Displays all of the global and local variables currently defined.

chdir W:
Changes the current directory to the directory specified by W.  

exit i:
Exits the shell and returns the status i.  If no exit code is specified, it returns 0.

wait l:
The shell waits for the specified process to complete.

clr:
Clears the current command prompt screen.

dir:
Lists the contents of the current directory.

echo <comment>:
Same functionality as show.

help:
Displays the user manual.

pause:
Pauses the shell until the ENTER key is pressed.

history n:
Displays all of the commands that have been executed.  If n is specified, will only display the n last commands.

repeat n:
Repeats the command that was specified by n.  If n is not specified, the last command will be executed.  Before execution, the command will be printed.

kill [-n] pid: 
Kills the specified process.

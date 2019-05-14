# UNIX-homeMadeShell
Made a shell using C in UNIX
Supports the following commands:

1. Running an app from a given path with the command, for example "bin/ls -1"

2. Same as 1, but if there's a & in the end of the line, it runs on the background and you can keep using the homemade shell.

3. Given the command "tasks", you can see which apps are running right now on background.

4.Given the command "return [number]" it will make the homemade shell to wait till the given app has ended.
For example: "return 1921"

5. Redirect an output of a given app to stdout.
For example: "/usr/ls -l -t -a > output"

6. Initializing environment variables.
For example: "KEY=VALUE"

7. Redirect to stdin, for example: "/funapp.c < makenumers.c"

# StepEntityTreeDisplayer by Antonin Lionnet
Parses a STEP-encoded file and display recursively all entities and entities that they reference on the standard output. It doesn't care about the EXPRESS Schema that is explictly or implicitly used, as it just requires for the file to follow STEP standard.
Analyzed files should be encoded in STEP, and more precisely standard ISO-10303-21

This is thought as a small help to people who start working with STEP files and need a more global picture of the structure of a given file, since it can be hard to picture the global structure when looking at the raw file.

I am not an expert of STEP format, so it might not work on all cases, but it can be a nice help for files that are not too complicated (a few hundred lines). You can in theory use it for large files but the output may be quite unintelligible.

## Usage
To use it, simply pass the name of the file to read as a command line argument. On Visual Studio, you might need to use a Debug configuration so that the console does not close by itself.

## Limits
SetD will not display  any member variables contained by the entities, it will only display references to other entities, whether they are directly a parameter, or in a list in a list in another list, without any visual distinction.

For files with a few thousands or more entities, your standard output may not have enough lines to display all of them (And even then it is quite hard to find what you are looking for).
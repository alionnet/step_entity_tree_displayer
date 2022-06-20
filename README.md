# StepEntityTreeDisplayer by Antonin Lionnet
Parses a STEP-encoded file and display recursively all entities and entities that they reference on the standard output. It doesn't care about the EXPRESS Schema that is explictly or implicitly used, as it just requires for the file to follow STEP standard.
Analyzed files should be encoded in STEP, and more precisely standard ISO-10303-21.

This is thought as a small help to people who start working with STEP files and need a more global picture of the structure of a given file, since it can be hard to picture the global structure when looking at the raw file.

I am a beginner with STEP format, so it might not work on all cases, but it can be a nice help for files that are not too complicated (a few hundred lines). You can in theory use it for large files but the output may be quite unintelligible.

## Usage
To use it, simply pass the name of the file to read as a command line argument. On Visual Studio, you might need to use a Debug configuration so that the console does not close by itself.
```./SetD.exe filepath [options]```

### Options
Options can be passed as arguments after the filename. Currently available options are:
- ```-d maxDepth```: Limits the depth of the displayed tree to reduce the number of line on large files and ease analysis. 
- ```-n | --noDuplicates```: Prevents the same entity from being developped more than once, to shrink the output on larger files.
- ```-i | --filterID  id```: Only displays the trees in which the given entity is referenced.
- ```-t | --filterType  type```: Only displays the trees from entities of the given type.
- ```-T | --filterTypesOut```: Will not display a set of given types. The set is currently not passed as command line, and instead defined and filled in ```code\Options.cpp```, where you can edit it to your convenience. 

Internal treatment and parsing are unchanged for the aforementionned options, only the output is.

- ```-o | --onlyTypes```: Displays only the entity types used in the file. Fastens treatement by skipping parsing and storage of entity references. Useful to quickly get a grasp on what is used in the file or to find which type you don't know the role of. Ignores all other filters.

## Limits
SetD will not display any member variables contained by the entities, it will only display references to other entities, whether they are directly a parameter, or in a list in a list in another list, without any visual distinction.

Regarding complex entities, SetD will only display the entities directly referenced in the subentity. It has no way of knowing the parent subentity, so the output will be yours to interpret on this part.

References are displayed in numerical order, and not in the order they actually appear in the STEP file. This is due to the use of sets and maps as a way to provide quicker access to information, since indexes would not mean anything in a vector.

For files with a few thousands or more entities, your standard output may not have enough lines to display all of them (And even then it is quite hard to find what you are looking for). I would advise that you redirect the output to a text file so everything is available.

Optimization measures have been taken, so SetD should be usable on large files without requiring an unreasonnable amount of time. From my testing, a file of 80 000 lines took around 15 seconds to compute, but if you go too far above I can't guarantee a 'fast' output. Keep in mind that a huge output will also take some time to write.
# Coding Style Checklist

Based on google c++ style guide https://google.github.io/styleguide/cppguide.html

##### Brace placement
* [ ] All functions, `if` blocks and loops have braces
  - Exception: No braces are required for _very_ short `if` or loop statements on _one line_
* [ ] Opening braces are directly below the `if`/`for`/`while`/`function`

##### Naming convention

* [ ] Only use common abbreviations; otherwise spell it out
    - (rule of thumb: if it's listed on Wikipedia)
* [ ] Classes and functions follow the format `ThisIsMyClassOrFunction`
* [ ] Variables follow the format `my_variable`
* [ ] Class member variables have a trailing underscore: `class_variable_`
* [ ] Source and header files follow the format `source_file.cpp` `header_file.h`

##### Standard types
* [ ] Use the types below, especially for function parameters and class variables
    - Exception: use smaller types for very large arrays
    - Exception: Match Ch. 10 data types where applicable
```c++
	uint64_t -> when you need unsigned int
	int      -> when you need a signed int
	double	 -> when you need a float
	string	 -> when you need a string
	size_t	 -> when you need an index
	bool	 -> when you need boolean
```

##### Comments
* [ ] Comment all public variables and functions (in the header file)
* [ ] Comments for public member functions follow the format below

```c++
    /*
        Description of what it does
        Inputs:    <input parameter 1>   ->                       
                  <input parameter 2>    ->  
        Returns:           
    */
```

##### General
* [ ] 80 characters per line (whenever possible)
* [ ] Put all class functions and variables that don't need to be public in private
* [ ] Try to increase speed for massively repeated code
* [ ] Restructure code to be more testable (functions working as independently as possible) 
* [ ] Restructure code to be more readable
* [ ] Maintain data type consistency (general guideline shown below)

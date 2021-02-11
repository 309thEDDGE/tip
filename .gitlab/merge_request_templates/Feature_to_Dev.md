## Closes issue #XX

* [ ] TDD used for development
* [ ] Meets acceptance criteria for related issue
* [ ] Passes pipeline
* [ ] Passes end-to-end with translation on IL4 data
* [ ] Reviewed and approved by a Maintainer
  - [ ] Code is testable
  - [ ] Unit tests are complete
  - [ ] Code is thoroughly commented
  - [ ] Code follows our coding guide below
---

### Coding Style

Based on Google's _C++ Style Guide_, https://google.github.io/styleguide/cppguide.html

##### Brace placement
* [ ] All functions, `if` blocks and loops have braces
  - Exception: No braces are required for _very_ short `if` or loop statements on _one line_
* [ ] Opening braces are directly below the `if`/`for`/`while` or function

##### Naming convention

* [ ] Only use common abbreviations; otherwise spell it out
    - (rule of thumb: if it's listed on Wikipedia)
* [ ] Classes and functions follow the format `ThisIsMyClassOrFunction`
* [ ] Variables follow the format `my_variable`
* [ ] Class member variables have a trailing underscore: `class_variable_`
* [ ] Source and header files follow the format `my_class.cpp` and `my_class.h`

##### Standard types
* [ ] Use the types in the sample code below, especially for function parameters and class variables

##### Comments
* [ ] Comment all public variables and functions (in the header file)
* [ ] Comments for public member functions follow the format below

##### General
* [ ] 80 characters per line (whenever possible)
* [ ] Put all class functions and variables that don't need to be public in private
* [ ] Pass function parameters by `const ref` if larger than one byte (or just `ref` if the function can change the value)
* [ ] Try to increase speed for massively repeated code
* [ ] Restructure code to be more readable

#### Testability*

Based on _Writing Testable Code_, https://www.toptal.com/qa/how-to-write-testable-code-and-why-it-mattersNo global state

* [ ] No class enforces that it be a singleton
* [ ] No static (class-wide) data except constants (_i.e._, individual objects contain separate data)
* [ ] No static (class-wide) functions except pure functions that produce the same output every time for a given set of inputs
No external effects
* [ ] A function's behavior can only be affected by its arguments or its object's state 
* [ ] A function only affects its arguments or its object's state
* [ ] Any object affecting or affected by a function can be replaced with a mock by the object's owner
  * _E.g._, system calls are wrapped in objects that can be replaced with mocks
  * _E.g._, I/O is done through an object that can be replaced with a mock

#### Sample code
```c++
#include <header_file.h>

    /*
    Use these types
        uint64_t -> when you need unsigned int
        int      -> when you need a signed int
        double	 -> when you need a float
        string	 -> when you need a string
        size_t	 -> when you need an index
        bool	 -> when you need boolean
          - Exception: use smaller types for very large arrays
          - Exception: Match Ch. 10 data types where applicable
    */

    /*
        Description of what my function does
        Inputs:   big_input   -> A big number to count to
                  name        -> The name of the number
        Returns:           
    */
    int MyFunction(const uint64_t& big_input, const string& name)
    {
        for (uint64_t i=0; i < big_input; i++)
        {
            printf("%" PRIu64 " steps nearer to %s\n", i, name);
        }
    }
```

# YamlSV - Yaml Schema Validator

Simple YAML schema validator which can be used as a library or command line tool.

## Dependencies

- Yaml-cpp 0.6.0 or greater (https://github.com/jbeder/yaml-cpp)
- ParseText from TIP "util" library

## Usage

Instantiate an object of YamlSV, then call the Validate function,

```
bool Validate(const YAML::Node& test_node, const YAML::Node& user_schema_node,
		std::vector<LogItem>& log_output);
```

which takes as its first and second arguments a root-level (yaml-cpp) Node
for the test and schema yaml matter, respectively. The third argument is 
an empty vector of LogItem that will be filled during validation. Log items 
can be consumed into the logging library of your choice. Returns true if
yaml test matter is validated against the schema.	

## Schema

A schema in YAML format is written to essentially mirror the yaml document
to be tested. Map keys are copied from the test document. Scalars and values
in sequences in the test document are replaced in the schema document by a 
string value representing the data type to be tested against.

The data type strings that may be used in the schema are:

- STR: No test is performed if this type
- INT: Integer type. The value 23.3 is not an INT.
- FLT: Float type. The values 23.3 and 23 can both be verified as FLT. 
- BOOL: Boolean string. The values "true", "TRUE", "True", "tRuE", 
"False", "false", etc., can be interpreted as BOOL. 

See the [simple test document and schema](#simple-example) test example. 
There are two strings which may replace a map key in the schema:

- \_NOT\_DEFINED\_: A string used as a wildcard if the value to which 
it maps is known and the key is variable. There may be 1 to n many 
structures in the test data which have mapped values described by the single
schema which is mapped by this wildcard. See examples [one](#wildcard-example-1)
and [two](#wildcard-example-2).
- \_NOT\_DEFINED\_OPT\_: A string used as wildcard similar to the 
previous definition and also indicates optional inclusion in the test data.
The key-value pair may be omitted from the test data. This is used to allow
variable count of mappings to a structure or scalar and to allow zero mappings 
to be present. See the [example](#wildcard-opt-example).

Only one of the map name wild cards may be used at a given mapping level within
the data structure hierarchy. 

Modifier characters can be prepended to the schema data type strings
to control test functionality:

- OPT: Indicates that a sequence is optional or a mapped value with a
specified key is optional.
- Nxx: (future release). Specify the sequence size. 

## Examples

### Simple Example

Test schema:

```
---
data: INT
other: [FLT]
map1: 
  - [BOOL]
  - {v1: STR, v2: [INT]}
...
```

Test data (validated):

```
---
data: 23
other: [5341.2, 14]
map1: 
  - [true, False, false, true]
  - {v1: "sky is blue", v2: [10, 20, 29]}
...
```

### Wildcard Example 1

Test schema:

```
---
data: INT
_NOT_DEFINED_: [FLT]
map1: BOOL
...
```

Test data (validated):

```
---
data: 23
other: [5341.2, 14]
map1: True
...
```

### Wildcard Example 2

Test schema (same as previous example):

```
---
data: INT
_NOT_DEFINED_: [FLT]
map1: BOOL
...
```

Test data (validated):

```
---
data: 23
other: [5341.2, 14]
arbitrary: [7.53, 12.25]
map1: True
...
```

### Wildcard Opt Example

Test schema:

```
---
data: INT
_NOT_DEFINED_OPT_: [FLT]
map1: BOOL
...
```

Test data (validated):

```
---
data: 23
map1: True
...
```
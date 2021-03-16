# YamlSV - Yaml Schema Validator

Simple YAML schema validator which can be used as a library or command line tool.

## Dependencies

- Yaml-cpp 0.6.0 or greater (https://github.com/jbeder/yaml-cpp)
- ParseText from TIP "util" library

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
"False", etc., can be interpreted as BOOL. 
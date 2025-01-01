# JSOG
[![Build and Test](https://github.com/sakars/JSOG/actions/workflows/cmake-single-platform.yml/badge.svg)](https://github.com/sakars/JSOG/actions/workflows/cmake-single-platform.yml)

This is a tool for generating C++ files from JSON Schemas.

## Usage
`JSOG [options] <schema files>`

## Options
- `--help`: Display help message.
- `-h`: Alias for `--help`.
- `--output-directory <path>`: Output directory for generated files. Default is current directory.
- `-o`: Alias for `--output-directory`.
- `--dump-schemas`: Logs intermediate representation of schemas
for debugging purposes.
- `-d`: Alias for `--dump-schemas`.
- `--require <uri>`: Adds a JSON Schema URI to the list of required schemas.
- `-r`: Alias for `--require`.
- `--preferred-identifier <uri> <identifier>`: Sets the preferred identifier for a schema.
- `-p`: Alias for `--preferred-identifier`.
- `--namespace <namespace>`: Sets the default global namespace for generated files.
- `-n`: Alias for `--namespace`.
- `--no-namespace`: Disables generation of namespaces.
- `-nn`: Alias for `--no-namespace`.
- `--indent <indent>`: Sets the indentation string for generated files. 2 spaces by default.
- `-i`: Alias for `--indent`.

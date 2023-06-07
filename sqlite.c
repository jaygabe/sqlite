#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// This is the input buffer used to store the input from stdin
// It contains 3 properties:
// buffer (The actual string)
// buffer_length (The expected length of buffer max)
// input_length (The length of inputed string)
typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

// Create type to label result of commands that begin with .
// .exit
typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

// Create type to label whether a statement has valid command
// select
// insert
typedef enum {
  PREPARE_SUCCESS,
  PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

// Create type to label the type of command that was prepared
typedef enum {
  STATEMENT_INSERT,
  STATEMENT_SELECT
} StatementType;

// Create type to classify statement
// to determine preparation success or failure
// to execute correctly
typedef struct {
  StatementType type;
} Statement;

// inline functions to replace every instance of this
// function call in the program. This works well because
// this function will not be called too many times, it
// is simple, and it is not long or complicated
static inline void print_prompt() { printf("db > "); }

// Create a new input buffer struct
InputBuffer* new_input_buffer();
// Read input from user and store in input_buffer
void read_input(InputBuffer* input_buffer);
// Free the memory used by the input_buffer
void close_input_buffer(InputBuffer* input_buffer);
// Handle the built-in commands of sqlite
MetaCommandResult do_meta_command(InputBuffer* input_buffer);
// Handle the SQL commands
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);
// Execute the SQL command based on statement type
void execute_statement(Statement* statement);

int main(int argc, char* argv[])
{
  // Upon execution of the main function, the first thing we do
  // is create a new input buffer struct to use through the
  // the length of the program. This will create space in memory
  // for the input_buffer, and we'll be able to manage the input 
  // from the user (command line) with this struct
  InputBuffer* input_buffer = new_input_buffer();

  while (true) {
    // This function is declared as a static inline function up top
    // This function simply prints a prompt to the terminal
    print_prompt();
    // This function is defined below the main function
    // This function takes in the input_buffer struct
    // and makes changes to its properties
    read_input(input_buffer);

    // If the command entered begins with a "." it is a meta command
    if (input_buffer->buffer[0] == '.') {
      // Check to see whether this command is built in or unrecognized
      // with the do_meta_command function. This function returns either:
      // META_COMMAND_SUCCESS
      // or
      // META_COMMAND_UNRECOGNIZED_COMMAND
      switch (do_meta_command(input_buffer)) {
        case (META_COMMAND_SUCCESS):
          // If success, continue
          continue;
        case (META_COMMAND_UNRECOGNIZED_COMMAND):
          // if unrecognized, notify user of unrecognized command and continue
          printf("Unrecognized command '%s'\n", input_buffer->buffer);
          continue;
      }
    }

    // If the command entered was not a meta command that begins with "."
    // It is a SQL command to communicate with the database
    // Create a statement variable to store the type of statement
    Statement statement;
    // Check whether the preparation of the statement returns:
    // a successful statement preparation
    // or
    // an unrecognized statement
    switch (prepare_statement(input_buffer, &statement)) {
      // If successful
      case (PREPARE_SUCCESS):
        // just break from switch statement 
        // and execute statement below with execute_statement function
        break;
      // If unrecognized statement command
      case (PREPARE_UNRECOGNIZED_STATEMENT):
        // notify user of unrecognized command and continue
        printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
        continue;
    }

    execute_statement(&statement);
    printf("Executed.\n");
  }
}

// Create a new input buffer struct
InputBuffer* new_input_buffer() {
  // Allocate space in memory for the new_input_buffer using the InputBuffer
  // struct passed into the sizeof function to calculate the number of bytes
  // required to store one InputBuffer passed into the malloc function to do
  // the actual allocation of the memory on the hardware, and type cast the
  // memory to ensure the data stored inside this allocated memory is indeed
  // an InputBuffer
  InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
  // Set the initial value of the buffer to NULL
  input_buffer->buffer = NULL;
  // Set the initial value of the buffer_length to 0
  input_buffer->buffer_length = 0;
  // Set the initial value of the input_length to 0
  input_buffer->input_length = 0;
  // Return the new input_buffer object (struct)
  return input_buffer;
}

// This function receives the input_buffer struct created when the database is started up
void read_input(InputBuffer* input_buffer) {
  // Use getline to collect the input string from the user in the terminal
  // The first argument is the location we want to store the string
  // The second argument is the space available for the string
  // The third argument is the location this stream is being collected from - In our case, stdin
  // This function returns the number of bytes (characters) read from the stdin
  ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  // If bytes read is less than 0, that means there was an error reading the input
  // and the function returns an error code (an integer less than 0)
  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  // Set the actual length of the string in the input_buffer struct
  // using the input_length property
  input_buffer->input_length = bytes_read - 1;
  // Set a null terminator in place of the newline character to ensure
  // you're able to figure out where the end of the buffer is
  input_buffer->buffer[bytes_read - 1] = 0;
}

// Frees space from the memory
void close_input_buffer(InputBuffer* input_buffer) {
  // Free up the actual buffer space
  free(input_buffer->buffer);
  // Free up the space that contains the struct
  free(input_buffer);
}

// DESCRIBE THIS FUNCTION
MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    close_input_buffer(input_buffer);
    exit(EXIT_SUCCESS);
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}

// DESCRIBE THIS FUNCTION
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
  if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
    statement->type = STATEMENT_INSERT;
    return PREPARE_SUCCESS;
  }

  if (strcmp(input_buffer->buffer, "select") == 0) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }

  return PREPARE_UNRECOGNIZED_STATEMENT;
}

// DESCRIBE THIS FUNCTION
void execute_statement(Statement* statement) {
  switch (statement->type) {
    case (STATEMENT_INSERT):
      printf("This is where we will do an insert\n");
      break;
    case (STATEMENT_SELECT):
      printf("This is where we will do a select\n");
      break;
  }
}
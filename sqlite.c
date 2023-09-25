#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// Each row struct contains an id integer, a username string, and an email string
// The COLUMN_USERNAME_SIZE is used to set the size of the username string
// The COLUMN_EMAIL_SIZE is used to set the size of the email string
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
// This function allows us to modify the size
// of a struct in the future without requireing
// us to change the code where ever the sizeof 
// attribute is required. If we didn't have this
// function, anytime we changed the attribute of
// a struct, we would be required to manually update
// the sizes everywhere in the code. This function
// gives our code robustness and flexibility
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
// Define the number of pages allowed per table
#define TABLE_MAX_PAGES 100
// Tables contain rows
// Each row contains an id, a username, and an email
// Below is the structure of each row
typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;

// This enum represents the possible outcomes
// of executing a database operation
// Our execute functions will return one of these
// EXECUTE_SUCCESS = 0
// EXECUTE_TABLE_FULL = 1
// EXECUTE_DUPLICATE_KEY = 2
typedef enum {
  EXECUTE_SUCCESS,
  EXECUTE_TABLE_FULL,
  EXECUTE_DUPLICATE_KEY,
} ExecuteResult;

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
  PREPARE_NEGATIVE_ID,
  PREPARE_STRING_TOO_LONG,
  PREPARE_SYNTAX_ERROR,
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
  Row row_to_insert; // Only used by insert statement
} Statement;

// This ID_SIZE constant uses the size_of_attribute function 
// to determine the size of the id property of the Row struct 
// in memory
const uint32_t ID_SIZE = size_of_attribute(Row, id);
// This USERNAME_SIZE constant uses the size_of_attribute function
// to determine the size of the email property of the Row struct in
// in memory
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
// This EMAIL_SIZE constant uses the size_of_attribute function
// to determine the size of the email attribute of the Row Struct
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
// ID_OFFSET specifies the starting byte position of the 'id' 
// field within a serialized row.
const uint32_t ID_OFFSET = 0;
// USERNAME_OFFSET specifies the starting byte position of the 
// `username` field within a serialized row
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
// EMAIL_OFFSET specifies the starting byte position of the `email`
// field within a serialized row
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
// ROW_SIZE creates a constant using the attributes of the Row
// Struct to determine th esize of each row
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
// This constant specifies that each page in the database will be
// 4096 bytes. This size is chosen because it matches the block size
// used by many file systems and storage devices, making it efficient
// for I/O operations
const uint32_t PAGE_SIZE = 4096;
// This constant calculates the number of rows that can fit on a single
// page. It assumes that each row will occupy `ROW_SIZE` bytes
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
// This constant sets a limit on the number of rows that a table can contain
// It is calcualted based on the maximum number of pages allocated for the table
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

// Define a struct to represent a table
typedef struct {
  uint32_t num_rows;
  void* pages[TABLE_MAX_PAGES];
} Table;

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
// Execute the INSERT command
ExecuteResult execute_insert(Statement* statement, Table* tables);
// Execute the SELECT command
ExecuteResult execute_select(Statement* statement, Table* table);
// Execute the SQL command based on statement type
ExecuteResult execute_statement(Statement* statement, Table* table);
// Convert to the compact representation of row
void serialize_row(Row* source, void* destination);
// Convert from the compact representation of row
void deserialize_row(void* source, Row* destination);
// To figure out where to read/write in memory for a particular row
void* row_slot(Table* table, uint32_t row_num);
// Initialize the table
Table* new_table();
// Release the table memory
void free_table(Table* table);
// Print the row of the database
void print_row(Row* row);

int main(int argc, char* argv[])
{
  // DESCRIBE THIS CODE
  Table* table = new_table();
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
      // DESCRIBE WHAT THIS CODE DOES
      case (PREPARE_SYNTAX_ERROR):
        printf("Syntax error. Could not parse statement.\n");
        continue;
      // If unrecognized statement command
      case (PREPARE_UNRECOGNIZED_STATEMENT):
        // notify user of unrecognized command and continue
        printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
        continue;
    }

    switch(execute_statement(&statement, table)) {
      case (EXECUTE_SUCCESS):
        printf("Executed.\n");
        break;
      case (EXECUTE_TABLE_FULL):
        printf("Error: Table full.\n");
        break;
    }
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
    // DESCRIBE THIS
    int args_assigned = sscanf(input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id), statement->row_to_insert.username, statement->row_to_insert.email);
    if (args_assigned < 3) {
      return PREPARE_SYNTAX_ERROR;
    }
    return PREPARE_SUCCESS;
  }

  if (strcmp(input_buffer->buffer, "select") == 0) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }

  return PREPARE_UNRECOGNIZED_STATEMENT;
}

// DESCRIBE THIS FUNCTION
ExecuteResult execute_insert(Statement* statement, Table* table) {
  if (table->num_rows >= TABLE_MAX_ROWS) {
    return EXECUTE_TABLE_FULL;
  }

  Row* row_to_insert = &(statement->row_to_insert);

  serialize_row(row_to_insert, row_slot(table, table->num_rows));
  table->num_rows += 1;

  return EXECUTE_SUCCESS;
}

// DESCRIBE THIS FUNCTION
ExecuteResult execute_select(Statement* statement, Table* table) {
  Row row;
  for (uint32_t i = 0; i < table->num_rows; i++) {
    deserialize_row(row_slot(table, i), &row);
    print_row(&row);
  }
  return EXECUTE_SUCCESS;
}

// DESCRIBE THIS FUNCTION
ExecuteResult execute_statement(Statement* statement, Table* table) {
  // TODO MAKE CHANGES
  switch (statement->type) {
    case (STATEMENT_INSERT):
      return execute_insert(statement, table);
    case (STATEMENT_SELECT):
      return execute_select(statement, table);
  }
}

// Describe the compacting the representation of a row
void serialize_row(Row* source, void* destination) {
  memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
  memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
  memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}
// Describe uncompacting the representation of a row
void deserialize_row(void* source, Row* destination) {
  memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

// Describe the implementation details for 
// figuring out where to read/write in memory for a particular row
void* row_slot(Table* table, uint32_t row_num) {
  uint32_t page_num = row_num / ROWS_PER_PAGE;
  void* page = table->pages[page_num];
  if (page == NULL) {
    // Allocate memory only when we try to access page
    page = table->pages[page_num] = malloc(PAGE_SIZE);
  }
  uint32_t row_offset = row_num % ROWS_PER_PAGE;
  uint32_t byte_offset = row_offset * ROW_SIZE;
  return page + byte_offset;
}

// DESCRIBE THIS FUNCTION
Table* new_table() {
  Table* table = (Table*)malloc(sizeof(Table));
  table->num_rows = 0;
  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    table->pages[i] = NULL;
  }
  return table;
}

// DESCRIBE THIS FUNCTION
void free_table(Table* table) {
  for (int i = 0; table->pages[i]; i++) {
    free(table->pages[i]);
  }
  free(table);
}

void print_row(Row* row) {
  printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}
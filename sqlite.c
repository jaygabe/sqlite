#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// inline functions to replace every instance of this
// function call in the program. This works well because
// this function will not be called too many times, it
// is simple, and it is not long or complicated
inline void print_prompt() { printf("db > "); }

// Describe this struct
typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

// Shortly Describe this function declaration
InputBuffer* new_input_buffer();

int main(int argc, char* argv[])
{
  // Upon execution of the main function, the first thing we do
  // is create a new input buffer struct to use through the
  // the length of the program. This will create space in memory
  // for the input_buffer, and we'll be able to manage the input 
  // from the user (command line) with this struct
  InputBuffer* input_buffer = new_input_buffer();

  while (true) {
    // Describe this function
    print_prompt();
    // Describe this function
    read_input(input_buffer);

    if (strcmp(input_buffer->buffer, ".exit") == 0) {
      // Describe this function
      close_input_buffer(input_buffer);
      exit(EXIT_SUCCESS);
    } else {
      printf("Unrecognized command '%s'.\n", input_buffer->buffer);
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
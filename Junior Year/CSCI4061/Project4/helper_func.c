#include "helper_func.h"

//logging timer handler
void timer_handler (int signum) {
  fprintf(catalog_file, "Time\t %d ms\t #dir\t %d\t #files\t %d \n", ++runtime, dir_traversed, file_traversed);
}

// full time handler
void fulltimer_handler (int signum) {
  fulltime = fulltime + 1;
}

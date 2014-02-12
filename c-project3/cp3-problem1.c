/*
 * CS3600, Spring 2014
 * C Bootcamp, Homework 3, Problem 1
 * (c) 2012 Alan Mislove
 *
 * In this problem, your goal is to fill in the itoaaa function.
 * This function will take in a 32-bit signed integer, and will
 * return a malloc'ed char * containing the English representation
 * of the number.  A few examples are below:
 *
 * 0 -> "zero"
 * 9 -> "nine"
 * 45 -> "forty five"
 * -130 -> "negative one hundred thirty"
 * 11983 -> "eleven thousand nine hundred eighty three"
 * 234201 -> two hundred thirty four thousand hundred one"
 *
 * Do not touch anything outside of the itoaaa function (you may,
 * of course, define any helper functions you wish).  You may also
 * use any of the functions in <string.h>. 
 *
 * Finally, you must make sure to free() any intermediate malloced()
 * memory before you return the result.  You should also return a 
 * char* that is malloced to be as small as necessary (the script 
 * checks for this).  For example, if you returned "forty five", you
 * should put this into a malloced space of 12 bytes (11 + '\0').
 *
 */

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

char *ones[10] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };
char *teens[10] = {"ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen" };
char *tens[8] = {"twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};

char *strings[20];
int index;

char *itoaaa(int i);
void setup(int num);
void million(int num);
void thousand(int num);
void hundred(int num);
void ten(int num);
void one(int num);
char *combine();

int main(int argc, char **argv) {
  // check for the right number of arguments
  if (argc != 2) {
    printf("Error: Usage: ./cp3-problem1 [int]\n");
    return 0;
  }

  // create the time structure
  long long arg = atoll(argv[1]);
  if (arg == (int) arg) {
    // call the function
    char *result = itoaaa((int) arg);

    // print out the result
    printf("%d is: %s\n", (int) arg, result);
  } else {
    printf("Error: Number out of range.\n");
  }

  return 0;
}

/**
 * This function should print out the English full representation
 * of the passed-in argument.  See above for more details.
 */
char *itoaaa(int arg) {
  index = 0;
  if (arg < 0) {
    strings[0] = "negative";
    index++;
    setup(abs(arg));
    return combine();
  }
  setup(arg);
  return combine();


}


/**
* This function iterates over the the integer by examining each digit in 
* context to the digits around it
* Utilizes many helper functions to add each necessary string to an array
*/
void setup(int num) {
  if (num >= 1000000000) {
    if ((num % 1000000000) == 0) {
      hundred(num / 1000000000);
      strings[index] = "billion";
      index++;
    } else {
      hundred(num / 1000000000);
      strings[index] = "billion";
      index++;
      million(num % 1000000000);
    }
  } else {
    million(num);
  }
}

void million(int num) {
if (num >= 1000000) {
     if ((num % 1000000) == 0) {
      hundred(num / 1000000);
      strings[index] = "million";
      index++;
     } else {
      hundred(num / 1000000);
      strings[index] = "million";
      index++;
      thousand(num % 1000000);
     }
  } else {
    thousand(num);
  }
}

void thousand(int num) {
if (num >= 1000) {
     if ((num % 1000) == 0) {
      hundred(num / 1000);
      strings[index] = "thousand";
      index++;
     } else {
      hundred(num / 1000);
      strings[index] = "thousand";
      index++;
      hundred(num % 1000);  
     }
  } else {
    hundred(num);
  }
}

void hundred(int num) {
if (num >= 100) {
    if ((num % 100) == 0) {
      strings[index] = "hundred";
      index++;
      } 
      else {
      strings[index] = ones[num / 100];
      index++;
      strings[index] = "hundred";
      index++;
      ten(num % 100);
    }
  } else {
    ten(num);
  }
}

void ten(int num) {
if (num >= 20) {
    if ((num % 10) == 0) {
      char *temp = tens[(num / 10) - 2];
      strings[index] = temp;
      index++;

    } else {
      char *temp1 = tens[(num / 10) - 2];
      char *temp2 = ones[num % 10];
      strings[index] = temp1;
      index++;
      strings[index] = temp2;
      index++;
    }
  } else if (num >= 10) {
    char *temp = teens[num-10];
    strings[index] = temp;
    index++;
  } else {
    one(num);
  }
}

void one(int num) {
    char *temp = ones[num];
    strings[index] = temp;
    index++;
}

/**
* Computes the total size of the strings placed in the strings array 
* accounting for spaces and terminator byte
* Then loops over array again adding each string element to a calloced string
*/

char *combine(){
  int size;
  int i;
  for(i = 0; i < index; i++){
      char* temp = strings[i];
      size += strlen(temp);
      size++;
  }
  
  char str[500];
  sprintf(str, "%d",size);

  char *final;
 
 final = "";
 final = calloc(size, sizeof(char));

 for(i = 0; i < index; i++){
   strcat(final, strings[i]);
   if (i + 1 != index) {
   strcat(final, " ");
   }
 }
  return final;
}


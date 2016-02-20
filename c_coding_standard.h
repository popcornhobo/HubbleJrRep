/* c_coding_standard.h
 * 
 * This file contains a description of coding standard for C code.
 *
 * Author: Thomas Rader
 * 
 * 
 * Revisions:
 * 2016-02-20	v0.1	File Creation
 *
 * */

/* Defines */
#define SOME_DEFINE value

/* Global Variables */
type Some_Variable_Name;

/*
 * FunctionName
 * Description: A brief function description
 * 	Input Parameters:
 * 		type parameter1 - a brief description of this parameter
 *
 * 	Returns:
 * 		type - a brief description of what is being returned
 *
*/
type FunctionName(type parameter1)
{
	/* Local Variables */
	type some_local_variable;

	/* Comment on its own line */
	SomeOtherFunction(); 	// Comment on the same line as code
	return type;

}
/*------------------------------ END FunctionName ------------------------------*/  

/* What goes where */

#ifndef __SOME_HEADER_H__
#define __SOME_HEADER_H__ 	// Notice that header guards use __NAME_H__ format

/* Header File:
 * 	* Defines
 * 	* Includes
 * 	* Public function prototypes
*/

#endif // __SOME_HEADER_H__

/* Source File:
 * 	* File-wide globals
 * 	* File local function prototypes
 * 	* File local function definitions
*/

/*--------------------------------------------------------------------*/
/* node.h                                                             */
/* Authors: Aidan Walsh, Konstantin Howard                            */
/*--------------------------------------------------------------------*/
#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include <stddef.h>
#include "a4def.h"

/* a Node_T is an object that contains a path, references to the 
parents, children, information stored (if file), length (if file) and 
whether or not it is a file or directory */
typedef struct node* Node_T;

/* given a parent node, directory string dir, a boolean variable isDir
that indicates if it is a directory or file, size_t length (if file)
and a void pointer contents (if file), returns a new Node_T or NULL
if any allocation error occurs in creating the node or its fields. 

It is initialized to have its path as the parent's path (if it 
exists) prefixed to the directory string parameter, separated by a 
slash. It is also initialized with its parent link as the parent 
parameter value, but the parent itself is not changed to link to the new
node. The children links are initialized but do not point to any 
children if it is a directory. Contents are NULL and length undefined if
directory. If a file is being created, all entries except parent must
 be valid (non NULL)
and length and contents are initialized to their arguments. 
If a directory is being created, contents and length are ignored
but everything else must be valid */
Node_T Node_create(const char* dir, Node_T parent, boolean isDir,
                   size_t length, void *contents);

/* destroys entire hierarchy of nodes rooted at node1, including node1. 
Node1 must be valid and a size_t variable of how many are destroyed is
returned. */
size_t Node_destroy(Node_T node1);

/* return the string that is the path of node1. This path is made up 
of the names of directories the precede it where a slash indicates
a new directory. Ensure that node1 is valid.*/
const char* Node_getPath(Node_T node1);

/* returns the number of children of directory node1 as size_t. 
The argument must be a valid directory but if it is a file, 
return 0.*/
size_t Node_getNumChildren(Node_T node1);

/* returns the child node of node1 with itentifier childID, if one 
   exists, otherwise returns NULL. Ensure that node1 and childID 
   are valid. Node1 is valid if it is non NULL and is a directory */
Node_T Node_getChild(Node_T node1, size_t childID);

/* returns the parent node of node1, if it exists, otherwise returns
   NULL. A directory will always be returned and ensure that node1 is 
   valid */ 
Node_T Node_getParent(Node_T node1);

/* Makes child a child of parent, if possible, and returns SUCCESS. 
This is not possible in the following cases: 
- parent already has a child with child's path, in which case: returns
ALREADY_IN_TREE
- child's path is not parent's path + / + directory, or the parent 
cannot link to the child, in which cases: returns PARENT_CHILD_ERROR 
- parent is file and so cannot link anything to the parent: returns
NOT_A_DIRECTORY
Ensure that parent and child are valid */
int Node_linkChild(Node_T parent, Node_T child);

/* unlinks node parent from its child node child. Child is unchanged. 
Returns PARENT_CHILD_ERROR if child is not a child of parent, 
NOT_A_DIRECTORY if parent is not a directory, and SUCCESS otherwise.
Make sure that parent and child are valid. */
int Node_unlinkChild(Node_T parent, Node_T child);

/* changes the contents of file node1 to newContents and the length of 
the file to the size_t newLength, such that it is a
valid file and keep is false. The old contents of the file is returned.
If it is a 
directory, return NULL. If the goal is to just retreive the contents,
then keep must be true, and the contents of node1 is just returned, and
newLength and newContents are ignored.*/  
void* Node_changeContents(Node_T node1, void *newContents,
                          size_t newLength, boolean keep);

/* returns boolean, where if TRUE then node1 is a directory, otherwise
   it is a file. Validate the node1 */
boolean Node_isDirectory(Node_T node1);

/* returns a size_t that is the length of the file, given that node1 is
   a file. If it is a directory, there is an error. Node1 is validated*/
size_t Node_getLength(Node_T node1); 

#endif

/*--------------------------------------------------------------------*/
/* ft.h                                                               */
/* Authors: Konstantin Howard, Aidan Walsh                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "dynarray.h"
#include "ft.h"
#include "node.h"

/* A file tree is an AO with several state variables: */

/* flag for if it is in an an initialized state (TRUE) or not (FALSE)*/
static boolean isInitialized;

/* a pointer to the root in the hierarchy */
static Node_T root;

/* a counter of the number of nodes in the hierarchy */
static size_t count;

/* Starting at the node curr, traverse as far down the hierarchy 
as possible while still matching the path parameter. Returns a 
pointer to the farthest matching node down that path, or NULL
if there is no node in curr's hierarchy that matches a prefix
of that path. Validate path. */
static Node_T FT_traversePathFrom(char* path, Node_T curr) {
   Node_T found;
   size_t i;
   
   assert(path != NULL);
   /* at end or not found so return NULL */
   if (curr == NULL)
      return NULL;

   /* if we have found the exact end, return node we
      are on */
   else if (!strcmp(path,Node_getPath(curr)))
      return curr;

   /* otherwise, compare up to length of the node we are on, and iterate
      through children recursively */
   else if (!strncmp(path, Node_getPath(curr), strlen(Node_getPath(curr)))) {
      for (i = 0; i < Node_getNumChildren(curr); i++) {
         found = FT_traversePathFrom(path, Node_getChild(curr, i));
         if(found != NULL)
            return found; 
      }
      return curr; 
   }
   return NULL; 
}

/* static helper function to call recursive DT_traversePathFrom function.
Returns Node at ends of given path always starting from the root or NULL
if path does not exist in tree. Validates path. */
static Node_T FT_traversePath(char* path) {
   assert(path != NULL);

   return FT_traversePathFrom(path, root);

}
/*inserts a new path into the tree rooted at parent, or, if parent is 
NULL, at the root of the data structure. This new path may belong to file or
directory, specified by isDir. If a file, contents and length pass along the
new file's contents and length. If a node representing a path already exists, 
return ALREADY_IN_TREE. If there is an allocation error in creating any
nodes or their fields, return MEMORY_ERROR. If there is an error 
linking any of the nodes, returns PARENT_CHILD_ERROR. Return 
CONFLICTING_PATH if path is not underneath existing root. Validate path
 */
static int FT_insertRestOfPath(char* path, Node_T parent, boolean isDir,
                               void* contents, size_t length) {
   Node_T curr = parent;
   Node_T firstNew = NULL;
   Node_T new;
   char* copyPath;
   char* restPath = path;
   char* dirToken;
   char* nextDirToken;
   int result;
   size_t newCount = 0;

   assert(path != NULL);

   /* if curr is NULL then when we traversed, it could 
      only be the root node, but a root is already inserted
      so we have conflicting paths */
   if(curr == NULL) {
      if(root != NULL || isDir == FALSE) {
         return CONFLICTING_PATH; 
      }
   }
   /* otherwise, if the path of our current node is the same
      as what we are trying to insert, then it already exists */
   else {
      if (!strcmp(path, Node_getPath(curr)))
         return ALREADY_IN_TREE;

      /* point restPath to the front of the new contents being 
         added */
      restPath += (strlen(Node_getPath(curr)) + 1);
   }

   /* allocate memory for what we will be calling strtok on */
   copyPath = malloc(strlen(restPath)+1);
   if (copyPath == NULL)
      return MEMORY_ERROR;
   strcpy(copyPath, restPath);

   /* store the character for after because this lets us keep 
      track of the end and know when to create a file or directory */
   dirToken = strtok(copyPath, "/");
   nextDirToken = strtok(NULL, "/");
   
   while(nextDirToken != NULL) {
      new = Node_create(dirToken, curr, TRUE, 0, NULL);

      /* ensure memory allocated properly, if not, then we destroy
       what all the new nodes are connected to: firstNew */
      if (new == NULL) {
         if (firstNew != NULL)
            (void) Node_destroy(firstNew);
         free(copyPath);
         return MEMORY_ERROR; 
      }
      
      newCount++;

      /* if firstNew is NULL then this is our first iteration
         so we adjust it to new */
      if(firstNew == NULL)
         firstNew = new;
      else {

         /* otherwise, we link our new node to the one we made 
            in the previous iteration and ensure it does so 
            properly */
         result = Node_linkChild(curr, new);
         if (result != SUCCESS) {
            (void) Node_destroy(new);
            (void) Node_destroy(firstNew);
            free(copyPath);
            return result; 
         }
      }

      curr = new;
      dirToken = nextDirToken;
      nextDirToken = strtok(NULL, "/"); 
   }

   /* still have one last node to insert and so we either
      insert a directory or file */
   if (isDir == TRUE) {
      new = Node_create(dirToken, curr, TRUE, 0, NULL);
   }
   else {
      new = Node_create(dirToken, curr, FALSE, length, contents);
   }
      /* ensure memory allocated properly, if not, then we destroy 
         what all the new nodes are connected to: firstNew */
      if (new == NULL) {
         if (firstNew != NULL)
            (void) Node_destroy(firstNew);
         free(copyPath);
         return MEMORY_ERROR;
      }

      newCount++;

      /* if firstNew is NULL then this is our first iteration 
         so we adjust it to new */
      if(firstNew == NULL)
         firstNew = new;
      else {

         /* otherwise, we link our new node to the one we made in the 
            previous iteration and ensure it does so properly */
         result = Node_linkChild(curr, new);
         if (result != SUCCESS) {
            (void) Node_destroy(new);
            (void) Node_destroy(firstNew);
            free(copyPath);
            return result;
         }
      }

      free(copyPath);
      /* if parent is NULL then we are just inserting it as 
         the root node */
      if(parent == NULL) {
         root = firstNew;
         count = newCount;
         return SUCCESS; 
      }
      else {
         /* otherwise, we link our firstNew to the parent to 
           create the final connection and ensure that this is 
           done successfully */
         result = Node_linkChild(parent, firstNew);
         if (result == SUCCESS)
            count += newCount;
         else
            (void) Node_destroy(firstNew);

         return result; 
      }
               
   }


/* see ft.h for specification */
int FT_insertDir(char *path) {
   Node_T curr; 
   int result;

   assert(path != NULL);

   if(!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);
   result = FT_insertRestOfPath(path, curr, TRUE, NULL, 0);
   return result; 
   
}

/* see ft.h for specification */
boolean FT_containsDir(char* path) {
/* make sure not null */
   Node_T curr;
   boolean result;

   assert(path != NULL);

   /* make sure structure is initialized) */
   if (!isInitialized)
      return FALSE;

   /* traverse to get node with matching path, if it exists */
   curr = FT_traversePath(path);

   /* if it is NULL, then it is not present, if paths do not match, 
      and if it is not a directory, else it is found and return TRUE */
   if (curr == NULL)
      result = FALSE;
   else if (strcmp(path, Node_getPath(curr)))
      result = FALSE;
   else if (Node_isDirectory(curr) == FALSE)
      result = FALSE; 
   else
      result = TRUE;

   return result; 
   
}

/* see ft.h for specification */
int FT_rmDir(char *path) {
   assert(path != NULL);
   Node_T curr;
   Node_T parent; 
   int result;

   if (!isInitialized)
      return INITIALIZATION_ERROR;
   /* traverse path to look for node and ensure
      it isn't NULL or doesn't exist */
   curr = FT_traversePath(path);
   if (curr == NULL)
      result = NO_SUCH_PATH;
   else {
      parent = Node_getParent(curr);
      /* if path and path of curr are same, then we found
         the right node */
      if(!strcmp(path, Node_getPath(curr))) {
         /* make sure that this correct path is a directory */
         if(Node_isDirectory(curr) == FALSE)
            return NOT_A_DIRECTORY;
         /* if parent is NULL, then we are at root and so we just set
            root to NULL, then destroy everything else */
         else if(parent == NULL)
            root = NULL;
         else
            /* if it is not the root,  make sure we unlink before
               destroying everything below */
            Node_unlinkChild(parent, curr);

         count -= Node_destroy(curr);

         result = SUCCESS;
      }

      else
         result = NO_SUCH_PATH; 
   }
   return result;
}


/* see ft.h for specification */
int FT_insertFile(char *path, void *contents, size_t length) {
   Node_T curr;
   int result;

   assert(path != NULL);

   if (!isInitialized)
      return INITIALIZATION_ERROR;
   
   /* traverse path to get parent node, then attempt to insert */
   curr = FT_traversePath(path);
   result = FT_insertRestOfPath(path, curr, FALSE, contents, length);
   return result; 
 
   
}


/* see ft.h for specification */
boolean FT_containsFile(char *path) {
   Node_T curr;
   boolean result;

   assert(path != NULL);
   /*ensure it is initialized */
   if (!isInitialized)
      return FALSE;
   /* traverse path to look for it */
   curr = FT_traversePath(path);

   /* if not found, return FALSE, or if 
      our furthest node does not have the 
      same path, or if it is a directory */
   if (curr == NULL)
      result = FALSE;
   else if(strcmp(path, Node_getPath(curr)))
      result = FALSE;
   else if (Node_isDirectory(curr)==TRUE)
      result = FALSE;
   else
      result = TRUE;

   return result; 
}

/* see ft.h for specification */
int FT_rmFile(char *path) {
   Node_T curr;
   Node_T parent; 
   int result;

   assert(path != NULL);

   /* make sure it is initialized */
   if (!isInitialized)
      return INITIALIZATION_ERROR;

   /* first traverse to see if it is contained */
   curr = FT_traversePath(path);

   /* if NULL it is returned then there is no such path
      but if we get a non null result, we must take its parent
      to unlink if it is a match. */
   if (curr == NULL)
      result = NO_SUCH_PATH;
   else {
      parent = Node_getParent(curr);

      /* if we have a match, make sure that it is a file, 
         then test to see if it is root to set root to NULL. 
         Otherwise, always unlink child and destroy
         recursively */
      if (!strcmp(path, Node_getPath(curr))) {
         if(Node_isDirectory(curr)==TRUE)
            return NOT_A_FILE;
         else if(parent == NULL)
            root = NULL;
         else
            Node_unlinkChild(parent, curr);

         count -= Node_destroy(curr);

         result = SUCCESS; 
      }
      else
         result = NO_SUCH_PATH;
   }
   return result; 
}

/* see ft.h for specification */
void *FT_getFileContents(char *path) {
   Node_T curr;

   assert(path != NULL);

   /* traverse the path to look for the file */
   curr = FT_traversePath(path);

   /* if NULL then it is not found */
   if (curr == NULL)
      return NULL;

   /* if the path matches the path of the node we found
      then test to see if it is a file, then we may return
      the contents by calling our Node function */
   else if (!strcmp(path, Node_getPath(curr))) {
      if (Node_isDirectory(curr)==TRUE)
         return NULL;
      else
         return (void*)Node_changeContents(curr, NULL, 0, TRUE);  
   }
      else
         return NULL; 
}

/* see ft.h for specification */
void *FT_replaceFileContents(char *path, void *newContents,
                             size_t newLength) {
   Node_T curr;

   assert(path != NULL);

   /* traverse path to look for the file with the given path */
   curr = FT_traversePath(path);

   /* if returned node is NULL, then it is not found */
   if (curr == NULL)
      return NULL;

   /* if we have matching paths, check to see if it is directory
      then we may return old value and change it to new value */
   else if (!strcmp(path, Node_getPath(curr))) {
      if (Node_isDirectory(curr)==TRUE)
         return NULL;
      else
         return (void*)Node_changeContents(curr, newContents,
                                           newLength, FALSE); 
   }
   else
      return NULL; 
}

/* see ft.h for specification */
int FT_stat(char *path, boolean *type, size_t *length) {
   Node_T curr;

   assert(path != NULL);
   assert(type != NULL);
   assert(length != NULL);

   /* make sure structure is initialized */
   if (!isInitialized)
      return INITIALIZATION_ERROR;

   /* traverse path and store last found node into curr */
   curr = FT_traversePath(path);

   /* if curr is NULL then it was not found */
   if (curr == NULL)
      return NO_SUCH_PATH;

   /* if the path and path of curr are equal, then it has been found */
   else if (!strcmp(path, Node_getPath(curr))) {

      /* if it is a directory, adjust type and return success */
      if (Node_isDirectory(curr)==TRUE) {
         *type = FALSE;
         return SUCCESS; 
      }
      else {

         /* if it is a file, adjust type, get the length, and return
            success */
         *type = TRUE;
         *length = Node_getLength(curr);
         return SUCCESS; 
      }
   }
   else
      return NO_SUCH_PATH; 
}

/* see ft.h for specification */
int FT_init(void) {

   /* if it is already initialized, there is an error) */
   if (isInitialized)
      return INITIALIZATION_ERROR;

   /* adjust variables within */
   isInitialized = 1;
   root = NULL;
   count = 0;
   return SUCCESS; 
}

/* see ft.h for specification */
int FT_destroy(void) {

   /* must have been initialized */
   if (!isInitialized)
      return INITIALIZATION_ERROR;

   /* destroy from root */
   if (root != NULL)
      count -= Node_destroy(root);

   root = NULL;
   isInitialized = 0;
   
   /* check count is 0 after destruction */
   assert(count == 0);
   return SUCCESS; 
}

/* performs a preorder traversal of the tree rooted at node1, 
inserting each payload to DynArray_T dArray beginning at index "index". 
Returns the next usused index in dArray after the insertion(s). */
static size_t FT_preOrderTraversal(Node_T node1, DynArray_T dArray,
                                   size_t index) {
   size_t i;

   assert(dArray != NULL);

   /* make sure root is not null */
   if (node1 != NULL) {

      /* first set path to index 0 then increment index and we may
       begin recursion*/
      (void) DynArray_set(dArray, index, Node_getPath(node1));
      index++;

      /* iterate through children, then call function recursively
         to set DynArray at index "index" and continue going
         through children */
      for(i = 0; i < Node_getNumChildren(node1); i++)
         index = FT_preOrderTraversal(Node_getChild(node1, i), dArray,
                                      index); 
   }
   return index; 
}

/* Alternate version of strlen that uses pAcc as an in-out parameter
   to accumulate a string length, rather than returning the length of
   str, and also always adds one more in addition to str's length. */
static void FT_strlenAccumulate(char* str, size_t* pAcc) {
   assert(pAcc != NULL);
   assert(str != NULL);

   /* add to pAcc if the string str is not NULL. */
   if (str != NULL)
      *pAcc += (strlen(str)+1); 
}

/* Alternate version of strcat that inverts teh typical argument order, 
   appending str onto acc, and also always adds a newline at the end of
   the concatenated string. */
static void FT_strcatAccumulate(char* str, char* acc) {
   assert(acc != NULL);
   assert(str != NULL);

   /* if str is not NULL, then concatenate in the reverse order, and
      append the newline */
   if (str != NULL)
      strcat(acc, str); strcat(acc, "\n"); 
}

/* see ft.h for specification */
char *FT_toString(void) {
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char* result = NULL;

   /* make sure it is initialized */
   if (!isInitialized)
      return NULL;

   /* call the preorder traversal to place the paths into nodes */
   nodes = DynArray_new(count);
   (void) FT_preOrderTraversal(root, nodes, 0);

   /* get total length to allocate memory for string that will be 
      returned */
   DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate,
                (void*) &totalStrlen);

   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      return NULL; 
   }

   /* make it have something so that we may use string operations */
   *result = '\0';

   /* call next mapping function to append the strings and 
      the newlines to get it in the format we want */
   DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate,
                (void *) result);

   DynArray_free(nodes);
   return result; 
}


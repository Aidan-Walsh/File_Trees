/*--------------------------------------------------------------------*/
/* node.c                                                             */
/* Authors: Konstantin Howard, Aidan Walsh                            */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "dynarray.h"
#include "node.h"

/* A node structure represents a directory or file in the file tree */
struct node {
   /* the full path of the file/directory */
   char* path;

   /* the parent directory of this file/directory. NULL for the root 
      of the file tree */
   Node_T parent;

   /* the subdirectories/subfiles stored in sorted order by pathname */
   DynArray_T children;

   /* the contents that is stored with a file. NULL if a directory */
   void* contents;

   /* the length of the file. NULL if a directory */
   size_t length;

   /* if TRUE, then it is a directory, FALSE, then file. NULL means
      not yet assigned */
   boolean isDir; 
};

   
/* see node.h for specification */
   Node_T Node_create(const char* dir, Node_T parent, boolean isDir,
                      size_t length, void *contents) {
      Node_T new;
      char* path; 
      
      assert(dir != NULL);

      /* allocate memory for the new node and make sure allocated
         properly */
      new = malloc(sizeof(struct node));
      if (new == NULL) {
         return NULL; 
      }

      /* no parent path so just malloc dir and + 1 for terminating char
       */
      if (parent == NULL) {
         path = malloc(strlen(dir)+1); 
      }
      else {
         /* other case where there is previous path so add that and 
            another +1 for the "/" */
         path = malloc(strlen(parent->path) + 1 + strlen(dir) + 1); 
      }

      if (path == NULL)
         return NULL;

      /* must be non null to use string functions */
      *path = '\0';

      /* if parent exists, first place its path into "path" */
      if(parent != NULL) {
         strcpy(path, parent->path);
         strcat(path, "/"); 
      }

      /* then concat dir onto end of "path" */
      strcat(path, dir);
      new->path = path;

      if (new->path == NULL) {
         free(new);
         return NULL; 
      }

      new->parent = parent;

      /* if we are making directory, allocate memmory for children,
         set length to 0, contents to NULL, and isDir to TRUE */
      if (isDir == TRUE)
      {
         new->children = DynArray_new(0);
         if (new->children == NULL) {
            free(new->path);
            free(new);
            return NULL; 
         }
         new->contents= NULL;
         new->isDir = TRUE;
         new->length = 0; 
      }
      else {

         /* if file, then children are NULL, set length and contents
            and isDir */
         new->children = NULL;
         new->length = (size_t) length;
         new->isDir = FALSE;
         new->contents = (void*) contents; 
         
      }

      return new; 
   }

/* see node.h for specification */
size_t Node_destroy (Node_T node1) {
   size_t i;
   size_t count = 0;
   Node_T current;

   assert(node1 != NULL);

   /* case for freeing directory, we must call recursively, then
      free and update count */
   if (node1->isDir == TRUE) {
      for (i = 0; i <DynArray_getLength(node1->children); i++)
      {
         current = DynArray_get(node1->children, i);
         count += Node_destroy(current); 
      }
      DynArray_free(node1->children);
      free(node1->path);
      free(node1);
      count++;
   }
   else {
      /* since file is at the end of a structure, we just 
         free it and increment count by 1 */
      free(node1->path);
      free(node1);
      count++; 
   }
   return count; 
}

/* see node.h for specification */
const char* Node_getPath(Node_T node1) {
   assert(node1 != NULL);

   return node1->path; 
}

/* see node.h for specification */
size_t Node_getNumChildren(Node_T node1) {
   assert(node1 != NULL);

   /* if it is a directory, we just call the dynarray 
      function, but if it is a file, we return NULL */
   if (node1->isDir == TRUE)
      return DynArray_getLength(node1->children);
   else
      return (size_t) 0; 
}

/* see node.h for specification */
Node_T Node_getChild(Node_T node1, size_t childID) {
   assert(node1 != NULL);

   /* if it is a directory, make sure that childID is valid
      by checking to see that it is within bounds, then return 
      the child at its location. if invalid or node1 is a file
      then return NULL */
   if (node1->isDir == TRUE) {
      if (DynArray_getLength(node1->children) > childID) {
         return DynArray_get(node1->children, childID); 
      }
      else {
         return NULL; 
      }
   }
   else {
      return NULL; 
   }
}

/* see node.h for specification */
Node_T Node_getParent(Node_T node1) {
   assert(node1 != NULL);

   return node1->parent; 
}
/* compares paths of node1 and node2, regardless of file/dir status.
   Returns <0, 0, or >0 if node1 is less than, equal to, or greater
   then node2,m respectively */
static int Node_compare(Node_T node1, Node_T node2) {
   assert(node1 != NULL);
   assert(node2 != NULL);

   return strcmp(node1->path, node2->path);
}


/* see node.h for specification */
int Node_linkChild(Node_T parent, Node_T child) {
   size_t index;
   char* rest;
   
   assert(parent != NULL);
   assert(child != NULL);
   /* maintain invariant */
   if (parent->isDir != TRUE) {
      return NOT_A_DIRECTORY;
   }
   

   index = strlen(parent->path);
   /* if first parent path does not match beginning of child path, 
      failure */
   if(strncmp(child->path, parent->path, index)) {
      return PARENT_CHILD_ERROR;
   }

   /* point rest to the start of the child's unique path which starts 
      with a "/". If it doesn't start with this and should (it should
      when the length of the child path is greater than the parent) then
      there is an error */
   rest = child->path + index;
   if(strlen(child->path) >= index && rest[0] != '/') {
      return PARENT_CHILD_ERROR;
   }

   rest++;
   /* make sure that there are no more "/" in the child */
   if (strstr(rest, "/") != NULL) {
      return PARENT_CHILD_ERROR; 
   }
   child->parent = parent;

   /* binary search for child. If it returns 1, then it is contained 
      so there is an error. Store it in "index" so that if it is not 
      contained then add it in at location index */
   if(DynArray_bsearch(parent->children, child, &index,
              (int (*)(const void*, const void*)) Node_compare) == 1) {
      return ALREADY_IN_TREE;
   }

   if(DynArray_addAt(parent->children, index, child) == TRUE) {
      return SUCCESS;
   }
   else {
      return PARENT_CHILD_ERROR;
   }
}

/* see node.h for specification */
int Node_unlinkChild(Node_T parent, Node_T child) {
   size_t index = 0;

   assert(parent != NULL);
   assert(child != NULL);

   /* make sure invariant holds where parent must be directory */
   if (parent->isDir != TRUE) {
      return NOT_A_DIRECTORY; 
   } 

   /* conduct binary search for child. if it is not found, then we 
      cannot remove it. If found, then use dynArray to remove */
   if (DynArray_bsearch(parent->children, child, &index,
               (int (*)(const void*, const void*)) Node_compare) == 0) {
      return PARENT_CHILD_ERROR; 
   }
   (void) DynArray_removeAt(parent->children, index);

   return SUCCESS; 
}

/* see node.h for specification */
void* Node_changeContents(Node_T node1, void *newContents,
                          size_t newLength, boolean keep) {
   void *oldContents; 
   
   assert(node1 != NULL);
   assert(node1->isDir == FALSE);

   /* if we want to keep then just return contents, 
      otherwise, we adjust length and contents 
      and return the old contents */
   if (keep == TRUE) {
      oldContents = node1->contents; 
      return (void*) oldContents; 
   }
   else {
      oldContents = node1->contents;
      node1->contents= newContents;
      node1->length = newLength;
      return (void*) oldContents; 
   }
   
}

/* see node.h for specification */
boolean Node_isDirectory(Node_T node1) {
   boolean isDirectory;
   
   assert(node1 != NULL);

   isDirectory = node1->isDir;
   
   return isDirectory; 
}

/* see node.h for specification */
size_t Node_getLength(Node_T node1) {
   size_t getLength;
   
   assert(node1 != NULL);
   assert(node1->isDir == FALSE);
   
   getLength = node1->length;

   return getLength; 
}
/* make sure everything is ordered correctly to place return types together!!! */

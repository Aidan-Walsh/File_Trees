
/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author: Aidan Walsh, Konstantin Howard                             */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dynarray.h"
#include "checkerDT.h"


/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T n) {
   Node_T parent;
   Node_T previous; 
   const char* npath;
   const char* ppath;
   const char* rest;
   size_t i;
   size_t c;
   
   /* Sample check: a NULL pointer is not a valid node */
   if(n == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   parent = Node_getParent(n);

   /* test cases if the parent is non NULL */
   if(parent != NULL) {
      npath = Node_getPath(n);
      
      ppath = Node_getPath(parent);
      /*first, check to make sure that there is no parent with a non
        existent path*/
      if (ppath == NULL) {
         fprintf(stderr, "Parent of P has null path\n");
         return FALSE; 
      }
      /* Sample check that parent's path must be prefix of n's path*/
      i = strlen(ppath);
      if(strncmp(npath, ppath, i)) {
         fprintf(stderr, "P's path is not a prefix of C's path\n");
         return FALSE;
      }
      /* Sample check that n's path after parent's path + '/'
         must have no further '/' characters */
      rest = npath + i;
      rest++;
      if(strstr(rest, "/") != NULL) {
         fprintf(stderr, "C's path has grandchild of P's path\n");
         return FALSE;
      }
   }
   /* iterate through children, if present, and make sure they are
      sorted */
   for(c = 0; c < Node_getNumChildren(n); c++)
   {
      Node_T child = Node_getChild(n, c);
      if (c > 0) {
         if (strcmp(Node_getPath(child), Node_getPath(previous))<0) {
            fprintf(stderr, "children of P are not sorted\n");
            return FALSE; 
         }
      }
      previous = child; 

   }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at n.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerDT_treeCheck(Node_T n) {
   size_t c;
   

   if(n != NULL) {

      /* Sample check on each non-root node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(n))
         return FALSE;


      for(c = 0; c < Node_getNumChildren(n); c++)
      {
         Node_T child = Node_getChild(n, c);
         
         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(child))
            return FALSE;
      }
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean isInit, Node_T root, size_t count) {

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!isInit) {
      if(count != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }

      /* if it not initialized, test to make sure that the root 
         is null */
      if (root != NULL) {
         fprintf(stderr, "root initialized before initialization");
         return FALSE; 
      }
   }
   /* make sure that an initialized tree with a non null root has a 
      null parent. The root must always have null parent. */
    if (isInit && root != NULL) {
      if (Node_getParent(root)!= NULL) {
         fprintf(stderr, "non null root does not have null parent\n");
         return FALSE; 
      }
    } 
   /* make sure that if it is initialized and root is NULL that the 
      count is 0 */
   if (root == NULL && isInit) {
      if (count != 0) {
         fprintf(stderr, "count not properly initialized to 0\n");
         return FALSE; 
      }
   }

   /* if the root is not null, then our count must at least be non zero
      so we make sure that our count is at least adjusting */
   if (root != NULL) {
      if (count == 0) {
         fprintf(stderr, "count not properly adjusted\n");
         return FALSE; 
      }
      }
   /* Now checks invariants recursively at each node from the root. */
   return CheckerDT_treeCheck(root);
}

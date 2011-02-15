/* $Id: TraversalOrder.h,v 1.2 2005/08/31 05:34:47 johnmc Exp $ */
//****************************************************************************/
//                 Copyright (c) 1990-2005 Rice University
//                          All Rights Reserved
//****************************************************************************/

//***************************************************************************
// TraversalOrder.h
//
//   iterator traversal order specification
//
// Author: John Mellor-Crummey                                
//
// Creation Date: 30 August 2005
//
// Modification History:
//  30 August 2005 -- John Mellor-Crummey
//
//**************************************************************************/

  
#ifndef TraversalOrder_h
#define TraversalOrder_h


//****************************************************************************
// enumeration type declarations
//***************************************************************************/


typedef enum { Unordered, PreOrder, PostOrder,
           ReversePreOrder, ReversePostOrder, PreAndPostOrder } TraversalOrder;



#endif
